#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
  
#define PORT 8888
#define BUFFER_LEN 1025

typedef struct Node {
   int client_socket;
   char handle[256];
   struct Node *next;
} Node;

void add(Node **list, int client_socket) {
   while (*list) {
      list = &((*list)->next);
   }

   *list = calloc(sizeof(Node), 1);
   (*list)->client_socket = client_socket;
}

int delete(Node **list, int client_socket) {
   Node *tmp;
   do {
      tmp = (*list)->next;
      if ((*list)->client_socket == client_socket) {
         free(*list);
         *list = tmp;
         return 0;
      }
      list = &((*list)->next);
   } while (tmp);

   return -1;
}

int setup_socket() {
   int server_socket;
   int opt = 1;

   if (!(server_socket = socket(AF_INET, SOCK_STREAM, 0))) {
      perror("socket failed");
   }
  
   if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, ((char *) &opt),
      sizeof(opt)) < 0 ) {
      perror("setsockopt");
   }
   return server_socket;  
}

struct sockaddr_in setup_address(int server_socket) {
   struct sockaddr_in address;

   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(PORT);
     
   if (bind(server_socket, ((struct sockaddr *) &address), sizeof(address)) 
      < 0) {
      perror("bind failed");
   }
   printf("Listener on port %d\n", PORT);    
    
   if (listen(server_socket, 3) < 0) {
      perror("listen");
   }

   return address;
}

int initialize_fds (fd_set *readfds, int server_socket, Node *client_sockets) {
   int max_sd = server_socket;
   Node *tmp = client_sockets;

   FD_ZERO(readfds);  
   FD_SET(server_socket, readfds);
         
   while (tmp) {
      FD_SET(tmp->client_socket, readfds);
             
      if (tmp->client_socket > max_sd) {
         max_sd = tmp->client_socket;
      }

      tmp = tmp->next;
   }

   return max_sd + 1;
}

void handle_io(fd_set *readfds, struct sockaddr_in address, Node **client_sockets) {
   int valread, addrlen = sizeof(address), temp, er = 0;
   unsigned char buffer[BUFFER_LEN], response[3];
   Node *tmp = *client_sockets, *ptr;

   while (tmp) {
      if (FD_ISSET(tmp->client_socket, readfds)) {
         if (!(valread = read(tmp->client_socket, buffer, 1024))) {
            getpeername(tmp->client_socket, ((struct sockaddr*) &address),
               ((socklen_t*) &addrlen));
            printf("Host disconnected, ip %s, port %d\n",
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                 
            close(tmp->client_socket);
            temp = tmp->client_socket;
            tmp = tmp->next;
            delete(client_sockets, temp);
         }
              
         else {
            buffer[valread] = '\0';
            if (buffer[2] == 1) {
               valread = buffer[3];
               memmove(buffer, buffer + 4, valread);
               buffer[valread] = '\0';
               ptr = *client_sockets;
               response[0] = 0;
               response[1] = 3;
               while (ptr) {
                  if (!strcmp((char *) buffer, ptr->handle)) {
                     response[2] = 3;
                     send(tmp->client_socket, response, 3, 0);
                     close(tmp->client_socket);
                     temp = tmp->client_socket;
                     tmp = tmp->next;
                     delete(client_sockets, temp);
                     er = 1;
                     break;
                  }
                  ptr = ptr->next;
               }
               if (!er) {
                  response[2] = 2;
                  send(tmp->client_socket, response, 3, 0);
                  strcpy(tmp->handle, (char *) buffer);
                  tmp = tmp->next;  
               }
               
            }
            else {
               printf("Incoming message: %s\n", buffer);
               tmp = tmp->next;
            }
         }
      }

      else {
         tmp = tmp->next;
      }
   }
}
 
void handle_connections(int server_socket, struct sockaddr_in address) {
   int addrlen, new_socket, activity;
   Node *client_sockets = NULL;
   fd_set readfds;
     
   puts("Waiting for connections ...");
    
   while(1) {
  
      activity = select(initialize_fds(&readfds, server_socket, client_sockets),
         &readfds, NULL, NULL, NULL);
   
      if ((activity < 0) && (errno != EINTR)) {
         printf("select error");
      }
        
      if (FD_ISSET(server_socket, &readfds)) {
         if ((new_socket = accept(server_socket, ((struct sockaddr * ) &address),
            ((socklen_t*) &addrlen))) < 0) {
            perror("accept");
         }
        
         printf("New connection, socket fd is %d, ip is : %s, port : %d \n",
         new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
         add(&client_sockets, new_socket);
      }        
      handle_io(&readfds, address, &client_sockets); 
   }
}
     
  

int main(int argc , char *argv[]) {
   int server_socket = setup_socket();

   handle_connections(server_socket, setup_address(server_socket));
   return 0;
} 
