#define MAX_MSGLEN 1400
#define SIZELEN 2

typedef struct clientInfo
{
    int32_t socket;
    u_char handleLen;
    char *handle;
}clientInfo;

int safeRecv(int socket_num, char *buf, int bufSize);
int checkValidMsg(int socketNum, char *data);
void gotBroadcast(char *message, int recSocket, u_short expLen);
void gotDirectMessage(char *message, int recSocket, u_short expLen);
void sendHandles(char *message, int recSocket);
void initialConnect(int recSocket, char *message);
normal_header *makeNormHead(short len, char flag);
int max(int num1, int num2);
int getHandleSock(char len, char *handle);
int addHandle(char len, char *handle, int port);
void handleMessages(char *message, int recSocket);
clientInfo *connectClient(int sockNum);
void waitForClients(int serverSocket, int backlog);
void listenToSocket(int clientSocket);

void closeSocket(int toClose);
void checkClientSockets(fd_set msgFds);
