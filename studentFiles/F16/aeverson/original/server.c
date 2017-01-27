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
#include "chat.h"
#include "handleList.h"

void sendBadHandle(int socket) {
	handleErr data;
	
	data.packetLen = htons(HEAD_SIZE_CONST);
	data.flag = BAD_HANDLE;
	
	if(send(socket, (void*)&data, HEAD_SIZE_CONST, 0) < 0) {
		perror("Send on Server");
		exit(FAIL);
	}
}

void sendHandleACK(int socket) {
	handleACK data;

	data.packetLen = htons(HEAD_SIZE_CONST);
	data.flag = ACK_HANDLE;
	
	if(send(socket, (void*)&data, HEAD_SIZE_CONST, 0) < 0) {
		perror("Send on Server");
		exit(FAIL);
	}
	
}

void sendNumClientsPacket(int socket, int32_t numHandles) {
	char *data = malloc(NUM_CLIENT_CONST);
	int32_t num = htonl(numHandles);
	int flag = NUM_HANDLES;
	int length = htons(NUM_CLIENT_CONST);
	

	memcpy(data, &length, 2);
	memcpy(&data[FLAG_OFF], &flag, 1);
	memcpy(&data[HEAD_SIZE_CONST], &num, 4);
	
	if(send(socket, (void*)data, NUM_CLIENT_CONST, 0) < 0) {
		perror("Sending number of clients");
		exit(FAIL);
	}
}

void sendHandle(int socket, Handle *handle) {
	char *data = malloc(GIVE_HANDLE_CONST + handle->length);
	int flag = GIVE_HANDLE;
	int length = htons(GIVE_HANDLE_CONST + handle->length);

	memcpy(data, &length, 2);
	memcpy(&data[FLAG_OFF], &flag, 1);
	memcpy(&data[HEAD_SIZE_CONST], &length, 1);
	memcpy(&data[GIVE_HANDLE_CONST], handle->name, handle->length);

	if(send(socket, (void*)data, GIVE_HANDLE_CONST + handle->length, 0) < 0) {
		perror("Sending handle data");
		exit(FAIL);
	}
	free(data);
}

void sendHandlePackets(int socket, int32_t numHandles) {
	int ndx = 0;

	for(ndx = 0; ndx < numHandles; ndx++) {
		Handle *handle = getClientData(ndx);
		sendHandle(socket,handle);
	}		
}

void sendExitPacket(int socket) {
	handleACK data;
	
	data.packetLen = htons(HEAD_SIZE_CONST);
	data.flag = ACK_EXIT;

	if(send(socket, (void*)&data, HEAD_SIZE_CONST, 0) < 0) {
		perror("Send Exit ACK");
		exit(FAIL);
	}
}

void sendBadDest(int socketNum, cclient *handle) {
	badDest *packet = malloc(sizeof(badDest));
	int size = INIT_SIZE_CONST + handle->len;
	unsigned char *data = malloc(size);

	packet->head.packetLen = htons(size);
	packet->head.flag = BAD_DEST;
	packet->handle = handle;

	/* Copy: head, then length of bad handle and finally handle. */
	memcpy(data, &packet->head, HEAD_SIZE_CONST);
	memcpy(&data[HEAD_SIZE_CONST], &packet->handle->len, 1);
	memcpy(&data[INIT_SIZE_CONST], packet->handle->name, packet->handle->len);

	if(send(socketNum, data, size, 0) < 0) {
		perror("Send Bad Dest");
		exit(FAIL);
	}

	free(data);
	free(packet);
} 

void sendMessage(int socket, char *buffer, int length) {
	unsigned char *data = malloc(length);
	
	memcpy(data, buffer, length);
	
	if(send(socket, data, length, 0) < 0) {
		perror("Send Message");
		exit(FAIL);
	}

	free(data);
}

void constructMessage(Handle *destInfo, cclient *srcInfo, char *buffer,int length) {
	char *data = malloc(length + 1 + destInfo->length);
	int remainingLen = length - MSG_SRC_LEN_OFF;	
	buffer[FLAG_OFF] = MESSAGE;

	memcpy(data, buffer, HEAD_SIZE_CONST);		// Header
	memcpy(&data[MSG_DEST_LEN_OFF], &destInfo->length, 1); // Length of dest client
	memcpy(&data[MSG_DEST_OFF], destInfo->name, destInfo->length); // dest name
	memcpy(&data[MSG_SRC_LEN_OFF + destInfo->length], &buffer[BCAST_SRC_LEN_OFF],remainingLen);

	if(send(destInfo->socket, data, length + 1 + destInfo->length, 0) < 0) {
		perror("Send Message (To all)");
		exit(FAIL);
	}

	free(data);
}
	
