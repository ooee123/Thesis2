/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#define FLAG1 1		// initial packet to server		(c->s)
#define FLAG2 2		// good handle					(s->c)
#define FLAG3 3		// error on initial packet 		(s->c)
#define FLAG4 4		// broadcast					(c->c)
#define FLAG5 5		// message						(c->c)
#define FLAG6 6		// not used
#define FLAG7 7		// bad destination handle		(s->c)
#define FLAG8 8		// client asks to exit			(c->s)
#define FLAG9 9		// server says client can exit	(s->c)
#define FLAG10 10	// want other clients			(c->s)
#define FLAG11 11	// report number of clients		(s->c)
#define FLAG12 12	// the client names				(s->c)

struct node {
	int handle_length;
	char *handle;
	int socket_num;
	struct node *next;
};

void recieveData(char *message, int client_socket);
void handleConnection(int length, char *message, int client_socket);
void handleBroadcast(int length, char *message, int client_socket);
void handleMessage(int length, char *message, int client_socket);
void handleExit(int client_socket);
void handleReportClients(int client_socket);
void sendListClients(int client_socket);

void badHandle(int length, char *message, int destLength, int socket);
void sendData(char *send_buf, int send_len, int socket_num);
void waitForEvents();

// list methods
void removeFromList(int socket_num);
struct node *getElement(int socket_num);
void addToList(int length, char *name, int socket_num);
int nameTaken(char *name, int length);
void createList(int server_socket);

// variables
struct node *root;			// pointer to start of list
struct node *conductor;		// pointer to current spot of list
uint32_t list_length = 0;	// length of list
fd_set fdvar;				// sockets to listen to
int buffer_size= 1000;  	//packet size variable
int server_socket= 0;   	//socket descriptor for the server socket

int main(int argc, char *argv[])
{
	int port = 0;
	
	if (argc > 2)
	{
		printf("too many arguments: server [port-number]\n");
		exit(-1);
	}
	else if (argc == 2)
	{
		port = atoi(argv[1]);
	}
    //create the server socket
    server_socket= tcp_recv_setup(port);
	// create list
	createList(server_socket);
	// wait for data from clients
	waitForEvents();
	// should never leave waitForEvents
	return -1;
}

void waitForEvents()
{
	int client_socket= 0;   //socket descriptor for the client socket
    char *buf;              //buffer for receiving from client
    int message_len= 0;     //length of the received message
	static struct timeval timeout;	//select call
	fd_set readAble;
	int sk = 0;				// socket descriptor
	int i;
	sk = server_socket;
	FD_SET(sk, &fdvar);
	while (1)
	{
		buf=  (char *) calloc(sizeof(char), buffer_size);
		readAble = fdvar;
		timeout.tv_sec = 1;		// set timeout to 1 sec
		timeout.tv_usec = 0;	// set timeout (microseconds)
		select(sk + 1, (fd_set *) &readAble, (fd_set *) 0,
					(fd_set *) 0, &timeout);
		// for all sockets
		for (i = 0; i <= sk; i++)
		{
			// if socket needs attention
			if (FD_ISSET(i, &readAble))
			{
				//if new client wants to join
				if (i == server_socket)
				{
					// get new client
					client_socket= tcp_listen(server_socket, 5);
					//sk++;
					FD_SET(client_socket, &fdvar);
					
					if (client_socket > sk)
					{
						sk = client_socket;
					}
				}
				// if data needs to be read
				else if (i > server_socket)
				{
					// get data
					if ((message_len = recv(i, buf, buffer_size, 0)) < 0)
					{
						perror("recv call");
						exit(-1);
					}
					recieveData(buf, i);
				}
			}
		}
		free(buf);
	}
}

void createList(int server_socket)
{
	root = malloc(sizeof(struct node));
	root->next = 0;
	root->handle_length = 0;
	root->socket_num = server_socket;
	conductor = root;
}

void recieveData(char *message, int client_socket)
{
	uint16_t message_len = 0;
	uint16_t flag = 0;
	char *pointer = message;
	// get message length
	memcpy(&message_len, pointer, 2);
	pointer += 2;
	// get the flag
	memcpy(&flag, pointer, 1);
	message_len = ntohs(message_len);
	// initial packet to server		(c->s)
	if (flag ==	FLAG1)
	{
		handleConnection(message_len, message, client_socket);
	}
	// broadcast					(c->c)
	else if (flag == FLAG4)
	{
		handleBroadcast(message_len, message, client_socket);
	}
	// message						(c->c)
	else if (flag == FLAG5)
	{
		handleMessage(message_len, message, client_socket);
	}
	// client asks to exit			(c->s)
	else if (flag == FLAG8)
	{
		handleExit(client_socket);
	}
	// want other clients			(c->s)
	else if (flag == FLAG10)
	{
		handleReportClients(client_socket);
		// now get list of client names and send them
		sendListClients(client_socket);
	}
	else
	{
		// never should hit here - bad flag
	}
}


