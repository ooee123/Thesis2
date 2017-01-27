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

#define MAX_BUFFER 1259
#define MAX_MESSAGE_LEN 1000
#define MAX_HANDLE 255
#define COMMAND_SIZE 3
#define STDIN 0
#define FLAG1 1
#define FLAG2 2
#define FLAG3 3
#define FLAG4 4
#define FLAG5 5
#define FLAG6 6
#define FLAG7 7
#define FLAG8 8
#define FLAG9 9
#define FLAG10 10
#define FLAG11 11
#define FLAG12 12
#define CHATHEAD_SIZE 3
#define HANDLEN_SIZE 1

//void GetSendData(int server_socket);
void CreateSendFlag1(char *handle, int server_socket);
int ServerReply(int server_socket);
int my_Select(int server_socket, char *handle, char *data_buf);

// Create connection
int ConnectionSetup(int argc, char *argv[], char **data_buf, int *server_socket, char **handle) {
   int flag = 0;

   // check command line arguments
   if (argc != 4) {
      printf("usage: %s handle server-name server-port \n", argv[0]);
      exit(-1);
   }

   if ((*data_buf = (char *) malloc(MAX_BUFFER)) == NULL) {
      perror("malloc() error");
      exit(-1);
   }

   if ((*handle = (char *) malloc(MAX_HANDLE)) == NULL) {
      perror("malloc() error");
      exit(-1);
   }

   strcpy(*handle, argv[1]);

   // set up the TCP Client socket
   *server_socket = tcpClientSetup(argv[2], argv[3]);

   // create send flag 1 header
   CreateSendFlag1(*handle, *server_socket);

   // wait for server's reply
   flag = ServerReply(*server_socket);

   return flag;
}

int ServerReply(int server_socket) {
   fd_set fdvar;
   int message_len;
   char data_buf[MAX_BUFFER];
   char *temp = data_buf;
   uint16_t pack_len;
   uint8_t flag;
   struct timeval tv;

   FD_ZERO(&fdvar);
   FD_SET(server_socket, &fdvar);

   // wait 5 seconds
   tv.tv_sec = 5;
   tv.tv_usec = 0;

   if (select(server_socket + 1, &fdvar, NULL, NULL, &tv) < 0) {
      perror("select call");
      exit(-1);
   }

   if ((message_len = recv(server_socket, data_buf, MAX_BUFFER, 0)) <= 0) {
      if (message_len < 0) {
         perror("recv call");
         exit(-1);
      }

      if (close(server_socket) < 0) {
         perror("close call");
         exit(-1);
      }
   }

   temp += sizeof(pack_len);

   memcpy(&flag, temp, sizeof(flag));

   return flag;
}

void CreateSendFlag1(char *handle, int server_socket) {
   uint16_t pack_length, pack_len_net;
   uint8_t flag = 1;
   uint8_t hand_length = strlen(handle);
   char *data_buf = malloc(MAX_BUFFER);
   char *temp = data_buf;

   // create header flag 1
   pack_length = sizeof(pack_length) + sizeof(flag) + sizeof(hand_length) + hand_length;
   pack_len_net = htons(pack_length);

   memcpy(temp, &pack_len_net, sizeof(pack_length));
   temp += sizeof(pack_length);

   memcpy(temp, &flag, sizeof(flag));
   temp += sizeof(flag);

   memcpy(temp, &hand_length, sizeof(hand_length));
   temp += sizeof(hand_length);

   memcpy(temp, handle, hand_length);

   // send flag 1 to server
   send(server_socket, data_buf, pack_length, 0);

   free(data_buf);
   data_buf = NULL;
}

void AddChatHeader(char *data_buf, int message_len, int header_len, uint8_t flag) {
   char *temp = data_buf;
   uint16_t pack_len, pack_len_net;

   pack_len = message_len + header_len;
   pack_len_net = htons(pack_len);
   memcpy(temp, &pack_len_net, sizeof(pack_len));
   temp += sizeof(pack_len);

   memcpy(temp, &flag, sizeof(flag));
   temp += sizeof(flag);
}

