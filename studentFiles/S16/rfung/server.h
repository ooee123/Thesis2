#ifndef SERVER_H
#define SERVER_H

#include <sys/select.h>
#include "chat.h"

#define LISTEN_BACKLOG 5
#define NUM_INITIAL_MAX_CLIENTS 50

typedef struct client_info {
    int32_t sock_num;
    char handle[MAX_HANDLE_LEN + 1];
} client_info; 

void init_server(int argc, char **argv);
void monitor_server();
void set_fds();
void service_client(client_info *clients, uint32_t client_num);
void remove_client(uint32_t client_num);

void add_handle(uint32_t client_num, char *buf);
void add_new_client ();

uint16_t retrieve_data(uint32_t sock_num, uint16_t buff_size, char *buffer, uint16_t recv_tot);
void redirect_msg(uint32_t client_num, char * buf, uint32_t data_len);
void client_req_exit(uint32_t client_num);
void send_broadcast(uint32_t client_num, char *buf, uint32_t data_len);
void send_client_list(uint32_t client_num);


#endif