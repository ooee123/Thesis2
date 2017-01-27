/******************************************************************************
 * tcp_client.c
 *
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

#include "testing.h"
#include "networks.h"
#include "cclient.h"

int main(int argc, char **argv) {
   int socket_num = process_arguments(argc, argv);
   fd_set fds, original;
   char *handle = argv[1];

   init_client(socket_num, &fds, &original);
   send_connection_request(socket_num, argv[1]);
   while (1) {
      data_ready(socket_num, &fds);
      get_server_data(socket_num, &fds);
      get_user_input(socket_num, handle, &fds, &original);
   }
   close(socket_num);
   return 0;
}

void init_client(int socket_number, fd_set *fds, fd_set *original) {
   FD_ZERO(fds);
   FD_SET(0, fds);
   FD_SET(socket_number, fds);
   *original = *fds;
}

int process_arguments(int argc, char **argv) {
   if (argc != 4) {
      printf("usage: %s handle server-name server-port\n", argv[0]);
      exit(-1);
   }

   if (strlen(argv[1]) > 255) {
      perror("error: maximum length is 255 characters");
      exit(-1);
   }

   return tcp_send_setup(argv[2], argv[3]);;
}

void send_connection_request(int socket_number, char *handle) {
   uint8_t *packet = (uint8_t *)calloc(4 + strlen(handle), 1);
   uint16_t message_length = htons(HEADER_SIZE + 1 + strlen(handle));
   uint8_t flag = 1;
   uint8_t handle_length = strlen(handle);

   memcpy(packet, &message_length, 2);
   memcpy(packet + 2, &flag, 1);
   memcpy(packet + 3, &handle_length, 1);
   memcpy(packet + 4, handle, strlen(handle));

   send_to_server(socket_number, (char *)packet, 4 + strlen(handle));

   free(packet);

   if (!get_connection_response(socket_number)) {
      printf("Handle already in use: %s\n", handle);
      close(socket_number);
      exit(-1);
   }
   else {
      printf("$: ");
      fflush(NULL);
   }
}

int get_connection_response(int socket_number) {
   uint8_t packet[3];
   uint8_t flag;

   recv_from_server(socket_number, (char *)packet, HEADER_SIZE);
   memcpy(&flag, packet + 2, 1);

   return flag != 3;
}

int tcp_send_setup(char *host_name, char *port)
{
   int socket_num;
   struct sockaddr_in remote;       // socket address for remote side
   struct hostent *hp;              // address of remote host

   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket call");
      exit(-1);
   }

   // designate the addressing family
   remote.sin_family= AF_INET;

   // get the address of the remote host and store
   if ((hp = gethostbyname(host_name)) == NULL)
   {
     printf("Error getting hostname: %s\n", host_name);
     exit(-1);
   }

   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   // get the port used on the remote side and store
   remote.sin_port= htons(atoi(port));

   if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
   {
      perror("connect call");
      exit(-1);
   }

   return socket_num;
}

void data_ready(int socket_number, fd_set *fds) {
   if (select(socket_number + 1, fds, NULL, NULL, NULL) < 0) {
      perror("Error with select in data_ready\n");
      exit(-1);
   }
}

void get_server_data(int socket_number, fd_set *fds) {
   uint16_t message_length = 0;
   uint8_t flag = 0;
   static unsigned int handle_count = 0;

   if (FD_ISSET(socket_number, fds)) {
      recv_from_server(socket_number, (char *)&message_length, 2);
      recv_from_server(socket_number, (char *)&flag, 1);

      message_length = ntohs(message_length);

      switch (flag) {
         case 4:
            handle_receive_broadcast(socket_number, message_length);
            break;
         case 5:
            handle_receive_message(socket_number, message_length);
            break;
         case 7:
            handle_receive_message_bad(socket_number);
            break;
         case 9:
            quit_chat(socket_number);
         case 11:
            handle_count = get_handle_count(socket_number);
            printf("\n");
            break;
         case 12:
            print_handles(socket_number, handle_count);
            break;
         default:
            printf("\nError invalid flag received\n");
            break;
      }
   }
}

void quit_chat(int socket_number) {
   printf("\n");
   close(socket_number);
   exit(-1);
}

void get_user_input(int socket_number, char *handle, fd_set *fds, fd_set *original) {
   char *buffer = NULL;
   size_t getline_size = 0;
   char command;
   unsigned int buffer_length;

   if (FD_ISSET(0, fds)) {
      if ((buffer_length = getline(&buffer, &getline_size, stdin)) < 0) {
         perror("Error with getline in get_user_input\n");
         exit(-1);
      }

      if (buffer[0] != '%') {
         printf("ERROR: Invalid command\n");
      }
      else {
         command = buffer[1];

         buffer[buffer_length - 1] = 0;
         buffer_length--;

         if (command == 'M' || command == 'm') {
            handle_send_message(socket_number, handle, buffer + 3, buffer_length - 3);
         }
         else if (command == 'B' || command == 'b') {
            handle_send_broadcast(socket_number, handle, buffer + 3, buffer_length - 3);
         }
         else if (command == 'L' || command == 'l') {
            handle_list_handles_command(socket_number);
         }
         else if (command == 'E' || command == 'e') {
            handle_exit_command(socket_number);
         }
         else {
            printf("ERROR: Invalid command\n");
         }
      }

      free(buffer);
   }

   *fds = *original;
   printf("$: ");
   fflush(NULL);
}

void handle_exit_command(int socket_number) {
   uint16_t message_length = htons(HEADER_SIZE);
   uint8_t flag = 8;
   char packet[3];

   // Build exit packet
   memcpy(packet, &message_length, 2);
   memcpy(packet + 2, &flag, 1);

   send_to_server(socket_number, (char *)packet, HEADER_SIZE);
}

void handle_list_handles_command(int socket_number) {
   uint16_t message_length = htons(HEADER_SIZE);
   uint8_t flag = 10;
   char packet[3];

   memcpy(packet, &message_length, 2);
   memcpy(packet + 2, &flag, 1);

   send_to_server(socket_number, (char *)packet, HEADER_SIZE);
}

int get_handle_count(int socket_number) {
   unsigned int packet;

   recv_from_server(socket_number, (char *)&packet, 4);

   return ntohl(packet);
}

void print_handles(int socket_number, unsigned int handle_count) {
   unsigned int i;
   uint8_t handle_length;
   char *handle;

   printf("\nNumber of clients: %d\n", handle_count);
   for (i = 0; i < handle_count; i++) {
      recv_from_server(socket_number, (char *)&handle_length, 1);
      handle = (char *)calloc(handle_length + 1, 1);

      recv_from_server(socket_number, (char *)handle, handle_length);
      handle[handle_length] = 0;

      printf("\t%s\n", handle);
   }
}

void handle_send_broadcast(int socket_number, char *handle, char *buffer, int buffer_length) {
   int amount_sent = 0;
   char *packet;
   uint16_t message_length;
   uint8_t flag = 4;
   uint8_t handle_length = strlen(handle);
   int original_buffer_length = strlen(buffer);

   while (amount_sent < original_buffer_length) {
      if (strlen(buffer) <= MAX_MESSAGE - 1) {
         message_length = htons(strlen(buffer) + 1 + HEADER_SIZE + 1 + handle_length);

         packet = (char *)calloc(HEADER_SIZE + 2 + handle_length + strlen(buffer), 1);
         memcpy(packet, &message_length, 2);
         memcpy(packet + 2, &flag, 1);
         memcpy(packet + 3, &handle_length, 1);
         memcpy(packet + 4, handle, handle_length);
         memcpy(packet + 4 + handle_length, buffer, strlen(buffer));
         packet[4 + handle_length + strlen(buffer)] = 0;

         send_to_server(socket_number, packet, strlen(buffer) + 1 + HEADER_SIZE + 1 + handle_length);
         amount_sent = original_buffer_length;
      }
      else {
         message_length = htons(MAX_MESSAGE + HEADER_SIZE + 1 + handle_length);

         packet = (char *)calloc(HEADER_SIZE + 1 + handle_length + MAX_MESSAGE, 1);
         memcpy(packet, &message_length, 2);
         memcpy(packet + 2, &flag, 1);
         memcpy(packet + 3, &handle_length, 1);
         memcpy(packet + 4, handle, handle_length);
         memcpy(packet + 4 + handle_length, buffer, MAX_MESSAGE - 1);
         packet[4 + handle_length + MAX_MESSAGE - 1] = 0;

         send_to_server(socket_number, packet, HEADER_SIZE + 1 + handle_length + MAX_MESSAGE);
         buffer += MAX_MESSAGE - 1;
         amount_sent += MAX_MESSAGE - 1;
      }
   }
}

void handle_send_message(int socket_number, char *handle, char *buffer, int buffer_length) {
   int amount_sent = 0;
   char *packet;
   uint16_t message_length;
   uint8_t flag = 5;
   uint8_t handle_length = 0;
   char *dest_handle;
   uint8_t src_handle_length = strlen(handle);
   int original_buffer_length;

   while (*(buffer + handle_length) != ' ' && *(buffer + handle_length) != 0) {
      handle_length++;
   }

   dest_handle = (char *)calloc(handle_length, 1);
   memcpy(dest_handle, buffer, handle_length);
   buffer += handle_length + 1;  // Move past the handle and space; buffer should now just be the message
   original_buffer_length = strlen(buffer);

   while (amount_sent < original_buffer_length) {
      if (strlen(buffer) <= MAX_MESSAGE - 1) {
         message_length = htons(strlen(buffer) + 3 + src_handle_length + handle_length + HEADER_SIZE);
         packet = (char *)calloc(HEADER_SIZE + 1 + src_handle_length + 1 + handle_length + strlen(buffer), 1);
         memcpy(packet, &message_length, 2);
         memcpy(packet + 2, &flag, 1);
         memcpy(packet + 3, &handle_length, 1);
         memcpy(packet + 4, dest_handle, handle_length);
         memcpy(packet + 4 + handle_length, &src_handle_length, 1);
         memcpy(packet + 5 + handle_length, handle, src_handle_length);
         memcpy(packet + 5 + handle_length + src_handle_length, buffer, strlen(buffer));

         send_to_server(socket_number, (char *)packet, HEADER_SIZE + 1 + src_handle_length + 1 + handle_length + strlen(buffer));
         amount_sent = original_buffer_length;
      }
      else {
         message_length = htons(MAX_MESSAGE + HEADER_SIZE + 2 + src_handle_length + handle_length);
         packet = (char *)calloc(HEADER_SIZE + 1 + src_handle_length + 1 + handle_length + MAX_MESSAGE, 1);
         memcpy(packet, &message_length, 2);
         memcpy(packet + 2, &flag, 1);
         memcpy(packet + 3, &handle_length, 1);
         memcpy(packet + 4, dest_handle, handle_length);
         memcpy(packet + 4 + handle_length, &src_handle_length, 1);
         memcpy(packet + 5 + handle_length, handle, src_handle_length);
         memcpy(packet + 5 + handle_length + src_handle_length, buffer, MAX_MESSAGE - 1);
         packet[HEADER_SIZE + 1 + handle_length + 1 + src_handle_length + MAX_MESSAGE - 1] = 0;

         send_to_server(socket_number, (char *)packet, HEADER_SIZE + 1 + src_handle_length + 1 + handle_length + MAX_MESSAGE);
         buffer += MAX_MESSAGE - 1;
         amount_sent += MAX_MESSAGE - 1;
      }
   }
}

void handle_receive_broadcast(int socket_number, uint16_t message_length) {
   char *handle;
   uint8_t handle_length;
   char *message;

   recv_from_server(socket_number, (char *)&handle_length, 1);
   handle = (char *)calloc(handle_length + 1, 1);
   recv_from_server(socket_number, handle, handle_length);
   handle[handle_length] = 0;

   message = (char *)calloc(message_length - 1 - handle_length, 1);
   recv_from_server(socket_number, message, message_length - 1 - handle_length);

   printf("\n%s: %s\n", handle, message);
   free(handle);
   free(message);
}

void handle_receive_message(int socket_number, uint16_t message_length) {
   char *dest_handle;
   uint8_t dest_handle_length;
   char *src_handle;
   uint8_t src_handle_length;
   char *message;

   recv_from_server(socket_number, (char *) &dest_handle_length, 1);
   dest_handle = (char *)calloc(dest_handle_length, 1);
   recv_from_server(socket_number, dest_handle, dest_handle_length);

   recv_from_server(socket_number, (char *) &src_handle_length, 1);
   src_handle = (char *)calloc(src_handle_length + 1, 1);
   recv_from_server(socket_number, src_handle, src_handle_length);
   src_handle[src_handle_length] = 0;

   message = (char *)calloc(message_length - 2 - dest_handle_length - src_handle_length, 1);
   recv_from_server(socket_number, message, message_length - 2 - dest_handle_length - src_handle_length);

   printf("\n%s: %s\n", src_handle, message);
   free(message);
   free(dest_handle);
}

void handle_receive_message_bad(int socket_number) {
   char *handle;
   uint8_t handle_length;

   recv_from_server(socket_number, (char *)&handle_length, 1);

   handle = (char *)calloc(handle_length + 1, 1);
   recv_from_server(socket_number, handle, handle_length);
   handle[handle_length] = 0;

   printf("\nClient with handle %s does not exist.\n", handle);
}

void send_to_server(int socket_number, char *data, int data_size) {
   if (send(socket_number, data, data_size, 0) < 0) {
      perror("Error sending to server\n");
      exit(-1);
   }
}

void recv_from_server(int socket_number, char *data, int data_size) {
   int amount_recv;

   if ((amount_recv = recv(socket_number, data, data_size, 0)) < 0) {
      perror("Error recving from server\n");
      exit(-1);
   }

   if (amount_recv == 0) {
      printf("\nServer Terminated\n");
      close(socket_number);
      exit(-1);
   }
}