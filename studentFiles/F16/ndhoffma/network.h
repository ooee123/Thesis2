#ifndef __NETWORKS_H__
#define __NETWORKS_H__


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

#define BACKLOG 5
#define MAX_BUF_SIZE 2048
#define MAX_MSG_LEN 1024
#define MAX_HANDLE_LEN 255

#define FLAG_1  0x01
#define FLAG_2  0x02
#define FLAG_3  0x03
#define FLAG_4  0x04
#define FLAG_5  0x05
#define FLAG_7  0x07
#define FLAG_8  0x08
#define FLAG_9  0x09
#define FLAG_10 0x0a
#define FLAG_11 0x0b
#define FLAG_12 0x0c

/* server side */
int serverInit(char *portNum);
int serverAccept(int server_socket);
int parseServerArgs(int argc, char *argv[]);

/* client side */
int clientInit(char *serverName, char *portNum);
int parseClientArgs(int argc, char *argv[], char **handle);

struct user {
   uint16_t hLen;
   uint8_t handle[MAX_HANDLE_LEN];
   int socket;
};

struct chat_header {
   uint16_t length;
   uint8_t  flag;
}__attribute__((__packed__));

struct handle {
   uint8_t hLen;
   uint8_t handle[MAX_HANDLE_LEN];
}__attribute__((__packed__));

#endif
