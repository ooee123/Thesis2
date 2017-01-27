/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 2
 *****************************************************************************/

#include "tcp_server_helper.h"

static uint32_t tot_len = 0;
static uint32_t bad_client = 0;

//will shift over the deleted struct
void Shift_Fds(Handle *clients, uint32_t offset) {
   uint32_t left = tot_len - offset - 1; 

   memset(clients + offset, '\0', 257);

   while (left) {
      memcpy((clients + offset), (clients + offset + 1), sizeof(Handle));
      left--;
   }

   tot_len--;
}

//cleans up a client and updates list
void Clean_Up_Client(Handle *clients, uint32_t offset) {
   close((clients + offset)->socket);
   (clients + offset)->socket = -1;
   Shift_Fds(clients, offset);
}

//checks if the client is still running, returns -1 if client disconected
int Check_Client_Status(uint16_t packet_len, Handle *clients, uint32_t offset) {
   if (!packet_len) {
      Clean_Up_Client(clients, offset);
      return 0;
   }
   return 1;
}

//r0 means use given length, 1 means look for 1 byte length
//will recieve a full packet, if expected len is 0, then find it from socket
uint16_t Smart_Recv_Server(int socket_num, char *buf, int buffer_size,
 int flg, int len, Handle *clients, int offset) {
   
   ssize_t tot_recv = 0;
   uint16_t total_len = len, rec = 0;
   
   if (flg) {
      tot_recv = recv(socket_num, buf, 1, 0);
      memcpy(&total_len, buf, 1);
      total_len++;
   }
   
   //get the rest of the message
   while (tot_recv < total_len && tot_recv < buffer_size) {
      rec = recv(socket_num, buf + tot_recv, total_len - tot_recv, 0);
      tot_recv += rec;
      
      Check_Client_Status((int)rec, clients, offset);
   }
   
   return total_len;
}

//checks if the handle already exists
int Contains_Handle(Handle *clients, char *handle, uint32_t *offset) {
   uint32_t equal = 0, index = 0;
   
   while (index < tot_len && !equal) {
      
      if (!strcmp((clients + index)->name, handle)) {
         
         equal = -1;
         
         if (offset) {
            *offset = index;
         }
      }
      index++;
   }
   
   return equal;
}

//will assemble the list of handles for flag = 12 call
void Assemble_Handles(Handle *clients, int offset, uint32_t num_handles) {
   uint32_t buf_size = 3 + num_handles * 256, index = 0;
   uint16_t pkt_len = 3;
   char buf[buf_size];
   
   while (index < tot_len) {
      //save handle length
      *(buf + pkt_len) = strlen((clients + index)->name);
         
      //copy in handle
      memcpy(buf + pkt_len + 1, (clients + index)->name, *(buf + pkt_len));
         
      //update length for handle + 1 byte length
      pkt_len += strlen((clients + index)->name);
      pkt_len++;
      
      index++;
   }
   
   pkt_len = htons(pkt_len);
   memcpy(buf, &pkt_len, 2);
   memset(buf + 2, HANDLES, 1);
   
   Safe_Send(buf, ntohs(pkt_len), (clients + offset)->socket); 
}

//will assemble the number of handles for flag = 11 call
void Assemble_List(Handle *clients, int offset) {
   char buf[7];
   uint16_t buf_size = 7;
   uint32_t num_handles = 0;
   
   num_handles = htonl(tot_len);
   buf_size = htons(buf_size);
   
   memcpy(buf, &buf_size, 2);
   memset(buf + 2, HANDLE_TOT, 1);
   memcpy(buf + 3, &num_handles, 4);
   
   Safe_Send(buf, 7, (clients + offset)->socket); 
   
   //immediately execute flag = 12
   Assemble_Handles(clients, offset, ntohl(num_handles));
}

//sends and ok to end, cleans up
void Assemble_Ack(Handle *clients, int offset) {
   char buf[3];
   uint16_t len = htons(3);
   
   memcpy(buf, &len, 2);
   memset(buf + 2, ACK_EXIT, 1);
   
   Safe_Send(buf, 3, (clients + offset)->socket);
   
   Clean_Up_Client(clients, offset);
}

//assembles and sends a regular message
void Assemble_Message(Handle *clients, int offset, int packet_len) {
   char buf[2000], dest_handle[257];
   uint32_t buf_size = 2000, dest_offset = 0;
   uint16_t len = htons(packet_len);
   
   memcpy(buf, &len, 2);
   memset(buf + 2, MESSAGE, 1);
   
   Smart_Recv_Server((clients + offset)->socket, buf + 3, buf_size, 0, 
    packet_len - HEADER_LEN, clients, offset);
   
   memcpy(dest_handle, buf + 4, buf[3]);
   memset(dest_handle + buf[3], '\0', 1);
   
   if (Contains_Handle(clients, dest_handle, &dest_offset)) {
      Safe_Send(buf, packet_len, (clients + dest_offset)->socket); 
   }
   else {
      len = htons(4 + buf[3]);
      memcpy(buf, &len, 2);
      memset(buf + 2, FAIL_MESSAGE, 1);
      Safe_Send(buf, ntohs(len), (clients + offset)->socket); 
   }
}

