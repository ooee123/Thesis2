//packet decs
#include <vector>
#include <iostream>
#include "testing.h"

using namespace std;


//enum for packet recieving states
enum {START, PROCESS_LENGTH , GET_DATA, HANDLE_LENGTH, HANDLE_LIST};


int string_compare(uint8_t *s1, uint8_t *s2);

int string_length(uint8_t *s);


//server
int check_flag(uint8_t *packet);

int tcp_recv_setup(int argc, char* port);

void tcp_listen(int server_socket, int back_log);

void check_init_connections(int server_socket, fd_set fdvar, vector<int> *client_sock);

void check_data_connections(fd_set fdvar, vector<int> *client_sock, vector<uint8_t *> *handle_list);

void select_setup(int server_socket,vector<int> *client_socket, fd_set* fdvar);

void send_okay_exit(int client_socket);

void send_bad_handle(int client_socket);

void send_okay_handle(int client_socket);

void send_bad_message(int client_socket, uint8_t *handle, uint8_t handle_length );

void send_good_message(uint8_t *buf, int dup_pos,  vector<int> *client_socket_list);

void send_broadcast_packet(uint8_t *buf, int src_socket,  vector<int> *client_socket_list);

void send_num_handle(int socket, vector<uint8_t *> *handle_list );

void send_handle_list(int socket, vector<uint8_t *> *handle_list);

void print_list(vector<int> *client_socket_list);

int max_list(vector<int> *client_socket_list);

void cleanup(vector<int> *client_socket_list, vector<uint8_t *> *handle_list);

int handle_dup_check(uint8_t *handle, vector<uint8_t *> *handle_list  );

// for the client side
int tcp_send_setup(char *host_name, char *port);

void init_handle(int socket, char *handle);

void client_listen(int socket);

void client_get_packet(int socket);

void client_process_packet(uint8_t *buf, int socket);

void client_parse_input (string s, int socket);

void client_send_exit(int socket);

void client_create_message(int socket,vector <string> tokens);

void client_create_broadcast(int socket, vector<string> tokens);

void client_send_message_packet(int socket, int length,  string handle, uint8_t *msg);

void client_send_broadcast(int socket, int msg_length, uint8_t *msg);

void print_bad_msg_handle(uint8_t *buf);

void print_msg_packet(uint8_t *buf);

void print_broadcast_packet(uint8_t *buf);

void print_handle_list( vector<uint8_t *> *handle_list);

void client_request_list(int socket);




//packet handling server
void get_packet(int socket, vector<int> *client_socket_list,vector<uint8_t *> *handle_list);

void process_packet(uint8_t *buf, int client_socket, vector<int> *client_socket_list, vector<uint8_t *> *handle_list);

void process_handle_packet(uint8_t *buf,int client_socket,   vector<uint8_t *> *handle_list);

void process_message_packet(uint8_t *buf, int client_socket, vector<int> *client_socket_list, vector<uint8_t *> *handle_list );