void sendMessageToAll(char *buffer, int length, cclient *srcInfo) {
	int ndx = 0, numHandles = getNumHandles();

	for(ndx = 0; ndx < numHandles; ndx++) {
		Handle *destInfo = getClientData(ndx);
		if(strcmp(destInfo->name, srcInfo->name)) {
			constructMessage(destInfo, srcInfo, buffer, length);
		}
	}
}

void parseDetails(int numBytes, char *buffer, int socket, int clientSocket) {
	uint16_t length;
	int32_t numHandles = 0;
	uint8_t flag;
	cclient srcInfo;
	cclient destInfo;
	int destSocket = 0;
	
	/* Get the length & flag of the packet. */
	memcpy(&length, buffer, 2);
	length = ntohs(length);
	memcpy(&flag, &buffer[FLAG_OFF], 1);

	switch(flag) {
		case INIT_HANDLE:
			memcpy(&srcInfo.len, &buffer[HEAD_SIZE_CONST], 1);
			srcInfo.name = malloc(srcInfo.len);
			memcpy(srcInfo.name, &buffer[INIT_SIZE_CONST], srcInfo.len);

			if(findHandle(srcInfo.name) > 0) {
				sendBadHandle(clientSocket);
			}
			else {
				addHandle(srcInfo.name, srcInfo.len, clientSocket);
				sendHandleACK(clientSocket);
			}
			free(srcInfo.name);
		break;

		case BROADCAST:
			memcpy(&srcInfo.len, &buffer[BCAST_SRC_LEN_OFF], 1);
			srcInfo.name = malloc(srcInfo.len + 1);
			memcpy(srcInfo.name, &buffer[BCAST_SRC_OFF], srcInfo.len);
			srcInfo.name[srcInfo.len] = '\0';
			
			sendMessageToAll(buffer, length, &srcInfo);
		break;

		case MESSAGE:
			memcpy(&destInfo.len, &buffer[HEAD_SIZE_CONST], 1);
			destInfo.name = malloc(destInfo.len + 1);
			memcpy(destInfo.name, &buffer[INIT_SIZE_CONST], destInfo.len);
			destInfo.name[destInfo.len] = '\0';

			if(findHandle(destInfo.name) == NOT_FOUND) {
				sendBadDest(clientSocket, &destInfo);
			} 

			else {
				destSocket = getSocket(destInfo.name);
				if(destSocket == FAIL) {
					printf("Something went wrong in handleList!\n");
					exit(FAIL);
				}
				sendMessage(destSocket, buffer, length);
			}
			free(destInfo.name);
		break;

		case EXIT_REQ:
			sendExitPacket(clientSocket);
		break;

		case REQ_HANDLES:
			numHandles = getNumHandles();	
			sendNumClientsPacket(clientSocket, numHandles);
			/* Need to then go send all #numHandles worth of handleData packets*/
			sendHandlePackets(clientSocket, numHandles);			
		break;

		default:
			printf("Bad flag!\n");
			printf("Flag is: %d\n", flag);
		break;
	}
}
	
void startServerChat(int socketNum) {
	fd_set useFds, saveFds;
	int ndx, numFds = 0;
	int client_socket = 0;

	FD_ZERO(&saveFds);
	FD_SET(socketNum, &saveFds);
	
	while(1) {
		useFds = saveFds;
		if(select(FD_SETSIZE, &useFds, NULL, NULL, NULL) < 0) {
			perror("select");
			exit(FAIL);
		}

		for(ndx = 0; ndx < FD_SETSIZE; ndx++) {
			if(FD_ISSET(ndx, &useFds)) {
				if(ndx == socketNum) {
					client_socket = tcpAccept(socketNum);
					FD_SET(client_socket,&saveFds);
					numFds++;
				}
				else {
					char *buffer = malloc(MAXBUF);
					int nbytes = read(ndx, buffer, MAXBUF);
					if(nbytes < 0) {
						perror("read");
						exit(FAIL);
					}
					else if(nbytes == 0) {
						FD_CLR(ndx, &saveFds);
						removeSocket(ndx);
					}
					else { 
						parseDetails(nbytes, buffer, socketNum, ndx);
					}
					free(buffer);
				}
			}
		}
	}
	close(client_socket);
}

int main(int argc, char *argv[]) {
	int server_socket = 0;   //socket descriptor for the server socket
	int port_number = 0;	 //optional 
	
	if(argc == 2) {
		port_number = atoi(argv[1]);
	}

	/* Create the server socket */
	server_socket = tcpServerSetup(port_number);

	/* Get commands from clients and send packages. */
	startServerChat(server_socket);
	
	/* close the sockets */
	close(server_socket);
	
	return 0;
}
