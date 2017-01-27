/******************************************************************************
 * tcp_client.c
 *
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

#include <ctype.h>
#include "networks.h"
#include "testing.h"

#define NORMAL_HEADER_LEN 3
// client = c
// server = s
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

// global variables
int socket_num;         			//socket descriptor
char *client_name;					//name of client
int nameLen = 0;					//length of client name
int bufsize = 1000;					//data buffer size
int numberClients = 0;				//number of clients to print - FLAG12
// functions
void inputOptions(char *send_buf, int send_len);
void listenUser(char *send_buf, int send_len);
int sendMessage(char *send_buf, int send_len);
int sendMessage2(char *send_buf, int send_len);
int sendBroadcast(char *send_buf, int send_len);
int sendClientList();
int sendExit();
void sendData(char *send_buf, int send_len);
void largeMessage(char *message, int messageLen, int handleLen, int nameLen, int dataLen);
char *createMessage(char *handle, int handleLen, char *data, int dataLen, int messageLen);
int getName(char *send_buf, char *name);
void waitInput(char *send_buf, int send_len);
int handleData(char *message, int recieved, int socket);
void printBroadcast(int length, char *message);
void printMessage(int length, char *message);
void printNumClients(int length, char *message);
void printClients(int length, char *message, int socket_num);
char *convertMessageToNetwork(char *message, int messageLen);
void printBadHandle(int length, char *message);
char *getHandle(char *message, int *length);
int saveMessageLength(char *message, int messageLen);
int saveFlags(char *message, int flag);
int saveHandle(char *message, char *handle, int length);

int main(int argc, char * argv[])
{
    char *send_buf;         //data buffer     	
    /* check command line arguments  */
    if(argc!= 4)
    {
        printf("usage: %s handle-name host-name port-number \n", argv[0]);
		exit(1);
    }
    /* set up the socket for TCP transmission  */
    socket_num= tcp_send_setup(argv[2], argv[3]);
	send_buf= (char *) malloc(bufsize);
	// name stuff
	nameLen = strlen(argv[1]);
	client_name = argv[1];
	// name was good
	if (getName(send_buf, argv[1]))
	{
		waitInput(send_buf, bufsize);
	}
    close(socket_num);
	free(send_buf);
    return 0;
}

int getName(char *send_buf, char *name)
{
	uint16_t message_len = 0;
	int out = 0;
	int flag = FLAG1;
	char *pointer = send_buf;
	int handleLen = strlen(name);
	int messageLen = 4 + handleLen;
	// put in the message length
	pointer += saveMessageLength(pointer, messageLen);
	// put in the message flags
	pointer += saveFlags(pointer, FLAG1);
	// put in the requested name
	pointer += saveHandle(pointer, name, handleLen);
	// send request
	sendData(send_buf, messageLen);
	// wait for server response
	if (recv(socket_num, send_buf, 3, 0) < 0)
	{
		perror("recv call");
		exit(-1);
	}
	// handle return
	pointer = send_buf;
	// get message length
	memcpy(&message_len, pointer, 2);
	message_len = ntohs(message_len);
	pointer += 2;
	// get flag
	memcpy(&flag, pointer, 1);
	// good handle?
	if (flag == FLAG2)
	{
		out = 1;
		nameLen = handleLen;
	}
	// bad handle? - flag 3
	else
	{
		out = 0;
	}
	return out;
}

char *getHeader(int socket_num, char *send_buf)
{
	int recieved = 0;
	int got;
	char *temp = malloc(sizeof(char) *3);
	while (recieved < 3)
	{
		got = 0;
		// wait for server response
		if ((got= recv(socket_num, temp, 3 - recieved, 0)) < 0)
		{
			perror("recv call");
			exit(-1);
		}
		if (got == 0)
		{
			printf("Server Terminated\n");
			exit(-1);
		}
		memcpy(send_buf+recieved, temp, got);
		recieved+=got;
	}
	free(temp);
	return send_buf;
}

