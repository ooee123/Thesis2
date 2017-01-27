//
//  tcp_client_helper.c
//  Chat_TCP
//
//  Created by Rebecca McKinley on 4/22/16.
//  Copyright © 2016 Rebecca McKinley. All rights reserved.
//

#include "tcp_client_helper.h"


//for flag = 9, for terminating
void Clean_Up(uint32_t socket_num, char *packet, uint32_t ex) {
   
   close(socket_num);
   free(packet);
   exit(ex);
}

//will check and end if server is terminated
void Check_Server_Status(uint32_t message_len, uint32_t socket_num, uint32_t flg, 
 char *packet) {
   
   if (!message_len) {
      if (flg == 1)
         printf("\nServer Terminated\n");
      
      Clean_Up(socket_num, packet, -1);
   }
} 

//r0 means use given length, 1 means look for 1 byte length
//will recieve a full packet, if expected len is 0, then find it from socket
uint16_t Smart_Recv(uint32_t socket_num, char *buf, uint32_t buffer_size,
 uint32_t flg, uint32_t len, char *packet) {
   
   ssize_t tot_recv = 0;
   uint16_t tot_len = len;
   
   if (flg) {
      tot_recv = recv(socket_num, buf, 1, 0);
      memcpy(&tot_len, buf, 1);
      tot_len++;
   }
   
   //get the rest of the message
   while (tot_recv < tot_len && tot_recv < buffer_size) {
      tot_recv += recv(socket_num, buf + tot_recv, tot_len - tot_recv, 0);
      Check_Server_Status((int)tot_recv, socket_num, 1, packet);
   }
   
   return tot_len;
}

//will extract the data from stdin given parameters
void Extract_Data(char *buffer, uint16_t *len, char end_char,
 uint32_t max_val) {   
   
   *len = 0;
   while (*len < max_val && (*(buffer + *len) = getchar()) != end_char) {
      (*len)++;
   }
}

//populates the fields of a B header
int Populate_B_Header(B_Header *b_head, char *handle) {
   char parse;
   
   //first 3 bytes of header, 1 1-byte length, and null termination
   b_head->tot_len = 5; 
   
   parse = getchar();
   
   if (parse != ' ' && parse != '\n') {
      return -1;
   }
   else if (parse == '\n') {
      ungetc('\n', stdin);
   }
   
   //update fields of the struct
   b_head->tot_len += strlen(handle);
   b_head->src_handle_len = strlen(handle);
   memcpy(b_head->src_handle, handle, strlen(handle));
   
   //for internal reference
   b_head->header_len = b_head->tot_len;
   
   //set up flags
   b_head->flag = BROADCAST;
   
   return 0; 
}

//does not update total length fully
//formats the header for an M command, returns -1 if error, zero otherwise
int Populate_M_Header(M_Header *m_head, char *handle) {
   char parse, dest_handle_buf[256];
   uint16_t len = 0;
   
   //first 3 bytes of header, 2 1-byte lengths, and null termination
   m_head->tot_len = 6; 
   
   if ((parse = getchar()) != ' ')
      return -1;
   
   //update fields of the struct
   m_head->tot_len += strlen(handle);
   memcpy(m_head->src_handle, handle, strlen(handle));
   m_head->src_handle_len = strlen(handle);
   
   //get the length and name of destination handle
   Extract_Data(dest_handle_buf, &len, ' ', 256);
   
   //update fields of the struct
   m_head->dest_handle_len = len;
   m_head->tot_len += m_head->dest_handle_len;
   memcpy(m_head->dest_handle, dest_handle_buf, m_head->dest_handle_len);
   
   //for internal reference
   m_head->header_len = m_head->tot_len;
   
   //set up pointer for packet
   m_head->tot_len = htons(m_head->tot_len);
   
   //set up flags
   m_head->flag = 5;
   
   return 0;   
}

//will block and process a response for the handle
void Init_Response(fd_set *read_fds, uint32_t socket_num, char *handle, char *packet) {
   char buf[HEADER_LEN];
   uint16_t message_len = 0;
   
   //block until you recieve something from the server
   message_len = Smart_Recv(socket_num, buf, HEADER_LEN, 0, HEADER_LEN, packet);
   
   if (buf[2] == FAIL_INIT) {
      printf("Handle already in use: %s\n", handle);
      Clean_Up(socket_num, packet, -1);
   }
   Check_Server_Status(message_len, socket_num, 0, packet);
}

//assembles the packet for the initial state
void Init_State(fd_set *read_fds, char *handle, uint32_t socket_num, char *packet) {
   uint32_t size = (uint32_t)strlen(handle) + 4;
   
   size = htons(size);
   
   memcpy(packet, &size, 2);
   memset(packet + 2, INIT, 1);
   
   size = ntohs(size) - 4;
   memcpy(packet + HEADER_LEN, &size, 1);
   memcpy(packet + 4, handle, size);
   
   Safe_Send(packet, (uint32_t)strlen(handle) + 4, socket_num);
   
   Init_Response(read_fds, socket_num, handle, packet);
}

//will zero and set the fds
void Set_Fds(fd_set *read_fds, uint32_t socket_num) {
   
   //clear select fields
   FD_ZERO(read_fds);
   
   //add file descriptors
   FD_SET(0, read_fds);
   FD_SET(socket_num, read_fds);
}

//sets up fds reader and socket
void Set_Up(char *handle, char *server, char *port, fd_set *read_fds,
 uint32_t *socket_num, char **packet) {

   if (strlen(handle) > 256) {
      printf("Handle name exceeds max\n");
      exit(-1);
   }
   
   *socket_num = tcp_send_setup(server, port);
   
   Set_Fds(read_fds, *socket_num);
   
   //set up global packet    
   *packet = calloc(2000, sizeof(char));
   
   //set up initial handshake
   Init_State(read_fds, handle, *socket_num, *packet);
}

//prints out a char of given length
void Print_Char(char *buf, uint32_t len, uint32_t new_line) {
   int index = 0;
   
   //print out the client handle
   while (index < len) {
      printf("%c", *(buf + index++));
   }
   
   if (new_line) {
      printf("\n");
   }
}

void Finish_Get() {
   char c;
   
   while ((c = getchar()) != '\n');
}

