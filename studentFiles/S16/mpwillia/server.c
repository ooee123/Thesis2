
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/select.h> 
#include <arpa/inet.h>
#include "testing.h"
#include "server.h"
#include "client_handler.h"
#include "packet.h" 

int main(int argc, char** argv) {
   
   if(argc > 2) {
      printf("Usage: %s [port-number]\n", argv[0]);
      exit(1);
   } 

   int server_socket = 0;
   server_socket = setup((argc >= 1 ? argv[1] : NULL));
   run(server_socket);

   close(server_socket);
   return 0;
} 


int setup(char* port_arg) {
   
   // parse the port_arg if we were given any
   int port = 0; 
   if(port_arg != NULL) {
      if(sscanf(port_arg, "%d", &port) != 1) {
         printf("Invalid port number '%s'", port_arg);
         exit(1);
      }  
   } 

   int server_socket = 0;
   struct sockaddr_in local;
   socklen_t len = sizeof(local);

   // create the socket
   server_socket = socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0) {
      perror("Error creating socket");
      exit(1);
   } 

   local.sin_family = AF_INET;
   local.sin_addr.s_addr = INADDR_ANY;
   local.sin_port = htons(port);
   
   // bind the socket to the parsed port if we can
   if(bind(server_socket, (struct sockaddr*) &local, sizeof(local)) < 0) {
      perror("Bind failed");
      exit(1);
   } 

   if(getsockname(server_socket, (struct sockaddr*) &local, &len) < 0) {
      perror("Failed to get socket name");
      exit(1);
   }

   // setup the server to listen for connections
   if(listen(server_socket, LISTEN_BACKLOG) < 0) {
      perror("Listen failed");
      exit(1);
   } 

   printf("Server is using port %d\n", ntohs(local.sin_port));
   return server_socket;
}


void run(int server_socket) {

   // setup our empty list of clients
   struct connections cons;
   cons.num = 0;
   cons.max = INITIAL_CAPACITY;
   cons.clients = calloc(cons.max, sizeof(struct client*));

   // setup our file descriptor set
   fd_set master;
   int fdmax = server_socket;
   FD_ZERO(&master);
   FD_SET(server_socket, &master);
  
   int client_socket = -1;
   for(;;) {
      fd_set readfds = master;
      if(select(fdmax+1, &readfds, NULL, NULL, NULL) == -1) {
         perror("Select failed");
         exit(1);
      }
      
      // handle incomming connections
      if(FD_ISSET(server_socket, &readfds)) {
         client_socket = accept_connection(server_socket, &cons);
         
         // update the fd set and fdmax
         FD_SET(client_socket, &master);
         if(client_socket > fdmax)
            fdmax = client_socket;
      } 
      // otherwise handle incomming messages 
      else if((client_socket = getReady(&readfds, &cons)) >= 0) {

         uint8_t* data = receive_packet(client_socket);

         if(data != NULL) {
            struct header* head = (struct header*)data; 
            
            if(head->len > 0 && head->flags != C_EXIT) {
               handle_packet(data, client_socket, &cons);
            } 
            else {
               // handle the client exiting, do we need to acknowledge it? 
               client_exit(client_socket, head->flags, &master, &cons);
               fdmax = getFDMax(&cons);
            } 
            free(data);
         } 
      }
      else {
         printf("ERROR: Select signaled a readable fd but found non!\n");
         exit(1);
      } 
     
   } 
} 

int accept_connection(int server_socket, struct connections* cons) {
     
   int client_socket = -1;
   if((client_socket = accept(server_socket, (struct sockaddr*)0, (socklen_t*)0)) < 0) {
      perror("Accept failed");
      exit(1);
   } 
   
   if(addClient(client_socket, cons) < 0) {
      perror("Failed to add new client.");
      exit(1);
   } 

   return client_socket;
} 

int getReady(const fd_set* check, struct connections* cons) {
   int i;
   for(i = 0; i < cons->max; i++) {
      if(cons->clients[i] != NULL) {
         if(FD_ISSET(cons->clients[i]->socket, check))  
            return cons->clients[i]->socket;
      } 
   }
   return -1;
} 

