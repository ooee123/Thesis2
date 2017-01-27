
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include "testing.h"
#include "client.h"
#include "packet.h"

int main(int argc, char** argv) {
   
   // verify command line args 
   if(argc != 4) {
      printf("Usage: %s handle host-name port-number\n", argv[0]);
      exit(1);
   } 

   if(strlen(argv[1]) > MAX_HANDLE_LEN) {
      printf("Invalid handle.\n");
      exit(1);
   }
   
   int server_socket = 0;
   server_socket = setup(argv[2], argv[3]);
   request_handle(server_socket, argv[1]);
   start_interactive(server_socket, argv[1]);

   close(server_socket);
   return 0;
} 


int setup(char* hostname, char* port_arg) {
   
   // parse the port_arg and check for bad arg
   int port = 0; 
   if(sscanf(port_arg, "%d", &port) != 1) {
      printf("Invalid port number '%s'", port_arg);
      exit(1);
   }  

   int socket_num;
   struct sockaddr_in remote;
   struct hostent *hp;

   // create the socket
   if((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("Failed to create socket");
      exit(1);
   } 
   
   remote.sin_family = AF_INET;

   // get the address of the server
   if((hp = gethostbyname(hostname)) == NULL) {
      printf("Error getting hostname: %s\n", hostname);
      exit(1);
   } 

   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   // get the server port
   remote.sin_port = htons(port);
   
   // make the connection to the server
   if(connect(socket_num, (struct sockaddr*) &remote, sizeof(struct sockaddr_in)) < 0) {
      perror("Failed to connect");
      exit(1);
   } 
   
   return socket_num;
} 


void request_handle(int server_socket, char* handle) {

   uint8_t* packet = malloc(sizeof(struct header));
   struct header* head = (struct header*)packet;
   head->len = htons(HEADER_LEN);
   head->flags = C_CONNECT;

   struct unpacked_address addr;
   addr.len = strlen(handle);
   addr.handle = handle;

   int packet_len = addAddress(&addr, &packet);

   // send connect packet
   if(send(server_socket, packet, packet_len, 0) < 0) {
      perror("Send failed");
      exit(1);
   } 
   
   free(packet);

   // wait for response from server
   int flag = recvHeader(server_socket);

   if(flag == TERM) {
      printf("\rServer Terminated\n");
      exit(1);
   }
   else if(flag == S_ERR_HANDLE) {
      printf("\rHandle already in use: %s\n", handle);
      exit(1);
   }
   else if(flag != S_ACCEPT) {
      printf("\rUnexpected flag: %d\n", head->flags);
      exit(1);
   }
} 



void start_interactive(int server_socket, char* handle) {
   fd_set master;
   int fdmax = server_socket;
   FD_ZERO(&master);
   FD_SET(0, &master);
   FD_SET(server_socket, &master);

   for(;;) {
      printf("$: ");
      fflush(stdout);
      fd_set readfds = master;
      if(select(fdmax+1, &readfds, NULL, NULL, NULL) == -1) {
         perror("Select failed");
         exit(1);
      } 

      if(FD_ISSET(0, &readfds)) {
         char* buf = NULL;
         size_t len = 0;
         getline(&buf, &len, stdin);

         int i;
         for(i = 0; i < len; i++) {
            if(buf[i] == '\n') 
               buf[i] = '\0';
         } 
         
         parse_command(buf, server_socket, handle);
      }
      else if(FD_ISSET(server_socket, &readfds)) {
         
         uint8_t* data = receive_packet(server_socket);

         if(data != NULL) {
            struct header* head = (struct header*)data;

            if(head->len > 0) {
               // process message 
               if(handle_packet(data) == 0) {
                  printf("\n");
                  break;
               }
            } 
            else {
               printf("\rServer Terminated\n");
               break;
            } 
         } 

      }
      else {
         printf("Error: select returned but neither fd was set!\n");
         exit(1);
      } 
   } 
} 


// parses the given command, sending relevant packets to the server if needed
// returns 1 if the client should continue running, 0 on exit
void parse_command(char* command, int server_socket, char* handle) {
   
   // veryify the format of the command
   // expects single letter commands
   // so '%x [args]'
   if(command[0] != '%' || strcspn(command, " \n\r") > 2) {
      printf("Invalid command\n");
      return;
   } 
   
   switch(command[1]) {
      case 'm':
      case 'M': {
         // verify command arguments
         // Expected: %m handle message
         char* dst = command + strcspn(command, " \n\r") + 1;
         char* msg = NULL; 
         
         // splits dst and msg
         dst[strcspn(dst, " ")] = '\0';
         // sets message
         msg = dst + strcspn(dst, "\0") + 1;
       
         if(strlen(dst) <= 0 || strlen(msg) <= 0) {
            printf("Invalid command\n");
            return;
         }
         
         // send the message
         send_message(server_socket, dst, handle, msg);
         break;
      }

      case 'b':
      case 'B': {
         char* msg = command + strcspn(command, " \n\r")+1;
         msg[strcspn(msg, "\n\r")] = '\0';

         send_message(server_socket, NULL, handle, msg);
         break;
      }

      case 'l':
      case 'L':
         sendHeader(server_socket, C_LIST);
         break;
      
      case 'e':
      case 'E':
         sendHeader(server_socket, C_EXIT);
         break;
   } 
} 


void send_message(int server_socket, char* dst, char* src,  char* message) {

   // split packet if needed
   while(strlen(message) > MAX_MSG_CHAR_LEN) {
      char cutmsg[MAX_MSG_LEN];
      memcpy(cutmsg, message, MAX_MSG_CHAR_LEN);
      cutmsg[MAX_MSG_CHAR_LEN]='\0';
      send_message(server_socket, dst, src, cutmsg);
      message += MAX_MSG_CHAR_LEN;
   } 
   
   uint8_t* packet = NULL;
   
   // check if we should broadcast or not
   if(dst != NULL) {
      createPacket(C_MSG, &packet);
      struct unpacked_address dst_addr;
      dst_addr.len = strlen(dst);
      dst_addr.handle = dst;
      addAddress(&dst_addr, &packet);
   }
   else {
      createPacket(C_BCAST, &packet);
   } 

   struct unpacked_address src_addr;
   src_addr.len = strlen(src);
   src_addr.handle = src;
   addAddress(&src_addr, &packet);  

   int packet_len = addMessage(message, &packet);

   safeSend(server_socket, packet, packet_len, 0);
   free(packet);
} 



int handle_packet(uint8_t* packet) {
   struct header* head = (struct header*)packet;
   uint8_t* ptr = packet + HEADER_LEN;

   switch(head->flags) {
      case S_EXIT_OK:
         printf("\r   ");
         return 0; 
         break;

      case S_ERR_DST: {
         
         uint8_t dst_len = *ptr;
         char dst[dst_len+1];
         memcpy(dst, ptr + 1, dst_len);
         dst[dst_len] = '\0';

         printf("\rClient with handle %s does not exist.\n", dst);
         break; 
      }

      case C_MSG: {
         // only difference between MSG and BCAST is a msg has a dst that we don't need anymore
         uint8_t dst_len = *ptr;
         ptr += dst_len + 1;
      }
      case C_BCAST: {

         uint8_t src_len = *ptr;
         char src[src_len+1];
         memcpy(src, ptr+1, src_len);
         src[src_len] = '\0';
         ptr += src_len + 1;
         
         printf("\r%s: %s\n", src, (char*)ptr);
         break; 
      }

      case S_LIST_NUM:
      case S_LIST_WHO:
         handle_client_list(packet);
         break;
   } 
   
   return 1;
} 


void handle_client_list(uint8_t* packet) {
  
   struct header* head = (struct header*)packet;
   static int num = 0;

   switch(head->flags) {
      case S_LIST_NUM: {
         num = ntohl(*((uint32_t*)(packet + HEADER_LEN)));
         break;
      }

      case S_LIST_WHO: {
         printf("\rNumber of clients: %d\n", num);
         uint8_t* ptr = packet + HEADER_LEN;

         int i;
         for(i = 0; i < num; i++) {
            uint8_t handle_len = *ptr;
            char handle[handle_len+1];
            memcpy(handle, ptr+1, handle_len);
            handle[handle_len]='\0';
            ptr += handle_len+1;
            printf("  %s\n", handle);
         } 

         break;
      }
   } 
} 


