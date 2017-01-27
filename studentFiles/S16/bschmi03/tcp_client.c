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
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "testing.h"
#include "networks.h"
#include "tcp_client.h"

#define INPUTBUFFER 3000
#define PACKETBUFFER 1550
#define MAXMSGSIZE 1000
#define NORMALSIZE 3
#define FLAGOFFSET 2
#define HANDLESIZE 1
#define READYTOPARSE 1
#define PRINTED 1
#define NEWCLIENT1 1
#define ADDEDCLIENT2 2
#define CLIENTEXISTS3 3
#define BROADCAST4 4
#define NEWMESSAGE5 5
#define BADHANDLE7 7
#define EXIT8 8
#define EXITOK9 9
#define REQUESTLIST10 10
#define RECEIVELISTLENGTH11 11
#define RECEIVELIST12 12
#define STANDIN 0

int main(int argc, char *argv[]) {
	checkArgs(argc, argv);	
	runClient(argv[1], argv[2], argv[3]);
	
	return 0;   
}

void checkArgs(int argc, char *argv[]) {
	if(argc != 4) {
		printf("Error: Improper argument count!\n");
		exit(1);
  }
	
	if(strlen(argv[3]) != strspn(argv[3], "1234567890")) {
		printf("Error: Bad port number!\n");
		exit(1);	
	}		
}

void runClient(char *handle, char *hostName, char *port) {
  int index;
	struct Client *client;
  char *input;
	fd_set fdSet;
	int highestDesc;

	initializeClient(&client, handle);	
  connectToServer(client, hostName, port);

	printf("$: ");
	fflush(stdout);

	while(1) {
		index = 0;
		input = (char *) calloc(INPUTBUFFER, 1);
		highestDesc = setupDescriptors(&fdSet, client);
	
		if(select(highestDesc + 1, &fdSet, 0, 0, 0) < 0) {
			perror("Select call");
			exit(-1);
		}
		
		if(FD_ISSET(STANDIN, &fdSet)) {
			while((input[index] = getchar()) != '\n')
				index++;
			
			parseCommand(client, input);
		}
	
		if(FD_ISSET(client->socket, &fdSet)) {
			fetchData(client);		
			
			if(processLength(client) == READYTOPARSE) {
				if(parseFlag(client) == PRINTED) {
					printf("$: ");
					fflush(stdout);
				}
			}
		}

		free(input);
  }
}

void parseCommand(struct Client *client, char *input) {
	char command;
	
	if(input[0] == '%') {
		command = tolower(input[1]);

		if(command == 'm')
			sendMessage(client, (u_char *) input);
		else if (command == 'b')
			sendBroadcast(client, (u_char *) input);
		else if(command == 'l')
			requestList(client);
		else if(command == 'e')
			requestExit(client);
		else
			printf("Invalid command\n");
	}
	else
		printf("Invalid command\n");

	printf("$: ");
	fflush(stdout);
}

void sendMessage(struct Client *client, u_char *input) {
	int emptyMessage = 0;	
	u_char *packet;
	uint32_t leftToSend = 0;
	u_char destHandleSize = 0;
	uint32_t totalToSend = 0;
	uint32_t msgLenToSend = 0;
	uint32_t messageOffset = 0;
	u_char *inputOffset;
	
	emptyMessage = parseMessageInput(input, &destHandleSize, &leftToSend);
	inputOffset = input + 3 + destHandleSize + 1;
	
	if(emptyMessage == 1)
		leftToSend = 1;
		
	while(leftToSend > 0) {
		if(leftToSend >= MAXMSGSIZE) {
			msgLenToSend = MAXMSGSIZE;
			leftToSend = leftToSend - (MAXMSGSIZE - 1); //We have to count null, so MAX - 1 (999 bytes)
		}
		else {
			msgLenToSend = leftToSend + 1; //Account for null character
			leftToSend = 0; //We've sent it all
		}
		
		totalToSend = buildMessage(client, input, &packet, &destHandleSize, msgLenToSend);
		messageOffset = NORMALSIZE + HANDLESIZE + destHandleSize + HANDLESIZE + client->handleLength;

		if(emptyMessage == 1)
			packet[totalToSend - 2] = '\n';
		else
			memcpy(packet + messageOffset, inputOffset, msgLenToSend - 1);
		
		packet[totalToSend - 1] = '\0';

		inputOffset += (msgLenToSend - 1);
		
		sendPacket(client->socket, packet, totalToSend);		
		free(packet);
	}
}

