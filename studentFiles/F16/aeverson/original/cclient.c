/******************************************************************************
* tcp_client.c
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#include "networks.h"
#include "chat.h"

#define INVALID_CMD 'I'
#define CMD_SIZE 3
#define CMD_NDX 1

#define CONSOLE 0

char BUFFER[MAXBUF];

void sendInitPacket(cclient *clientInfo, int socket_num) {
	initPacket sendInfo;
	unsigned char *data = malloc(INIT_SIZE_CONST + clientInfo->len);

	sendInfo.head.flag = INIT_HANDLE;
	sendInfo.head.packetLen = htons(INIT_SIZE_CONST + clientInfo->len);
	sendInfo.handle = clientInfo;

	memcpy(data, &sendInfo.head, HEAD_SIZE_CONST);
	memcpy(&data[HEAD_SIZE_CONST], &clientInfo->len, 1);
	memcpy(&data[INIT_SIZE_CONST], clientInfo->name, clientInfo->len);

	if(send(socket_num, data, INIT_SIZE_CONST + clientInfo->len, 0) < 0) {
		perror("Send on Client");
		exit(FAIL);
	} 

	free(data);
}
	
void sendMessage(cclient *src, cclient *dest, msgData *msg, int socket) {
	messagePacket *msgPck = malloc(sizeof(messagePacket));
	int size = MSG_SIZE_CONST + src->len + dest->len + msg->size;
	unsigned char *data = malloc(size);
	int index = HEAD_SIZE_CONST;

	msgPck->head.packetLen = htons(size);
	msgPck->head.flag = MESSAGE;
	msgPck->dest = dest;
	msgPck->src = src;
	msgPck->text = msg;

	memcpy(data, &msgPck->head, HEAD_SIZE_CONST);
	memcpy(&data[HEAD_SIZE_CONST], &msgPck->dest->len, 1);
	index += 1;
	memcpy(&data[index], msgPck->dest->name, msgPck->dest->len);
	index += msgPck->dest->len;
	memcpy(&data[index], &msgPck->src->len, 1);
	index += 1;
	memcpy(&data[index], msgPck->src->name, msgPck->src->len);
	index += msgPck->src->len;
	memcpy(&data[index], msgPck->text->msg, msgPck->text->size);
	
	if(send(socket, (void*)data, size, 0) < 0) {
		perror("Send Message");
		exit(FAIL);
	}

	free(msgPck);
	free(data);
}

void sendMsgToAll(cclient *myInfo, msgData *textMsg, int socketNum) {
	chatHead head;
	int length = BCAST_SIZE_CONST + myInfo->len + textMsg->size;
	char *data = malloc(length);

	head.packetLen = htons(length);
	head.flag = BROADCAST;

	memcpy(data, &head, HEAD_SIZE_CONST);
	memcpy(&data[BCAST_SRC_LEN_OFF], &myInfo->len, 1);
	memcpy(&data[BCAST_SRC_OFF], myInfo->name, myInfo->len);
	memcpy(&data[BCAST_SRC_OFF + myInfo->len], textMsg->msg, strlen(textMsg->msg));

	if(send(socketNum, (void*)data, length, 0) < 0) {
		perror("Send broadcast");
		exit(FAIL);
	}

	free(data);
}

void sendListRequest(cclient *myInfo, int socket) {
	listReq *data = malloc(sizeof(listReq));

	data->packetLen = htons(HEAD_SIZE_CONST);
	data->flag = REQ_HANDLES;

	if(send(socket, (void*)data, HEAD_SIZE_CONST, 0) < 0) {
		perror("Send List Req");
		exit(FAIL);
	}
	free(data);
}

void sendExitRequest(cclient *myInfo, int socket) { 
	exitReq *data = malloc(sizeof(exitReq));

	data->packetLen = htons(HEAD_SIZE_CONST);
	data->flag = EXIT_REQ;

	if(send(socket, (void*)data, HEAD_SIZE_CONST, 0) < 0) {
		perror("Send Exit Req");
		exit(FAIL);
	}
	free(data);
}


int getHandle(cclient *handleInfo, char *buff) {
	handleInfo->len = strcspn(buff, " \n");
	
	if(handleInfo->len == 0) {
		return FAIL;
	}
	handleInfo->name = malloc(handleInfo->len);
	memcpy(handleInfo->name, buff, handleInfo->len);
	
	return PASS;
}
	
void getText(msgData *textMsg, char *buff) {
	textMsg->size = strcspn(buff, "\n");

	if(textMsg->size > MAX_MSG) {
		textMsg->msg = malloc(MAX_MSG);
		memcpy(textMsg->msg, buff, MAX_MSG - 1);
		textMsg->msg[MAX_MSG] = '\0';

		textMsg->remaining = malloc(textMsg->size - (MAX_MSG - 1));
		memcpy(textMsg->remaining, &buff[MAX_MSG], textMsg->size - (MAX_MSG - 1));
		
	}
	else {
		textMsg->msg = malloc(textMsg->size + 1);
		memcpy(textMsg->msg, buff, textMsg->size);
		textMsg->msg[textMsg->size] = '\0';
	}
}


void processCommands(cclient *myInfo, int socketNum) {
   char option, buff[MAXBUF];
	char *data;
	uint32_t cmdSize;
	cclient destInfo;
	msgData textMsg;
	
	/* Get user input, parse and determine if cmd valid.*/
	fgets(buff, MAXBUF, stdin);
	cmdSize = strcspn(buff, " \n") + 1;

	data = &buff[cmdSize];
	if(cmdSize == CMD_SIZE && buff[0] == '%')
		option = toupper(buff[CMD_NDX]);
	else
		option = INVALID_CMD;	

	switch(option) {
		case 'M':
			
			/* Parse dest handle & copy over to struct. */
			if(getHandle(&destInfo, data) < 0) {
				fprintf(stderr, "Invalid command.\n");
				break;
			}

			/* Advance buffer and get message data. */
			data += destInfo.len + 1;
			getText(&textMsg, data);

			sendMessage(myInfo, &destInfo, &textMsg, socketNum);

			free(destInfo.name);
			free(textMsg.msg);

			break;

		case 'B':
			/* Get message data. */
			getText(&textMsg, data);
			
			/* Send message data to all clients! */
			sendMsgToAll(myInfo, &textMsg, socketNum);

			free(textMsg.msg);
			break;
			
		case 'L':
			sendListRequest(myInfo, socketNum);
			break;

		case 'E':
			sendExitRequest(myInfo, socketNum);			
		break;
		
		default:
			fprintf(stderr,"Invalid command.\n");
		break;
	}
	fprintf(stderr,"$:");

}
void getPacketHandle(cclient *dest, int offset) {
	memcpy(&dest->len, &BUFFER[offset], 1);
	dest->name = malloc(dest->len + 1);
	memcpy(dest->name, &BUFFER[offset + 1], dest->len);
	dest->name[dest->len] = '\0';
}
	
