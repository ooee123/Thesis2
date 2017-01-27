#define BUFFER_LEN 8192
#define DEF_CLIENTS 4
#define MAX_NAME_SIZE 256

//statuses
#define OFFLINE 0
#define JOINING 1
#define ONLINE 2

//indices
#define LENGTH_IDX 0
#define FLAG_IDX 2
#define DEST_LEN_IDX 3
#define DEST_HANDLE_IDX 4

//flags
#define JOIN 1
#define ACCEPTED 2
#define REJECTED 3
#define BROADCAST 4
#define MESSAGE 5

#define INVALID_NAME 7
#define QUIT 8
#define KICKED 9
#define GET_HANDLES 10
#define NUM_HANDLES 11
#define HANDLES 12


typedef struct client {
   unsigned int socket;
   unsigned char status;
   char* handle;
} client;

// for the server side
int tcp_recv_setup();
int tcp_listen(int server_socket, int back_log);

// for the client side
int tcp_send_setup(char *host_name, char *port);

int formatMessage(char* src, char* dest, char* message, uint16_t length, int flag, char* buffer);
int receiveMessage(int socket, char* src, char* dest, char* message, int* flag,
      int handles);
