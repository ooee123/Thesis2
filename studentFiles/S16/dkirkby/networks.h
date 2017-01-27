#ifndef NETWORKS_H
#define NETWORKS_H

// Defines useful TCP networking utility functions

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

#define MAX_HANDLE_SIZE 255
#define MAX_MESSAGE_LENGTH 1000
#define INITIAL_PACKET_FLAG 1
#define INITIAL_PACKET_RESPONSE_FLAG_OK 2
#define INITIAL_PACKET_RESPONSE_FLAG_NOT_OK 3
#define BROADCAST_MESSAGE_FLAG 4
#define PRIVATE_MESSAGE_FLAG 5
#define HANDLE_DOES_NOT_EXIST_FLAG 7
#define EXIT_REQUEST_FLAG 8
#define EXIT_REQUEST_GRANTED_FLAG 9
#define HANDLE_LIST_REQUEST_FLAG 10
#define SIZE_OF_HANDLE_LIST_FLAG 11
#define HANDLE_LIST_FLAG 12

#define DEFAULT_BACKLOG 5

typedef struct {
	uint16_t totalLen;
	uint8_t flags;
} NormalHeader;

// Both
int safeSocket(int domain, int type, int protocol);
ssize_t safeSend(int sockfd, const void *buf, size_t len, int flags);
ssize_t safeRecv(int sockfd, void *buf, size_t len, int flags);

// Client Side
int setupTCPSend(char *hostname, int port);
void safeConnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

// Server Side
int setupTCPRecv(uint16_t serverPort);
void safeBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void safeListen(int sockfd, int backlog);
int safeAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

#endif