void getPacketText(msgData *msg, int index, int packetLen) {
	int length = packetLen - index;

	msg->msg = malloc(length);
	strcpy(msg->msg, &BUFFER[index]);
}

void parseDetails(int numBytes, int socket, int destSocket, cclient *myInfo) {
	int32_t numHandles;
	uint16_t length;
	uint8_t flag;
	cclient dest;
	cclient src;
	msgData msg;
	
	/* Get the length & the flag of the packet */
	memcpy(&length, BUFFER, 2);
	length = ntohs(length);
	memcpy(&flag, &BUFFER[FLAG_OFF], 1);
	
	switch(flag) {
		case ACK_HANDLE: 
			/* Don't need to do anything. Just proceed.*/	
		break;

		case BAD_HANDLE:
			fprintf(stderr,"\nHandle already in use: %s\n", myInfo->name);
			close(socket);
			exit(PASS);
		break;
		
		case MESSAGE:
			getPacketHandle(&dest, MSG_DEST_LEN_OFF);
			getPacketHandle(&src, MSG_SRC_LEN_OFF + dest.len);
			getPacketText(&msg, MSG_TEXT_LEN_OFF + dest.len + src.len, length);
					
			fprintf(stderr, "\n%s: %s\n", src.name, msg.msg);
			fprintf(stderr, "$:");

			free(dest.name);
			free(src.name);
			free(msg.msg);
		break;

		case BAD_DEST:
			getPacketHandle(&dest, BAD_DEST_LEN_OFF);
			fprintf(stderr,"\nClient with handle %s does not exist.\n", dest.name);
			fprintf(stderr,"$:");
			free(dest.name);
		break;
	
		case ACK_EXIT:
			close(socket);
			exit(PASS);
		break;

		case NUM_HANDLES:
			memcpy(&numHandles, &BUFFER[HEAD_SIZE_CONST], 4);
			numHandles = ntohl(numHandles);
			printf("Number of clients: %d\n", numHandles);
			int received = 0;

			while(received != numHandles) {
				int nbytes = recv(socket, BUFFER, MAXBUF, 0);
				if(nbytes < 0) {
					perror("read");
					exit(FAIL);
				}
				else if(nbytes == 0) {
					fprintf(stderr, "\nServer Terminated.\n");
					exit(PASS);
				}
				else {
					getPacketHandle(&dest, REQ_HANDLE_LEN_OFF);
					printf("\t%s\n",dest.name);
					received++;
					free(dest.name);
				}									
			}
		break;

		case GIVE_HANDLE:
		break;

		default:
			fprintf(stderr,"\nBad flag!\n");
			fprintf(stderr,"$:");
		break;	
	}
}

