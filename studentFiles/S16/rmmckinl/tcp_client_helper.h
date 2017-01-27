//
//  tcp_client_helper.h
//  Chat_TCP
//
//  Created by Rebecca McKinley on 4/22/16.
//  Copyright © 2016 Rebecca McKinley. All rights reserved.
//

#ifndef tcp_client_helper_h
#define tcp_client_helper_h

#include "networks.h"

typedef struct B_Header {
   unsigned short header_len;
   uint16_t tot_len;
   unsigned char flag;
   char src_handle_len;
   unsigned char src_handle[256];
}__attribute__((packed)) B_Header;

typedef struct M_Header {
   unsigned short header_len;
   uint16_t tot_len;
   unsigned char flag;
   unsigned char dest_handle_len;
   char dest_handle[256];
   unsigned char src_handle_len;
   char src_handle[256];
}__attribute__((packed)) M_Header;


void Clean_Up(uint32_t socket_num, char *packet, uint32_t ex);
void Check_Server_Status(uint32_t message_len, uint32_t socket_num, uint32_t flg, 
 char *packet);
uint16_t Smart_Recv(uint32_t socket_num, char *buf, uint32_t buffer_size,
 uint32_t flg, uint32_t len, char *packet);
void Extract_Data(char *buffer, uint16_t *len, char end_char,
 uint32_t max_val);
int Populate_B_Header(B_Header *b_head, char *handle);
int Populate_M_Header(M_Header *m_head, char *handle);
void Init_Response(fd_set *read_fds, uint32_t socket_num, char *handle, char *packet);
void Init_State(fd_set *read_fds, char *handle, uint32_t socket_num, char *packet);
void Set_Fds(fd_set *read_fds, uint32_t socket_num);
void Set_Up(char *handle, char *server, char *port, fd_set *read_fds,
 uint32_t *socket_num, char **packet);
void Print_Char(char *buf, uint32_t len, uint32_t new_line);
void Finish_Get();

#endif /* tcp_client_helper_h */


