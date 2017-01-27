
#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>

int setup(char* hostname, char* port);
void request_handle(int server_socket, char* handle);
void start_interactive(int server_socket, char* handle);
void parse_command(char* command, int server_socket, char* handle);


// sends a message
// if a dst is specificed sends directly to dst
// if dst is NULL broadcasts
void send_message(int server_socket, char* dst, char* src, char* message);


int handle_packet(uint8_t* packet);

// handles the two phase request for a list of connected clients
void handle_client_list(uint8_t* packet);

#endif

