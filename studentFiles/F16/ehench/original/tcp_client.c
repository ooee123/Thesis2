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

#include "networks.h"

// Created static variables because there were so many variables being
// passed between functions. This allows all functions to use this data as it
// pertains to them.
static int32_t server_socket;
static int32_t num_clients;
static int8_t *handle;
static int8_t *rec_buf;
static int8_t *send_buf;
static char *input_buf;
static struct timeval time;
static fd_set socket_fds;

void check_cmdline(int argc, char **argv) {
   if (EXPECTED_ARGUMENTS != argc) {
      printf("usage: %s handle server-name server-port\n", argv[0]);
      exit(EXIT_FAILURE);
   }
}

int16_t get_handle(char **argv) {
   int8_t *temp;
   int16_t size = 0;

   temp = memcpy(handle, argv[HANDLE], MAX_HANDLE_LEN);
   while (temp[size] != '\0')
      ++size;

   return size;
}

void initialize_data() {
   time.tv_sec = 0;
   time.tv_usec = 0;

   FD_ZERO(&socket_fds);
   FD_SET(server_socket, &socket_fds);
   FD_SET(STDIN_FILENO, &socket_fds);
}

void process_flag_client(int8_t flag, Client *socket) {
   char *buffer;
   int32_t index, increment = CHAT_HEADER_SIZE;
   int32_t msg;
   int8_t handle_len, other_len;

   create_buffer((int8_t **)&buffer, MAX_BUF);

   // Don't need to handle FLAG_2 obviously
   if (FLAG_2 == flag) {

   }
   else if (FLAG_3 == flag) {
      printf("Handle already in use: %s\n", socket->handle);
      exit(EXIT_FAILURE);
   }
   else if (FLAG_4 == flag) {
      handle_len = rec_buf[CHAT_HEADER_SIZE];
      msg = CHAT_HEADER_SIZE + BYTE + handle_len;
      memcpy(buffer, &rec_buf[CHAT_HEADER_SIZE + BYTE], handle_len);
      printf("\n%s: %s\n", buffer, &rec_buf[msg]);
      free(buffer);
   }
   else if (FLAG_5 == flag) {
      handle_len = rec_buf[CHAT_HEADER_SIZE];
      other_len = rec_buf[CHAT_HEADER_SIZE + BYTE + handle_len];
      msg = CHAT_HEADER_SIZE + 2 * BYTE + handle_len + other_len;
      memcpy(buffer, &rec_buf[CHAT_HEADER_SIZE + 2 * BYTE + handle_len],
       other_len);
      printf("\n%s: %s\n", buffer, &rec_buf[msg]);
      free(buffer);
   }
   else if (FLAG_7 == flag) {
      memcpy(buffer, &rec_buf[CHAT_HEADER_SIZE + BYTE],
       rec_buf[CHAT_HEADER_SIZE]);
      printf("Client with handle %s does not exist\n", buffer);
      free(buffer);
   }
   else if (FLAG_9 == flag) {
      exit(EXIT_SUCCESS);
   }
   else if (FLAG_11 == flag) {
      printf("Number of clients: ");
      num_clients = ntohl(*(int32_t *)(rec_buf + CHAT_HEADER_SIZE));
      printf("%d\n", num_clients);
   }
   else if (FLAG_12 == flag) {
      printf("\n");
      for (index = 0; index < num_clients; index++) {
         handle_len = rec_buf[increment];
         memcpy(buffer, rec_buf + increment + BYTE, rec_buf[increment]);
         increment += BYTE + rec_buf[increment];
         printf("       %s\n", buffer);
         reset((int8_t *)buffer, MAX_BUF);
      }
      free(buffer);
      num_clients = 0;
   }

   reset(rec_buf, MAX_BUF);
   reset(send_buf, MAX_BUF);
}

