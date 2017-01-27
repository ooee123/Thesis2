//Nick Verbos
//networks.h for chat program

#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#define BACKLOG 5
#define INITIAL_PACKET 1
#define INITIAL_SUCCESS 2
#define INITIAL_ERROR 3
#define BROADCAST 4
#define MESSAGE 5
#define MESSAGE_ERROR 7
#define EXIT 8
#define ACK 9
#define REQUEST_HANDLES 10
#define NUMBER_HANDLES 11
#define HANDLE 12
#define MAXBUF 1024
#define MAX_MESSAGE_LEN 1000
#define MAX_HANDLE_LEN 256

typedef struct {
	u_short packetLen;
	u_char flag;
}__attribute__((packed)) ChatHeader;

// for the server side
int tcpServerSetup(int port);
int tcpAccept(int server_socket);

// for the client side
int tcpClientSetup(char *host_name, char *port);

#endif
