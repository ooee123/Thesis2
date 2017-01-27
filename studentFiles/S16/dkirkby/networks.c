// Implements useful TCP networking utility functions

#include "networks.h"

int safeSocket(int domain, int type, int protocol) {
	int fd;

	if ((fd = socket(domain, type, protocol)) < 0) {
		perror("Socket Call");
		exit(1);
	}

	return fd;
}

ssize_t safeSend(int sockfd, const void *buf, size_t len, int flags) {
	ssize_t nsent;

	if ((nsent = send(sockfd, buf, len, flags)) < 0) {
		perror("Send Call");
		exit(1);
	}

	return nsent;
}

ssize_t safeRecv(int sockfd, void *buf, size_t len, int flags) {
	ssize_t nrecvd;

	if ((nrecvd = recv(sockfd, buf, len, flags)) < 0) {
		perror("Recv Call");
		exit(1);
	}

	return nrecvd;
}

int setupTCPSend(char *hostname, int port) {
	int fd;
	struct sockaddr_in remote;

	fd = safeSocket(AF_INET, SOCK_STREAM, 0);

	struct hostent *host = gethostbyname(hostname);
	if (!host) {
		printf("Host Name Lookup Failed!\n");
		exit(1);
	}

	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	memcpy(&(remote.sin_addr), host->h_addr_list[0], host->h_length);

	safeConnect(fd, (struct sockaddr *)&remote, sizeof(struct sockaddr_in));

	return fd;
}

void safeConnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	if (connect(sockfd, addr, addrlen) < 0) {
		perror("Connect Call");
		exit(1);
	}
}

int setupTCPRecv(uint16_t serverPort) {
	int recvFd;
	struct sockaddr_in local;
	socklen_t len = sizeof(struct sockaddr_in);

	recvFd = safeSocket(AF_INET, SOCK_STREAM, 0);

	local.sin_family = AF_INET;
	local.sin_port = htons(serverPort);			// Let the system choose the port
	local.sin_addr.s_addr = INADDR_ANY;		// Wildcard address

	safeBind(recvFd, (struct sockaddr *)&local, sizeof(struct sockaddr));

	if (getsockname(recvFd, (struct sockaddr *)&local, &len) < 0) {
		perror("Get Socket Name Call");
		exit(1);
	}

	safeListen(recvFd, DEFAULT_BACKLOG);

	printf("Server is using port: %d\n", ntohs(local.sin_port));

	return recvFd;
}

void safeBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	if (bind(sockfd, addr, addrlen) < 0) {
		perror("Bind Call");
		exit(1);
	}
}

void safeListen(int sockfd, int backlog) {
	if (listen(sockfd, backlog) < 0) {
		perror("Listen Call");
		exit(1);
	}
}

int safeAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	int fd;

	if ((fd = accept(sockfd, addr, addrlen)) < 0) {
		perror("Accept Call");
		exit(1);
	}

	return fd;
}