void client_initial_packet(Client *socket, int8_t handle_len) {
   int32_t num_sockets;
   int8_t flag;

   chat_header_setup(&send_buf, FLAG_1, CHAT_HEADER_SIZE + BYTE + handle_len);
   memcpy(&send_buf[CHAT_HEADER_SIZE], &handle_len, BYTE);
   memcpy(&send_buf[CHAT_HEADER_SIZE + BYTE], handle, handle_len);
   send_data(&send_buf, server_socket, CHAT_HEADER_SIZE + BYTE + handle_len);
   reset(send_buf, MAX_BUF);

   while (TRUE) {
      initialize_data();
      num_sockets = tcp_select(&socket_fds, server_socket + 1, time);
      if (num_sockets < 0) {
         perror("select call");
         exit(EXIT_FAILURE);
      }
      else if (0 < num_sockets && FD_ISSET(server_socket, &socket_fds)) {
         flag = check_socket(rec_buf, socket, CLIENT);
         if (FALSE != flag)
            process_flag_client(flag, socket);
         break;
      }
   }

   reset(rec_buf, MAX_BUF);
   reset(send_buf, MAX_BUF);
}

void initialize_socket(Client *socket, int8_t handle_len) {

   socket->handle_len = handle_len;
   create_buffer(&(socket->handle), MAX_HANDLE_LEN + 1);
   memcpy(socket->handle, handle, MAX_HANDLE_LEN);
   socket->socket = server_socket;
}

void client_exit() {
   int8_t *buffer;

   create_buffer(&buffer, CHAT_HEADER_SIZE);
   chat_header_setup(&buffer, FLAG_8, CHAT_HEADER_SIZE);
   send_data(&buffer, server_socket, CHAT_HEADER_SIZE);
}

void list_handles() {
   int8_t *buffer;

   create_buffer(&buffer, CHAT_HEADER_SIZE);
   chat_header_setup(&buffer, FLAG_10, CHAT_HEADER_SIZE);
   send_data(&buffer, server_socket, CHAT_HEADER_SIZE);
}

void broadcast(Client *socket) {
   Broadcast message;
   int32_t count = 0;
   int16_t size = 0;
   char *temp;
   char *tok;

   while ('%' != input_buf[count])
      ++count;
   count += 2;

   create_buffer((int8_t **)&temp, MAX_BUF);
   memcpy(temp, input_buf, MAX_BUF);
   tok = strtok(temp, " ");

   tok = strtok(NULL, " ");
   if (NULL == tok)
      printf("Invalid Command\n");
   else {
      message.sender_len = socket->handle_len;
      message.sender_handle = socket->handle;

      memcpy(send_buf + CHAT_HEADER_SIZE, &message.sender_len, BYTE);

      memcpy(send_buf + CHAT_HEADER_SIZE + BYTE, message.sender_handle,
       message.sender_len);

      memcpy(send_buf + CHAT_HEADER_SIZE + BYTE + message.sender_len,
       &input_buf[count], MAX_MESSAGE_LEN);

      size = CHAT_HEADER_SIZE + BYTE + message.sender_len + strlen(input_buf);
      chat_header_setup(&send_buf, FLAG_4, size);
      send_buf[size - 1] = '\0';
      send_data(&send_buf, server_socket, size);
   }
}

