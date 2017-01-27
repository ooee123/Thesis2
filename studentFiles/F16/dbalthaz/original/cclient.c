#include <stdio.h>
#include <stdarg.h>
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
//#include "testing.h"

typedef enum {MT_M, MT_B, MT_E, MT_J} MESSAGE_TYPE;

int construct_message(MESSAGE_TYPE mt, int serv_addr, ...);
void handle_input(int serv_addr, char *handle);
void parse_message(char *data);
void list_clients(int serv_addr);


int main(int argc, char* argv[]) {
   int serv_addr, port_addr, handle_len;
   char* handle;

   if (argc != 4) {
      printf("Usage:  cclient handle server-name server-port\n");
      return -1;
   }

   port_addr = atoi(argv[3]);
   serv_addr = tcpClientSetup(argv[2], argv[3]);

   handle = argv[1];
   handle_len = strlen(handle);

   if (construct_message(MT_J, serv_addr, handle) >= 0)
      handle_input(serv_addr, handle);

   close(port_addr);
   close(serv_addr);
   return 0;
}

void handle_input(int serv_addr, char* handle) {

   fd_set active_fds, read_fds;
   char recv_buf[MAX_BUF];
   char screen_buf[MAX_BUF];
   char dest_handle[HDL_BUF];
   char message[MAX_BUF];
   int i = 0;

   FD_ZERO(&active_fds);

   FD_SET(serv_addr, &active_fds);
   FD_SET(0, &active_fds);

   while(1) {
      read_fds = active_fds;
      printf("$: ");
      fflush(stdout);
      if (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) > 0) {
         if (FD_ISSET(0, &read_fds)) {
            scanf("%s", screen_buf);
            if (memcmp(screen_buf, "%E", 2) == 0) {
               construct_message(MT_E, serv_addr);
               return;
            }
            else if (memcmp(screen_buf, "%e", 2) == 0) {
               construct_message(MT_E, serv_addr);
               return;
            }
            else if (memcmp(screen_buf, "%M", 2) == 0) {
               scanf("%s", dest_handle);
               i = 0;
               while((message[i] = getchar()) != '\n' && i < MAX_BUF-1)
               {
                  i++;
               }
               message[i] = '\0';
               memmove(message, message+1, strlen(message));
               construct_message(MT_M, serv_addr, handle, dest_handle, message);
            }
            else if (memcmp(screen_buf, "%m", 2) == 0) {
               scanf("%s", dest_handle);
               i = 0;
               while((message[i] = getchar()) != '\n' && i < MAX_BUF-1)
               {
                  i++;
               }
               message[i] = '\0';
               memmove(message, message+1, strlen(message));
               construct_message(MT_M, serv_addr, handle, dest_handle, message);
            }
            else if (memcmp(screen_buf, "%L", 2) == 0) {
               printf("This command is not supported at this time\n");
            }
            else if (memcmp(screen_buf, "%l", 2) == 0) {
               printf("This command is not supported at this time\n");
            }
            else if (memcmp(screen_buf, "%B", 2) == 0) {
               i = 0;
               while((message[i] = getchar()) != '\n' && i < MAX_BUF-1)
               {
                  i++;
               }
               message[i] = '\0';
               memmove(message, message+1, strlen(message));
               construct_message(MT_B, serv_addr, handle, message);
            } 
            else if (memcmp(screen_buf, "%b", 2) == 0) {
               i = 0;
               while((message[i] = getchar()) != '\n' && i < MAX_BUF-1)
               {
                  i++;
               }
               message[i] = '\0';
               memmove(message, message+1, strlen(message));
               construct_message(MT_B, serv_addr, handle, message);
            }
         }
         else {
            if (recv(serv_addr, recv_buf, MAX_BUF, 0) <= 0) {
               return;
            }
            parse_message(recv_buf);
            fflush(stdout);
         }
      }
      //break;

   }
   
   return;

}
   
