/******************************************************************************
 * tcp_server.h
 *
 * Chris Clark
 * CPE 464
 * Program 2
 *****************************************************************************/

#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <sys/socket.h>

#include <list>

#include "chat_util.h"

// Limits the amount of pending clients that can be on the backlog
// of the main socket at one time.
#define CLIENT_BACKLOG 10

using namespace std;

// Class containing data about a single client
class Client {
public:
	char handle[MAX_HANDLE_SIZE + 1];
	int socket;
	struct packet_read_state state;
	int accepted;
};

// Handles command line parameters.
// Returns the requested port number, or 0 if none is specified
int read_input(int argc, char **argv);

// Creates and binds a socket for listening on the port.
// Returns the socket file descriptor.
int get_server_socket(int port_number);

// Handles all server listening
void server_listen(int server_socket);

// Handles a pending client, establishing a connection and added them to
// the list of current clients. Returns the new socket for that client.
int handle_pending_client(int server_socket, list<Client> &clients);

// Accepts a connection from a potential client and returns a new
// socket for that client.
int listen_for_new_client(int server_socket);

// Handles the data sent from an existing client connection.
// Returns 0 to indicate that the current client should be removed,
// 0 otherwise.
int process_client_packet(Client& client, list<Client> clients);

// Handles a direct message sent from a client to another
void handle_message(Client client, list<Client>& clients);

// Handles a broadcast message sent from a client
void handle_broadcast(Client client, list<Client>& clients);

// Acknowledges a handle list request with the number of handles to be sent
void send_num_handles(int socket, uint32_t num_handles);

// Sends all client handles
void send_handles(int socket, list<Client> clients);

// Sends a single client handle
void send_handle(int socket, char *handle);

// Returns 1 if an active client with that handle already exists, and 0 otherwise
int client_name_exists(char *handle, list<Client>& clients);

// Gets the socket for a client by their handle
int get_client_socket_by_handle(list<Client>& clients, char *handle);

#endif