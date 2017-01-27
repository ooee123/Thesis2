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

#include "networks.h"

// Created static variables because there were so many variables being
// passed between functions. This allows all functions to use this data as it
// pertains to them.
static int32_t arr_size;
static int32_t max;
static int32_t server_socket;
static int8_t *rec_buf;
static int8_t *send_buf;
static struct timeval time;
static Client *sockets;
static fd_set socket_fds;

void initialize_sockets(int32_t size) {
   int32_t count;
   Client c;

   if (NULL == (sockets = calloc(1, size))) {
      perror("malloc() error");
      exit(EXIT_FAILURE);
   }

   c.handle_len = 0;
   c.handle = NULL;
   c.socket = 0;

   for (count = 0; count < size; count++) {
      sockets[count] = c;
   }
}

void initialize_fd_set() {
   int32_t count;

   FD_ZERO(&socket_fds);
   FD_SET(server_socket, &socket_fds);

   for (count = 1; count < max + 1; count++)
      if (0 != sockets[count].socket)
         FD_SET(sockets[count].socket, &socket_fds);
}

void add_socket(int32_t accept_socket) {
   Client client;

   while (accept_socket >= arr_size) {
      if (NULL == (sockets = realloc(sockets, arr_size + SOCK_INCREMENT))) {
         perror("realloc() error");
         exit(EXIT_FAILURE);
      }
      arr_size += SOCK_INCREMENT;
   }

   client.handle_len = 0;
   client.handle = NULL;
   client.socket = accept_socket;
   sockets[accept_socket] = client;
   if (accept_socket > max)
      max = accept_socket;
}

int32_t handle_exists(int32_t index) {
   int32_t count;
   int8_t *temp = &rec_buf[CHAT_HEADER_SIZE + BYTE];
   int8_t size = rec_buf[CHAT_HEADER_SIZE];

   for(count = 0; count < max + 1; count++)
      if(NULL != sockets[count].handle &&
       !strncmp((char *)(sockets[count].handle), (char *)temp, size))
         return TRUE;

   return FALSE;
}

void process_new_client(int32_t index) {
   if (handle_exists(index)) {
      chat_header_setup(&send_buf, FLAG_3, CHAT_HEADER_SIZE);
      send_data(&send_buf, sockets[index].socket, CHAT_HEADER_SIZE);
   }
   else {
      chat_header_setup(&send_buf, FLAG_2, CHAT_HEADER_SIZE);
      send_data(&send_buf, sockets[index].socket, CHAT_HEADER_SIZE);

      sockets[index].handle_len = rec_buf[CHAT_HEADER_SIZE];
      create_buffer((int8_t **)&(sockets[index].handle),
       sockets[index].handle_len);
      memcpy(sockets[index].handle, &rec_buf[CHAT_HEADER_SIZE + BYTE],
       sockets[index].handle_len);
   }
}

void server_handles(int32_t index) {
   int32_t count = 0, ndx, size, increment = CHAT_HEADER_SIZE;

   for (ndx = 0; ndx < max + 1; ndx++)
      if (NULL != sockets[ndx].handle)
         ++count;

   count = htonl(count);
   chat_header_setup(&send_buf, FLAG_11, CHAT_HEADER_SIZE);
   memcpy(send_buf + CHAT_HEADER_SIZE, &count, INTEGER);
   size = CHAT_HEADER_SIZE + INTEGER;
   send_data(&send_buf, sockets[index].socket, size);
   reset(send_buf, MAX_BUF);

   chat_header_setup(&send_buf, FLAG_12, 0);
   for (ndx = 0; ndx < max + 1; ndx++)
      if (NULL != sockets[ndx].handle) {
         memcpy(&send_buf[increment], &sockets[ndx].handle_len, BYTE);
         increment += BYTE;
         memcpy(&send_buf[increment], sockets[ndx].handle,
          sockets[ndx].handle_len);
         increment += sockets[ndx].handle_len;
      }

   send_data(&send_buf, sockets[index].socket, increment);
   reset(send_buf, MAX_BUF);
}

