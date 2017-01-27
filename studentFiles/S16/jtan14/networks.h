// for the server side
int rcvSetup();
void sockListen(int servSock, int back);
void loopClients(int servSock);
int acceptSock(int servSock);
int nextSock (int servSock, fd_set *read, fd_set *fds);
void validHandle(struct handleList* list, char* rcvBuff, fd_set* fds, 
                 int curClient);
void directMsg(struct handleList* list, char* rcvBuff, int curClient);
void broadCast(struct handleList* list, char* rcvBuff, int curClient);
void reqHandles(struct handleList* list, char* rcvBuff, int curClient);
void processExit(struct handleList* list, char* rcvBuff, int curClient, 
                 fd_set* fds);
void removeFromList(struct handleList* list, int curClient, fd_set* fds);

// for the client side
void setupSocket(int *sockNum, char *host, char *port);
void chkCmdLine(int argc, char** argv);
void sendMsg(char *handle, int socketNum, char *locHandle);
void runClient(int socketNum, char* handleName);
void checkHandle(char *handleName, int sockNum);
void sendResponse(int servSock, char* rcvBuff, int curClient,
                  struct handleList* list, int msgLen, fd_set* fds);
void getMsg(int socketNum, int *numClients);
void readInput(int *stopSig, int socketNum, char* handleName);
void processMsg(char *rcvBuff);
void nonExist(char *rcvBuff);
void processBroadCast(char *rcvBuff);
void processLength(char *rcvBuff, int* numClients);
void processHandleInfo(int socketNum, char *rcvBuff, int* numClients);
void execExitCmd(int socketNum, char* handleName);