#ifndef CCLIENT_H
#define CCLIENT_H

#include "networkFuncs.h"

typedef struct clientInfo {
   char *ipAddr;        /*the name of the host*/
   uint16_t port;       /*port number*/
   char *handle;        /*handle of client*/
   int socketNum;       /*socket to server*/
   fd_set fdvar;        /*set of fds to call select on*/
   uint8_t *recBuf;     /*receive buffer*/
   int recLen;          /*keeps track of how much data has been received*/
   uint8_t *sendBuf;    /*send buffer*/
   int curSendSize;     /*used for making memory larger if needed*/
   int sendLen;         /*length of message to send*/
   uint32_t numHandles; /*num handles when asking
                          server for list of handles*/
} clientInfo;

void getArg(int argc, char **argv, int ndx, char **dest);
void parseArgs(clientInfo *ci, int argc, char **argv);
void setupClientSocket(clientInfo *ci);
void initClientInfo(clientInfo *ci, int argc, char **argv);
void cleanup(clientInfo *ci);
void displayMsg(clientInfo *ci);
void saveHandleNum(clientInfo *ci);
void processHandleList(clientInfo *ci);
int parsePacket(clientInfo *ci);
void verifyHandle(clientInfo *ci);
void buildBroadcast(clientInfo *ci);
int buildMessage(clientInfo *ci);
int processUserInput(clientInfo *ci);
void sendClientInput(clientInfo *ci);
int getMsg(clientInfo *ci);
int checkFds(clientInfo *ci);

#endif