void waitInput(char *send_buf, int send_len)
{
	fd_set input;					// data to read from stdin
	fd_set temp;
	fd_set read;
	int sk = socket_num;			// server's socket number
	int STDIN = fileno(stdin);		// STDIN's socket number
	int loop = 1;					// if you want to keep chatting
	static struct timeval timeout;	//select call time to wait
	int recieved;
	// set to stdin
	FD_SET(sk, &input);
	free(send_buf);
	printf("$:");
	while(loop != 2)
	{
		fflush(stdout);
		send_buf = calloc(sizeof(char), bufsize);
		recieved = 0;
		FD_ZERO(&read);
		FD_SET(socket_num, &read);
		FD_SET(STDIN, &read);
		temp = input;
		timeout.tv_sec = 1;		// set timeout to 1 sec
		timeout.tv_usec = 0;	// set timeout (microseconds)
		select(sk + 1, (fd_set *) &read, (fd_set *) 0,
					(fd_set *) 0, NULL);
		// if there is data from the server
		if (FD_ISSET(socket_num, &read))
		{
			// get header of data
			send_buf = getHeader(socket_num, send_buf);
			loop = handleData(send_buf, 3, socket_num);
		}
		// if there data to read from terminal
		else if (FD_ISSET(STDIN, &read))
		{
			send_len = 0;
			while ((send_buf[send_len] = getchar()) != '\n' && send_len < 80)
			   send_len++;

			send_buf[send_len] = '\0';
			inputOptions(send_buf, send_len);
			if (loop != 2)
			{
				printf("$:");
			}
		}
	}
}

int tcp_send_setup(char *host_name, char *port)
{
    int socket_num;
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host
    // create the socket
    if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    perror("socket call");
	    exit(-1);
	}
    // designate the addressing family
    remote.sin_family= AF_INET;
    // get the address of the remote host and store
    if ((hp = gethostbyname(host_name)) == NULL)
	{
	  printf("Error getting hostname: %s\n", host_name);
	  exit(-1);
	}
    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);
    // get the port used on the remote side and store
    remote.sin_port= htons(atoi(port));
	// try to connect
    if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
    {
	perror("connect call");
	exit(-1);
    }
    return socket_num;
}

// checks input and sees if it is incorrect
void inputOptions(char *send_buf, int send_len)
{
	int out = 1;
	char first = *send_buf;
	char second = tolower(*(send_buf + 1));
	char third = *(send_buf + 2);
	if (first != '%')
	{
		out = 0;
	}
	else
	{
		// send message to a certain client
		if (second == 'm' && third == ' ')
		{
			out = sendMessage(send_buf + 3, send_len - 3);
		}
		// send broadcast
		else if (second == 'b' && third == ' ')
		{
			out = sendBroadcast(send_buf + 3, send_len - 3);
		}
		// ask server for other client names
		else if (second == 'l')
		{
			out = sendClientList();
		}
		// ask server for exit permission
		else if (second == 'e')
		{
			out = sendExit();
		}
		// incorrect input
		else
		{
			out = 0;
		}
	}
	// if bad input
	if (out == 0)
	{
		printf("Invalid command\n");
		fflush(stdout);
	}
}

// handles sending messages
int sendMessage(char *send_buf, int send_len)
{
	int handleLen = 0, dataLen = 0, out = 1;
	char current = *send_buf;
	char *handle, *data;
	char *message;
	uint16_t messageLen = 0;
	while (handleLen < send_len && current != ' ')
	{
		handleLen++;
		current = *(send_buf + handleLen);
	}
	// data len = characters after the handle
	dataLen = send_len - handleLen;
	// get destination
	handle = malloc(sizeof(char) * handleLen);
	memcpy(handle, send_buf, handleLen);
	// if there is data to send
	if (dataLen > 1)
	{
		// allocate size
		data = malloc(sizeof(char) * dataLen);
		// don't send the space inbetween the handle and the data (why the plus 1)
		memcpy(data, send_buf + handleLen + 1, dataLen);
	}
	messageLen = NORMAL_HEADER_LEN + 1 + handleLen + 1 + nameLen + dataLen;
	message = createMessage(handle, handleLen, data, dataLen, messageLen);
	// normal message
	// decrement it properly to not count the space that wasn't added
	if (dataLen >= 1)
	{
		messageLen--;
	}
	sendData(message, messageLen);
	
	// only free datalen if it was allocated
	if (dataLen > 1)
	{
		free(data);
	}
	free(handle);
	free(message);
	return out;
}

