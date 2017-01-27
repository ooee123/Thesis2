/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
 *****************************************************************************/

#include <stdio.h>
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
#include <netdb.h>
#include <sys/select.h>

#include "networks.h"

typedef struct handle_struct {
   char handle[256];
   int socket;
   int alive;
} handle_struct;

static handle_struct *handle_list;
static int handle_list_size = 1;
static uint32_t num_handles = 0;

void select_fun (int server_socket, int back_log);

int main (int argc, char *argv[]) {
   int server_socket= 0;   //socket descriptor for the server socket
   int client_socket= 0;   //socket descriptor for the client socket

   handle_list = calloc(handle_list_size, sizeof(struct handle_struct));

   printf("sockaddr: %d sockaddr_in %d\n", sizeof(struct sockaddr), sizeof(struct sockaddr_in));

   //create the server socket
   if (argc > 1) {
      server_socket = tcp_recv_setup(strtol(argv[1], NULL, 10));
   } else {
      server_socket = tcp_recv_setup(0);
   }
   select_fun(server_socket, 5);

   /* close the sockets */
   close(server_socket);
   close(client_socket);
   return 0;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup (int port) {
   int server_socket= 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */

   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0) {
      perror("socket call");
      exit(1);
   }

   local.sin_family= AF_INET;         //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= htons(port);          //let system choose the port

   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
      exit(-1);
   }
    
   //get the port name and print it out
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
	  perror("getsockname call");
	  exit(-1);
   }

   printf("socket has port %d \n", ntohs(local.sin_port));
	        
   return server_socket;
}

int send_message (char *buf, int buffer_size, int client_socket) {
   int i = 0;
   uint8_t *dest_handle = malloc(buf[1] + 1);
   uint8_t *src_handle = malloc(buf[2 + buf[1]] + 1);
   uint8_t * packet;
   uint16_t packet_len = 0;
   int dest_handle_len = buf[1];
   int src_handle_len = buf[2 + buf[1]];
   int dest_port = -1;

   // printf("Packet received, length: %d\n", buffer_size + 2);
   // printf("Flag: %d\n", buf[0]);
   // printf("Dest. handle length = %d\n", buf[1]);

   // Get destination handle
   for (i = 0; i < dest_handle_len; i ++) {
      dest_handle[i] = buf[i + 2];
   }
   dest_handle[dest_handle_len] = '\0';

   // Try to find destination port
   for (i = 0; i < handle_list_size; i++) {
      if (handle_list[i].alive && !strcmp(handle_list[i].handle, (char *)dest_handle)) {
         dest_port = handle_list[i].socket;
      }
   }

   // Send packet 
   if (dest_port < 0) {
      packet_len = 4 + dest_handle_len;
      packet = malloc(packet_len);
      packet_len = htons(packet_len);
      memcpy(packet, &packet_len, 2);
      memset(packet + 2, 7, 1);
      memset(packet + 3, dest_handle_len, 1);
      memcpy(packet + 4, dest_handle, dest_handle_len);
      send(client_socket, packet, 4 + dest_handle_len, 0);
   } else {
      packet_len = 3 + buffer_size;
      packet = malloc(packet_len);
      packet_len = htons(packet_len);
      memcpy(packet, &packet_len, 2);
      memcpy(packet + 2, buf, buffer_size);
      packet[2 + buffer_size] = '\0';
      send(dest_port, packet, ntohs(packet_len), 0);
   }

   // Get source handle
   for (i = 0; i < src_handle_len; i ++) {
      src_handle[i] = buf[i + 3 + buf[1]];
   }
   src_handle[src_handle_len] = '\0';

   // printf("dest_handle = %s\n", dest_handle);
   // printf("src_handle = %s\n", src_handle);
   // printf("Message = %s\n", (uint8_t *)(buf + 3 + buf[1] + buf[2 + buf[1]]));

   free(packet);
   free(dest_handle);
   free(src_handle);

   return !dest_port;
}

int broadcast_message (char *buf, int buffer_size) {
   uint8_t *src_handle = malloc(buf[1] + 1);
   int src_handle_len = buf[1];
   int packet_len = htons(3 + buffer_size);
   uint8_t *packet = malloc(ntohs(packet_len));
   int i = 0;

   // Get source handle
   for (i = 0; i < src_handle_len; i ++) {
      src_handle[i] = buf[i + 2];
   }
   src_handle[src_handle_len] = '\0';

   memcpy(packet, &packet_len, 2);
   memcpy(packet + 2, buf, buffer_size);
   packet[2 + buffer_size] = '\0';

   for (i = 0; i < handle_list_size; i++) {
      if (handle_list[i].alive && strcmp(handle_list[i].handle, (char *)src_handle)) {
         send(handle_list[i].socket, packet, ntohs(packet_len), 0);
      }
   }

   free(src_handle);
   return 0;
}

void client_exit (char *buf, int client_socket) {
   uint8_t * packet;
   uint16_t packet_len = 0;
   int i = 0;

   packet_len = 3;
   packet = malloc(packet_len);
   packet_len = htons(packet_len);
   memcpy(packet, &packet_len, 2);
   memset(packet + 2, 9, 1);

   for (i = 0; i < handle_list_size; i++) {
      if (handle_list[i].alive && (client_socket == handle_list[i].socket)) {
         send(client_socket, packet, ntohs(packet_len), 0);
         handle_list[i].alive = 0;
         num_handles--;
         close(client_socket);
         break;
      }
   }

   free(packet);
}