void parse_message(char *data) {
 
   char dest_handle[HDL_BUF];
   char src_handle[HDL_BUF];
   char message[MAX_BUF];
   int dest_len, src_len, flag = data[2];
   short data_len;

   switch(flag) {
      case 0x04:
         memcpy(&data_len, data, 2);
         data_len = ntohs(data_len);
         
         src_len = data[3];
         memcpy(src_handle, data+4, src_len);

         strncpy(message, data+4+src_len, MAX_BUF-1);
         printf("%s: %s\n", src_handle, message);
         break;
      case 0x05:
         dest_len = data[3];

         memcpy(dest_handle, data+4, dest_len);
         dest_handle[dest_len] = '\0';

         src_len = data[4+dest_len];
         memcpy(src_handle, data+5+dest_len, src_len);

         strncpy(message, data+5+dest_len+src_len, MAX_BUF-1);
         printf("%s: %s\n", src_handle, message);
         break;
      case 0x07:
         dest_len = data[3];
         
         memcpy(dest_handle, data+4, dest_len);
         dest_handle[dest_len] = '\0';

         printf("Client with handle %s does not exist\n", dest_handle);
         break;
   }
   return;
}

void list_handles(int serv_addr, uint32_t num_handles, char *recv_buf, int recv_len) {

   return;
}

int construct_message(MESSAGE_TYPE mt, int serv_addr, ...) {
//format
//MT_M: src_handle, dest_handle, message
//MT_B: src_handle, message
//MT_L: nothing
//MT_E: nothing
//MT_J: handle

   char src_handle[HDL_BUF];
   char dest_handle[HDL_BUF];
   char mesg_buf[MAX_BUF];
   char send_buf[MAX_BUF];
   char recv_buf[MAX_BUF];
   uint16_t send_len, send_len_no;
   va_list params;

   
   switch (mt) {
      case MT_B:
         va_start(params, serv_addr);
         sprintf(src_handle, "%s", va_arg(params, char*)); 
         sprintf(mesg_buf, "%s", va_arg(params, char*));
         va_end(params);

         send_buf[2] = 0x04;
         send_buf[3] = strlen(src_handle);
         memcpy(send_buf+4, src_handle, send_buf[3]);
         strcpy(send_buf+4+strlen(src_handle), mesg_buf);
         send_len = 5+strlen(src_handle)+strlen(mesg_buf);
        
         send_len_no = htons(send_len);
         memcpy(send_buf, &send_len_no, 2);
         send(serv_addr, send_buf, send_len, 0);
         return 0;
         break;
      case MT_J:
         
         va_start(params, serv_addr);
         sprintf(src_handle, "%s", va_arg(params, char*));  
         va_end(params);
         send_len = strlen(src_handle)+4;
         //send_len = htons(send_len);
         memcpy(send_buf, &send_len, 2);
         send_buf[2] = 1;
         send_buf[3] = strlen(src_handle);
         memcpy(send_buf+4, src_handle, strlen(src_handle));
     
         send(serv_addr, send_buf, strlen(src_handle)+4, 0);
         recv(serv_addr, recv_buf, MAX_BUF, 0);
         if (recv_buf[2] == 0x02) {
            //printf("handle accepted\n");
            return 0;
         }
         else if (recv_buf[2] == 0x03) {
            printf("ERROR: handle already exists\n");
            return -1;
         }
         else {
            printf("Unexpected error\n");
            return -1;
         }        
         break;
      case MT_E:
         send_buf[0] = 3;
         send_buf[1] = 0;
         send_buf[2] = 8;
         send(serv_addr, send_buf, 3, 0);
         recv(serv_addr, recv_buf, MAX_BUF, 0);
         return 1;
      case MT_M: 
         va_start(params, serv_addr);
         sprintf(src_handle, "%s", va_arg(params, char*)); 
         sprintf(dest_handle, "%s", va_arg(params, char*));
         sprintf(mesg_buf, "%s", va_arg(params, char*));
         va_end(params);

         send_buf[2] = 5;

         send_buf[3] = strlen(dest_handle);
         sprintf(send_buf+4, "%s %s%s", dest_handle, src_handle, mesg_buf);
         send_buf[4+strlen(dest_handle)] = strlen(src_handle);
         //printf("--packet: |%s|--\n", send_buf+3);

         send_len = strlen(send_buf+3) + 4;
         //send_buf[send_len] = '\0';
         send_len_no = htons(send_len);
         memcpy(send_buf, &send_len_no, 2);

         send(serv_addr, send_buf, send_len, 0);
         return 0;          
   }
   return -1;
}
