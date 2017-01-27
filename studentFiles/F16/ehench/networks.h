/* networks.h
  	Code originally give to Prof. Smith by his TA in 1994.
	No idea who wrote it.  Copy and use at your own Risk
*/


#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#include "testing.h"
#include <inttypes.h>

#define BACKLOG 5
#define BYTE 1
#define CHAT_HEADER_SIZE 3
#define CLIENT 0
#define CLIENT_END -1
#define COMMAND_LEN 2
#define EXPECTED_ARGUMENTS 4
#define FALSE 0
#define FLAG_BYTE 2
#define FLAG_1 1
#define FLAG_2 2
#define FLAG_3 3
#define FLAG_4 4
#define FLAG_5 5
#define FLAG_6 6
#define FLAG_7 7
#define FLAG_8 8
#define FLAG_9 9
#define FLAG_10 10
#define FLAG_11 11
#define FLAG_12 12
#define HANDLE 1
#define INTEGER 4
#define MAX_BUF 1620
#define MAX_HANDLE_LEN 255
#define MAX_MESSAGE_LEN 1000
#define MESSAGE_START 3
#define SERVER 1
#define SERVER_NAME 2
#define SERVER_PORT 3
#define SERVER_SOCKET 0
#define SOCK_INCREMENT 10
#define TRUE 1

/********************************* Structs *********************************/
typedef struct Client {
   int8_t handle_len;
   int8_t *handle;
   int32_t socket;
} __attribute__ ((packed)) Client;

// These should all be in network order

typedef struct ChatHeader {
   int16_t packet_len;
   int8_t flag;
} __attribute__ ((packed)) ChatHeader;


// Flag == 1
// Client to server -- Initial packet asking for new handle
typedef struct HandleRequest {
   int8_t handle_len;
   int8_t *handle;
} __attribute__ ((packed)) HandleRequest;

// Flag == 2
// Server back to client -- Confirming a good handle
// No struct needed

// Flag == 3
// Server back to client -- Error on initial packet (handle already exists)
// No struct needed

// Flag == 4
// Client to all other clients via server -- Broadcast packet
typedef struct Broadcast {
   int8_t sender_len;
   int8_t *sender_handle;
   int8_t *message; // NULL terminated
} __attribute__ ((packed)) Broadcast;

// Flag == 5
// Client to another client via server -- Message packet
typedef struct Message {
   int8_t dest_len;
   int8_t *dest_handle;
   int8_t sender_len;
   int8_t *sender_handle;
   int8_t *message; // NULL terminated
} __attribute__ ((packed)) Message;

// Flag == 6
// No longer used

// Flag == 7
// Server to client -- Error packet (destination handle in a message packet
// does not exist)
typedef struct MessageError {
   int8_t dest_len;
   int8_t *dest_handle;
} __attribute__ ((packed)) MessageError;

// Flag == 8
// Client to server -- Client asking if it can exit
// No struct needed

// Flag == 9
// Server to client -- ACKing client's exit packet
// No struct needed

// Flag == 10
// Client to server -- Requesting list of handles
// No struct needed

// Flag == 11
// Server to client -- Number of handles stored on server (followed
// immediately by Flag == 12 packet)
typedef struct NumHandles {
   int32_t num_handles;
} __attribute__ ((packed)) NumHandles;

// Flag == 12
// Handles currently in use by server. One of these per handle
typedef struct Handle {
   int8_t handle_len;
   int8_t *handle;
} __attribute__ ((packed)) Handle;

/********************************* Functions *********************************/

// for the server side
int32_t tcpServerSetup();
int32_t tcpAccept(int32_t server_socket);

// for the client side
int32_t tcpClientSetup(char *host_name, char *port);

// for both
int32_t tcp_select(fd_set *socket_fds, int32_t max, struct timeval time);
void chat_header_setup(int8_t **send_buf, int8_t flag, int16_t packet_len);
void reset(int8_t *buf, int32_t message_len);
void create_buffer(int8_t **buf, int32_t size);
void send_data(int8_t **send_buf, int32_t socket, int16_t packet_len);
void process_flag(int8_t flag, int8_t *buf, Client *socket);
int8_t check_flag(int32_t message_len, int8_t *buf);
int8_t check_socket(int8_t *buf, Client *socket, int8_t client_or_server);

#endif
