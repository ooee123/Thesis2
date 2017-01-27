#include <stdint.h>

// flags
#define CLIENT_INIT 1
#define SERVER_GOOD 2
#define SERVER_BAD 3
#define BROADCAST 4
#define MESSAGE 5
#define MESSAGE_ERR 7
#define CLIENT_EXIT 8
#define SERVER_ACK 9
#define CLIENT_LIST 10
#define NUM_CLIENTS 11
#define LIST_CLIENTS 12

// limits 
#define MAX_MSG_LEN 1024
#define MAX_HANDLE_LEN 255

// exit codes
#define EXIT_SUCCESS 0
#define EXIT_FAIL -1

// misc
#define DECIMAL 10
#define BACK_LOG 5

struct NormalHeader {
   uint16_t packetLen;
   uint8_t flag;
} __attribute__ ((packed));

typedef struct Client {
   uint8_t handleLen;
   char* handle;
   int socket;
} Client;

// for the server side
int tcp_recv_setup();
void tcp_listen();

// for the client side
int tcp_send_setup(char *host_name, char *port);
