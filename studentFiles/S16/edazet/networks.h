#ifndef NETWORKS_H
#define NETWORKS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

enum states {START, GET_LEN, GET_FLAG, GET_MESSAGE, HANDLE_NEXIST,
	GET_NUM_HANDLES, GET_HANDLES};

#define MESSAGE "%m"
#define BROADCAST "%b"
#define LIST "%l"
#define EXIT "%e"

#define F_INIT 1
#define F_HANDLE_CNFM 2
#define F_HANDLE_EXIST 3
#define F_BROADCAST 4
#define F_MESSAGE 5
#define F_HANDLE_NEXIST 7
#define F_CLIENT_EXIT 8
#define F_ACK_EXIT 9
#define F_HANDLE_LIST 10
#define F_NUM_HANDLES 11
#define F_HANDLES 12

#define CMD_LEN 2
#define MAX_HANDLE_LEN 255
#define PKT_LEN_FIELD 2
#define MAX_MSG_LEN 1000
#define MAX_PKT_LEN 1515

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct __attribute__((packed)) {
   uint16_t pkt_length;    //network order (htons)
   uint8_t flag;
} normal_header;

typedef struct __attribute__((packed)) client{
   int socket_num;
   uint16_t amountToRead;
   uint8_t handle_len;
   uint8_t *handle;
   struct client *next;
} client;

typedef struct {
   int numClients;
   client *clients;
} client_list;

// shared between client and server
int sendPacket(int socket_num, uint8_t *send_buf, uint16_t send_len);
int recvPacket(int socket_num, uint8_t *rcv_buf, uint16_t rcv_len);
int recvPacketClient(int socket_num, uint8_t *rcv_buf, uint16_t rcv_len);
normal_header *safeNormalHeader();
uint8_t *safeCharMalloc(int size);
uint8_t findStringLength(char *string);
int getChars(uint8_t **buf, uint16_t *len, uint8_t stop, uint16_t max_len, int null_term);
void sendNormalPacket(int socket_num, uint8_t flag);
void clearStdin();

// for the server side
void createClientList(client_list **cl);
client *createClient(int socket, uint8_t handle_len, uint8_t *handle);
int addClient(client_list **cl, uint8_t handle_len, uint8_t *handle, int c_socket);
void removeClient(client_list **cl, client *c);
uint8_t readNewClientPacket(int client_socket, uint8_t **handle_name, int amountToRead);
int setClients(client_list *cl, fd_set *r, int s_sock, int l_sock);
void prepMessage(client_list *cl, uint8_t *buf, client *c, uint8_t f);
void relayMessageToClient(client_list *cl, client *c, uint8_t *buf, uint16_t pkt_len);
void broadcastMessage(client_list *cl, client *c, uint8_t *buf, uint16_t pkt_len);
void sendHandles(client_list *cl, client *c, normal_header *h);
void sendNumHandles(client_list *cl, client *c);
void serverResponse(client_list **cl, uint8_t *buf, client *c);
int readFromClientSocket(client_list **cl, client **c);
void serverWork(int server_socket);
int tcp_recv_setup();
void tcp_listen(int server_socket, int back_log);
int newClient(int server_socket);

// for the client side
void checkArguments(int argc, char **argv);
uint8_t notifyServer(int socket_num, char *handle);
void buildMessage(int socket_num, uint8_t *com, char *handle, uint8_t h_len);
void sendMessage(int socket_num, uint8_t flag, uint8_t *msg, uint16_t msg_len,
   char *handle, uint8_t handle_len, uint8_t *d_handle, uint8_t dh_len);
void printMessage(uint8_t *buf, uint8_t flag, uint16_t pkt_len);
void printHandles(int socket_num, int numHandles);
void printHandleError(int socket_num, uint16_t pkt_len);
int checkCommand(int socket_num, char *handle, uint8_t handle_len);
int readSocket(int socket_num);
void userPrompt(int socket_num, char *handle, uint8_t handle_len);
int tcp_send_setup(char *host_name, char *port);
void resetValues(uint8_t **buf, uint8_t *flag, uint16_t *pkt_len, int *offset,
   int *state, int *retValue);
int setState(uint8_t flag);

#endif
