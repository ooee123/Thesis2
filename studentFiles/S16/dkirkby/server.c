#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>

#include "networks.h"
#include "handle.h"
#include "testing.h"

#define NUM_INITIAL_CLIENTS 20

typedef struct  {
	char handle[MAX_HANDLE_SIZE];
	int handleLen;
	int fd;
	int isOpen;
} Client;

typedef struct {
	Client *clients;
	int lastClientNdx;
	int currentSize;
} ClientList;

void acceptIncomingConnection(int recvFd, ClientList *clist);
void acceptIncomingData(Client *cli, ClientList *clist);
void parseIncomingData(char *buf, int fromFd, ClientList *clist);

void handleInitialPacket(char *buf, int fromFd, ClientList *clist);
void routeBroadcastMessage(char *buf, int fromFd, ClientList *clist);
void routePrivateMessage(char *buf, int fromFd, ClientList *clist);
void handleClientExit(int fromFd, ClientList *clist);
void handleClientListRequest(int fromFd, ClientList *clist);

void mutliplexServerIO(int recvFd, fd_set *recvFds, ClientList *clist);
void setupServerSelectFds(int recvFd, fd_set *recvFds, ClientList *clist);

Client *getAvailableClient(ClientList *clist);
Client *getClientByFd(ClientList *clist, int fromFd);
void closeClient(Client *cli);
void initializeClientList(ClientList *clist);
void printClients(ClientList *clist);
int maxFd(ClientList *clist, int recvFd);
int numClients(ClientList *clist);
int handleExists(ClientList *clist, char *handle, int handleLen);

void parseServerArgs(int argc, char *argv[], uint16_t *portNum);

void memcpyIncrement(char **destPtr, void *src, size_t len);

// Main Function
int main(int argc, char *argv[])
{
	int recvFd;
	uint16_t serverPort = 0;
	ClientList clist;
	fd_set recvFds;

	parseServerArgs(argc, argv, &serverPort);
	initializeClientList(&clist);
	recvFd = setupTCPRecv(serverPort);

	while (1) {
		mutliplexServerIO(recvFd, &recvFds, &clist);
	}

	return 0;
}


