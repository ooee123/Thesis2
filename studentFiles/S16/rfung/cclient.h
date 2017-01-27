#ifndef CCLIENT_H
#define CCLIENT_H

#include "chat.h"
#define NUM_CMD_ARGS 4
#define MAX_MESSAGE_LEN 999

void init_client(int argc, char **argv);
void send_handle();
uint16_t retrieve_data(uint16_t buff_size, char *buffer, uint16_t recv_tot);

void run_client();
void set_fds();
void read_stdin();
void read_server_msg();

void message_client();
void broadcast_msg();
void append_and_send_msg(char *buff, char *msg_offset, int32_t dest_len);
void req_client_list();
void req_exit_chat();

void read_chat_msg(char *buf, uint32_t data_len);
void read_broadcast_msg(char *buf, uint32_t data_len);
void print_bad_dest(char *buf, uint32_t data_len);
void print_clients_list(char *buf);

#endif