void broadcast_server(int32_t index) {
   int32_t count;

   for (count = 0; count < max + 1; count++) {
      if (count != index && NULL != sockets[count].handle) {
         if ('\n' == rec_buf[0]) {
            printf("Terminating %s...\n", sockets[count].handle);
            chat_header_setup(&send_buf, FLAG_9, CHAT_HEADER_SIZE);
            send_data(&send_buf, sockets[index].socket, CHAT_HEADER_SIZE);
            sockets[index].handle_len = 0;
            if (sockets[index].handle != NULL)
               free(sockets[index].handle);
            sockets[index].handle = NULL;
            sockets[index].socket = 0;
         }
         else {
            send_data(&rec_buf, sockets[count].socket, MAX_BUF);
         }
      }
   }
}

void message_server(int32_t index) {
   int32_t count;
   int8_t *temp;
   int8_t size = rec_buf[CHAT_HEADER_SIZE];

   create_buffer(&temp, MAX_HANDLE_LEN + 1);
   memcpy((char *)temp, (char *)&rec_buf[CHAT_HEADER_SIZE + BYTE], size);
   for (count = 0; count < max + 1; count++)
      if (NULL != sockets[count].handle &&
       !strncmp((char *)sockets[count].handle, (char *)temp, size))
            break;

   if ('\n' == rec_buf[0]) {
      printf("Terminating %s...\n", sockets[index].handle);
      chat_header_setup(&send_buf, FLAG_9, CHAT_HEADER_SIZE);
      send_data(&send_buf, sockets[index].socket, CHAT_HEADER_SIZE);
      sockets[index].handle_len = 0;
      if (sockets[index].handle != NULL)
         free(sockets[index].handle);
      sockets[index].handle = NULL;
      sockets[index].socket = 0;
   }
   else {
      if (!handle_exists(index)) {
         memcpy(&send_buf[CHAT_HEADER_SIZE], &size, BYTE);
         memcpy(&send_buf[CHAT_HEADER_SIZE + BYTE], temp, size);
         size = CHAT_HEADER_SIZE + BYTE + rec_buf[CHAT_HEADER_SIZE];
         chat_header_setup(&send_buf, FLAG_7, size);
         send_data(&send_buf, sockets[index].socket, MAX_BUF);
      }
      else
         send_data(&rec_buf, sockets[count].socket, MAX_BUF);
   }

   free(temp);
}

void process_flag_server(int8_t flag, int32_t index) {
   if (FLAG_1 == flag) {
      process_new_client(index);
   }
   else if (FLAG_4 == flag) {
      broadcast_server(index);
   }
   else if (FLAG_5 == flag) {
      message_server(index);
   }
   else if (FLAG_8 == flag) {
      chat_header_setup(&send_buf, FLAG_9, CHAT_HEADER_SIZE);
      send_data(&send_buf, sockets[index].socket, CHAT_HEADER_SIZE);
   }
   else if (FLAG_10 == flag) {
      server_handles(index);
   }

   reset(rec_buf, MAX_BUF);
   reset(send_buf, MAX_BUF);
}

void handle_returned_sockets(int32_t num_sockets) {
   int32_t accept_socket, count, temp;
   int8_t flag;

   if (num_sockets < 0) {
      perror("select call");
      exit(EXIT_FAILURE);
   }
   else if (0 < num_sockets) {
      if (FD_ISSET(server_socket, &socket_fds)) {
         accept_socket = tcpAccept(server_socket);
         add_socket(accept_socket);
      }

      for (count = 1; count < max + 1; count++) {
         temp = sockets[count].socket;
         if (0 != temp && FD_ISSET(temp, &socket_fds)) {
            flag = check_socket(rec_buf, &sockets[count], SERVER);
            if (FALSE != flag) {
               process_flag_server(flag, count);
            }
         }
      }
   }
}

void server_loop() {
   int32_t num_sockets;

   while (TRUE) {
      initialize_fd_set(max);
      time.tv_sec = 0;
      time.tv_usec = 0;

      num_sockets = tcp_select(&socket_fds, max, time);
      handle_returned_sockets(num_sockets);
   }
}

void initialize_global_variables() {
   create_buffer(&rec_buf, MAX_BUF);
   create_buffer(&send_buf, MAX_BUF);
   initialize_sockets(SOCK_INCREMENT);
   max = server_socket;
   arr_size = SOCK_INCREMENT;
}

int main(int argc, char **argv) {
   // TODO: make server_socket non-blocking?
   server_socket = tcpServerSetup(); //create the server socket
   initialize_global_variables();
   server_loop(server_socket);
   close(server_socket);
   
   return EXIT_SUCCESS;
}
