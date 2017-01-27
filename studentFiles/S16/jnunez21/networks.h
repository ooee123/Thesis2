#include <stdio.h>
#include <stdint.h>
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
#include <limits.h>

#include "networks.c"

#define INIT_PKT_FLG 1
#define GOOD_REPLY_FLG 2
#define BAD_REPLY_FLG 3
#define BRDCST_FLG 4
#define MSG_FLG 5
#define BAD_HDL_FLG 7
#define EXIT_REQ_FLG 8
#define EXIT_ACK_FLG 9
#define LIST_REQ_FLG 10
#define LIST_NUM_FLG 11
#define LIST_FLG 12

#define RECV_FAILED -1
#define DUP_HANDLE -2
#define DROPPED_CLIENT -3

#define STARTING_MAX_NUM_CLIENTS 5
#define MAX_MSG_SIZE 1000

#pragma pack(1)

/* for the server side */
int TcpRecvSetup();
int TcpListen(int serverSocket, int backlog);

/* for the client side */
int tcpSendSetup(char *hostName, char *port);

/* for both */
int safeSend(int, char *, int);
int safeRecv(int, char *, int);
