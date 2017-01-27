
#define HEADER_LEN 7
#define MAX_MSG_LEN 32000
#define MAX_BACKLOG 128
#define MAX_HANDLE_LEN 255
#define DUPLICATE_HANDLE_MSG_LEN 23

#define SETUP 1
#define HANDLE_CONF 2
#define HANDLE_DENY 3
#define BROADCAST_MSG 4
#define PRIVATE_MSG 5
#define HANDLE_FOUND 6
#define HANDLE_NOT_FOUND 7
#define CLIENT_REQ_EXIT 8
#define SERVER_ACK_EXIT 9
#define CLIENT_REQ_HANDLE 10
#define SERVER_NUM_HANDLE 11
#define SERVER_HANDLE_MSG 12

// for the server side
int tcp_recv_setup(uint16_t port);
void tcp_listen(int server_socket, int back_log);

// for the client side
int tcp_send_setup(char *host_name, char *port);

void my_send(uint dest_socket, char *buf, uint buf_size);

typedef struct packet_header {
   uint32_t seq_num;
   uint16_t pkt_len;
   uint8_t flag;
} __attribute__((packed)) packet_header;


