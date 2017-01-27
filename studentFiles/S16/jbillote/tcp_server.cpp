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

#include <vector>

#include "testing.h"
#include "networks.h"
#include "server.h"


using namespace std;

int main(int argc, char **argv) {
   int server_socket = process_arguments(argc, argv), max_fd;
   vector<struct chat_client> clients; // Dynamic list of clients
   fd_set fds, original;

   init_server(server_socket, &fds, &original, &max_fd);

   while (1) {
      look_for_clients(server_socket, &max_fd, &fds, &original);
      check_sockets(server_socket, &max_fd, &fds, &original, &clients);
   }
   close(server_socket);
   return 0;
}

int process_arguments(int argc, char **argv) {
   int port_number = 0;

   if (argc > 2) {
      printf("usage: %s port-number \n", argv[0]);
   }
   else if (argc == 2) {
      port_number = atoi(argv[1]);
   }

   return tcp_recv_setup(port_number);
}

void init_server(int server_socket, fd_set *fds, fd_set *original, int *max_fd) {
   *max_fd = server_socket;
   FD_ZERO(fds);
   FD_SET(server_socket, fds);
   *original = *fds;
   listen(server_socket, 5);
}

int tcp_recv_setup(int requested_port) {
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
   local.sin_port= htons(requested_port);

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

   printf("server has port %d \n", ntohs(local.sin_port));

   return server_socket;
}

void look_for_clients(int sk, int *max_fd, fd_set *read_fds, fd_set *original) {
   int client_socket = 0;

   if (select((*max_fd) + 1, read_fds, NULL, NULL, NULL) < 0) {
      perror("Error with select in look_for_clients\n");
      exit(-1);
   }

   if (FD_ISSET(sk, read_fds)) {
      if ((client_socket = accept(sk, NULL, NULL)) < 0) {
         perror("Error with accept\n");
         exit(-1);
      }

      FD_SET(client_socket, read_fds);
      FD_SET(client_socket, original);

      if (*max_fd < client_socket) {
         *max_fd = client_socket;
      }
   }
}

void check_sockets(int sk, int *max_fd, fd_set *fds, fd_set *original, vector<struct chat_client> *clients) {
   int amount_received;
   int i;
   char buffer[MAX_SIZE];
   struct chat_header header;

   for (i = 4; i <= *max_fd; i++) {
      if (FD_ISSET(i, fds)) {
         amount_received = 0;

         if ((amount_received = recv(i, buffer, HEADER_SIZE - amount_received, 0)) < 0) {
            perror("Error with recv in check_sockets\n");
            exit(-1);
         }

         if (amount_received == 0) {
            remove_fd(i, clients, fds, original, max_fd);
         }
         else {
            get_header(buffer, &header);

            if (process_packet(i, &header, clients)) {
               remove_fd(i, clients, fds, original, max_fd);
            }
         }
      }
   }

   *fds = *original;
}

void remove_fd(int socket_number, vector<struct chat_client> *clients, fd_set *fds, fd_set *original, int *max_fd) {
   unsigned int i;
   int found = 0;

   for (i = 0; i < clients->size() && !found; i++) {
      if (clients->at(i).fd == socket_number) {
         found = 1;
         free(clients->at(i).handle);
         clients->erase(clients->begin() + i);
      }
   }

   close(socket_number);
   FD_CLR(socket_number, fds);
   FD_CLR(socket_number, original);

   if (socket_number == *max_fd) {
      (*max_fd)--;
   }
}

void get_header(char *buffer, struct chat_header *header) {
   memcpy(&header->message_length, buffer, 2);
   memcpy(&header->flag, buffer + 2, 1);

   header->message_length = ntohs(header->message_length);
}

int process_packet(int client_socket, struct chat_header *header, vector<chat_client> *clients) {
   switch(header->flag) {
      case 1:
         return handle_new_client(client_socket, clients);
      case 4:
         handle_sent_broadcast(client_socket, header, clients);
         break;
      case 5:
         handle_sent_message(client_socket, header, clients);
         break;
      case 8:
         return handle_exit_request(client_socket);
         break;
      case 10:
         handle_list_handles_request(client_socket, clients);
         break;
      default:
         printf("Error unknown flag\n");
         exit(-1);
   }

   return 0;
}

