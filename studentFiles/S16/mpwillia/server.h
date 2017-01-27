
#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include "client_handler.h"

#define LISTEN_BACKLOG 5

//int parse_requested_port(int argc, char** argv);
int setup(char* port_arg);
void run(int server_socket);
void handle_packet(uint8_t* packet, int client_socket, struct connections* cons);


int getReady(const fd_set* check, struct connections* cons);
int accept_connection(int server_socket, struct connections* cons);
void client_exit(int client_socket, int flags, fd_set* master, struct connections* cons);
void client_connect(const uint8_t* handle_data, int client_socket, struct connections* cons);
int client_message(uint8_t* packet, int client_socket, struct connections* cons);
void client_broadcast(uint8_t* packet, int client_socket, struct connections* cons);

int check_dest(int client_socket, char* dst, struct connections* cons);

void send_client_list(int client_socket, struct connections* cons);

#endif

