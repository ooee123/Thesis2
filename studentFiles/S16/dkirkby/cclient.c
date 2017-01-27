#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>

#include "networks.h"
#include "handle.h"
#include "testing.h"

#define NUM_REQUIRED_ARGS 4
#define STDIN_FD 0

#define M_HANDLE_POS 1
#define M_TEXT_POS 2
#define B_TEXT_POS 1

typedef struct {
	char handle[MAX_HANDLE_SIZE];
	uint8_t handleLen;
	char *serverName;
	int serverPort;
	int serverFd;
	int numKnownHandles;
} ClientOptions;

void handleServerMessage(ClientOptions *ops);
void parseIncomingData(ClientOptions *ops, char *buf);
void handleUserInput(ClientOptions *ops);

void sendInitialPacket(ClientOptions *ops);
void sendPrivateMessage(ClientOptions *ops, char *destHandle, int destHandleLen, char *text);
void sendBroadcastMessage(ClientOptions *ops, char *text);
void sendHandleListRequest(ClientOptions *ops);
void sendExitRequest(ClientOptions *ops);

void handleInitialPacketResponse(ClientOptions *ops, uint8_t flags);
void handleBroadcastMessage(char *buf);
void handlePrivateMessage(char *buf);
void handleInvalidHandle(ClientOptions *ops, char *buf);
void handleExitResponse();
void handleSizeOfHandleList(ClientOptions *ops, char *buf);
void handleHandleList(ClientOptions *ops, char *buf);

void multiplexClientIO(ClientOptions *ops);
void setupClientSelectFds(int serverFd, fd_set *recvFds);
char *getHandle(int pos, char *input);
int getHandleLength(int pos, char *input);
char *getText(int pos, char *input);
void printMessageOnNewLine(char *srcHandle, int srcHandleLen, char *message);
void printPrompt();

void memcpyIncrement(char **destPtr, void *src, size_t len);

void parseClientArgs(int argc, char *argv[], ClientOptions *ops);


// Main Function
int main(int argc, char *argv[]) {
	ClientOptions ops;

	parseClientArgs(argc, argv, &ops);
	ops.serverFd = setupTCPSend(ops.serverName, ops.serverPort);
	sendInitialPacket(&ops);
	printPrompt();

	while (1) {
		multiplexClientIO(&ops);
	}

	close(ops.serverFd);
	return 0;
}


// Chat Utility
void handleServerMessage(ClientOptions *ops) {
	int messageLen = 0;
	char recvBuf[MAX_MESSAGE_LENGTH];

	messageLen = safeRecv(ops->serverFd, recvBuf, MAX_MESSAGE_LENGTH, 0);
	if (messageLen == 0) {
		close(ops->serverFd);
		printf("Server Terminated\n");
		exit(1);
	}
	else {
		// printf("Got data on fd %d of length %d\n", ops->serverFd, messageLen);
		parseIncomingData(ops, recvBuf);
	}
}

void parseIncomingData(ClientOptions *ops, char *buf) {
	NormalHeader header;

	memcpy(&header.totalLen, buf, sizeof(uint16_t));
	memcpy(&header.flags, buf + sizeof(uint16_t), sizeof(uint8_t));

	header.totalLen = ntohs(header.totalLen);

	if (header.flags == INITIAL_PACKET_RESPONSE_FLAG_OK || header.flags == INITIAL_PACKET_RESPONSE_FLAG_NOT_OK) {
		handleInitialPacketResponse(ops, header.flags);
	}
	else if (header.flags == BROADCAST_MESSAGE_FLAG) {
		handleBroadcastMessage(buf);
	}
	else if (header.flags == PRIVATE_MESSAGE_FLAG) {
		handlePrivateMessage(buf);
	}
	else if (header.flags == HANDLE_DOES_NOT_EXIST_FLAG) {
		handleInvalidHandle(ops, buf);
	}
	else if (header.flags == EXIT_REQUEST_GRANTED_FLAG) {
		handleExitResponse();
	}
	else if (header.flags == SIZE_OF_HANDLE_LIST_FLAG) {
		handleSizeOfHandleList(ops, buf);
	}
	else if (header.flags == HANDLE_LIST_FLAG) {
		handleHandleList(ops, buf);
	}
}

void handleUserInput(ClientOptions *ops) {
	int sendLen = 0;
	char input[MAX_MESSAGE_LENGTH];
	memset(input, 0, MAX_MESSAGE_LENGTH);

	// get rid of second while loop condidtion?
	while ((input[sendLen] = getchar()) != '\n' && sendLen < MAX_MESSAGE_LENGTH - 1) {
		sendLen++;
	}
	input[sendLen] = '\0';

	if (input[0] == '%') {
		if (input[1] == 'M' || input[1] == 'm') {
			// Private message
			// %M handle [text]
			sendPrivateMessage(ops, getHandle(M_HANDLE_POS, input), getHandleLength(M_HANDLE_POS, input), getText(M_TEXT_POS, input));
		}
		else if (input[1] == 'B' || input[1] == 'b') {
			// Broadcast message
			// %B [text]
			sendBroadcastMessage(ops, getText(B_TEXT_POS, input));
		}
		else if (input[1] == 'L' || input[1] == 'l') {
			sendHandleListRequest(ops);
			// List handles
			// %L
		}
		else if (input[1] == 'E' || input[1] == 'e') {
			sendExitRequest(ops);
			// Exit
			// %E
		}
	}
}


