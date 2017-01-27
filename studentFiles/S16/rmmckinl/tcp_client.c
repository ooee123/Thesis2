/******************************************************************************
 * tcp_client.c
 *
 *****************************************************************************/

#include "tcp_client_helper.h"

static uint16_t is_end = 0;
static uint32_t num_handles = 0;

//puts together packet for broadcast
void Assemble_B_Packet(B_Header *b_head, char *is_more, char *packet) {
   char data_buf[MAX_MESSAGE], next = 0;
   uint16_t data_len = 0, pkg_len = 0;
   
   //get the length and data of the message to send
   Extract_Data(data_buf, &data_len, '\n', MAX_MESSAGE);
   
   //check if there is more data on the line
   if (data_len == MAX_MESSAGE) {
      if ((next = getchar()) != '\n') {
         *is_more = 1;
         ungetc(next, stdin);
      }
   }
   
   //update total length with payload length
   b_head->tot_len = b_head->header_len + data_len;
   
   //assemble entire packet
   pkg_len = htons(b_head->tot_len);  
   
   //header
   memcpy(packet, &pkg_len, 2);
   memcpy(packet + 2, &(b_head->flag), 1);
   
   //src info
   memcpy(packet + HEADER_LEN, &(b_head->src_handle_len), 1);
   memcpy(packet + 4, b_head->src_handle, b_head->src_handle_len);
   
   //message
   memcpy(packet + 4 + b_head->src_handle_len, data_buf, data_len);
   memset(packet + b_head->tot_len - 1, '\0', 1); 
}

//will compile the M packet with message
void Assemble_M_Packet(M_Header *m_head, char *is_more, char *packet) {
   char data_buf[MAX_MESSAGE], next;
   uint16_t data_len = 0, pkg_len = 0;
   
   //get the length and data of the message to send
   Extract_Data(data_buf, &data_len, '\n',MAX_MESSAGE);
   
   //check if there is more data on the line
   if (data_len == MAX_MESSAGE) {
      if ((next = getchar()) != '\n') {
         *is_more = 1;
         ungetc(next, stdin);
      }
   }
   
   //update total length with payload length
   m_head->tot_len = m_head->header_len + data_len;
   
   //assemble entire packet
   pkg_len = htons(m_head->tot_len);  
   
   //header
   memcpy(packet, &pkg_len, 2);
   memcpy(packet + 2, &(m_head->flag), 1);
   
   //dest info
   memcpy(packet + HEADER_LEN, &(m_head->dest_handle_len), 1);
   memcpy(packet + 4, m_head->dest_handle, m_head->dest_handle_len);
   
   //src info
   memcpy(packet + 4 + m_head->dest_handle_len, &(m_head->src_handle_len), 1);
   memcpy(packet + 5 + m_head->dest_handle_len, m_head->src_handle, 
    m_head->src_handle_len);
   
   //message
   memcpy(packet + 5 + m_head->dest_handle_len + m_head->src_handle_len, 
          data_buf, data_len);
   memset(packet + m_head->tot_len - 1, '\0', 1);
}

//will assemble the E packet
void Assemble_E_Packet(uint32_t socket_num, char *packet) {
   uint16_t tot_len = HEADER_LEN;
   
   //get the new line out of the stdin
   getchar();
   
   tot_len = htons(tot_len);
   
   memcpy(packet, &tot_len, 2);
   memset(packet + 2, EXITING, 1);
   
   is_end = 1;
   Safe_Send(packet, HEADER_LEN, socket_num);
}

//will assemble and send an L packet request
void Assemble_L_Packet(uint32_t socket_num, char *packet) {
   uint16_t tot_len = HEADER_LEN;
   
   //get the new line out of the way
   getchar();
   
   tot_len = htons(tot_len);
   
   memcpy(packet, &tot_len, 2);
   memset(packet + 2, LIST, 1);
   
   Safe_Send(packet, HEADER_LEN, socket_num);
}

//if the user types a B command
void B_Command(B_Header b, char *handle, uint32_t socket_num, char *packet) {
   char is_more = 0;
   
   if (Populate_B_Header(&b, handle) < 0) {
      printf("Invalid command\n");
   }
   else {
      Assemble_B_Packet(&b, &is_more, packet);
      
      //send the packet safely on its way
      Safe_Send(packet, b.tot_len, socket_num);
      
      while (is_more) {
         is_more = 0;
         Assemble_B_Packet(&b, &is_more, packet);
         Safe_Send(packet, b.tot_len, socket_num);
      }
   }
}

//if the user types an M command
void M_Command(M_Header m, char *handle, uint32_t socket_num, char *packet) {
   char is_more = 0;
   
   if (Populate_M_Header(&m, handle) < 0) {
      printf("Invalid command\n");
   }
   else {
      //assemble the full M packet
      Assemble_M_Packet(&m, &is_more, packet);
      
      //send the packet on its way
      Safe_Send(packet, m.tot_len, socket_num);
      
      while (is_more) {
         is_more = 0;
         Assemble_M_Packet(&m, &is_more, packet);
         Safe_Send(packet, m.tot_len, socket_num);
      }
   }
}