void startChat(cclient *myInfo, int socketNum) {
	fd_set useFds, saveFds;
	int ndx;

	FD_ZERO(&saveFds);
	FD_SET(CONSOLE, &saveFds);	  // STDIN
	FD_SET(socketNum, &saveFds); // SERVER

	while(1) {
		useFds = saveFds;
		if(select(FD_SETSIZE, &useFds, NULL, NULL, NULL) < 0) {
			perror("select");
			exit(FAIL);
		}	

		for(ndx = 0; ndx < FD_SETSIZE; ndx++) {
			if(FD_ISSET(ndx, &useFds)) {
			
				/* Data coming in from server. */
				if(ndx == socketNum) {
					int nbytes = read(ndx, BUFFER, MAXBUF);
					if(nbytes < 0) {
						perror("read");
						exit(FAIL);
					}	
					/* Server closed. */
					else if(nbytes == 0) {
						FD_CLR(ndx, &saveFds);
						fprintf(stderr, "\nServer Terminated.\n");
						exit(PASS);
					}
					else {
						parseDetails(nbytes, socketNum, ndx, myInfo);
					}
				}
				/* Data coming in from stdin. */
				else if(ndx == CONSOLE) {
					processCommands(myInfo,socketNum);					
				}
			}
		}
	}
}

int main(int argc, char * argv[]) {
	int socketNum = 0;      //socket descriptor
	cclient *myInfo = malloc(sizeof(cclient));

	/* check command line arguments  */
	if (argc != 4) {
		printf("usage: %s handle server-name server-port\n", argv[0]);
		exit(1);
	}

	/* Capture the client's name. */
	myInfo->len = strlen(argv[1]);
	myInfo->name = malloc(myInfo->len);
	memcpy(myInfo->name, argv[1], myInfo->len);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3]);

	/* Need to create init_packet and send data to server. */
	sendInitPacket(myInfo, socketNum);

	fprintf(stderr,"$:");
	
	/* Start communicating with the server, other clients and console. */
	startChat(myInfo, socketNum);

	close(socketNum);
	free(myInfo);

	return 0;
}