// Sending Utility
void sendInitialPacket(ClientOptions *ops) {
	char *buf;
	char *tmp;
	NormalHeader header;

	header.totalLen = htons(ops->handleLen + 4);
	header.flags = INITIAL_PACKET_FLAG;

	tmp = buf = calloc(1, header.totalLen);
	memcpyIncrement(&tmp, &header.totalLen, sizeof(uint16_t));
	memcpyIncrement(&tmp, &header.flags, sizeof(uint8_t));
	memcpyIncrement(&tmp, &(ops->handleLen), sizeof(uint8_t));
	memcpyIncrement(&tmp, ops->handle, ops->handleLen);

	safeSend(ops->serverFd, buf, ntohs(header.totalLen), 0);
	free(buf);
}

void sendPrivateMessage(ClientOptions *ops, char *destHandle, int destHandleLen, char *text) {
	char *buf;
	char *tmp;
	NormalHeader header;

	header.totalLen = htons(3 + 1 + destHandleLen + 1 + ops->handleLen + strlen(text) + 1);
	header.flags = PRIVATE_MESSAGE_FLAG;

	tmp = buf = calloc(1, header.totalLen);

	// Normal Header
	memcpyIncrement(&tmp, &(header.totalLen), sizeof(uint16_t));
	memcpyIncrement(&tmp, &(header.flags), sizeof(uint8_t));

	// Dest Handle
	memcpyIncrement(&tmp, &(destHandleLen), sizeof(uint8_t));
	memcpyIncrement(&tmp, destHandle, destHandleLen);

	// Source Handle
	memcpyIncrement(&tmp, &(ops->handleLen), sizeof(uint8_t));
	memcpyIncrement(&tmp, ops->handle, ops->handleLen);

	// Message
	memcpyIncrement(&tmp, text, strlen(text) + 1);

	safeSend(ops->serverFd, buf, ntohs(header.totalLen), 0);
	free(buf);
	printPrompt();
}

void sendBroadcastMessage(ClientOptions *ops, char *text) {
	char *buf;
	char *tmp;
	NormalHeader header;

	header.totalLen = htons(3 + 1 + ops->handleLen + strlen(text) + 1);
	header.flags = BROADCAST_MESSAGE_FLAG;

	tmp = buf = calloc(1, header.totalLen);

	// Normal Header
	memcpyIncrement(&tmp, &(header.totalLen), sizeof(uint16_t));
	memcpyIncrement(&tmp, &(header.flags), sizeof(uint8_t));

	// Source Handle
	memcpyIncrement(&tmp, &(ops->handleLen), sizeof(uint8_t));
	memcpyIncrement(&tmp, ops->handle, ops->handleLen);

	// Message
	memcpyIncrement(&tmp, text, strlen(text) + 1);

	safeSend(ops->serverFd, buf, ntohs(header.totalLen), 0);
	free(buf);
	printPrompt();
}

void sendHandleListRequest(ClientOptions *ops) {
	char *buf;
	char *tmp;
	NormalHeader header;

	header.totalLen = htons(3);
	header.flags = HANDLE_LIST_REQUEST_FLAG;

	tmp = buf = calloc(1, header.totalLen);

	// Normal Header
	memcpyIncrement(&tmp, &(header.totalLen), sizeof(uint16_t));
	memcpyIncrement(&tmp, &(header.flags), sizeof(uint8_t));

	safeSend(ops->serverFd, buf, ntohs(header.totalLen), 0);
	free(buf);
}

void sendExitRequest(ClientOptions *ops) {
	char *buf;
	char *tmp;
	NormalHeader header;

	header.totalLen = htons(3);
	header.flags = EXIT_REQUEST_FLAG;

	tmp = buf = calloc(1, header.totalLen);

	// Normal Header
	memcpyIncrement(&tmp, &(header.totalLen), sizeof(uint16_t));
	memcpyIncrement(&tmp, &(header.flags), sizeof(uint8_t));

	safeSend(ops->serverFd, buf, ntohs(header.totalLen), 0);
	free(buf);
}


// Recieve Utility
void handleInitialPacketResponse(ClientOptions *ops, uint8_t flags) {
	if (flags == INITIAL_PACKET_RESPONSE_FLAG_OK) {
		// printf("Handle Acceptable!\n");
	}
	else {
		printf("Handle already in use: ");
		printHandle(ops->handle, ops->handleLen);
		exit(1);
	}
}