int parseMessageInput(u_char *input, u_char *destHandleSize, uint32_t *messageSize) {
	int emptyMsg = 0;
	int index = 3;
	
	while(input[index] != ' ' && emptyMsg == 0) {
	  if(input[index] != '\n') {
			(*destHandleSize)++;
			index++;	
		}
		else
			emptyMsg = 1;	
	}
	
	if(emptyMsg == 0) {
		index++;
		
		if(input[index] == '\n')
			emptyMsg = 1;

		while(input[index] != '\n') {
			(*messageSize)++;
			index++;
		}
	}
	
	return emptyMsg;
}

uint32_t buildMessage(struct Client *client, u_char *input, u_char **packet, u_char *destHandleSize, uint32_t msgLenToSend) {
	uint32_t totalToSend;
	
	totalToSend = NORMALSIZE + HANDLESIZE + *destHandleSize + HANDLESIZE + client->handleLength + msgLenToSend;
	
	*packet = (u_char *) calloc(1, totalToSend);
	makeNormalHeader(*packet, totalToSend, NEWMESSAGE5);
	memcpy(*packet + NORMALSIZE, destHandleSize, sizeof(u_char));
	memcpy(*packet + NORMALSIZE + HANDLESIZE, input + 3, *destHandleSize);	
	memcpy(*packet + NORMALSIZE + HANDLESIZE + *destHandleSize, &(client->handleLength), sizeof(u_char));
	memcpy(*packet + NORMALSIZE + HANDLESIZE + *destHandleSize + HANDLESIZE, client->handle, client->handleLength); 

	return totalToSend;
}

void receiveMessage(struct Client *client) {
	u_char destHandleLength;
	u_char srcHandleLength;
	u_char *srcHandle;
	u_char *message;

	destHandleLength = *((u_char *) client->packet + NORMALSIZE);
	srcHandleLength = *((u_char *) client->packet + NORMALSIZE + HANDLESIZE + destHandleLength);
	
	srcHandle = calloc(srcHandleLength + 1, 1);
	memcpy(srcHandle, client->packet + NORMALSIZE + HANDLESIZE + destHandleLength + HANDLESIZE, srcHandleLength);
	srcHandle[srcHandleLength] = '\0';
	
	message = client->packet + NORMALSIZE + HANDLESIZE + destHandleLength + HANDLESIZE + srcHandleLength;
	
	printf("\n%s: %s", srcHandle, message);
	
	if(message[0] != '\n')
		printf("\n");
	
	free(srcHandle);
}

void receiveMessageError(struct Client *client) {
	u_char badHandleSize;
	u_char *badHandle;
	
	badHandleSize = *(client->packet + NORMALSIZE);
	
	badHandle = calloc(badHandleSize + 1, 1);
	memcpy(badHandle, client->packet + NORMALSIZE + 1, badHandleSize);
	badHandle[badHandleSize] = '\0';
	
	printf("\nClient with handle %s does not exist.\n", badHandle);
	
	free(badHandle);
}

void sendBroadcast(struct Client *client, u_char *input) {
	int emptyMessage = 0;	
	u_char *packet;
	uint32_t leftToSend = 0;
	uint32_t totalToSend = 0;
	uint32_t msgLenToSend = 0;
	uint32_t messageOffset = 0;
	u_char *inputOffset;
	
	emptyMessage = parseBroadcastInput(input, &leftToSend);
	inputOffset = input + 3;
	
	if(emptyMessage == 1)
		leftToSend = 1;
		
	while(leftToSend > 0) {
		if(leftToSend >= MAXMSGSIZE) {
			msgLenToSend = MAXMSGSIZE;
			leftToSend = leftToSend - (MAXMSGSIZE - 1); //We have to count null, so MAX - 1 (999 bytes)
		}
		else {
			msgLenToSend = leftToSend + 1; //Account for null
			leftToSend = 0; //We've sent it all
		}
		
		totalToSend = buildBroadcast(client, input, &packet, msgLenToSend);
		messageOffset = NORMALSIZE + HANDLESIZE + client->handleLength;

		if(emptyMessage == 1)
			packet[totalToSend - 2] = '\n';
		else
			memcpy(packet + messageOffset, inputOffset, msgLenToSend - 1);
		
		packet[totalToSend - 1] = '\0';

		inputOffset += (msgLenToSend - 1);
		
		sendPacket(client->socket, packet, totalToSend);		
		free(packet);
	}
}

