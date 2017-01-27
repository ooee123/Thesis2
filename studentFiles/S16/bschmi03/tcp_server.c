//Bradley Schmidt 464 9am
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
#include <arpa/inet.h>
#include <netdb.h>
#include "testing.h"
#include "networks.h"
#include "tcp_server.h"

#define PACKETBUFFER 1550
#define CLIENTCAP 10
#define NORMALSIZE 3
#define FLAGOFFSET 2
#define HANDLESIZE 1
#define HANDLEOFFSET 4
#define READYTOPARSE 1
#define NEWCLIENTPACKET 3
#define DUPLICATEHANDLE 1
#define NONEXISTANTHANDLE -1
#define LISTLENGTH 4
#define NEWCLIENT1 1
#define ADDEDCLIENT2 2
#define CLIENTEXISTS3 3
#define BROADCAST4 4
#define NEWMESSAGE5 5
#define BADHANDLE7 7
#define EXITREQUEST8 8
#define EXITOK9 9
#define RECEIVELISTREQUEST10 10
#define SENDLISTLENGTH11 11
#define SENDLIST12 12

int main(int argc, char *argv[]) {
	char *port = NULL;
	
	if (argc > 1)
		checkArgs(&port, argc, argv);
	
	runServer(port);
	
	return 0;
}

void checkArgs(char **port, int argc, char *argv[]) {
	if(strlen(argv[1]) != strspn(argv[1], "1234567890")) {
		printf("Error: Bad port number!\n");
		exit(1);	
	}		

	*port = argv[1];
}

void runServer(char *port) { 
	int index;
	int clientAdded;
	int clientRemoved;
	struct Client *clients;
	struct Client *currentClient;
	uint32_t clientCount = 0;
	int clientCapacity = CLIENTCAP;
	uint32_t serverSocket;
	uint32_t highestDesc;
	fd_set fdSet;

	clients = calloc(clientCapacity, sizeof(struct Client));
	serverSocket = tcp_recv_setup(port);
	
	while(1) {
		clientAdded = 0;
		highestDesc = setupDescriptors(&fdSet, serverSocket, clients, clientCount);

		if(select(highestDesc + 1, &fdSet, 0, 0, 0) < 0) {
			perror("Select call");
			exit(-1);
		}
		
		if(FD_ISSET(serverSocket, &fdSet))
			clientAdded += addNewSocket(serverSocket, clients, clientCount);		
		
		currentClient = clients;
		for(index = 0; index < clientCount; index++) {		
			clientRemoved = 0;
			
			if(FD_ISSET(currentClient->socket, &fdSet)) {	
				fetchData(currentClient);		
				
				if(processLength(currentClient) == READYTOPARSE)
					clientRemoved = parseFlag(currentClient, &clients, clientCount, clientCapacity);			
			}
						
			if(clientRemoved == 1)
				clientCount--;
			else			
				currentClient += 1;
		}
		
		clientCount += clientAdded;

		if(clientCount == clientCapacity)
			clientCapacity = increaseClientCapacity(&clients, clientCount);
	}
}

uint32_t setupDescriptors(fd_set *fdSet, uint32_t serverSocket, struct Client *clients, uint32_t count) {
	int index;
	int highestDesc;
	FD_ZERO(fdSet);
	FD_SET(serverSocket, fdSet);
	
	highestDesc = serverSocket;
	
	for(index = 0; index < count; index++) {
		FD_SET(clients->socket, fdSet);

		if(clients->socket > highestDesc)
			highestDesc = clients->socket;
		
		clients += 1;		
	}
	
	return highestDesc;
}

int addNewSocket(uint32_t serverSocket, struct Client *clients, uint32_t count) {
	int clientSocket;
	struct Client *newClient;

  if((clientSocket = accept(serverSocket, 0, 0)) < 0) {
  	perror("accept call");
    exit(-1);
  }

	newClient = clients;
	newClient += count;
	
	newClient->packet = calloc(1, PACKETBUFFER);
	newClient->socket = clientSocket;
	newClient->expecting = 0;
	newClient->have = 0;
	
	return 1;
}

int increaseClientCapacity(struct Client **clients, uint32_t count) {
	*clients = realloc(*clients, ((count + 10) * ((uint32_t) sizeof(struct Client))));
	
	return count + 10;
}

void fetchData(struct Client *client) {
	int bytesReceived = 0;
	int bytesToGet = 0;
	
	if(client->have < 2)
		bytesToGet = 2 - client->have;
	else
		bytesToGet = client->expecting - client->have;
	
	if((bytesReceived = recv(client->socket, client->packet + client->have, bytesToGet, 0)) < 0) {
		perror("recv call");
		exit(-1);
	}

	client->have += bytesReceived;
}

int processLength(struct Client *client) {	
	if(client->expecting == 0 && client->have >= 2) 
		client->expecting = ntohs(*((uint16_t *) client->packet));

	if(client->have > 2 && client->have == client->expecting)
		return 1;
	
	if(client->have == 0)
		return 1;
	
	return 0;
}

