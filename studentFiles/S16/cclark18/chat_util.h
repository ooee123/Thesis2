/******************************************************************************
 * chat_util.h
 *
 * Chris Clark
 * CPE 464
 * Program 2
 *****************************************************************************/

#ifndef __CHAT_UTIL_H__
#define __CHAT_UTIL_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>

#include <list>
#include <iostream>

using namespace std;

// Size constraints
#define MAX_MESSAGE_SIZE 1000
#define MAX_HANDLE_SIZE 255

// Message flags
#define FLAG_CLIENT_INIT 1
#define FLAG_SRVR_GOOD_INIT 2
#define FLAG_SRVR_BAD_INIT 3
#define FLAG_BROADCAST 4
#define FLAG_CLIENT_MESSAGE 5
#define FLAG_SRVR_BAD_MESSAGE 7
#define FLAG_CLIENT_EXIT 8
#define FLAG_SRVR_ACK_EXIT 9
#define FLAG_CLIENT_REQ_HANDLES 10
#define FLAG_SRVR_ACK_HANDLES 11
#define FLAG_SRVR_SEND_HANDLES 12

// Non-packed struct to be used for convenience when reading/writing message headers.
struct message_header {
	uint16_t msg_len;
	uint8_t flag;
} __attribute__((packed));

// message packet can consist of at most one header, two handles, and a message
// handle size should include handle size byte
// this does not apply to handle packet
#define MAX_PACKET_SIZE sizeof(struct message_header) + 2 * (MAX_HANDLE_SIZE + 1) + MAX_MESSAGE_SIZE

// States for packet reading reading state machine
#define STATE_START 0
#define STATE_PROC_LEN 1
#define STATE_GET_DATA 2

// Struct representing a state machine to be used for parsing a basic message.
struct packet_read_state {
	int state;
	int len_read;
	int msg_len;
	char data[MAX_PACKET_SIZE];
};

// Struct representing the logical contents of a message between two clients.
struct direct_message {
  char src_handle[MAX_HANDLE_SIZE + 1];
  char dest_handle[MAX_HANDLE_SIZE + 1];
  char message[MAX_MESSAGE_SIZE];
};

// Struct representing the logical contents of a broadcast message.
struct broadcast_message {
  char src_handle[MAX_HANDLE_SIZE + 1];
  char message[MAX_MESSAGE_SIZE];
};

// Initializes a message reading state machine.
void init_packet_read_state(struct packet_read_state *state);

// Functional implementation of state machine for handling normal message packet (the list of client
// handles is a special case, to be dealt with separately).
// Data received will be written into state buffer, and will persist until the state machine
// is reinitialized for another packet.
// Returns -1 if 0 is returned by recv, indicating socket closure on remote end.
int read_packet(int socket_fd, struct packet_read_state *state);

// Reads the packet header, including message length and the flag. Leaves the message length
// in network order.
struct message_header read_header(char *data);

// Reads handle from 'data' (which points to handle length) into 'dest' (adding null terminating character).
// Returns handle length.
int read_handle(char *dest, char *data);

// Reads packet representing direct message between clients into struct with null
// terminated strings.
// 'data' should point to the start of the packet.
void parse_direct_message(struct direct_message *dm, char *data);

// Reads packet representing broadcast message into struct with null
// terminated strings.
// 'data' should point to the start of the packet.
void parse_broadcast_message(struct broadcast_message *bm, char *data);

// Writes a message header onto 'data', returning the number of bytes written.
int write_header(char *data, int msg_len, int flag);

// Writes a handle onto 'data' (consisting of handle length and non-padded, non-null terminated
// handle contents), returning the number of bytes written.
int write_handle(char *data, char *handle);

// Sends a packet to a socket, consisting of only a flag and a message length header.
void send_flag(int socket, int flag);

// Takes a struct will null terminated message information, and sends a packet formatted as a
// direct message between clients.
void send_direct_message(int socket, struct direct_message dm);

// Takes a struct will null terminated message information, and sends a packet formatted as a
// broadcast message.
void send_broadcast_message(int socket, struct broadcast_message bm);

// Resets a struct timeval to have a value of 1 second, 0 microseconds.
// Convenience function to help with select() calls.
void reset_timer(struct timeval *timer);

// Resets an fd_set according to a list of sockets to watch.
// Convenience function to help with select() calls.
void reset_sockets(list<int> socket_fds, fd_set *sockets);

// Gets the highest file descriptor from an unsorted list.
// Convenience function to help with select() calls.
int get_highest_fd(list<int> socket_fds);

// Safe system calls (exit with message if return value < 0)
int safe_socket(int domain, int type, int protocol);
int safe_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int safe_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int safe_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
ssize_t safe_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t safe_recv(int sockfd, void *buf, size_t len, int flags);
int safe_listen(int sockfd, int backlog);
int safe_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
struct hostent* safe_gethostbyname(const char *name);
int safe_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

// Prints out a block of bytes as space separated hexadecimal values. Used for debugging.
void print_bytes(const void *data, int len);

#endif