void handleConnection(int length, char *message, int socket)
{
	char *pointer = message + 3;
	int destLen = 0;
	char *destName;
	int flag;
	// get length of destination
	memcpy(&destLen, pointer, 1);
	pointer++;
	// get destination
	destName = malloc(sizeof(char) *destLen + 1);
	memcpy(destName, pointer, destLen);
	destName[destLen] = '\0';
	// put in length for new message
	pointer = message;
	length = 3;
	memcpy(pointer, &length, 1);
	pointer = message + 2;
	// if name is free
	if (nameTaken(destName, destLen) == 0)
	{
		// put in good handle flag
		flag = FLAG2;
		memcpy(pointer, &flag, 1);
		sendData(message, 3, socket);
		addToList(destLen, destName, socket);
	}
	// if name is taken
	else
	{
		// put in bad handle flag
		flag = FLAG3;
		memcpy(message, &flag, 1);
		sendData(message, 3, socket);
		FD_CLR(socket, &fdvar);
	}
}

void handleBroadcast(int length, char *message, int client_socket)
{
	struct node *pointer = root;
	int i;
	for (i = 0; i < list_length; i++)
	{
		pointer = pointer->next;
		// if not the person sending the broadcast
		if (pointer->socket_num != client_socket)
		{
			sendData(message, length, pointer->socket_num);
		}
	}
}

void handleMessage(int length, char *message, int client_socket)
{
	char *pointer = message + 3;
	int destLen = 0;
	char *destName;
	int destSocket = 0;
	// get length of destination
	memcpy(&destLen, pointer, 1);
	pointer++;
	// get destination
	destName = calloc(sizeof(char), destLen + 1);
	memcpy(destName, pointer, destLen);
	destName[destLen] = '\0';
	pointer+= destLen;
	// was the destination valid?
	destSocket = nameTaken(destName, destLen);
	// bad destination
	if (destSocket == 0)
	{
		badHandle(length, message, destLen, client_socket);
	}
	else
	{
		sendData(message, length, destSocket);
	}
	free(destName);
}

void handleExit(int client_socket)
{
	int flag = FLAG9;
	int messageLen = 3;
	char *message = malloc(sizeof(char) * messageLen);
	char *pointer = message;
	// put in the message length
	messageLen = htons(messageLen);
	memcpy(pointer, &messageLen, 2);
	messageLen = ntohs(messageLen);
	pointer += 2;
	// put in the message flags
	memcpy(pointer, &flag, 1);
	// send exit ACK
	sendData(message, messageLen, client_socket);
	// remove the element from the list of clients
	removeFromList(client_socket);
	FD_CLR(client_socket, &fdvar);
	
	free(message);
}

void sendListClients(int client_socket)
{
	int messageLen = 0;
	int nameLen;
	char *message = NULL;
	char *check = NULL;
	struct node *element = root->next;
	int i = 0;
	int index = 0;
	// there is always 1 element
	nameLen = element->handle_length;
	messageLen += nameLen + 4;
	message = malloc(sizeof(char) * messageLen + 3);
	// put in size
	i = htons(i);
	memcpy(message, &i, 2);
	index +=2;
	// put in flag
	i = FLAG12;
	memcpy(message + 2, &i, 1);
	index++;
	// put in first element's name size
	*(message + index) = nameLen;
	index++;
	// put in name of first element
	memcpy(message + index, element->handle, nameLen);
	index += nameLen;
	// for every element in the list
	for (i = 1; i < list_length; i++)
	{
		element = element->next;
		nameLen = element->handle_length;
		messageLen += nameLen + 1;
		check = realloc(message, messageLen);
		// if realloc failed
		if (check == NULL)
		{
			check = malloc(sizeof(char) * messageLen);
			memcpy(check, message, messageLen - nameLen);
			free(message);
		}
		message = check;
		// put in element name's size
		*(message + index) = nameLen;
		index++;
		// put in name of element
		memcpy(message + index, element->handle, nameLen);
		index += nameLen;
	}
	sendData(message, messageLen, client_socket);
	free(message);
}

