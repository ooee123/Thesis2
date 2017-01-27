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

#include "networks.h"

void push(node_t *head, char *handle, int sock_num) {
   node_t *current = head;

   while (current->next != NULL) {
      current = current->next;
   }

   current->next = malloc(sizeof(node_t));
   current->next->handle = handle;
   current->next->sock_num = sock_num;
   current->next->next = NULL;
}

void delete_node(node_t **head, node_t *node) {
   node_t *current = *head;

   while (current->next->sock_num != node->sock_num) {
      current = current->next;
   }
   current->next = node->next;
   free(node);
}

int set_blocking(int sock, int block) {
   int flags;

   if ((flags = fcntl(sock, F_GETFL, 0)) < 0) {
      perror("fcnftl(F_GETFL)");
      exit(-1);
   }

   if (block) {
      flags &= ~O_NONBLOCK;
   } else {
      flags |= O_NONBLOCK;
   }
   return fcntl(sock, F_SETFL, flags) != -1;
}

void build_select_list(node_t *head, int *maxsock, fd_set *socks, int sock) {
   node_t *current = head;

   FD_ZERO(socks);

   while (current != NULL) {
      FD_SET(current->sock_num, socks);
      if (current->sock_num > *maxsock) {
         *maxsock = current->sock_num;
      }
      current = current->next;
   }
}

void handle_new_connections(node_t *head, int server_socket) {
   int sockfd;

   sockfd = accept(server_socket, (struct sockaddr*)0, (socklen_t *)0);

   if (sockfd < 0) {
      perror("accept");
      exit(-1);
   }
   set_blocking(sockfd, 0);

   push(head, "_NEW_CONNECTION", sockfd);
}

void close_connection(node_t *head, node_t *node) {
   close(node->sock_num);
   delete_node(&head, node);
}

void handle_flag(int socket, int flag) {
   char *buffer;
   uint16_t len = htons(3);

   if (flag == 2 || flag == 3 || flag == 9) {

      buffer = (char *)malloc(3);

      memcpy(buffer + 0, &len, 2);
      memcpy(buffer + 2, &flag, 1);

      if (send(socket, buffer, ntohs(len), 0) < 0) {
         perror("send\n");
         exit(-1);
      }
   }
}

int confirm_handle(node_t *head, node_t *node, char *buffer) {
   node_t *current = head;
   int handle_len = buffer[3];
   char *handle;

   handle = (char *)malloc(handle_len);
   handle = buffer + 4;

   while (current != NULL) {
      if (strcmp(current->handle, handle) == 0) {
         handle_flag(node->sock_num, 3);
         return -1;
      }
      current = current->next;
   }
   
   node->handle = handle;
   handle_flag(node->sock_num, 2);
   return 0;
}

void broadcast_message(node_t *head, node_t *node, char *buffer) {
   node_t *current = head->next;
   int buf_size = 0;
   
   memcpy(&buf_size, buffer, 2);
   buf_size = ntohs(buf_size);
  
   while (current != NULL) {  
      if (strcmp(current->handle, node->handle) != 0) {
         if (send(current->sock_num, buffer, buf_size, 0) < 0) {
             perror("send");
             exit(-1);
         }
      }
      current = current->next;
   }
   return;
}

void no_dest(node_t *node, char *dest) {
   int buf_len = 0;
   int flag = 7;
   char *buffer;
   uint16_t buf_len_h;

   buf_len = strlen(dest) + 3;
   buf_len_h = htons(buf_len);

   buffer = (char *)malloc(buf_len);
   memcpy(buffer + 0, &buf_len_h, 2);
   memcpy(buffer + 2, &flag, 1);
   memcpy(buffer + 3, dest, strlen(dest));

   send(node->sock_num, buffer, buf_len, 0);
}

void relay_message(node_t *head, node_t *node, char *buffer) {
   node_t *current = head;
   char *dest;
   int dest_len;  
   int buf_len = 0;

   memcpy(&buf_len, buffer, 2);
   buf_len = ntohs(buf_len);

   dest_len = buffer[3];
   dest = (char *)malloc(dest_len + 1);

   memcpy(dest, buffer + 4, dest_len);
   dest[dest_len] = '\0';

   while (current != NULL) {
      if (strcmp(current->handle, dest) == 0) {
         if (send(current->sock_num, buffer, buf_len, 0) < 0) {
            perror("send call");
            exit(-1);
         }
         return;
      }
      current = current->next;
   }

   if (current == NULL) {
      no_dest(node, dest);
   }
   
}

