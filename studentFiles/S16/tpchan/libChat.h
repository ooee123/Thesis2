#define MAX_HANDLE_LEN 256 //255 chars + 1 for null (for simplicity sake)
#define HEADER_SIZE 3

#define CS_INIT 1
#define SC_INIT 2
#define SC_INIT_ERR 3
#define BROADCAST 4
#define MESSAGE 5
#define SC_DEST_ERR 7
#define CS_EXIT 8
#define SC_EXIT 9
#define CS_HANDLES 10
#define SC_HANDLES 11
#define SC_HANDLES_STREAM 12

struct client {
   char handle[MAX_HANDLE_LEN];
   uint8_t handle_len;
   int socket;
   struct client *next;
} client;

struct myHeader {
   uint16_t length;
   uint8_t flag;
} myHeader;

int recv_e(int socket, void *buffer, size_t length, int flags);
void send_e(int client_socket, const void *buf, size_t len, int flags);
char *createHeader(uint16_t length, uint8_t flag);
char *getPacket(int socketNum, int packLength);
struct myHeader *tcp_recieve_header(int socketNum);
