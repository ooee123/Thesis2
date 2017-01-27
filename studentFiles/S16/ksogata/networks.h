typedef struct node {
   char *handle;
   int sock_num;
   struct node *next;
} node_t;

uint32_t clients_num;

typedef struct {
   uint16_t len;
   uint8_t flag;
} header;

typedef struct {
   uint8_t handle_len;
   char *handle;
} init;

typedef struct {
   uint8_t to_len;
   char *handle_to;
   uint8_t from_len;
   char *handle_from;
   char *message;
} msg_data;

typedef struct {
   uint8_t from_len;
   char *handle_from;
   char *message;
} msg_broad;



// for the server side
int tcp_recv_setup();
int tcp_listen(int server_socket, int back_log);

// for the client side
int tcp_send_setup(char *host_name, char *port);
