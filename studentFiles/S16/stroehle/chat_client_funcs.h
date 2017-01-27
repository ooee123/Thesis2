

#ifndef _CHAT_CLIENT_FUNCS_
#define _CHAT_CLIENT_FUNCS_

#include "stdint.h"

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

// for the server side
int tcp_recv_setup();
int tcp_listen(int server_socket, int back_log);

// for the client side
int tcp_send_setup(char *host_name, char *port);
void send_packet(char *packet, int length);
void send_intial_packet();
void send_broadcast_packet(char *data);
void send_handle_request_packet();
void send_exit_ack_packet();
void get_packet();
void send_msg_packet(char *data, int dst_handle_len);
#endif