int CreateFlag5(char *handle_dest, char *handle_src, char *data_buf, int message_len) {
   int header_len;
   char *temp = data_buf;
   uint8_t handleLen_dest = strlen(handle_dest);
   uint8_t handleLen_src = strlen(handle_src);

   header_len = CHATHEAD_SIZE + HANDLEN_SIZE + HANDLEN_SIZE + handleLen_dest + handleLen_src;

   // add chat-header
   AddChatHeader(data_buf, message_len, header_len, 5);
   temp += CHATHEAD_SIZE;

   // add handle length, handle (Dest)
   memcpy(temp, &handleLen_dest, HANDLEN_SIZE);
   temp += HANDLEN_SIZE;
   memcpy(temp, handle_dest, handleLen_dest);
   temp += handleLen_dest;

   // add handle length, handle (Src)
   memcpy(temp, &handleLen_src, HANDLEN_SIZE);
   temp += HANDLEN_SIZE;
   memcpy(temp, handle_src, handleLen_src);

   return header_len;
}

// return header length, put header in **data_buf
int CreateFlag4(char *handle, char *data_buf, int message_len) {
   int header_len;
   char *temp = data_buf;
   uint8_t handle_len = strlen(handle);

   header_len = CHATHEAD_SIZE + HANDLEN_SIZE + handle_len;

   // add chat-header
   AddChatHeader(data_buf, message_len, header_len, FLAG4);
   temp += CHATHEAD_SIZE;

   // add handle length
   memcpy(temp, &handle_len, sizeof(handle_len));
   temp += HANDLEN_SIZE;

   // add handle
   memcpy(temp, handle, handle_len);

   return header_len;
}

int GetMessage(char **message_buf) {
   int message_len = 0;

   // get data
   while (((*message_buf)[message_len] = getchar()) != '\n' && message_len < MAX_MESSAGE_LEN) {
      if ((*message_buf)[message_len] != ' ') {
         message_len++;
      }
   }
   (*message_buf)[message_len++] = '\0';

   return message_len;
}

int GetInput(char *handle, char **data_buf) {
   char *message_buf = malloc(MAX_MESSAGE_LEN + 1);
   char command[COMMAND_SIZE + 1];
   char handle_dest[MAX_HANDLE];
   int headerLen, message_len, pack_len;

   headerLen = 0;
   message_len = 0;
   scanf("%s", command);

   if (command[0] == '%') {
      if (command[1] == 'B' || command[1] == 'b') { // broadcast
         message_len = GetMessage(&message_buf);

         // create flag 4 header
         headerLen = CreateFlag4(handle, *data_buf, message_len);

         // add header to the data
         memcpy((*data_buf) + headerLen, message_buf, message_len);
      }
      else if (command[1] == 'M' || command[1] == 'm') { // direct message
         // get handle_dest
         scanf("%s", handle_dest);
         message_len = GetMessage(&message_buf);

         // create flag 5 header
         headerLen = CreateFlag5(handle_dest, handle, *data_buf, message_len);

         // add header to the data
         memcpy((*data_buf) + headerLen, message_buf, message_len);
      }
      else if (command[1] == 'L' || command[1] == 'l') { // request list
         // create flag 10 header
         message_len = 0;
         headerLen = CHATHEAD_SIZE;
         AddChatHeader(*data_buf, message_len, CHATHEAD_SIZE, FLAG10);
      }
      else if (command[1] == 'E' || command[1] == 'e') { // request exit
         // create flag 8 header
         message_len = 0;
         headerLen = CHATHEAD_SIZE;
         AddChatHeader(*data_buf, message_len, CHATHEAD_SIZE, FLAG8);
      }
      else {
         printf("Invalid command\n");
      }
   }
   else {
      printf("Invalid command\n");
   }

   // free malloc
   free(message_buf);
   message_buf = NULL;

   pack_len = headerLen + message_len;

   return pack_len;
}

void InterpreteFlag4(char *data_buf) {
   uint8_t handle_len;
   char handle[MAX_HANDLE + 1];
   int i;

   handle_len = *data_buf;
   data_buf += sizeof(handle_len);

   for (i = 0; i < handle_len; i++) {
      handle[i] = *data_buf;
      data_buf++;
   }

   handle[i] = '\0';

   printf("\n%s: %s\n", handle, data_buf);
}

void InterpreteFlag5(char *data_buf) {
   uint8_t handle_len;
   char handle[MAX_HANDLE + 1];
   int i;

   // skip handle dest
   handle_len = *data_buf;
   data_buf += sizeof(handle_len);
   data_buf += handle_len;

   // get handle src
   handle_len = *data_buf;
   data_buf += sizeof(handle_len);
   for (i = 0; i < handle_len; i++) {
      handle[i] = *data_buf;
      data_buf++;
   }
   handle[i] = '\0';

   printf("\n%s: %s\n", handle, data_buf);
}