void client_exit(int client_socket, int flags, fd_set* master, struct connections* cons) {
   // client signaled the exit, acknowledge it
   if(flags == C_EXIT) {
      sendHeader(client_socket, S_EXIT_OK);
   } 
   
   // cleanup client data
   removeClient(client_socket, cons);
   FD_CLR(client_socket, master);
   close(client_socket);
} 

void handle_packet(uint8_t* packet, int client_socket, struct connections* cons) {
   
   struct header* head = (struct header*)packet;

   switch(head->flags) {
      case C_CONNECT: {
         client_connect(packet + HEADER_LEN, client_socket, cons);
         break;
      }

      case C_BCAST: {
         client_broadcast(packet, client_socket, cons);
         break;
      }

      case C_MSG: {
         client_message(packet, client_socket, cons);
         break;
      }

      case C_LIST: {
         send_client_list(client_socket, cons);
         break;
      }
   } 
} 


void client_connect(const uint8_t* handle_data, int client_socket, struct connections* cons) {
   
   uint8_t handle_len = *(handle_data);
   char handle[handle_len+1];
   memset(handle, 0, handle_len+1);
   memcpy(handle, handle_data+1, handle_len);

   if(getClientByHandle(handle, cons) != NULL) {
      // another client has that handle 
      sendHeader(client_socket, S_ERR_HANDLE);
   } 
   else {
      // no client has this handle, accept it     
      sendHeader(client_socket, S_ACCEPT);
      setClientHandle(handle, client_socket, cons);
   } 
} 

// returns 1 if the message was passed on, 0 otherwise
int client_message(uint8_t* packet, int client_socket, struct connections* cons) {
   
   uint8_t* ptr = packet + HEADER_LEN;
   uint8_t dst_len = *ptr;
   char dst[dst_len+1];
   memcpy(dst, ptr+1, dst_len);
   dst[dst_len] = '\0';
   
   if(check_dest(client_socket, dst, cons) == 0)
      return 0;

   safeSend(getClientSocket(dst, cons), packet, ntohs(((struct header*)packet)->len), 0);

   return 1;
} 

// returns 1 if the message was passed on, 0 otherwise
void client_broadcast(uint8_t* packet, int client_socket, struct connections* cons) {

   int packet_len = ntohs(((struct header*) packet)->len);
   
   // relay the packet to all the connected clients other than the sender
   int i;
   for(i = 0; i < cons->max; i++) {
      struct client* c = cons->clients[i]; 
      if(c != NULL && c->socket != client_socket)
         safeSend(c->socket, packet, packet_len, 0);
   } 
}


// check that the given destination exists
int check_dest(int client_socket, char* dst, struct connections* cons) {
   
   // client exists, good
   if(getClientByHandle(dst, cons) != NULL)
      return 1;
   
   // client doesn't exist, error
   uint8_t* packet = NULL;
   createPacket(S_ERR_DST, &packet);

   struct unpacked_address dst_addr;
   dst_addr.len = strlen(dst);
   dst_addr.handle = dst;
   int packet_len = addAddress(&dst_addr, &packet);

   safeSend(client_socket, packet, packet_len, 0);
   free(packet);

   return 0;
} 


void send_client_list(int client_socket, struct connections* cons) {
   // sending S_LIST_NUM packet
   int num_packet_len = HEADER_LEN + 4;
   uint8_t* num_packet = malloc(sizeof(uint8_t) * num_packet_len);
   struct header* num_header = (struct header*)num_packet;
   num_header->len = htons(num_packet_len);
   num_header->flags = S_LIST_NUM;

   *(num_packet+HEADER_LEN) = htonl(cons->num);
   uint32_t* ptr = (uint32_t*)(num_packet+HEADER_LEN);
   *ptr = htonl(cons->num);

   safeSend(client_socket, num_packet, num_packet_len, 0);

   // sending S_LIST_WHO packet   
   uint8_t* list_packet = NULL;
   int list_packet_len = createPacket(S_LIST_WHO, &list_packet);
   
   int i;
   for(i = 0; i < cons->max; i++) {
      struct client* c = cons->clients[i];
      if(c != NULL) {
         struct unpacked_address addr;
         addr.len = strlen(c->handle);
         addr.handle = c->handle;

         list_packet_len = addAddress(&addr, &list_packet);
      }
   } 
   
   safeSend(client_socket, list_packet, list_packet_len, 0);
} 

