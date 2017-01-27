#ifndef NETWORKS_H
#define NETWORKS_H

#define TRUE 1
#define FALSE 0

#define FLAG_CTS_INIT 1
#define FLAG_STC_GOOD_HANDLE 2
#define FLAG_STC_BAD_HANDLE 3
#define FLAG_BROADCAST 4
#define FLAG_MESSAGE 5
#define FLAG_STC_DEST_NOT_EXIST 7
#define FLAG_CTS_EXIT 8
#define FLAG_STC_CONFIRM_EXIT 9
#define FLAG_CTS_REQ_LIST_CLIENTS 10
#define FLAG_STC_CLIENT_COUNT 11
#define FLAG_STC_CLIENT_LIST 12
#define SERVER_BUFF_SIZE 2000
#define MAX_MSG_SIZE 1000
#define MAX_HANDLE_SIZE 255
#define BASE_FD 4

typedef struct Header Header;
typedef struct Packet Packet;
typedef struct MessageHandle MessageHandle;
typedef struct Handle Handle;
typedef struct HandleList HandleList;

struct Header {
   u_short packetLength;
   u_char flag;
} __attribute__((packed));

struct HandleList {
   int socket;
   int length;
   char *handle;
   struct HandleList *next;
} __attribute__((packed));

// for the server side
int tcp_recv_setup(int socket_num);
int tcp_listen(int server_socket, int back_log);

// for the client side


#endif