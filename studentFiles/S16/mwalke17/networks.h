#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "testing.h"

#define STARTING_COUNT 4
#define NORMAL_HEADER_SIZE 3
#define STD_IN 0
#define STD_OUT 1
#define SERVER_FD 3
#define HANDLE_SIZE 256
#define BUFFER_MAX_SIZE 1515
#define MESSAGE_SIZE 1000
#define FIELD_LENGTH 2
#define LIST_RESPONSE_LENGTH 7
#define FLAG_SUCCESS 2
#define FLAG_FAILURE 3
#define FLAG_HANDLE_REQUEST 1
#define FLAG_HANDLE_SUCCESS 2
#define FLAG_HANDLE_FAILURE 3
#define FLAG_BROADCAST 4
#define FLAG_MESSAGE 5
#define FLAG_BAD_CLIENT 7
#define FLAG_EXIT_REQUEST 8
#define FLAG_EXIT_RESPONSE 9
#define FLAG_HANDLE_LIST_REQUEST 10
#define FLAG_HANDLE_LIST_RESPONSE_ONE 11
#define FLAG_HANDLE_LIST_RESPONSE_TWO 12
#define BUF_SIZE 1000
#define TRUE 1
#define FALSE 0
#define INITIAL_LIST_SIZE 301

struct packet_t {
   short total_len;
   char flag; 
   char dest_len;
   char *dest_handle;
   char src_len;
   char *src_handle;
   short data_len;
   char *data;
};

struct handles_t {
   char **list;
   int count;
   int largest_fd;
   int max;
};

//Server side
void serve(int server_socket, fd_set* client_fds);
void handle_flag(struct packet_t* packet, struct handles_t* h_list, char* buf, int curr_client);
void build_list_response(int socket_num, struct handles_t* h_list);
int tcp_recv_setup();
int tcp_listen(int server_socket, int back_log);
void parse_flag(char* buf, struct packet_t* packet);
void parse_message(char *buf, struct packet_t* packet);
int add_client(int client_socket, struct handles_t* h_list, char* handle);
void parse_handle(char *buf, struct packet_t* packet);
char *build_handle_response(int code);
int check_for_handle(struct handles_t* h_list, char* handle);
int find_client(struct handles_t* h_list, char* handle);
void print_client_list(struct handles_t* h_list);
void remove_client(struct handles_t* h_list, int fd);
char *build_handle_response(int code);
void init_handle_list(struct handles_t* h_list, int socket_num);
char *build_exit_response();
void send_message_failure(int socket_num, struct packet_t* packet);

//Client side
int chat(int socket_num, fd_set* select_fds, char* handle);
void get_input(char** buf, char** stream, char* handle, int socket_num);
int parse_list_response_one(char* buf);
void parse_list_response_two(int socket_num, int list_length);
int tcp_send_setup(char *host_name, char *port);
int parse_buffer(int socket_num, char** buffer, char* src, char** stream); 
void build_message_packet(char* src_handle, char* dest_handle, char flag, char* data, char** packet); 
char *build_handle_request(char* handle);
int request_handle(int socket, char* handle);
void print_message(struct packet_t* packet);
void build_broadcast_packet(char* src_handle, char flag, char* data, char** stream);
void build_list_packet(char flag, char** stream);
void build_exit_packet(char flag, char** packet);
void send_message_failure(int socket_num, struct packet_t* packet);
void parse_message_failure(struct packet_t* packet);

//Both
int connect_and_fd_setup(char** argv, fd_set* set);
void send_pack(int socket_num, char* packet);
int recv_pack(int socket_num, char** recv_buf);
void init_packet(struct packet_t* packet);
void free_packet(struct packet_t* packet); 
void refresh_packet(struct packet_t* packet);
void print_struct(struct packet_t* packet);
void print_buffer(char* buf, int len);
void print_data(char* buf, int len);
void parse_broadcast(char *buf, struct packet_t* packet);
void close_socket(int socket_num);