//for when you get to read from stdin
void Read_Stdin(char *handle, uint32_t socket_num, char *packet) {
   char command[2];
   M_Header m;
   B_Header b;
   
   //scan in user input
   command[0] = getchar();
   
   if (command[0] != '%') {
      printf("Invalid command\n");
      if (command[0] != '\n')
         Finish_Get();
   }
   else {
      command[1] = toupper(getchar());
      
      if (command[1] == 'M') {
         M_Command(m, handle, socket_num, packet);
      }
      else if (command[1] == 'B') {
         B_Command(b, handle, socket_num, packet);   
      }
      else if (command[1] == 'L') {
         Assemble_L_Packet(socket_num, packet);
      }
      else if (command[1] == 'E') {
         Assemble_E_Packet(socket_num, packet);
      }
      else {
         printf("Invalid command\n");
         Finish_Get();
      }
   }
}

//for flag = 12, print out all the handles
void Read_Handles(uint32_t socket_num, char *packet) {
   char buf[257];
   uint32_t buf_size = 257, handles_recv = 0;
   
   printf("\nNumber of clients: %d\n", num_handles);
   
   while (handles_recv < num_handles) {
      
      Smart_Recv(socket_num, buf, buf_size, 1, 0, packet);
      
      printf("   ");
      
      Print_Char(buf + 1, *buf, 1);
      
      handles_recv++;
   }
   num_handles = 0;
}

//for flag = 11, record the number of handles
void Read_Handle_Tot(uint32_t socket_num, char *packet) {
   char buf[4];
   
   Smart_Recv(socket_num, buf, 4, 0, 4, packet);
   memcpy(&num_handles, buf, 4);
   
   num_handles = ntohl(num_handles);
   
}

//for flag = 7
void Dest_Err(uint32_t socket_num, uint32_t packet_len, char *packet) {
   char buf[257];
   uint32_t buf_size = 257;
   
   Smart_Recv(socket_num, buf, buf_size, 0, packet_len - HEADER_LEN, packet);
   
   printf("\nClient with handle ");
   
   Print_Char(buf + 1, *buf, 0);
   
   printf(" does not exist\n");
}

//for flag = 4
void Print_Broadcast(uint32_t socket_num, uint32_t packet_len, char *packet) {
   char buf[2000];
   uint32_t buf_size = 2000, handle_len;
   
   Smart_Recv(socket_num, buf, buf_size, 0, packet_len - HEADER_LEN, packet);
   
   handle_len = *buf;
   
   printf("\n");
   
   //print out the src client handle
   Print_Char(buf + 1, handle_len, 0); 
   
   printf(": ");
   
   //find length of message: total = 3 bytes header - 1 byte len - handle len
   packet_len -= (4 + handle_len);
   
   //print out the message
   Print_Char(buf + 1 + handle_len, packet_len, 1);
}

//will print a message flag = 5
void Print_Message(uint32_t socket_num, uint32_t packet_len, char *packet) {
   char buf[2000];
   uint32_t buf_size = 2000, src_handle_len = 0;
   
   //get packet
   Smart_Recv(socket_num, buf, buf_size, 0, packet_len - HEADER_LEN, packet);
   
   src_handle_len = *(buf + 1 + buf[0]);
   
   printf("\n");
   
   //print source
   Print_Char(buf + 2 + buf[0], src_handle_len, 0);
   
   printf(": ");
   
   packet_len -= (5 + src_handle_len + buf[0]);
   
   //print message
   Print_Char(buf + 2 + src_handle_len + buf[0], packet_len, 1);
   
}

//will read the packet and execute flags as necessary
void Read_Packet(uint32_t socket_num, char *packet) {
   char buf[HEADER_LEN], flag = 0;
   uint16_t packet_len = 0;
   
   Smart_Recv(socket_num, buf, HEADER_LEN, 0, HEADER_LEN, packet);
   
   memcpy(&packet_len, buf, 2);
   packet_len = ntohs(packet_len);
   
   Check_Server_Status(packet_len, socket_num, 1, packet);
   flag = buf[2];
   
   if (flag == BROADCAST) {    //you have a broadcast
      Print_Broadcast(socket_num, packet_len, packet);
   }
   else if (flag == MESSAGE) {    //you have a message from another client
      Print_Message(socket_num, packet_len, packet);
   }
   else if (flag == FAIL_MESSAGE) {    //your destination client was bad
      Dest_Err(socket_num, packet_len, packet);
   }
   else if (flag == ACK_EXIT) {    //you can terminate now
      Clean_Up(socket_num, packet, 0);
   }
   else if (flag == HANDLE_TOT) {    //will have the 4-byte number for how many handles there are
      Read_Handle_Tot(socket_num, packet);
   }
   else if (flag == HANDLES) {    //means the rest of the handles are coming, give this priority
      Read_Handles(socket_num, packet);
   }
}

//main driver for code
int main(int argc, char **argv) {
   
   uint32_t socket_num;         //socket descriptor
   fd_set read_fds;
   char *packet;
   
   /* check command line arguments */
   if (argc != 4) {
      printf("usage: %s client-name host-name port-number \n", argv[0]);
      exit(-1);
   }
   
   //set up socket and file descriptors for select
   Set_Up(argv[1], argv[2], argv[3], &read_fds, &socket_num, &packet);
   
   while (1) {
      if (!is_end) {
         printf("$");
         fflush(stdout);
      }
      
      Safe_Select(&read_fds, socket_num + 1);
      
      if (FD_ISSET(0, &read_fds)) {
         Read_Stdin(argv[1], socket_num, packet);
      }
      if (FD_ISSET(socket_num, &read_fds)) {
         Read_Packet(socket_num, packet);
      }
      
      Set_Fds(&read_fds, socket_num); 
   }
}
