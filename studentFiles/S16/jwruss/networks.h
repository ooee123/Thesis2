// #define variables
#define EXIT_FLAG 8
#define EXIT_ACK 9
#define NF_FLAG 7
#define BRDCST_FLAG 4
#define LIST_FLAG 10
#define COUNT_FLAG 11
#define NAME_FLAG 12
#define L_LENGTH 0
#define BRDCST_FLAG 4
#define MSG_FLAG 5
#define NORM_HEADER 3
#define FLAG
#define USER_INPUT 0
#define GOOD_ACK 2
#define BAD_ACK 3
#define BASE_TEN 10
#define LARGE_BUFFER 10000
#define MAX_SIZE 1520

// for the server side
int tcp_recv_setup();
void tcp_listen(int server_socket, int back_log);
int tcp_accept(int server_socket);
void sendListCount(int socket, uint8_t flag, uint16_t length);
void sendListNames(int socket, uint8_t flag);
void sendPacket(char *packet, int socket_num);
int findFD(char *name);
char *recvPacket(int fdIndex);
char *getName(char* packet);
void handOffPacket(char *packet, int fdIndex);
void removeClient(int fdIndex);
void sendExitAck(int fdIndex);
void sendNotFoundPacket(int fdIndex, char *destName);
void sendBroadcast(char *packet, int socket);
void sendACK(uint8_t flag, int socket_num);
uint8_t processACK(int fdIndex, int client_socket, char *packet);
void setup_and_run(int argc, char **argv);
int makeBuffer(int argc, char **argv);
void initSet(int server_socket);
void get_listening(fd_set curr_set, int fdIndex, int server_socket);
void setupNewClient(int server_socket, int fdIndex);
void beginReceive(int fdIndex);

// for the client side
int tcp_send_setup(char *host_name, char *port);
char *parseMessage(char *buf, uint16_t length, uint8_t flag, char *sender,
 uint8_t nameLen);
void initClient(int socket_num, uint8_t dataLen, char *name);
char *listRequest(uint16_t length, uint8_t flag);
uint32_t recvListNum(char *packet);
void recvFullList(int socket_num);
void printMessage(char *message);
void processList(int socket_num);
void initRecv(int socket_num);
void processPacket(char *packet);
void recvMessage(char *packet);
void sendExitPacket(int socket_num);
char *parseBroadcast(char *buf, uint16_t msg_len, uint8_t flag, char *sender,
 uint8_t senderLen);
void recvBroadcast(char *packet);
void printNotFoundError(char *packet);
void handleBroadcast(char *send_buf, int send_len, char *extra_buf, char *name,
 uint8_t nameLen, int socket_num);
void handleMessage(char *send_buf, int send_len, char *extra_buf, char *name,
 uint8_t nameLen, int socket_num);
void listen_and_receive(int socket_num, uint8_t nameLen, char *name,
 fd_set working_set);
void processCommand(char *name, uint8_t nameLen, int socket_num);
char *getClientName(int nameLen, char **argv);
void checkArguments(int argc, char **argv);
void initialize_and_loop(int argc, char **argv);