int parseBroadcastInput(u_char *input, uint32_t *broadcastSize) {
	int emptyMsg = 0;
	int index = 3;
	
	if(input[index - 1] == '\n')
		emptyMsg = 1;
	else if(input[index] == '\n')
		emptyMsg = 1;
	
	while(input[index] != '\n' && emptyMsg == 0) {
	  if(input[index] != '\n') {
			(*broadcastSize)++;
			index++;	
		}
		else
			emptyMsg = 1;	
	}
	
	return emptyMsg;
}

uint32_t buildBroadcast(struct Client *client, u_char *input, u_char **packet, uint32_t msgLenToSend) {
	uint32_t totalToSend;
	
	totalToSend = NORMALSIZE + HANDLESIZE + client->handleLength + msgLenToSend;
	
	*packet = (u_char *) calloc(1, totalToSend);
	makeNormalHeader(*packet, totalToSend, BROADCAST4);
	memcpy(*packet + NORMALSIZE, &client->handleLength, sizeof(u_char));
	memcpy(*packet + NORMALSIZE + HANDLESIZE, client->handle, client->handleLength);

	return totalToSend;
}

void receiveBroadcast(struct Client *client) {
	u_char srcHandleLength;
	u_char *srcHandle;
	u_char *broadcast;

	srcHandleLength = *((u_char *) client->packet + NORMALSIZE);
	
	srcHandle = calloc(srcHandleLength + 1, 1);
	memcpy(srcHandle, client->packet + NORMALSIZE + HANDLESIZE, srcHandleLength);
	srcHandle[srcHandleLength] = '\0';
	
	broadcast = client->packet + NORMALSIZE + HANDLESIZE + srcHandleLength;
	
	printf("\n%s: %s", srcHandle, broadcast);
	
	if(broadcast[0] != '\n')
		printf("\n");
	
	free(srcHandle);
}

void requestList(struct Client *client) {
	u_short totalToSend;
	u_char *packet;
	
	totalToSend = NORMALSIZE;
	packet = calloc(NORMALSIZE, 1);

	makeNormalHeader(packet, totalToSend, REQUESTLIST10);
	sendPacket(client->socket, packet, totalToSend);
	
	free(packet);
}

void receiveListLength(struct Client *client) {
	uint32_t listLength;	
	listLength = ntohl(*((uint32_t *) (client->packet + NORMALSIZE)));
	
	client->listLength = listLength;
}

void receiveList(struct Client *client) {
	int index;
	int bytesReceived;
	int bytesToGet;
	u_char *handle;
	
	printf("\nNumber of clients: %u\n", client->listLength);
	
	for(index = 0; index < client->listLength; index++) {
		clearPacket(client);
		
		if((bytesReceived = recv(client->socket, client->packet, 1, 0)) < 0) {
			perror("recv call");
			exit(-1);
		}
	
		client->expecting = *((u_char *) client->packet);
	
		while(client->have < client->expecting) {
			bytesToGet = client->expecting - client->have;
			
			if((bytesReceived = recv(client->socket, client->packet + client->have, bytesToGet, 0)) < 0) {
				perror("recv call");
				exit(-1);
			}
			
			client->have += bytesReceived;
		}
		
		handle = calloc(client->expecting + 1, 1);
		memcpy(handle, client->packet, client->expecting);
		handle[client->expecting] = '\0';
		
		printf("   %s\n", handle);
		
		free(handle);
	}
	
	client->listLength = 0;
	clearPacket(client);
}

void requestExit(struct Client *client) {
	u_short totalToSend;
	u_char *packet;
		
	totalToSend = NORMALSIZE;
	packet = calloc(NORMALSIZE, 1);
	
	makeNormalHeader(packet, totalToSend, EXIT8);
	sendPacket(client->socket, packet, totalToSend);
	
	free(packet);
}

void exitClient(struct Client *client) {
	close(client->socket);
	free(client->handle);
	free(client->packet);
		
	exit(1);
}

