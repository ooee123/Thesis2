#ifndef NETWORKFUNCS_H
#define NETWORKFUNCS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "testing.h"

#define BUF_SIZE 1024
#define MAX_HANDLE_LEN 255
/*header flags*/
#define FLAG_CLIENT_INIT 1        /*client's initial packet*/
#define FLAG_CLIENT_HANDLE_GOOD 2 /*server response of valid handle*/
#define FLAG_CLIENT_HANDLE_ERR 3  /*server response of invalid handle*/
#define FLAG_BROADCAST 4          /*broadcast message*/
#define FLAG_MESSAGE 5            /*message to specific client*/
#define FLAG_BAD_HANDLE 7         /*handle not found*/
#define FLAG_REQ_EXIT 8           /*client is requesting to exit*/
#define FLAG_ACK_EXIT 9           /*server acknowledges a clients exit request*/
#define FLAG_REQ_LIST 10          /*client requesting handle list*/
#define FLAG_LIST_SIZE 11         /*server sends number of handles*/
#define FLAG_LIST 12              /*server sending handle list*/
/*receive msg states*/
#define INIT_RECV 0
#define ONLY_ONE 1
/*flags to receiveMessage*/
#define BLOCKING_SERVER 1         /*server*/
#define NONBLOCKING 0             /*nonblocking*/
#define BLOCKING_CLIENT 2         /*blocking client*/
#define HANDLE_LIST 4             /*blocking client but start read directly into front of buffer*/

typedef struct handleNode {
   char *handle;
   int socketNum;
   int curLen;
   struct handleNode *next;
} handleNode;

typedef struct header {
   uint16_t len;
   uint8_t flag;
} __attribute__((packed)) header;

/*handles*/
void initHandle(handleNode **list, int socketNum);
void freeHandle(handleNode **list, int socketNum);
int findHandle(handleNode *list, char *name);
void setHandle(handleNode *list, int socketNum, char name[MAX_HANDLE_LEN]);
int findHighestSocket(handleNode *list);

/*general functions*/
void createHeader(uint8_t **buf, int flag, uint16_t len);
void sendMessage(uint8_t *buf, int targetSocket, int sendLen);
int receiveMessage(uint8_t **buf, int recLen, int targetSocket, int isServer);
int selectCall(fd_set *fds, int highSock);
void extractHandle(char handle[MAX_HANDLE_LEN], uint8_t *buf, int offset, int len);
uint16_t getLen(uint8_t *buf);

#endif