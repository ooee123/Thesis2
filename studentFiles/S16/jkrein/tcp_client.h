#define SIZELEN 2
#define MAX_MSGLEN 1259
#define MAXTEXT 1000

void recieveHandles(int socketNum);
void printNoHandle(char *message);
void printBcast(char *message);
void sendBcast(char *msg);
void sendDm(char *msg, char *handle);
void requestList();
void establishHandle();
void runClient();
int procServMessage(char *msg);
int checkValidMsg(int socketNum, char *data);
int safeRecv(int socket_num, char *buf, int bufSize);
void waitForClients(int serverSocket, int backlog);
void listenToSocket(int clientSocket);
void printChars (char *message, int len);
int parseOption(char *option);
normal_header *makeNormHead(char flag, u_short len);