void connectToServer(struct Client *client, char *hostName, char *port) {	
	int bytesReceived = 0;
	uint32_t totalToSend;
	u_char *packet;
	
	client->socket = tcp_send_setup(hostName, port);

	totalToSend = NORMALSIZE + HANDLESIZE + client->handleLength; 
	packet = calloc(totalToSend, 1);
	makeNormalHeader(packet, totalToSend, NEWCLIENT1);
	memcpy(packet + NORMALSIZE, (u_char *) &(client->handleLength), sizeof(u_char));
	memcpy(packet + NORMALSIZE + HANDLESIZE, client->handle, client->handleLength);
	
	sendPacket(client->socket, packet, totalToSend);

	client->expecting = NORMALSIZE;
	while(client->have < client->expecting) {
		if((bytesReceived = recv(client->socket, client->packet + client->have, client->expecting - client->have, 0)) < 0) {
			perror("recv call");
			exit(-1);
		}
		
		client->have += bytesReceived;
	}
	
	if(*((u_char *) client->packet + FLAGOFFSET) == CLIENTEXISTS3) {
		printf("Handle already in use: %s\n", client->handle);
		close(client->socket);
		free(client->handle);
		free(client->packet);
		exit(1);
	}
	
	free(packet);
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
	
	if(bytesReceived == 0 && client->expecting == 0) {
		printf("\nServer Terminated\n");
		exitClient(client);
	}
	
	client->have += bytesReceived;
}

int processLength(struct Client *client) {	
	if(client->expecting == 0 && client->have >= 2) {
		if(ntohs(*((uint16_t *) client->packet)) == 0)
			client->expecting = NORMALSIZE;
		else
			client->expecting = ntohs(*((uint16_t *) client->packet));
	}

	if(client->have > 2 && client->have == client->expecting)
		return 1;
	
	return 0;
}

int parseFlag(struct Client *client) {
	int printed = 0;
	u_char flag = *((u_char *) (client->packet + FLAGOFFSET));

	if(flag == BROADCAST4) {
		receiveBroadcast(client);
		printed = 1;
	}
	else if(flag == NEWMESSAGE5) {
		receiveMessage(client);
		printed = 1;
	}
	else if(flag == BADHANDLE7) {
		receiveMessageError(client);
		printed = 1;
	}
	else if(flag == EXITOK9) {
		exitClient(client);
	}
	else if(flag == RECEIVELISTLENGTH11) {
		receiveListLength(client);
	}
	else if(flag == RECEIVELIST12) {
		receiveList(client);
		printed = 1;
	}

	clearPacket(client);

	return printed;
}

void clearPacket(struct Client *client) {
	memset(client->packet, 0, PACKETBUFFER);

	client->expecting = 0;
	client->have = 0;
}

int setupDescriptors(fd_set *fdSet, struct Client *client) {
	int highestDesc = client->socket;
	
	FD_ZERO(fdSet);
	FD_SET(STANDIN, fdSet); 
	FD_SET(client->socket, fdSet); 
	
	if(STANDIN > client->socket)
		highestDesc = STANDIN;
	
	return highestDesc;
}

void sendPacket(int socketNum, u_char *packet, uint16_t totalToSend) {
	int sent = send(socketNum, packet, totalToSend, 0);
		
	if(sent < 0) {
		perror("send call");
		exit(-1);
	}
}

void initializeClient(struct Client **client, char *handle) {
	*client = calloc(1, sizeof(struct Client)); 

	(*client)->handleLength = strlen(handle);;

	(*client)->have = 0;
	(*client)->expecting = 0;
	(*client)->handle = calloc(1, (*client)->handleLength + 1);
	memcpy((*client)->handle, handle, (*client)->handleLength);
	(*client)->handle[(*client)->handleLength] = '\0';
	(*client)->packet = calloc(PACKETBUFFER, 1);
}

void makeNormalHeader(u_char *header, uint16_t packetLength, u_char flag) {
	uint16_t netPckLen = packetLength;
	netPckLen = htons(netPckLen);
	
	memcpy(header, &netPckLen, 2);
  memcpy(header + FLAGOFFSET, &flag, sizeof(u_char));
}
	
int tcp_send_setup(char *host_name, char *port) {
	int socket_num;
  struct sockaddr_in remote;       // socket address for remote side
  struct hostent *hp;              // address of remote host

  // create the socket
  if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket call");
	  exit(-1);
	}
    
  // designate the addressing family
  remote.sin_family = AF_INET;

  // get the address of the remote host and store
  if ((hp = gethostbyname(host_name)) == NULL) {
	  printf("Error getting hostname: %s\n", host_name);
	  exit(-1);
	}
    
	//Copy server connection info into remote struct
  memcpy((char*) &remote.sin_addr, (char*) hp->h_addr, hp->h_length);

  // get the port used on the remote side and store
  remote.sin_port = htons(atoi(port));

  if(connect(socket_num, (struct sockaddr*) &remote, sizeof(struct sockaddr_in)) < 0) {
		perror("connect call");
		exit(-1);
  }

  return socket_num;
}

