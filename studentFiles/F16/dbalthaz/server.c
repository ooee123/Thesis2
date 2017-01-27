#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "networks.h"
#include "chat.h"
#include "testing.h"

typedef struct 
{
   char **handles;
   int current_handles;
   int current_max;
 
   //index for handles is client FD-4
   //STDIN = 0
   //STDOUT = 1
   //STDERR = 2
   //socket = 3
   
} handle_list;

void main_op(int port_addr);
void handle_client(int port_addr, handle_list *hdl_list, fd_set *active_fds);
void new_connection(int port_addr, handle_list *hdl_list, fd_set *active_fds);
int send_message(int port_addr, char *data, handle_list *hdl_list, fd_set *active_fds); 
void close_client(int port_addr, handle_list *hdl_list, fd_set *active_fds);
int find_handle(handle_list *hdl_list, char* toFind);
void broadcast(int port_addr, char* data, handle_list *hdl_list, fd_set *active_fds);

int main(int argc, char *argv[]) {

   int port_addr, i; 
   char **temp = (char**) calloc(20, sizeof(char *)); 
   for (i = 0; i < 20; i++ )
      temp[i] = calloc(256, sizeof(char));
   handle_list hdl_list = {
      .handles = temp,
      .current_handles = 0, 
      .current_max = 20
   };

   fd_set active_fds, read_fds;
   
   FD_ZERO(&active_fds);
   port_addr =  (argc == 2) ? atoi(argv[1]) : 0;
   port_addr = tcpServerSetup(port_addr);
   FD_SET(port_addr, &active_fds);

   while (1) {
      read_fds = active_fds;
      if (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) < 0) {
         perror("select");
         return -1;
      }
      else {    
         for (i = 0; i < FD_SETSIZE; ++i) { 
            if (FD_ISSET(i, &read_fds)) {
               if (i == port_addr) {
                  new_connection(port_addr, &hdl_list, &active_fds);
                  FD_ZERO(&read_fds);
               }
               else {
                  handle_client(i, &hdl_list, &active_fds);
               }
            }
         }
      }
   } 
   close(port_addr);
   return 0;
}

void new_connection(int port_addr, handle_list *hdl_list, fd_set *active_fds) {
      
   uint16_t data_len; 
   char **temp;
   int i;
   uint8_t handle_len = 0;
   uint8_t data[MAX_BUF];
   uint8_t send_buf[MAX_BUF];
   uint8_t flag;
   char handle[HDL_BUF];

   int client_port = tcpAccept(port_addr);
   if (recv(client_port, data, MAX_BUF, 0) < 0) {
      printf("data not sent\n");
      return;
   }
   memcpy(&data_len, data, 2);
   data_len = ntohs(data_len);
   handle_len = data[3];
   flag = data[2];

   assert(flag == 0x01);
   assert(data_len > 3);
   //assert(client_port < hdl_list->current_max);

   handle_len = data[3];
   assert(handle_len > 0);

   if (hdl_list->current_max <= client_port-HDL_OFF) {
      //printf("Resizing client list\n");
      temp = realloc(hdl_list->handles, (hdl_list->current_max + 20) * sizeof(char *));
      for (i = 0; i < 20; i++) {
         temp[i+hdl_list->current_max] = calloc(256, sizeof(char));
      }
      hdl_list->handles = temp;
      hdl_list->current_max += 20;

   }
   

   memcpy(handle, data+4, handle_len);
   handle[handle_len] = '\0';

   if (find_handle(hdl_list, handle) == -1) {
      memcpy(hdl_list->handles[client_port-HDL_OFF], data+4, handle_len);
      hdl_list->handles[client_port-HDL_OFF][handle_len] = '\0';
      hdl_list->current_handles++;
      FD_SET(client_port, active_fds);
      send_buf[2] = 0x02;
   }
   else
      send_buf[2] = 0x03;
   send_buf[0] = 3;
   send_buf[1] = 0;
   send(client_port, send_buf, 3, 0);
   return;
}