// handles sending broadcasts
int sendBroadcast(char *send_buf, int send_len)
{
	int out = 1;
	int messageLen = 4 + nameLen + send_len;
	char *message = malloc(sizeof(char) * messageLen);
	char *pointer = message;
	// put in the message length
	pointer += saveMessageLength(pointer, messageLen);
	// put in the message flags
	pointer += saveFlags(pointer, FLAG4);
	// put in the source
	pointer += saveHandle(pointer, client_name, nameLen);
	// put in the data
	memcpy(pointer, send_buf, send_len);
	// send message
	sendData(message, messageLen);
	return out;
}

// handles sending ask for client list message
int sendClientList()
{
	//int flag = FLAG10;
	int messageLen = 3;
	char *message = malloc(sizeof(char) * messageLen);
	char *pointer = message;
	// put in the message length
	pointer += saveMessageLength(pointer, messageLen);
	// put in the message flags
	pointer += saveFlags(pointer, FLAG10);
	// send request notification
	sendData(message, messageLen);
	free(message);
	return 1;
}

// handles sending ask for exit permission message
int sendExit()
{
	int messageLen = 3;
	char *message = malloc(sizeof(char) * messageLen);
	char *pointer = message;
	// put in the message length
	pointer += saveMessageLength(pointer, messageLen);
	// put in the message flags
	pointer += saveFlags(pointer, FLAG8);
	// send exit notification
	sendData(message, messageLen);
	free(message);
	return 1;
}

char *createMessage(char *handle, int handleLen, char *data, int dataLen, int messageLen)
{
	char *message = malloc(messageLen * sizeof(char));
	char *pointer = message;
	// put in the message length
	pointer += saveMessageLength(pointer, messageLen);
	// put in the message flags
	pointer += saveFlags(pointer, FLAG5);
	// put in the destHandleLen
	pointer += saveHandle(pointer, handle, handleLen);
	// put in the sourceHandleLen
	pointer += saveHandle(pointer, client_name, nameLen);
	// put in the data if there is some - the space doesn't count
	if (dataLen > 1)
	{
		memcpy(pointer, data, dataLen);
	}
	return message;
}

// sends data to server, message must be formatted already
void sendData(char *send_buf, int send_len)
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

char *getAllData(char *message, int message_len, int recieved, int socket)
{
	int got;
	char *send_buf = malloc(bufsize);
	// while you don't have the correct number of bits
	while (recieved < message_len)
	{
		// wait for server response
		if ((got= recv(socket_num, send_buf, message_len - recieved, 0)) < 0)
		{
			perror("recv call");
			exit(-1);
		}
		memcpy(message+recieved, send_buf, got);
		recieved+=got;
	}
	free(send_buf);
	return message;
}

int handleData(char *message, int recieved, int socket)
{
	int out = 1;
	uint16_t message_len = 0;
	int flag = 0;
	char *pointer = message;
	// get message length
	memcpy(&message_len, pointer, 2);
	pointer += 2;
	message_len = ntohs(message_len);
	// if it isn't the list of clients - FLAG 12
	if (message_len != 0)
	{
		message = getAllData(message, message_len, recieved, socket);
	}
	// set flag
	flag = *pointer;
	// compare flags
	// broadcast					(c->c)
	if (flag == FLAG4)
	{
		printf("\n");
		printBroadcast(message_len, message);
		printf("$:");
	}
	// message						(c->c)
	else if (flag == FLAG5)
	{
		printf("\n");
		printMessage(message_len, message);
		printf("$:");
	}
	// bad handle
	else if (flag == FLAG7)
	{
		printBadHandle(message_len, message);
		printf("$:");
	}
	// server says client can exit	(s->c)
	else if (flag == FLAG9)
	{
		//printf("got exit permission\n");
		out = 2;
	}
	// want other clients			(c->s)
	else if (flag == FLAG11)
	{
		printf("\n");
		printNumClients(message_len, message);
	}
	// list of clients
	else if (flag == FLAG12)
	{
		printClients(message_len, message, socket);
		printf("$:");
	}
	
	return out;
}

