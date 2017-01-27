#ifndef SERVER_H
#define SERVER_H

#include "networkFuncs.h"

#define LISTEN_BACKLOG 5
#define NAME_OFFSET 4
/*message types*/
#define SEND_ONE -2
#define SEND_ALL -1

typedef struct serverInfo {
   int socketNum;     /*server socket*/
   handleNode *list;  /*list of all client*/
   uint32_t listSize; /*number of clients in the list*/
   fd_set fdvar;      /*set of fds to call select on*/
   int highSock;      /*current high socket number*/
   uint8_t *recBuf;   /*receving buffer*/  
   uint8_t *sendBuf;  /*sending buffer*/
   int sendLen;       /*length of sending message*/
   int processList;   /*flag to use for sending handle list*/
} serverInfo;

int getPortNum(int argc, char **argv);
void setupServerSocket(serverInfo *si, int port);
void tcp_listen(int socketNum);
int tcp_accept(int socketNum);
void removeSocket(serverInfo *si, int socketNum);
void checkInitPackage(serverInfo *si, int senderSocket);
void buildMessage(serverInfo *si, int initLen, int senderLoc);
int startMessage(serverInfo *si, int senderSocket);
void sendHandleList(serverInfo *si);
int parsePacket(serverInfo *si, int senderSocket);
void sendBroadcast(serverInfo *si, int sender);
void checkConnections(serverInfo *si);
void checkFds(serverInfo *si);
void initServerInfo(serverInfo *si, int argc, char **argv);
void cleanup(serverInfo *si);

#endif