void handle_client(int port_addr, handle_list *hdl_list, fd_set *active_fds) {

   char data[MAX_BUF];
   uint8_t flag;
   uint16_t data_len;
   int num_bytes;      

   num_bytes = recv(port_addr, data, MAX_BUF, 0);
   //if error
   if (num_bytes < 0) {
      printf("data not sent\n");
      return;
   }
   //if client unexpectedly closes
   else if(num_bytes == 0) {
      close(port_addr);
      FD_CLR(port_addr, active_fds);
      hdl_list->handles[port_addr-HDL_OFF][0] = '\0';
      hdl_list->current_handles--;
      return;
   }
      
   memcpy(&data_len, data, 2);
   data_len = ntohs(data[1]);
   flag = data[2];

   switch(flag) {
      case 0x01: 
         //should never happen
         //printf("Recieved bad packet from client on port :%d\n", port_addr);
         break;
      case 0x02:
         //also should never happen
         //printf("Recieved bad packet from client on port :%d\n", port_addr);
         break;
      case 0x03:
         //also should never happen either
         //printf("Recieved bad packet from client on port :%d\n", port_addr);
         break;
      case 0x04:
         //printf("Broadcast packet not implemented\n");
         broadcast(port_addr, data, hdl_list, active_fds);
         break;
      case 0x05:
         //printf("Message packet\n");
         send_message(port_addr, data, hdl_list, active_fds);
         break;
      case 0x08:
         //printf("Closing packet\n");
         close_client(port_addr, hdl_list, active_fds);
         break;
      case 0x09: //should never happen
         break;
      case 0x0A:
         printf("Listing clients not supported\n");
         break;
      case 0x0B:
         break;
      case 0x0C:
         break;
      default:
         printf("Unknown flag\n");
         printf("flag: %x\n", flag);
      
   }

   //close(port_addr);

   return;
}
     
void close_client(int port_addr, handle_list *hdl_list, fd_set *active_fds) {
   char close_mesg[] = "309";
   send(port_addr, close_mesg, 3, 0);
 
   close(port_addr);
   FD_CLR(port_addr, active_fds);
   hdl_list->handles[port_addr-HDL_OFF][0] = '\0';;
   hdl_list->current_handles--;
   return;
}
 
int send_message(int port_addr, char *data, handle_list *hdl_list, fd_set *active_fds) {

   char dest_handle[HDL_BUF];
   int dest_handle_len, dest_port;
   uint16_t data_len;

   memcpy(&data_len, data, 2);
   data_len = ntohs(data_len);

   dest_handle_len = data[3];
   memcpy(dest_handle, data+4, dest_handle_len);
   dest_handle[dest_handle_len] = '\0';

   dest_port = find_handle(hdl_list, dest_handle);

   if (dest_port != -1)
      send(dest_port, data, data_len, 0);
   else {
      data_len = ntohs(3+strlen(dest_handle));
      memcpy(data, &data_len, 2);
      data[2] = 0x07; 
      send(port_addr, data, 4+strlen(dest_handle), 0);
      
   }          

   return 0;
}

void broadcast(int port_addr, char* data, handle_list *hdl_list, fd_set *active_fds) {
   
   char src_handle[HDL_BUF];
   int src_handle_len, i;
   uint16_t data_len;

   memcpy(&data_len, data, 2);
   data_len = ntohs(data_len);

   src_handle_len = data[3];
   memcpy(src_handle, data+4, src_handle_len);
   src_handle[src_handle_len] = '\0';

   for (i = 0; i < hdl_list->current_handles; i++) {
      if((strcmp(hdl_list->handles[i], src_handle) != 0) && hdl_list->handles[i][0] != '\0') {
         send(i+HDL_OFF, data, data_len, 0);
      }
   } 

   return;
}
//returns port number to send on if found
//returns -1 if not found;
int find_handle(handle_list *hdl_list, char* toFind) {

   int i;

   for (i = 0; i < hdl_list->current_handles; i++) {
      if (strcmp(hdl_list->handles[i], toFind) == 0) {
         return i+HDL_OFF;
      }
   }
   return -1;

}