int check_handle (char *buf, int client_socket) {
   struct handle_struct *temp;
   uint8_t *response = malloc(3);
   char *handle;
   int handle_len = 0;
   uint16_t packet_len = htons(3);
   memcpy(response, &packet_len, 2);
   int i = 0;

   // Get handle
   handle_len = buf[1];
   handle = malloc(handle_len + 1);
   for (i = 0; i < handle_len; i ++) {
      handle[i] = buf[i + 2];
   }
   handle[handle_len] = '\0';

   // Expand if list is full
   if (num_handles == handle_list_size) {
      handle_list_size *= 2;
      temp = calloc(handle_list_size, sizeof(handle_struct));
      memcpy(temp, handle_list, (handle_list_size/2)*sizeof(handle_struct));
      free(handle_list);
      handle_list = temp;
   }

   // Check for duplicate handle
   for (i = 0; i < handle_list_size; i++) {
      if (handle_list[i].alive && !strcmp(handle_list[i].handle, handle)) {
         response[2] = 3;
         send(client_socket, response, 3, 0);
         free(response);
         free(handle);
         return -1;
      }
   }

   // Adds handle if not duplicate
   for (i = 0; i < handle_list_size; i++) {
      if (!handle_list[i].alive) {
         strcpy(handle_list[i].handle, handle);
         handle_list[i].socket = client_socket;
         handle_list[i].alive = 1;
         response[2] = 2;
         send(client_socket, response, 3, 0);
         num_handles++;
         free(response);
         free(handle);
         return 0;
      }
   }

   return 0;
}

void send_clients (int client_socket) {
   uint8_t * packet;
   uint16_t packet_len = 0;
   uint32_t list_len = 3;
   int i = 0;
   uint32_t cur_ptr = 3;

   packet_len = 7;
   packet = malloc(packet_len);
   packet_len = htons(packet_len);
   memcpy(packet, &packet_len, 2);
   memset(packet + 2, 11, 1);
   memcpy(packet + 3, &num_handles, 4);

   send(client_socket, packet, 7, 0);
   free(packet);

   for (i = 0; i < handle_list_size; i++) {
      if (handle_list[i].alive) {
         list_len += strlen(handle_list[i].handle) + 1;
      }
   }

   packet = malloc(list_len);
   memset(packet, 0, 2);
   memset(packet + 2, 12, 1);

   for (i = 0; i < handle_list_size; i++) {
      if (handle_list[i].alive) {
         memset(packet + cur_ptr++, strlen(handle_list[i].handle), 1);
         memcpy(packet + cur_ptr, handle_list[i].handle, strlen(handle_list[i].handle));
         cur_ptr += strlen(handle_list[i].handle);
      }
   }

   send(client_socket, packet, list_len, 0);
   free(packet);
}

void control_c_exit (int client_socket) {
   int i = 0;

   for (i = 0; i < handle_list_size; i++) {
      if (handle_list[i].alive && (client_socket == handle_list[i].socket)) {
         handle_list[i].alive = 0;
         num_handles--;
         close(client_socket);
         break;
      }
   }
}

void select_fun(int server_socket, int back_log) {
   static struct timeval timeout;
   fd_set fdvar;
   fd_set tempfds;
   timeout.tv_sec = 1;
   timeout.tv_usec = 0;

   char *buf;              //buffer for receiving from client
   int buffer_size= 1024;  //packet size variable
   int packet_len= 0;      //length of the received message
   
   uint16_t *header = malloc(2);
   int header_len = 0;

   int client_socket= 0;
   int numfds = 0;
   int i = 0;

   //create packet buffer
   buf = (char *) malloc(buffer_size);
   
   if (listen(server_socket, back_log) < 0) {
      perror("listen call");
      exit(-1);
   }

   FD_ZERO(&fdvar);
   FD_ZERO(&tempfds);
   FD_SET(server_socket, &fdvar);
   numfds = server_socket;

   for (;;) {
      tempfds = fdvar;
      if (select(numfds + 1, (fd_set *) &tempfds, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
         perror("select call");
         exit(-1);
      }

      for (i = 0; i <= numfds; i++) {
         // Check for clients ready to read
         if (FD_ISSET(i, &tempfds)) {
            // Accept new connections
            if (i == server_socket) {
               if ((client_socket = accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
                  perror("accept call");
                  exit(-1);
               } else {
                  FD_SET(client_socket, &fdvar);
                  if (client_socket > numfds) {
                     numfds = client_socket;
                  }
               }
            // Read new packets
            } else {
               header_len = recv(i, header, 2, 0);
               if (header_len == 0) {
                  control_c_exit(i);
                  FD_CLR(i, &fdvar);
               } else if (header_len == 2) {
                  free(buf);
                  buffer_size = ntohs(*header);
                  buf = malloc(buffer_size);
                  packet_len = recv(i, buf, buffer_size - 2, 0);

                  if (packet_len < 0) {
                     perror("recv call");
                     exit(-1);
                  } else {
                     switch (buf[0]) {
                        case 1:
                           check_handle(buf, client_socket);
                           break;
                        case 4:
                           broadcast_message(buf, buffer_size - 2);
                           break;
                        case 5:
                           send_message(buf, buffer_size - 2, i);
                           break;
                        case 8:
                           client_exit(buf, i);
                           FD_CLR(i, &fdvar);
                           break;
                        case 10:
                           send_clients(i);
                           break;
                        default:
                           break;
                     }
                  }
               }
            }
         }
      }  
   }
}