int handle_new_client(int client_socket, vector<chat_client> *clients) {
   char *handle_name;
   uint8_t handle_length = 0;
   struct chat_client client;

   recv_from_client(client_socket, (char *)&handle_length, 1);

   handle_name = (char *)malloc(handle_length + 1);

   recv_from_client(client_socket, handle_name, handle_length);
   handle_name[handle_length] = 0;

   if (!check_handles(clients, handle_name)) {
      send_new_client_response(client_socket, 0);

      client.fd = client_socket;
      client.handle = (char *)malloc(strlen(handle_name) + 1);
      strcpy(client.handle, handle_name);

      clients->push_back(client);

      free(handle_name);

      return 0;
   }
   else {
      send_new_client_response(client_socket, 1);

      free(handle_name);

      return 1;
   }
}

void handle_list_handles_request(int client_socket, vector<struct chat_client> *clients) {
   uint16_t message_length = 7;
   uint8_t flag = 11;
   char *packet;
   uint32_t handle_count = htonl(clients->size());
   unsigned int ndx;
   unsigned int current_packet_size;
   uint8_t current_handle_length;

   packet = (char *)malloc(7);
   message_length = ntohs(message_length);
   memcpy(packet, &message_length, 2);
   memcpy(packet + 2, &flag, 1);
   memcpy(packet + 3, &handle_count, 4);

   send_to_client(client_socket, packet, 7);

   free(packet);

   message_length = 0;
   flag = 12;
   packet = (char *)malloc(3);
   memcpy(packet, &message_length, 2);
   memcpy(packet + 2, &flag, 1);
   current_packet_size = 3;

   for (ndx = 0; ndx < clients->size(); ndx++) {
      packet = (char *)realloc(packet, current_packet_size + 1);
      current_handle_length = strlen(clients->at(ndx).handle);
      memcpy(packet + current_packet_size, &current_handle_length, 1);
      current_packet_size++;

      packet = (char *)realloc(packet, current_packet_size + current_handle_length);
      memcpy(packet + current_packet_size, clients->at(ndx).handle, current_handle_length);
      current_packet_size += current_handle_length;
   }

   send_to_client(client_socket, packet, current_packet_size);

   free(packet);
}

int handle_exit_request(int client_socket) {
   uint16_t message_length = 3;
   uint8_t flag = 9;
   char packet[HEADER_SIZE];

   message_length = ntohs(message_length);
   memcpy(packet, &message_length, 2);
   memcpy(packet + 2, &flag, 1);

   send_to_client(client_socket, packet, HEADER_SIZE);

   return 1;
}

void send_new_client_response(int client_socket, int response) {
   uint16_t message_length = 3;
   uint8_t flag;
   char packet[HEADER_SIZE];

   message_length = ntohs(message_length);
   memcpy(packet, &message_length, 2);

   if (response == 0) {
      flag = 2;
   }
   else {
      flag = 3;
   }

   memcpy(packet + 2, &flag, 1);

   send_to_client(client_socket, packet, HEADER_SIZE);
}

int check_handles(vector<struct chat_client> *clients, char *handle_name) {
   int found = 0;
   unsigned int ndx;

   for (ndx = 0; ndx < clients->size() && !found; ndx++) {
      if (!strcmp(handle_name, clients->at(ndx).handle)) {
         found = 1;
      }
   }

   return found;
}

void handle_sent_broadcast(int client_socket, struct chat_header *header, vector<struct chat_client> *clients) {
   char *message;
   uint8_t handle_length;
   char *handle;
   unsigned int ndx;

   recv_from_client(client_socket, (char *)&handle_length, 1);
   handle = (char *)malloc(handle_length + 1);
   recv_from_client(client_socket, handle, handle_length);
   handle[handle_length] = 0;

   message = (char *)calloc(ntohs(header->message_length) - HEADER_SIZE - 1 - handle_length, 1);

   recv_from_client(client_socket, message, ntohs(header->message_length) - HEADER_SIZE - 1 - handle_length);

   for (ndx = 0; ndx < clients->size(); ndx++) {
      if (clients->at(ndx).fd != client_socket) {
         send_broadcast(clients->at(ndx).fd, header, handle, message);
      }
   }
}

