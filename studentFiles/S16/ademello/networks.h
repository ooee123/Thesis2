#include <sys/types.h>

#define FLAG_INITIAL 1
#define FLAG_GOOD 2
#define FLAG_BAD 3
#define FLAG_BROADCAST 4
#define FLAG_MSG 5
#define FLAG_MSG_ERROR 7
#define FLAG_SERVER_EXIT 8
#define FLAG_CLIENT_CLOSE 9
#define FLAG_REQ_CLIENT_LIST 10
#define FLAG_CLIENT_COUNT 11
#define FLAG_CLIENT LIST 12

typedef struct user {
   char* handle;
   int socket;
}user;

// for the server side
int tcp_recv_setup();
int tcp_listen(int back_log);

// for the client side
int tcp_send_setup(char *host_name, char *port);
