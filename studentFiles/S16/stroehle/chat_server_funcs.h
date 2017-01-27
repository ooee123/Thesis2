




#ifndef _CHAT_SERVER_FUNCS_
#define _CHAT_SERVER_FUNCS_
#include "stdint.h"
#include <sys/types.h>
enum flags
{
   SEND_HANDLE = 1,
   GOOD_HANDLE = 2,
   BAD_HANDLE = 3,      
   BROADCAST = 4,
   PRIVATE_MESSAGE = 5,
   BAD_DEST_HANDLE = 7,
   CLIENT_EXITING = 8,
   EXIT_ACK = 9,
   PRINT_HANDLES = 10,
   NUM_HANDLES = 11,
   ALL_HANDLES = 12
};


struct client_info {
   char *handle;
   int port_number;
};

// for the server side
int tcp_recv_setup(int specified_port);

// for the client side
int tcp_send_setup(char *host_name, char *port);

void setup_new_client( int *max_fd);
void read_data_from_client(int fd_to_read);
void server_select();

void read_handle_packet(int fd_to_read);
void create_client_info(uint8_t handle_len, int socket, char *handle_strt);
void send_good_handle_packet(int c_socket);
void send_bad_handle_packet(int c_socket);
void send_packet(int socket, char *packet, int length);
#endif