void printBroadcast(int length, char *message)
{
	char *pointer = message + 3;
	char *src;
	char *data;
	int srcLen = 0;
	int dataLen = length - 4;
	// get source
	src = getHandle(pointer, &srcLen);
	pointer += srcLen + 1;
	// get data
	dataLen -= srcLen;
	data = malloc(sizeof(char) * dataLen + 1);
	memcpy(data, pointer, dataLen);
	data[dataLen] = '\0';
	// print out data
	printf("%s: %s\n", src, data);
	free(src);
	free(data);
}

void printMessage(int length, char *message)
{
	char *pointer = message + 3;
	int destLen = 0;
	char *destName;
	char *src;
	char *data;
	int srcLen = 0;
	int dataLen = length - 5;
	// get destination name
	destName = getHandle(pointer, &destLen);
	pointer += destLen + 1;
	// get source name
	src = getHandle(pointer, &srcLen);
	pointer += srcLen + 1;
	// get data
	dataLen -= destLen;
	dataLen -= srcLen;
	data = malloc(sizeof(char) * dataLen + 1);
	memcpy(data, pointer, dataLen);
	data[dataLen] = '\0';
	// print out data
	printf("%s: %s\n", src, data);
	free(destName);
	free(src);
	free(data);
}

char *getOneClient(int socket_num)
{
	int recieved = 0;
	int got;
	int nameLen = 0;
	char *temp;
	char *message;
	// get length of name
	if ((got= recv(socket_num, &nameLen, 1, 0)) < 0)
	{
		perror("recv call");
		exit(-1);
	}
	// wait till you have gotten the whole client name
	temp = malloc(sizeof(char) * nameLen);
	message = malloc(sizeof(char) * nameLen + 1);
	while (recieved < nameLen)
	{
		got = 0;
		// wait for server response
		if ((got= recv(socket_num, temp, nameLen - recieved, 0)) < 0)
		{
			perror("recv call");
			exit(-1);
		}
		memcpy(message+recieved, temp, got);
		recieved+=got;
	}
	message[nameLen] = '\0';
	free(temp);
	return message;
}

void printNumClients(int length, char *message)
{
	char *pointer = message + 3;
	uint32_t number;
	memcpy(&number, pointer, 4);
	printf("Number of clients: %i\n", number);
	numberClients = number;
}

void printClients(int length, char *message, int socket_num)
{
	char *name;
	int i = 0;
	int nameLen;
	for (i = 0; i < numberClients; i++)
	{
		nameLen = 0;
		// get name
		name = getOneClient(socket_num);
		printf("   %s\n", name);
		free(name);
	}
}

void printBadHandle(int length, char *message)
{
	char *src;
	int len;
	src = getHandle(message + 3, &len);
	printf("Client with handle %s does not exist\n", src);
	free(src);
}

// puts messageLen into message, returns spaces to increment pointer
int saveMessageLength(char *message, int messageLen)
{
	messageLen = htons(messageLen);
	memcpy(message, &messageLen, 2);
	return 2;
}

// puts flag into message, retursn spaces to increment pointer
int saveFlags(char *message, int flag)
{
	memcpy(message, &flag, 1);
	return 1;
}

// puts in source or destination into message, returns spaces to increment pointer
int saveHandle(char *message, char *handle, int length)
{
	memcpy(message, &length, 1);
	// put in the destHandle
	memcpy(message + 1, handle, length * sizeof(char));
	return length + 1;
}

// gets source or destination from the message and returns it
char *getHandle(char *message, int *length)
{
	char *pointer = message;
	char *src;
	int srcLen = 0;
	// get length of source
	memcpy(&srcLen, pointer, 1);
	*length = srcLen;
	pointer++;
	// get src
	src = malloc(sizeof(char) * srcLen + 1);
	memcpy(src, pointer, srcLen);
	src[srcLen] = '\0';
	return src;
}