void send_broadcast(int client_socket, struct chat_header *header, char *handle, char *message) {
   char *packet;
   uint8_t handle_length = strlen(handle);
   uint16_t message_length = htons(HEADER_SIZE + 1 + strlen(handle) + strlen(message) + 1);
   uint8_t flag = 4;

   packet = (char *)malloc(ntohs(message_length));
   memcpy(packet, &message_length, 2);
   memcpy(packet + 2, &flag, 1);
   memcpy(packet + 3, &handle_length, 1);
   memcpy(packet + 4, handle, handle_length);
   memcpy(packet + 4 + handle_length, message, strlen(message));
   packet[4 + handle_length + strlen(message)] = 0;

   send_to_client(client_socket, packet, ntohs(message_length) + 2);
   free(packet);
}

void handle_sent_message(int client_socket, struct chat_header *header, vector<struct chat_client> *clients) {
   char *message;
   int handle_index;
   char *src_handle;
   char *dest_handle;
   uint8_t src_handle_length;
   uint8_t dest_handle_length;

   recv_from_client(client_socket, (char *)&dest_handle_length, 1);
   dest_handle = (char *)malloc(dest_handle_length + 1);
   recv_from_client(client_socket, dest_handle, dest_handle_length);
   dest_handle[dest_handle_length] = 0;

   recv_from_client(client_socket, (char *)&src_handle_length, 1);
   src_handle = (char *)malloc(src_handle_length + 1);
   recv_from_client(client_socket, src_handle, src_handle_length);
   src_handle[src_handle_length] = 0;

   message = (char *)malloc(ntohs(header->message_length) - HEADER_SIZE - 2 - src_handle_length - dest_handle_length);

   recv_from_client(client_socket, message, ntohs(header->message_length) - HEADER_SIZE - 2 - src_handle_length - dest_handle_length);

   handle_index = get_handle_index(clients, dest_handle);

   if (handle_index == -1) {
      send_message(client_socket, header, src_handle, dest_handle, NULL, 0);
   }
   else {
      send_message(clients->at(handle_index).fd, header, src_handle, dest_handle, message, 1);
   }
}

int get_handle_index(vector<struct chat_client> *clients, char *handle) {
   int found_index = -1;
   unsigned int ndx;

   for (ndx = 0; ndx < clients->size() && found_index == -1; ndx++) {
      if (!strcmp(handle, clients->at(ndx).handle)) {
         found_index = ndx;
      }
   }

   return found_index;
}

void send_message(int client_socket, struct chat_header *header, char *src_handle, char *dest_handle, char *message, int response) {
   char *packet;
   uint16_t message_length;
   uint8_t flag;
   uint8_t src_handle_length;
   uint8_t dest_handle_length;
   int packet_size;

   if (!response) {
      packet = (char *)malloc(HEADER_SIZE + 1 + strlen(dest_handle));
      message_length = htons(HEADER_SIZE + 1 + strlen(dest_handle));
      flag = 7;
      dest_handle_length = strlen(dest_handle);

      memcpy(packet, &message_length, 2);
      memcpy(packet + 2, &flag, 1);
      memcpy(packet + 3, &dest_handle_length, 1);
      memcpy(packet + 4, dest_handle, dest_handle_length);

      packet_size = ntohs(message_length);
   }
   else {
      packet = (char *)malloc(HEADER_SIZE + 1 + strlen(dest_handle) + 1 + strlen(src_handle) + strlen(message) + 1);
      message_length = htons(HEADER_SIZE + 1 + strlen(dest_handle) + 1 + strlen(src_handle) + strlen(message) + 1);
      flag = 5;
      dest_handle_length = strlen(dest_handle);
      src_handle_length = strlen(src_handle);

      memcpy(packet, &message_length, 2);
      memcpy(packet + 2, &flag, 1);
      memcpy(packet + 3, &dest_handle_length, 1);
      memcpy(packet + 4, dest_handle, dest_handle_length);
      memcpy(packet + 4 + dest_handle_length, &src_handle_length, 1);
      memcpy(packet + 5 + dest_handle_length, src_handle, src_handle_length);
      memcpy(packet + 5 + dest_handle_length + src_handle_length, message, strlen(message) + 1);

      packet_size = ntohs(message_length);
   }

   send_to_client(client_socket, packet, packet_size);
   free(packet);
}

void send_to_client(int socket_number, char *data, int data_size) {
   if (send(socket_number, data, data_size, 0) < 0) {
      perror("Error sending to client\n");
      exit(-1);
   }
}

void recv_from_client(int socket_number, char *data, int data_size) {
   if (recv(socket_number, data, data_size, 0) < 0) {
      perror("Error recving from client\n");
      exit(-1);
   }
}