// Chat Utility
void acceptIncomingConnection(int recvFd, ClientList *clist) {
	int clientFd;
	struct sockaddr_in clientAddr;
	Client *newCli;
	socklen_t clientAddrLen;

	clientAddrLen = sizeof(struct sockaddr_in);
	clientFd = safeAccept(recvFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
	newCli = getAvailableClient(clist);
	// printf("Accepted connection on fd %d\n", clientFd);

	newCli->fd = clientFd;
	newCli->isOpen = 1;
	clist->lastClientNdx++;


	if (clist->lastClientNdx >= clist->currentSize) {
		// printf("Expanding client list from size %d to size %d\n", clist->currentSize, clist->currentSize * 2);
		clist->currentSize *= 2;
		clist->clients = realloc(clist->clients, clist->currentSize);
	}
}

void acceptIncomingData(Client *cli, ClientList *clist) {
	int messageLen = 0;
	char recvBuf[MAX_MESSAGE_LENGTH];

	messageLen = safeRecv(cli->fd, recvBuf, MAX_MESSAGE_LENGTH, 0);
	if (messageLen == 0) {
		closeClient(cli);
		// printClients(clist);
	}
	else {
		// printf("Got data from fd %d of length %d\n", cli->fd, messageLen);
		parseIncomingData(recvBuf, cli->fd, clist);
	}
}

void parseIncomingData(char *buf, int fromFd, ClientList *clist) {
	NormalHeader header;

	memcpy(&header.totalLen, buf, sizeof(uint16_t));
	memcpy(&header.flags, buf + sizeof(uint16_t), sizeof(uint8_t));
	header.totalLen = ntohs(header.totalLen);

	if (header.flags == INITIAL_PACKET_FLAG) {
		handleInitialPacket(buf, fromFd, clist);
	}
	else if (header.flags == BROADCAST_MESSAGE_FLAG) {
		routeBroadcastMessage(buf, fromFd, clist);
	}
	else if (header.flags == PRIVATE_MESSAGE_FLAG) {
		routePrivateMessage(buf, fromFd, clist);
	}
	else if (header.flags == EXIT_REQUEST_FLAG) {
		handleClientExit(fromFd, clist);
	}
	else if (header.flags == HANDLE_LIST_REQUEST_FLAG) {
		handleClientListRequest(fromFd, clist);
	}
}


// Recieve Utility
void handleInitialPacket(char *buf, int fromFd, ClientList *clist) {
	char handle[MAX_HANDLE_SIZE];
	int handleLen;
	NormalHeader header;
	Client *client;

	handleLen = *((uint8_t *)(buf + 3));
	memcpy(handle, buf + sizeof(uint16_t) + sizeof(uint8_t) + 1, handleLen);

	// Does the handle already exist
	if (handleExists(clist, handle, handleLen)) {
		header.flags = INITIAL_PACKET_RESPONSE_FLAG_NOT_OK;
	}
	else {
		header.flags = INITIAL_PACKET_RESPONSE_FLAG_OK;
		client = getClientByFd(clist, fromFd);
		memcpy(client->handle, handle, handleLen);
		client->handleLen = handleLen;
		// printClients(clist);
	}

	// Respond to Initial Packet
	header.totalLen = htons(3);
	buf = malloc(ntohs(header.totalLen));
	memcpy(buf, &header.totalLen, sizeof(uint16_t));
	memcpy(buf + sizeof(uint16_t), &header.flags, sizeof(uint8_t));
	safeSend(fromFd, buf, ntohs(header.totalLen), 0);
	free(buf);
}

void routeBroadcastMessage(char *buf, int fromFd, ClientList *clist) {
	int clientNdx;
	char *srcHandle;
	int srcHandleLen;
	NormalHeader header;

	srcHandleLen = *((uint8_t *)(buf + 3));
	srcHandle = buf + 3 + 1;
	
	header.totalLen = ntohs(*((uint16_t *)buf));

	for (clientNdx = 0; clientNdx < clist->lastClientNdx; clientNdx++) {
		Client *cli = clist->clients + clientNdx;
		if (cli->isOpen && !handleEquals(srcHandle, srcHandleLen, cli->handle, cli->handleLen)) {
			// printf("Broadcasting on fd %d\n", cli->fd);
			safeSend(cli->fd, buf, header.totalLen, 0);
		}
	}
}

void routePrivateMessage(char *buf, int fromFd, ClientList *clist) {
	int clientNdx;
	char *srcHandle;
	int srcHandleLen;
	char *destHandle;
	int destHandleLen;
	char *tmp;
	NormalHeader header;

	destHandleLen = *((uint8_t *)(buf + 3));
	destHandle = buf + 3 + 1;

	srcHandleLen = *((uint8_t *)(buf + 3 + 1 + destHandleLen));
	srcHandle = buf + 3 + 1 + destHandleLen + 1;

	header.totalLen = ntohs(*((uint16_t *)buf));

	for (clientNdx = 0; clientNdx < clist->lastClientNdx; clientNdx++) {
		Client *cli = clist->clients + clientNdx;
		if (handleEquals(destHandle, destHandleLen, cli->handle, cli->handleLen)) {
			safeSend(cli->fd, buf, header.totalLen, 0);
			return;
		}
	}

	// Error packet
	header.totalLen = htons(3 + 1 + destHandleLen);
	header.flags = HANDLE_DOES_NOT_EXIST_FLAG;

	tmp = buf = malloc(ntohs(header.totalLen));
	memcpyIncrement(&tmp, &header.totalLen, sizeof(uint16_t));
	memcpyIncrement(&tmp, &header.flags, sizeof(uint8_t));
	memcpyIncrement(&tmp, &destHandleLen, sizeof(uint8_t));
	memcpyIncrement(&tmp, destHandle, destHandleLen);

	safeSend(fromFd, buf, ntohs(header.totalLen), 0);
	free(buf);
}

void handleClientExit(int fromFd, ClientList *clist) {
	NormalHeader header;
	char *buf;
	char *tmp;

	header.totalLen = htons(3);
	header.flags = EXIT_REQUEST_GRANTED_FLAG;

	tmp = buf = malloc(ntohs(header.totalLen));
	memcpyIncrement(&tmp, &header.totalLen, sizeof(uint16_t));
	memcpyIncrement(&tmp, &header.flags, sizeof(uint8_t));

	safeSend(fromFd, buf, ntohs(header.totalLen), 0);
	free(buf);
}

void handleClientListRequest(int fromFd, ClientList *clist) {
	NormalHeader header;
	Client *cli;
	char *buf;
	char *tmp;
	int nClients;
	int clientNdx;
	int diff;

	header.totalLen = htons(7);
	header.flags = SIZE_OF_HANDLE_LIST_FLAG;

	tmp = buf = malloc(ntohs(header.totalLen));
	nClients = htonl(numClients(clist));
	memcpyIncrement(&tmp, &header.totalLen, sizeof(uint16_t));
	memcpyIncrement(&tmp, &header.flags, sizeof(uint8_t));
	memcpyIncrement(&tmp, &nClients, sizeof(int));

	safeSend(fromFd, buf, ntohs(header.totalLen), 0);
	free(buf);


	header.totalLen = htons(3);
	header.flags = HANDLE_LIST_FLAG;
	tmp = buf = malloc(ntohs(header.totalLen));
	memcpyIncrement(&tmp, &header.totalLen, sizeof(uint16_t));
	memcpyIncrement(&tmp, &header.flags, sizeof(uint8_t));

	for (clientNdx = 0; clientNdx < clist->currentSize; clientNdx++) {
		cli = clist->clients + clientNdx;
		if (cli->isOpen) {
			diff = tmp - buf;
			header.totalLen = htons(ntohs(header.totalLen) + cli->handleLen + 1);
			buf = realloc(buf, ntohs(header.totalLen));
			tmp = buf + diff;
			memcpyIncrement(&tmp, &(cli->handleLen), sizeof(uint8_t));
			memcpyIncrement(&tmp, cli->handle, cli->handleLen);
		}

	}

	safeSend(fromFd, buf, ntohs(header.totalLen), 0);
	free(buf);
}


// IO Utility
void mutliplexServerIO(int recvFd, fd_set *recvFds, ClientList *clist) {
	int clientNdx;
	int selectRet;
	Client *cli;

	setupServerSelectFds(recvFd, recvFds, clist);

	selectRet = select(maxFd(clist, recvFd) + 1, recvFds, NULL, NULL, NULL);
	if (selectRet < 0) {
		perror("Select Call");
		exit(1);
	}
	else if (selectRet) {
		if (FD_ISSET(recvFd, recvFds)) {
			acceptIncomingConnection(recvFd, clist);
		}
		else {
			for (clientNdx = 0; clientNdx < clist->lastClientNdx; clientNdx++) {
				cli = clist->clients + clientNdx;
				if (cli->isOpen && FD_ISSET(cli->fd, recvFds)) {
					acceptIncomingData(cli, clist);
				}
			}
		}
	}
}

void setupServerSelectFds(int recvFd, fd_set *recvFds, ClientList *clist) {
	int clientNdx;
	Client *cli;

	FD_ZERO(recvFds);
	FD_SET(recvFd, recvFds);
	for (clientNdx = 0; clientNdx < clist->lastClientNdx; clientNdx++) {
		cli = clist->clients + clientNdx;
		if (cli->isOpen) {
			FD_SET(cli->fd, recvFds);
		}
	}
}


// Client Utility
Client *getAvailableClient(ClientList *clist) {
	int clientNdx;
	Client *cli;

	for (clientNdx = 0; clientNdx < clist->currentSize; clientNdx++) {
		cli = clist->clients + clientNdx;
		if (cli->isOpen == 0) {
			return cli;
		}
	}

	return NULL;
}

Client *getClientByFd(ClientList *clist, int fromFd) {
	int clientNdx;

	for (clientNdx = 0; clientNdx < clist->lastClientNdx; clientNdx++) {
		if (clist->clients[clientNdx].fd == fromFd) {
			// printf("Index %d\n", clientNdx);
			return clist->clients + clientNdx;
		}
	}

	return NULL;
}

void closeClient(Client *cli) {
	char *handlePtr;

	cli->isOpen = 0;
	close(cli->fd);
	// printf("Client at fd %d closed!\n", cli->fd);

	cli->fd = 0;
	handlePtr = cli->handle;
	while (cli->handleLen--){
		*handlePtr++ = 0;
	}
	cli->handleLen = 0;
}

void initializeClientList(ClientList *clist) {
	clist->clients = calloc(NUM_INITIAL_CLIENTS, sizeof(Client));
	clist->lastClientNdx = 0;
	clist->currentSize = NUM_INITIAL_CLIENTS;
}

void printClients(ClientList *clist) {
	int clientNdx;

	for (clientNdx = 0; clientNdx < clist->currentSize; clientNdx++) {
		Client *cli = clist->clients + clientNdx;
		printf("Client (status = %s) at index %d on fd %d with handle ", cli->isOpen ? "Open" : "Closed", clientNdx, cli->fd);
		printHandle(cli->handle, cli->handleLen);
	}
}

int maxFd(ClientList *clist, int recvFd) {
	int clientNdx;
	int maxFd = recvFd;
	Client *cli;

	for (clientNdx = 0; clientNdx < clist->currentSize; clientNdx++) {
		cli = clist->clients + clientNdx;
		if (cli->fd > maxFd)
			maxFd = cli->fd;
	}

	return maxFd;
}

int numClients(ClientList *clist) {
	int clientNdx;
	int nClients = 0;
	Client *cli;

	for (clientNdx = 0; clientNdx < clist->currentSize; clientNdx++) {
		cli = clist->clients + clientNdx;
		if (cli->isOpen)
			nClients++;
	}

	return nClients;
}

int handleExists(ClientList *clist, char *handle, int handleLen) {
	int clientNdx;

	for (clientNdx = 0; clientNdx < clist->lastClientNdx; clientNdx++) {
		Client *cli = clist->clients + clientNdx;
		if (handleEquals(handle, handleLen, cli->handle, cli->handleLen)) {
			return 1;
		}
	}

	return 0;
}


// Cmd Line Utility
void parseServerArgs(int argc, char *argv[], uint16_t *portNum) {
	if (argc == 2) {
		*portNum = atoi(argv[1]);
	}
	else {
		*portNum = 0;
	}
}

// Memory Utility
void memcpyIncrement(char **destPtr, void *src, size_t len) {
	memcpy(*destPtr, src, len);
	*destPtr += len;
}