//assembles and sends a broadcast message
void Assemble_Broadcast(Handle *clients, int offset, int packet_len) {
   char buf[2000];
   uint32_t buf_size = 2000, index = 0;
   uint16_t len = htons(packet_len);
   
   memcpy(buf, &len, 2);
   memset(buf + 2, BROADCAST, 1);
   
   Smart_Recv_Server((clients + offset)->socket, buf + 3, buf_size, 0,
    packet_len - HEADER_LEN, clients, offset);
   
   while (index < tot_len) {
      if (index != offset) {
         Safe_Send(buf, packet_len, (clients + index)->socket); 
      }
      index++;
   }
   
}

//assembles the response packet for the inital client request
void Assemble_Init(Handle *clients, int offset, int packet_len) {
   char buf[258], packet[3];
   uint16_t send_len = htons(packet_len);
   
   //packet len is 3
   memcpy(packet, &send_len, 2);
   Smart_Recv_Server((clients + offset)->socket, buf, 257, 0, 
    packet_len - HEADER_LEN, clients, offset);
   
   //make a null termination for ease
   memset(buf + 1 + buf[0], '\0', 1);
   memset((clients + offset)->name, '\0', 1);
   
   if (Contains_Handle(clients, buf + 1, NULL)) {
      memset(packet + 2, FAIL_INIT, 1);
   }
   else {
      memcpy((clients + offset)->name, buf + 1, buf[0] + 1);
      memset(packet + 2, ACK_INIT, 1);
   }
   
   Safe_Send(packet, 3, (clients + offset)->socket);
}

//finds largest socket number
uint32_t Find_Largest(Handle *clients, uint32_t server_socket) {
   uint32_t index = 0, largest = server_socket;

   while (index < tot_len) {
      if ((clients + index)->socket > largest) {
         largest = (clients + index)->socket;
      }
      index++;
   }

   return largest;
}


//will parse and process packet
void Process_Packet(Handle *clients, int offset, uint32_t *largest_fds, uint32_t server_socket) {
   char buf[HEADER_LEN], flag = 0, alive = 1;
   uint16_t packet_len = 0;
   uint32_t socket_num = (clients + offset)->socket;
   
   alive = Smart_Recv_Server(socket_num, buf, HEADER_LEN, 0, HEADER_LEN, clients, offset);
   
   if (alive) { 
      memcpy(&packet_len, buf, 2);
      packet_len = ntohs(packet_len);
      flag = buf[2];
      
      if (flag == INIT){
         Assemble_Init(clients, offset, packet_len);
      }
      else if (flag == BROADCAST) {
         Assemble_Broadcast(clients, offset, packet_len);
      }
      else if (flag == MESSAGE) {
         Assemble_Message(clients, offset, packet_len);
      }
      else if (flag == EXITING) {
         Assemble_Ack(clients, offset);
      }
      else if (flag == LIST) {
         Assemble_List(clients, offset);
      }
   }

   if (bad_client) {
      Clean_Up_Client(clients, offset);
      *largest_fds = Find_Largest(clients, server_socket);
   }
}

void Init(fd_set *fds, Handle *clients, uint32_t *server_socket, 
 uint32_t *largest_fds) {
   //sets all clients to zero
   FD_ZERO(fds);
   Make_Zero(clients);
   
   //create the server socket
   *server_socket = tcp_recv_setup();
   *largest_fds = *server_socket;
   
   //set to listen to server socket
   FD_SET(*server_socket, fds);
}

void Make_New_Entry(fd_set *fds, Handle *clients, uint32_t server_socket, 
 uint32_t *largest_fds, uint32_t *max_clients) {
   uint32_t index = 0;
   
   //if there is a new entry
   if (tot_len == *max_clients) {
      clients = realloc(clients, *max_clients + 10);
      (*max_clients) += 10;
      Make_Zero(clients + 10);
   }
   
   index = tot_len;

   (clients + index)->socket = tcp_listen(server_socket, 5);           

   *largest_fds = Find_Largest(clients, server_socket);
   if ((clients + index)->socket > *largest_fds) {
      *largest_fds = (clients + index)->socket;
   }
   FD_SET((clients + index)->socket, fds);
   tot_len++;
   
}

void Check_Read(fd_set *fds, Handle *clients, uint32_t *largest_fds, uint32_t server_socket) {
   uint32_t index = 0;
   
   while (index < tot_len) {
      
      if (FD_ISSET((clients + index)->socket, fds)) {
         
         Process_Packet(clients, index, largest_fds, server_socket);
         
      }
      index++;
   }
}

void Reset(fd_set *fds, Handle *clients, uint32_t server_socket) {
   uint32_t index = 0;
   
   //clear
   FD_ZERO(fds);
   
   //set everything
   FD_SET(server_socket, fds);

   while (index < tot_len) {
      FD_SET((clients + index)->socket, fds);
      index++;
   }
}

int main(int argc, char **argv) {
   uint32_t server_socket= 0, largest_fds = 0, max_clients = 10;
   fd_set fds;
   Handle *clients = calloc(max_clients, sizeof(Handle));
   
   Init(&fds, clients, &server_socket, &largest_fds);
   
   while (1) {
   
      largest_fds = Find_Largest(clients, server_socket); 
      
      Safe_Select(&fds, largest_fds + 1);
      
      if (FD_ISSET(server_socket, &fds)) {
         
         Make_New_Entry(&fds, clients, server_socket, &largest_fds, 
          &max_clients);
         
      }
      
      Check_Read(&fds, clients, &largest_fds, server_socket);
      
      Reset(&fds, clients, server_socket);
   }
}