void handleReportClients(int client_socket)
{
	int flag = FLAG11;
	// normal header + 32 bit number of clients
	int messageLen = 3 + 4;
	char *message = malloc(sizeof(char) * messageLen);
	char *pointer = message;
	// put in the message length
	messageLen = htons(messageLen);
	memcpy(pointer, &messageLen, 2);
	messageLen = ntohs(messageLen);
	pointer += 2;
	// put in the message flags
	memcpy(pointer, &flag, 1);
	pointer++;
	// put in number of clients
	memcpy(pointer, &list_length, 4);
	// send number of clients
	sendData(message, messageLen, client_socket);
	free(message);
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup(int argPort)
{
    int server_socket= 0;
    struct sockaddr_in local;      /* socket address for local side  */
    socklen_t len= sizeof(local);  /* length of local address        */

    /* create the socket  */
    server_socket= socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
    {
      perror("socket call");
      exit(1);
    }

    local.sin_family= AF_INET;         //internet family
    local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
    local.sin_port= htons(argPort);    //let system choose the port

    /* bind the name (address) to a port */
    if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
      {
	perror("bind call");
	exit(-1);
      }
    
    //get the port name and print it out
    if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0)
      {
	perror("getsockname call");
	exit(-1);
      }

    printf("Server is using port %d \n", ntohs(local.sin_port));
	        
    return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

int tcp_listen(int server_socket, int back_log)
{
  int client_socket= 0;
  if (listen(server_socket, back_log) < 0)
    {
      perror("listen call");
      exit(-1);
    }
  
  if ((client_socket= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
    {
      perror("accept call");
      exit(-1);
    }
  
  return(client_socket);
}

void sendData(char *send_buf, int send_len, int socket_num)
{
	int sent= 0;            //actual amount of data sent
	/* now send the data */
	sent =  send(socket_num, send_buf, send_len, 0);
	if(sent < 0)
	{
		perror("send call");
	exit(-1);
	}
}

void badHandle(int length, char *message, int destLength, int socket)
{
	char *pointer = message;
	int flag = FLAG7;
	int messageLen = 4 + destLength;
	// change message length
	messageLen = htons(messageLen);
	memcpy(pointer, &messageLen, 2);
	// change the flag to FLAG7
	pointer = message + 2;
	memcpy(pointer, &flag, 1);
	// set length of message
	length = 4 + destLength;
	// who is it - send error message back
	// send to first client
	sendData(message, length, socket);
}

void removeFromList(int socket_num)
{
	struct node *previous = root;
	struct node *next;
	int i = 0;
	int found = 0;
	// for all elements in the list or until its found
	while (i < list_length && !found)
	{
		next = previous->next;
		if (next->socket_num == socket_num)
		{
			found = 1;
		}
		else
		{
			previous = next;
		}
		i++;
	}
	// removed it from list
	if (list_length > 1)
	{
		previous->next = next->next;
	}
	else
	{
		root->next = 0;
	}
	list_length--;
	// free elements of the node
	free(next->handle);
	free(next);
}

struct node *getElement(int socket_num)
{
	struct node *out = NULL;
	struct node *pointer = root;
	int i = 0;
	// for all elements in list
	while (i < list_length || out == NULL)
	{
		pointer = root->next;
		if (pointer->socket_num == socket_num)
		{
			out = pointer;
		}
		i++;
	}
	
	return out;
}

// returns socket number if it is taken
int nameTaken(char *name, int length)
{
	struct node *pointer = root;
	int i = 0;
	int taken = 0;
	while (i < list_length && taken == 0)
	{
		pointer = pointer->next;
		// if the name has been taken
		if (strcmp(name, pointer->handle) == 0)
		{
			taken = pointer->socket_num;
		}
		i++;
	}
	return taken;
}

void addToList(int length, char *name, int socket_num)
{
	struct node *next = malloc(sizeof(struct node));
	// put in values
	next->handle_length = length;
	next->handle = malloc(sizeof(char) * length + 1);
	memcpy(next->handle, name, length);
	next->handle[length] = '\0';
	next->socket_num = socket_num;
	next->next = 0;
	// set conductor to the new node
	if (list_length == 0)
	{
		root->next = next;
	}
	else
	{
		conductor->next = next;
	}
	conductor = next;
	list_length++;
}