void message(Client *socket) {
   Message message;
   int32_t adder = 0, count = 0;
   int16_t size = 0;
   char *temp;
   char *tok;

   while ('%' != input_buf[count])
      ++count;
   count += 2;

   create_buffer((int8_t **)&temp, MAX_BUF);
   memcpy(temp, input_buf, MAX_BUF);
   tok = strtok(temp, " ");

   tok = strtok(NULL, " ");
   if (NULL == tok) 
      printf("Invalid Command\n");
   else {
      message.dest_len = strlen(tok);
      memcpy(send_buf + CHAT_HEADER_SIZE, &message.dest_len, BYTE);
      memcpy(send_buf + CHAT_HEADER_SIZE + BYTE, tok, message.dest_len);
      adder = message.dest_len + BYTE;

      tok = strtok(NULL, " ");
      if (NULL == tok) 
         printf("Invalid Command\n");
      else {
         message.sender_len = socket->handle_len;
         message.sender_handle = socket->handle;
         memcpy(send_buf + CHAT_HEADER_SIZE + adder, &message.sender_len,
          BYTE);

         memcpy(send_buf + CHAT_HEADER_SIZE + BYTE + adder,
          message.sender_handle, message.sender_len);

         memcpy(send_buf + CHAT_HEADER_SIZE + BYTE + message.sender_len +
          adder, &input_buf[count + message.dest_len + 1], MAX_MESSAGE_LEN);

         size = CHAT_HEADER_SIZE + 2 * BYTE + message.sender_len +
          message.dest_len + strlen(input_buf);
         chat_header_setup(&send_buf, FLAG_5, size);
         send_buf[size - 1] = '\0';
         send_data(&send_buf, server_socket, size);
      }
   }
}

int32_t analyze_command(Client *socket) {
   int32_t check = TRUE, count = 0;

   while(' ' == input_buf[count])
      ++count;

   if ('%' == input_buf[count++]) {
      if ('M' == input_buf[count] || 'm' == input_buf[count])
         if (' ' == input_buf[count + 1])
            message(socket);
         else
            check = FALSE;
      else if ('B' == input_buf[count] || 'b' == input_buf[count])
         if (' ' == input_buf[count + 1])
            broadcast(socket);
         else
            check = FALSE;
      else if ('L' == input_buf[count] || 'l' == input_buf[count])
         list_handles(server_socket);
      else if ('E' == input_buf[count] || 'e' == input_buf[count])
         client_exit(server_socket);
      else
         check = FALSE;
   }
   else
      check = FALSE;

   return check;
}

int32_t client_sockets(Client *socket) {
   int32_t num_sockets, check = FALSE;
   int8_t flag;

   num_sockets = tcp_select(&socket_fds, server_socket + 1, time);

   if (num_sockets < 0) {
      perror("select call");
      exit(EXIT_FAILURE);
   }
   else if (0 < num_sockets) {
      if (FD_ISSET(STDIN_FILENO, &socket_fds)) {
         if (0 >= fgets(input_buf, MAX_BUF, stdin)) {
            perror("fgets stdin");
            exit(EXIT_FAILURE);
         }
         if (!analyze_command(socket))
            printf("Invalid Command\n");
         check = TRUE;
         reset((int8_t *)input_buf, MAX_BUF);
      }
      if (FD_ISSET(server_socket, &socket_fds)) {
         flag = check_socket(rec_buf, socket, CLIENT);
         if (FALSE != flag) {
            process_flag_client(flag, socket);
            check = TRUE;
         }
      }
   }

   return check;
}

void client_loop(int8_t handle_len) {
   Client socket;
   int8_t sock_check = TRUE;

   /* set up the TCP Client socket  */
   initialize_socket(&socket, handle_len);
   client_initial_packet(&socket, handle_len);

   while (TRUE) {
      if (sock_check) {
         printf("$: ");
         fflush(stdout);
      }

      initialize_data(server_socket);
      sock_check = client_sockets(&socket);
   }   

   close(server_socket);
}

void initialize_global_variables(char **argv) {
   server_socket = tcpClientSetup(argv[SERVER_NAME], argv[SERVER_PORT]);
   create_buffer(&handle, MAX_HANDLE_LEN + 1);
   create_buffer((int8_t **)&input_buf, MAX_BUF);
   create_buffer(&rec_buf, MAX_BUF);
   create_buffer(&send_buf, MAX_BUF);
}

int main(int argc, char **argv) {
   int8_t handle_len;

   check_cmdline(argc, argv);
   initialize_global_variables(argv);
   handle_len = get_handle(argv);
   client_loop(handle_len);
   
   return EXIT_SUCCESS;
}
