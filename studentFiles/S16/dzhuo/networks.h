#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define MAX_HNDL_LEN 255
#define MAX_MSG_LEN 1000
#define M_MIN_LEN 5   // pkt_len + flag + src byte + src handle len + dst byte
#define B_MIN_LEN 4   // pkt_len + flag + src byte + src handle len

// You have no idea how long it took for me to name these... too much ocd
#define FLAG_C_CONNECT 1
#define FLAG_S_CONN_OK 2
#define FLAG_S_CONN_NO 3
#define FLAG_BROADCAST 4
#define FLAG_C_MESSAGE 5
#define FLAG_S_DST_DNE 7
#define FLAG_C_EXITING 8
#define FLAG_S_EXIT_OK 9
#define FLAG_WANT_HNDL 10
#define FLAG_AMNT_HNDL 11
#define FLAG_LIST_HNDL 12

#define PKT_BUF_SIZE 1515

#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#define MAX(A, B) (((A) > (B)) ? (A) : (B))

typedef struct client_data {
    int socket;         // socket descriptor
    int comm_size;      // user input buffer size
    char *comm_buf;     // user input buffer
    char *send_buf;     // packet output buffer
    fd_set *set;
    int num_handles;    // number of handles to be received
    char *handle;       // username w/ null
    char handle_len;
} client_data;

typedef struct entry {
    int socket;         // which socket is this client connected to?
    char *handle;       // username w/o null
    char handle_len;    // length of handle
} entry;

typedef struct server_data {
    entry *dir;     // connected clients
    uint8_t* bit_array; // bit array of those logged in
    int capacity;       // resizable capacity
    uint32_t count;          // current # of clients logged in
    fd_set *set;
    int srv_fd;
} server_data;

// for the server side
void init_server(int argc, char** argv, server_data *sd);
void tcp_recv_setup(server_data* sd);
void tcp_port_setup(server_data* sd, uint16_t port);
void tcp_listen(int server_socket, int back_log);
int select_input(server_data *sd);
void refresh_set(server_data *sd);
void add_client(server_data* sd);
int tcp_accept(int server_socket);
void traffic_control(int src_socket, server_data *sd);
void update_entry(int src_socket, server_data *sd, char* buf, uint16_t pkt_len);
void broadcast(int src_socket, server_data *sd, char* buf, uint16_t pkt_len);
void message(int src_socket, server_data *sd, char* buf, uint16_t pkt_len);
void parse_handle(char* buf, int offset, char* handle_len, char** handle);
void find_socket(server_data *sd, char handle_len, char* handle, int* socket, int* index);
void allow_exit(int src_socket, server_data *sd);
void handle_stream(int src_socket, server_data *sd);
void handle_length_packet(int src_socket, server_data *sd);
void cleanup_sockets(server_data *sd);

// for the client side
void mail_sorter(client_data *cd);
void init_client(int argc, char** argv, client_data* cd);
void login(client_data* cd);
void parse_command(client_data* cd);
int tcp_send_setup(char *host_name, char *port);
void create_m_packet(client_data* cd);
void create_b_packet(client_data* cd);
void execute(client_data* cd);
void print_message(char* buf);
int check_inbox(client_data *cd);
void upload_info(client_data* cd);
void print_bad_handle(char* buf);
void get_num_handles(client_data* cd);
void get_list_handles(client_data* cd);
void prepend_handles(client_data *cd, int* offset, char* dst_handle, int dst_handle_len);

// common functions
int send_data(int socket, char* buffer, int len);
int recv_data(int socket, char* buffer, int len);
char* prep_base_packet(int len, int* packet_offset, char flag);

// helpers
void server_print_packets(char* buf, uint16_t pkt_len);

// server bitarray helper functions
int get_empty_entry(server_data* sd);
void mark_entry(int index, server_data *sd);
void unmark_entry(int index, server_data *sd);
int next_entry(int index, server_data *sd);
int expand_directory(server_data *sd);