void confirm_exit(node_t *node) {
   int flag = 9;

   handle_flag(node->sock_num, flag);     
}

void send_handles(node_t *head, node_t *node) {
   node_t *current = head->next;
   char *buffer;
   uint16_t len = 0;
   uint8_t flag = 12;
   uint8_t handle_len = 0;
   char *handle_buf;

   set_blocking(node->sock_num, 1);

   buffer = (char *)malloc(3);
   memcpy(buffer, &len, 2);
   memcpy(buffer + 2, &flag, 1);
   if (send(node->sock_num, buffer, 3, 0) < 0) {
      perror("send call");
      exit(-1);
   }

   recv(node->sock_num, buffer, 3, 0);

   while (current != NULL) {
      handle_len = strlen(current->handle);
      handle_buf = (char *)malloc(handle_len + 1);

      memcpy(handle_buf, &handle_len, 1);
      memcpy(handle_buf + 1, current->handle, handle_len);
      if (send(node->sock_num, handle_buf, handle_len + 1, 0) < 0) {
         perror("send call");
         exit(-1);
      }
      current = current->next;
   }
   set_blocking(node->sock_num, 0);
}

void send_list(node_t *head, node_t *node) {
   node_t *current = head->next;
   uint32_t n = 0;
   char *buffer;
   uint16_t len = 7;
   uint8_t flag = 11;
   uint16_t len_h = htons(len);

   while (current != NULL) {
      n++;
      current = current->next;
   }

   n = htonl(n);
   
   buffer = (char *)malloc(len);
   memcpy(buffer, &len_h, 2);
   memcpy(buffer + 2, &flag, 1);
   memcpy(buffer + 3, &n, 4);

   if (send(node->sock_num, buffer, len, 0) < 0) {
      perror("send call");
      exit(-1);
   }
   send_handles(head, node);
}

void deal_with_data(node_t *head, node_t *current) {
   char *buffer;
   int buffer_size = 1024;
   int flag = 0;

   buffer = (char *)malloc(buffer_size);

   if (recv(current->sock_num, buffer, buffer_size, 0) < 0) {
      perror("recv");
      close_connection(head, current);
   } else {
      flag = buffer[2];
      if (flag == 1) {
         //format: 1 byte handle len then handle
         if (confirm_handle(head, current, buffer) < 0) {
            close_connection(head, current);
         }
      } else if (flag == 4){
         //broadcast packet
         broadcast_message(head, current, buffer);
      } else if (flag == 5) {
         //msg packet
         relay_message(head, current, buffer);
      } else if (flag == 8) {
         //client is exiting
         confirm_exit(current);
         close_connection(head, current);
      } else if (flag == 10) {
         //client request list of handles
         send_list(head, current);
      }
   }
}

void read_socks(node_t *head, fd_set *socks, int server_socket) {
   node_t *current = head->next;

   if (FD_ISSET(server_socket, socks)) {
      handle_new_connections(head, server_socket);
   }

   while (current != NULL) {
      if (FD_ISSET(current->sock_num, socks)) {
         deal_with_data(head, current);
      }
      current = current->next;
   }
}

void start_server(int portnum) {
   int readsocks = 0;
   int server_socket = 0;
   node_t *head = NULL;
   fd_set socks;
   int maxsock = 0;
   struct timeval timeout;

   server_socket = tcp_recv_setup(portnum);
   
   listen(server_socket, 5);

   maxsock = server_socket;

   head = malloc(sizeof(node_t));
   head->handle = "server";
   head->sock_num = server_socket;

   while(1) {
      build_select_list(head, &maxsock, &socks, server_socket);
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      readsocks = select(maxsock + 1, &socks, NULL, NULL, &timeout);

      if (readsocks < 0) {
         perror("select");
         exit(-1);
      }
      if (readsocks != 0) {
         read_socks(head, &socks, server_socket);
      }
   }
}

int main(int argc, char *argv[]) {
   char *port;
   int portnum = 0;

   if (argc == 2) {
      port = argv[1];
      portnum = atoi(port);
   }
   start_server(portnum);
   
    return 0;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup(int portnum) {
   int server_socket= 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */
   int reuse_addr = 1;

   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0) {
      perror("socket call");
      exit(1);
   }

   setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
   set_blocking(server_socket, 0);

   local.sin_family= AF_INET;         //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= htons(portnum);                 //let system choose the port

   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
	   exit(-1);
   }
    
   //get the port name and print it out
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0)
     {
	perror("getsockname call");
	exit(-1);
     }

   printf("socket has port %d \n", ntohs(local.sin_port));
	        
   return server_socket;
}