void handleBroadcastMessage(char *buf) {
	// printf("Handling Braodcast\n");
	int srcHandleLen;
	char *srcHandle;
	char *message;

	srcHandleLen = *((uint8_t *)(buf + 3));
	srcHandle = buf + 3 + 1;
	message = buf + 3 + 1 + srcHandleLen;

	printMessageOnNewLine(srcHandle, srcHandleLen, message);
	printPrompt();
}

void handlePrivateMessage(char *buf) {
	// printf("Handling Private\n");
	// Normal Header len
	// Normal Header len
	// Normal Header flag
	// Dest Handle Size
	// Dest Handle
	// ...
	// Src Handle Size
	// Src Handle
	// ...
	// Message
	int srcHandleLen;
	int destHandleLen;
	char *srcHandle;
	char *message;

	destHandleLen = *((uint8_t *)(buf + 3));
	srcHandleLen = *((uint8_t *)(buf +3 + 1 + destHandleLen));
	srcHandle = buf + 3 + 1 + destHandleLen + 1;
	message = buf + 3 + 1 + destHandleLen + 1 + srcHandleLen;

	printMessageOnNewLine(srcHandle, srcHandleLen, message);
	printPrompt();
}

void handleInvalidHandle(ClientOptions *ops, char *buf) {
	// Normal Header
	// Handle Length
	// Handle
	printf("Client with handle ");
	printHandleWithoutNewline(buf + 4, *((uint8_t *)(buf + 3)));
	printf(" does not exist.\n");
	printPrompt();
}

void handleExitResponse() {
	// Exit
	exit(0);
}

void handleSizeOfHandleList(ClientOptions *ops, char *buf) {
	// Normal Header
	// 4 byte integer # handles
	memcpy(&(ops->numKnownHandles), buf + 3, sizeof(int));
	ops->numKnownHandles = ntohl(ops->numKnownHandles);
}

void handleHandleList(ClientOptions *ops, char *buf) {
	int cliNum;
	int handleLen;

	buf = buf + 3;	// skip normal header
	// May need to do the recv calls synchronously one handle at a time?
	printf("Number of clients: %d\n", ops->numKnownHandles);
	for (cliNum = 0; cliNum < ops->numKnownHandles; cliNum++) {
		handleLen = *((uint8_t *)(buf));
		printf("   ");
		printHandle(buf + 1, handleLen);
		buf = buf + 1 + handleLen;
	}
	printPrompt();
}


// IO Utility
void multiplexClientIO(ClientOptions *ops) {
	int selectRet;
	fd_set recvFds;

	setupClientSelectFds(ops->serverFd, &recvFds);

	// printf("Listening on fds\n");
	selectRet = select(ops->serverFd + 1, &recvFds, NULL, NULL, NULL);
	if (selectRet < 0) {
		perror("Select Call");
		exit(1);
	}
	else if (selectRet) {
		if (FD_ISSET(ops->serverFd, &recvFds)) {
			// printf("Got Message from server!\n");
			handleServerMessage(ops);
		}
		else if (FD_ISSET(STDIN_FD, &recvFds)) {
			// printf("Got Message from keyboard!\n");
			handleUserInput(ops);
		}
	}
}

void setupClientSelectFds(int serverFd, fd_set *recvFds) {
	FD_ZERO(recvFds);
	FD_SET(serverFd, recvFds);
	FD_SET(STDIN_FD, recvFds);
}

char *getHandle(int pos, char *input) {
	while(pos--) {
		while (*input++ != ' ')
			;		
	}

	return input;
}

int getHandleLength(int pos, char *input) {
	char *handleStart;

	while(pos--) {
		while (*input++ != ' ')
			;		
	}

	handleStart = input;

	while (*input != ' ' && *input != '\0')
		input++;

	return input - handleStart;
}

char *getText(int pos, char *input) {
	while(--pos) {
		while (*input++ != ' ')
			;		
	}

	while (*input != ' ' && *input != '\0') {
		input++;
	}

	return input + 1;
}

void printMessageOnNewLine(char *srcHandle, int srcHandleLen, char *message) {
	printf("\n");
	printHandleWithoutNewline(srcHandle, srcHandleLen);
	printf(": %s\n", message);
}

void printPrompt() {
	printf("$: ");
	fflush(stdout);
}

// Memory Utility
void memcpyIncrement(char **destPtr, void *src, size_t len) {
	memcpy(*destPtr, src, len);
	*destPtr += len;
}


// Cmd Line Utility
void parseClientArgs(int argc, char *argv[], ClientOptions *ops) {
	if (argc != NUM_REQUIRED_ARGS) {
		fprintf(stderr, "usage: %s handle serverName serverPort\n", argv[0]);
		exit(1);
	}

	char *firstChar = ops->handle;
	ops->handleLen = 0;
	while (*argv[1]){
		*firstChar++ = *argv[1]++;
		ops->handleLen++;
	}
	ops->serverName = argv[2];
	ops->serverPort = atoi(argv[3]);
}