int parseFlag(struct Client *client, struct Client **clients, uint32_t count, int capacity) {
	int deletedClient = 0;
	u_char flag;
	
	if(client->have > 0)
		flag = *((u_char *) (client->packet + FLAGOFFSET));

	if(client->have == 0) {
		deleteClient(client, clients, count, capacity);
		deletedClient = 1;
	}
	else if(flag == NEWCLIENT1) {
		if(addNewClient(client, *clients, count) == DUPLICATEHANDLE) {
			deleteClient(client, clients, count, capacity);	
			deletedClient = 1;
		}
	}
	else if(flag == BROADCAST4) {	
		transmitBroadcast(client, *clients, count);
	}
	else if(flag == NEWMESSAGE5) {
		transmitMessage(client, *clients, count);
	}
	else if(flag == EXITREQUEST8) {
		sendExitPermission(client);
		deleteClient(client, clients, count, capacity);
		deletedClient = 1;
	}
	else if(flag == RECEIVELISTREQUEST10) {
		sendClientListLength(client, count);
		sendClientList(client, *clients, count);
	}
	
	if(deletedClient == 0)
		clearPacket(client);
	
	return deletedClient;
}

int addNewClient(struct Client *client, struct Client *clients, uint32_t count) {
	int index;
	int duplicateHandle = 0;
	u_char handleLength;
	u_char *response;
	
	handleLength = *((u_char *) client->packet + NORMALSIZE);
	client->handleLength = handleLength;
	
	client->handle = calloc(handleLength + 1, sizeof(u_char));
	memcpy(client->handle, (u_char *) client->packet + HANDLEOFFSET, client->handleLength);
	client->handle[handleLength] = '\0';
	
	for(index = 0; index < count - 1; index++) {
		if(strcmp((const char *) client->handle, (const char *) clients->handle) == 0)
			duplicateHandle = 1;

		clients += 1;
	}
	
	response = calloc(1, NORMALSIZE);
		
	if(duplicateHandle == 0)
		makeNormalHeader(response, NORMALSIZE, ADDEDCLIENT2);	
	else
		makeNormalHeader(response, NORMALSIZE, CLIENTEXISTS3);
	
	sendPacket(client->socket, response, NORMALSIZE);
	free(response);
	
	return duplicateHandle;
}

void deleteClient(struct Client *delete, struct Client **clients, uint32_t count, int capacity) {
	int index;
	int clientsChecked = 0;
	struct Client *clientIndex;
	
	clientIndex = *clients;
	
	for(index = 0; index < count; index++) {
		if(clientIndex->socket == delete->socket)
			index = count;
		else
			clientIndex += 1;
		
		clientsChecked++;	
	}
	
	close(clientIndex->socket);	
	free(clientIndex->handle);
	free(clientIndex->packet);
	memset(clientIndex, 0, sizeof(struct Client));
	memmove(clientIndex, clientIndex + 1, (capacity - clientsChecked) * sizeof(struct Client));
	memset(clientIndex + (capacity - clientsChecked), 0, sizeof(struct Client));
}

void clearPacket(struct Client *client) {
	memset(client->packet, 0, PACKETBUFFER);

	client->expecting = 0;
	client->have = 0;
}

void transmitMessage(struct Client *client, struct Client *clients, uint32_t count) {
	u_short packetLength;
	u_char destHandleLength;
	u_char *destHandle;
	u_char srcHandleLength;
	u_char *srcHandle;
	int destSocket;

	packetLength = ntohs(*((u_short *) (client->packet)));
	
	destHandleLength = *((u_char *) (client->packet + NORMALSIZE));
  destHandle = calloc(destHandleLength + 1, 1);
	memcpy(destHandle, client->packet + NORMALSIZE + HANDLESIZE, destHandleLength);
	destHandle[destHandleLength] = '\0';
	
	srcHandleLength = *((u_char *) (client->packet + NORMALSIZE + HANDLESIZE + destHandleLength));
	srcHandle = calloc(srcHandleLength + 1, 1);
	memcpy(srcHandle, client->packet + NORMALSIZE + HANDLESIZE + destHandleLength + HANDLESIZE, srcHandleLength);
	srcHandle[srcHandleLength] = '\0';
	
	destSocket = getClientSocket(destHandle, clients, count);
	
	if(destSocket == NONEXISTANTHANDLE)
		replyToBadMessage(client, destHandleLength);
	else
		sendPacket(destSocket, client->packet, packetLength);
			
	free(destHandle);
	free(srcHandle);
}

void transmitBroadcast(struct Client *client, struct Client *clients, uint32_t count) {
	int index;
	u_short packetLength;
	
	packetLength = ntohs(*((u_short *) (client->packet)));

	for(index = 0; index < count; index++) {	
		if(client->socket != clients->socket)
			sendPacket(clients->socket, client->packet, packetLength);
		
		clients += 1;
	}
}