void InterpreteFlag7(char *data_buf) {
   uint8_t handle_len;
   char handle[MAX_HANDLE + 1];
   int i;

   handle_len = *data_buf;
   data_buf += sizeof(handle_len);

   for (i = 0; i < handle_len; i++) {
      handle[i] = *data_buf;
      data_buf++;
   }
   handle[i] = '\0';

   printf("Client with handle %s does not exist\n", handle);
}

void InterpreteFlag11(char *data_buf, uint32_t *handle_numb) {
   memcpy(handle_numb, data_buf, sizeof(handle_numb));
   *handle_numb = ntohs(*handle_numb);
   printf("Number of clients: %zu\n", (*handle_numb));
}

void InterpreteFlag12(uint32_t handle_numb, char *data_buf) {
   uint8_t handle_len;
   char handle[MAX_HANDLE + 1];
   int i, j;

   for (j = 0; j < handle_numb; j++) {
      handle_len = *data_buf;
      data_buf += sizeof(handle_len);

      for (i = 0; i < handle_len; i++) {
         handle[i] = *data_buf;
         data_buf++;
      }
      handle[i] = '\0';

      printf("\t%s\n", handle);
   }
}

int InterpreteData(char *data_buf, int server_socket, uint32_t *handle_numb) {
   uint16_t pack_len, pack_len_net;
   uint8_t flag;
   char *temp = data_buf;
   int isExit = 0;

   // get chat-header information
   pack_len_net = *temp;
   pack_len = ntohs(pack_len_net);
   temp += sizeof(pack_len);

   flag = *temp;
   temp += sizeof(flag);

   if (flag == FLAG4) { // direct message from server
      InterpreteFlag4(temp);
   }
   else if (flag == FLAG5) { // broadcast from server
      InterpreteFlag5(temp);
   }
   else if (flag == FLAG7) { // direct message reply: no handle
      InterpreteFlag7(temp);
   }
   else if (flag == FLAG11) { // list reply: number of clients
      InterpreteFlag11(temp, handle_numb);
   }
   else if (flag == FLAG12) { // list reply: handle names
      InterpreteFlag12(*handle_numb, temp); 
   }
   else if (flag == FLAG9) { // disconect reply
      isExit = 1;
   }
   return isExit;
}

int MySelect(int server_socket, char *handle, char *data_buf, uint32_t *handle_numb) {
   fd_set fdvar;
   int pack_len;
   int message_len;
   int isExit = 0;

   // set fd_set
   FD_ZERO(&fdvar);
   FD_SET(STDIN, &fdvar);
   FD_SET(server_socket, &fdvar);

   // select to wait block here
   if (select(server_socket + 1, &fdvar, NULL, NULL, NULL) < 0) {
      perror("select call");
      exit(-1);
   }

   // check if any input from stdin
   if (FD_ISSET(STDIN, &fdvar)) {
      // get input data
      pack_len = GetInput(handle, &data_buf);
      if (pack_len) {
         if (send(server_socket, data_buf, pack_len, 0) < 0) {
            perror("send call");
            exit(-1);
         }
      }
   }

   // check if message from server
   if (FD_ISSET(server_socket, &fdvar)) {
      // check if recv error or server is disconnected
      if ((message_len = recv(server_socket, data_buf, MAX_BUFFER, 0)) <= 0) {
         if (message_len < 0) {
            perror("recv call");
            exit(-1);
         }

         if (close(server_socket) < 0) {
            perror("close call");
            exit(-1);
         }

         printf("Server Termiated\n");
         exit(-1);
      }
      else {
         // interpret recevied data
         isExit = InterpreteData(data_buf, server_socket, handle_numb);
      }
   }

   return isExit;
}

void cleanAll(char *data_buf, char *handle) {
   printf("\n");
   free(handle);
   free(data_buf);
   handle = NULL;
   data_buf = NULL;
}

int main(int argc, char * argv[]) {
   int flag = 0, server_socket = 0, isExit = 0;
   char *data_buf = NULL, *handle = NULL;
   uint32_t handle_numb = 0;

   // malloc data_buf and handle, connect to server, get handle
   flag = ConnectionSetup(argc, argv, &data_buf, &server_socket, &handle);

   if (flag == FLAG3) {
      printf("Handle already in use: %s\n", handle);
   }
   else {
      while(!isExit) {
         printf("$: ");
         fflush(stdout);
         isExit = MySelect(server_socket, handle, data_buf, &handle_numb);
      }
   }

   cleanAll(data_buf, handle);

   return 0;
}
