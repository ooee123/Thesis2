/******************************************************************************
 * Timothy Chu 3pm Lab
 * CPE464 Program2
 *****************************************************************************/

#define C_TO_S_INITIAL_PACKET 1
#define S_TO_C_GOOD_HANDLE 2
#define S_TO_C_BAD_HANDLE 3
#define C_TO_S_BROADCAST 4
#define C_TO_S_MESSAGE 5
#define S_TO_C_NO_HANDLE 7
#define C_TO_S_EXIT 8
#define S_TO_C_ACK_EXIT 9
#define C_TO_S_LIST_HANDLES 10
#define S_TO_C_LIST_HANDLES_ACK 11
#define S_TO_C_LIST_HANDLES 12

#define OPCODE_LEN 2
#define MAX_MSG_LEN 1000
#define MAX_HANDLE_LEN 255

typedef struct normalHeader {
   uint16_t packetLen;
   uint8_t flag;
} __attribute__((packed)) normalHeader;

typedef struct handleFDPair {
   char *handle;
   int fd;
} __attribute__((packed)) handleFDPair;


/****** client side ******/
int setUpClient(int argc, char **argv, char *handle);
void sendInitialPacket(int socket_num, char *handle);
void checkReceivedServerData(int recvd, int socket_num);
void clientChat(int socket_num, char *handle);
void serviceClientFDS(int socket_num, char *handle);
void processServerData(int socket_num);
int receiveServerData(int socket_num, char **recv_buf, uint16_t *packetLen);
void printReceivedServerData(int socket_num, char *recv_buf, int packetLen);
void processSTDIN(int socket_num, char *handle);
void sendClientRequest(int socket_num, char *srcHandle, char *opcode, char *destHandle,
 char *buf, int send_len);
int getBroadcastMessage(char *buf, char **msg, uint16_t *msgLen);
void sendBroadcast(int socket_num, char *srcHandle, char *msg, 
 uint16_t msgLen);
int checkHandleAndMessage(char *destHandle, char *buf, uint16_t *msgLen);
void sendMessage(int socket_num, char *srcHandle, char *destHandle, 
 char *msg, uint16_t msgLen);
void sendExit(int socket_num);
void sendList(int socket_num);

/****** server side ******/
int setUpServer(int argc, char **argv);
void serverChat(int server_socket);
void serviceServerFDS(int server_socket, char ***handles, 
 int *numHandles, int *highestFD);
void processConnections(int server_socket, int *highestFD, char ***handles, 
 int *numHandles);
void checkClientInitialPacket(int recvd, int client_socket);
void initialPacketResponse(int client_socket, char *buf, 
 char ***handles, int *numHandles);
int verifyHandle(int client_socket, char *buf, char ***handles, 
 int *numHandles);
int compareHandle(char *handle, char ***hanldes, int i);
void ackHandle(int client_socket, int exist);
void processClientData(char ***handles, int *numHandles, int *highestFD, 
 fd_set read_fd_set);
int receiveClientData(char ***handles, int *numHandles, int socket_num, 
 char **recv_buf, uint16_t *packetLen);
int checkReceivedClientData(char ***handles, int *numHandles, int recvd, int client_socket);
void replyToClient(int client_socket, char *buf, int message_len, 
 char ***handles, int *numHandles);
void forwardBroadcast(int client_socket, char *buf, int message_len, 
 char ***handles, int *numHandles);
void forwardMessage(int client_socket, char *buf, int message_len,
 char ***handles, int *numHandles);
int destinationError(char *buf, char ***handles, int *numHandles, int *destFD);
void ackExit(int socket_num, char ***handles, int *numHandles);
void removeHandle(int socket_num, char ***handles, int *numHandles, int i);
void ackListNum(int socket_num, int *numHandles);
void ackListHandles(int socket_num, char ***handles, int *numHandles);