void replyToBadMessage(struct Client *client, u_char handleLength) {
	u_char *reply;
	u_short totalToSend;
	
	totalToSend = NORMALSIZE + HANDLESIZE + handleLength;
	reply = calloc(totalToSend, 1);
	
	makeNormalHeader(reply, totalToSend, BADHANDLE7);
	memcpy(reply + NORMALSIZE, client->packet + NORMALSIZE, HANDLESIZE + handleLength);

	sendPacket(client->socket, reply, totalToSend);
	
	free(reply);
}

void sendClientListLength(struct Client *client, uint32_t count) {
	u_char *packet;
	u_short totalToSend;
	uint32_t clientCount = count;
	clientCount = htonl(clientCount);
	
	totalToSend = NORMALSIZE + LISTLENGTH;	
	packet = calloc(totalToSend, 1);

	makeNormalHeader(packet, totalToSend, SENDLISTLENGTH11);
	memcpy(packet + NORMALSIZE, &clientCount, LISTLENGTH);

	sendPacket(client->socket, packet, totalToSend);
	free(packet);
}

void sendClientList(struct Client *client, struct Client *clients, uint32_t count) {
	int index;
	u_char *packet;
	u_short normalLength;	
	uint32_t totalToSend;
	u_char handleLength;
	
	totalToSend = NORMALSIZE;
	packet = calloc(NORMALSIZE, 1);
	normalLength = htons(0);
	
	makeNormalHeader(packet, normalLength, SENDLIST12);
	sendPacket(client->socket, packet, totalToSend);
	free(packet);
	
	for(index = 0; index < count; index++) {
		handleLength = clients->handleLength;
		totalToSend = HANDLESIZE + handleLength;
		
		packet = calloc(totalToSend, 1);
		memcpy(packet, &handleLength, HANDLESIZE);
		memcpy(packet + HANDLESIZE, clients->handle, handleLength);

		sendPacket(client->socket, packet, totalToSend);
		free(packet);
		
		clients += 1;
	}
}

int getClientSocket(u_char *handle, struct Client *clients, uint32_t count) {
	int index;
	int clientExists = NONEXISTANTHANDLE;
	
	for(index = 0; index < count; index++) {
		if(strcmp((const char *) handle, (const char *) clients->handle) == 0)
			clientExists = clients->socket;

		clients += 1;
	}
	
	return clientExists;
}

void sendExitPermission(struct Client *client) {
	u_char *packet;
	uint32_t totalToSend;
	
	totalToSend = NORMALSIZE;
	packet = calloc(NORMALSIZE, 1);
	
	makeNormalHeader(packet, NORMALSIZE, EXITOK9);
	sendPacket(client->socket, packet, totalToSend);
	
	close(client->socket);
	free(packet);
}

void makeNormalHeader(u_char *header, uint16_t packetLength, u_char flag) {
	uint16_t netPckLen = packetLength;
	netPckLen = htons(netPckLen);
	
	memcpy(header, &netPckLen, 2);
	memcpy(header + FLAGOFFSET, &flag, sizeof(u_char));
}

void sendPacket(int socketNum, u_char *packet, uint16_t totalToSend) {
	int sent = send(socketNum, packet, totalToSend, 0);
		
	if(sent < 0) {
		perror("send call");
		exit(-1);
	}
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */
int tcp_recv_setup(char *port) {
  int server_socket = 0;
  struct sockaddr_in local;      /* socket address for local side  */
  socklen_t len = sizeof(local);  /* length of local address        */

  /* create the socket  */
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  
	if(server_socket < 0) {
    perror("socket call");
    exit(1);
  }

  local.sin_family = AF_INET;         //internet family
  local.sin_addr.s_addr = INADDR_ANY; //wild card machine address
	local.sin_addr.s_addr = INADDR_ANY; 
	
	if(port == NULL)
		local.sin_port = htons(0); 
	else
		local.sin_port = htons(atoi(port)); 

  /* bind the name (address) to a port */
  if(bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
		perror("bind call");
		exit(-1);
  }
  
  //get the port name and print it out
  if(getsockname(server_socket, (struct sockaddr*) &local, &len) < 0) {
		perror("getsockname call");
		exit(-1);
  }

  printf("Server using port %hu \n", ntohs(local.sin_port));
        
  if(listen(server_socket, 5) < 0) {
		perror("listen call");
		exit(-1);
	}				
				
  return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */
int tcp_listen(int server_socket, int back_log) {
  int client_socket = 0;
	
  if(listen(server_socket, back_log) < 0) {
  	perror("listen call");
  	exit(-1);
  }
  
  if((client_socket = accept(server_socket, (struct sockaddr*) 0, (socklen_t *) 0)) < 0) {
  	perror("accept call");
    exit(-1);
  }
  
  return client_socket;
}
