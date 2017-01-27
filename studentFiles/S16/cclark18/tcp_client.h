/******************************************************************************
 * tcp_client.h
 *
 * Chris Clark
 * CPE 464
 * Program 2
 *****************************************************************************/

#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "chat_util.h"

// Holds the data for the input parameters to the client program.
// Just to make parse_input function only have to return one value.
struct input {
	char handle[MAX_HANDLE_SIZE + 1];
	char *server_name;
	int server_port;
};

// States for handle reading state machine
#define STATE_HANDLE_START 0
#define STATE_HANDLE_GET_DATA 1

// Struct representing a state machine to be used for parsing a single handle.
struct handle_state {
  int state;
  int len_read;
  int msg_len;
  uint8_t data[MAX_HANDLE_SIZE + 1];
};

// Reads command line input, handles bad formatting, and returns parsed data.
struct input read_input(int argc, char **argv);

// Initializes socket attached to server on hostname/port.
int setup_server_socket(char *host_name, int port);

// Sends initial connection request to server with client handle.
void send_init_message(int socket, char *handle);

// Gets initial connection response from server, exiting if the client handle
// already exists.
void handle_init_response(int socket, char *handle);

// Loops to listen for server responses/user input.
void client_listen(int socket, char *handle);

// Parses user input from stdin.
// Returns 1 if the client requests an exit, 0 otherwise.
int handle_user_input(int socket, char *handle);

// Sends a message address to another client to the server.
void send_user_message(int socket, char *handle, string dest_handle, string message);

// Sends a broadcast message to the server.
void broadcast_message(int socket, char *handle, string message);

// Parses the number of handles from a server packet acknowledging a client handle
// list request.
uint32_t parse_num_handles(char *data);

// Reads a stream of 'num_handles' handles from the server. Responsible for reading, parsing,
// and outputting the handles.
// To be called after reading the handle lsit packet header.
void process_handles(int socket, uint32_t num_handles);

// Functional implementation of state machine for reading in a single handle from a socket.
int read_handle_state(int socket, struct handle_state *state);

// Outputs a handle to the user.
void output_handle(char *handle);

// Outputs a message directed to the client
void output_direct_message(struct direct_message *dm);

// Outputs a response from the server indicating a bad client handle in a direct message.
void output_bad_message(char *data);

// Outputs the input prompt for the user.
void write_input_prompt();

#endif