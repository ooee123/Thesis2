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

#include "node.h"
#include "networks.h"

#define MAX_BUFFER 1029
#define MAX_MESSAGE_LEN 1000
#define MAX_HANDLE 255
#define PACK_LEN_SIZE 2
#define FLAG_SIZE 1
#define HAND_LEN_SIZE 1
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



// Create connection
int CreateServer(int argc, char *argv[], char **data_buf) {
   int port_numb;

   // create buffer for data
   if ((*data_buf = (char *) malloc(MAX_BUFFER)) == NULL) {
      perror("malloc() error");
      exit(-1);
   }
   if (argc == 2)
      port_numb = (unsigned short) strtod(argv[1], 0);

   //create the server socket
   return tcpServerSetup(port_numb);
}

int ResetFD(fd_set *fdvar, node **cli_sk_head, int cli_sk_count, int server_socket) {
   int max_fd = server_socket;
   int i = 0;
   node *cli_sk_cur = *cli_sk_head;

   // reset fd set
   FD_ZERO(fdvar);
   FD_SET(server_socket, fdvar); // added server only

   max_fd = server_socket; // initialize max as server

   // add client fds to the set
   for (i = 0; i < cli_sk_count; i++) {
      FD_SET(cli_sk_cur->fd, fdvar);
      if (cli_sk_cur->fd > max_fd)
         max_fd = cli_sk_cur->fd;
      cli_sk_cur = cli_sk_cur->next;
   }

   return max_fd;
}

int AcceptNewClient(int server_socket, node **cli_sk_head, int *cli_sk_count) {
   int new_client_sk;
   char data_buf[MAX_BUFFER];
   char *temp = data_buf;
   int message_len;
   uint16_t pack_len, pack_len_net;
   uint8_t flag, hand_len;
   char handle[MAX_HANDLE + 1];

   if ((new_client_sk = accept(server_socket, (struct sockaddr *)0, (socklen_t *) 0)) < 0) {
      perror("accept error");
      exit(-1);
   }

   // receive flag 1 header
   message_len = recv(new_client_sk, data_buf, MAX_BUFFER, 0);
   memcpy(&pack_len_net, temp, PACK_LEN_SIZE);
   pack_len = ntohs(pack_len_net);
   temp += 2;
   memcpy(&flag, temp, FLAG_SIZE);
   temp += 1;
   memcpy(&hand_len, temp, HAND_LEN_SIZE);
   temp += 1;
   memcpy(handle, temp, hand_len);
   handle[hand_len] = '\0';

   // check if handle exists
   pack_len = PACK_LEN_SIZE + FLAG_SIZE;
   if (findHandle(*cli_sk_head, handle)) {
      // make flag 3 header
      flag = FLAG3;
   }
   else {
      // make flag 2 header
      flag = FLAG2;
      // add to the linked list
      push(cli_sk_head, new_client_sk, handle);
      (*cli_sk_count)++;
   }

   // create packet
   temp = data_buf;
   pack_len_net = htons(pack_len);
   memcpy(temp, &pack_len_net, sizeof(pack_len));
   temp += sizeof(pack_len);
   memcpy(temp, &flag, sizeof(flag));
   temp += sizeof(flag);

   // reply back
   if (send(new_client_sk, data_buf, pack_len, 0) < 0) {
      perror("send call");
      exit(-1);
   }

   // bad connection
   if (flag == FLAG3)
      close(new_client_sk);

   return new_client_sk;
}

// pack_len - host order
void Broadcast(char *data_buf, node *cli_sk_head, int pack_len, int fd_src) {
   int client_sk;
   node *temp = cli_sk_head;

   while (temp != NULL && temp->fd != 0) {
      client_sk = temp->fd;
      if (client_sk != fd_src) {
         if (send(client_sk, data_buf, pack_len, 0) < 0 ) {
            perror("send call");
            exit(-1);
         }
      }
      temp = temp->next;
   }
}

// return total packet length
int CreateFlag7(char *handle, char *data_buf) {
   char *temp = data_buf;
   uint16_t pack_len, pack_len_net;
   uint8_t flag = 7;
   uint8_t handle_len = strlen(handle);

   pack_len = CHATHEAD_SIZE + sizeof(handle_len) + handle_len;

   pack_len_net = htons(pack_len);
   memcpy(temp, &pack_len_net, sizeof(pack_len));
   temp += sizeof(pack_len);

   memcpy(temp, &flag, sizeof(flag));
   temp += sizeof(flag);

   memcpy(temp, &handle_len, sizeof(handle_len));
   temp += sizeof(handle_len);

   memcpy(temp, handle, handle_len);

   return pack_len;
}

void DirectMessage(char *data_buf, node *cli_sk_head, int pack_len) {
   char *temp = data_buf;
   char handle_dest[MAX_HANDLE];
   char handle_src[MAX_HANDLE];
   uint8_t handleDest_len, handleSrc_len;
   int i, dest_fd, src_fd;

   // skip chat-header
   temp += CHATHEAD_SIZE;

   // get dest handle information
   handleDest_len = *temp;
   temp += sizeof(handleDest_len);

   for (i = 0; i < handleDest_len; i++) {
      handle_dest[i] = *temp;
      temp++;
   }
   handle_dest[i] = '\0';

   // get src handle information
   handleSrc_len = *temp;
   temp += sizeof(handleSrc_len);

   for (i = 0; i < handleSrc_len; i++) {
      handle_src[i] = *temp;
      temp++;
   }
   handle_src[i] = '\0';

   // find fd of handle_dest
   if (findHandle(cli_sk_head, handle_dest)) {
      dest_fd = getHandleFD(cli_sk_head, handle_dest);
      // send
      if (send(dest_fd, data_buf, pack_len, 0) < 0) {
         perror("send call");
         exit(-1);
      }
   }
   else {
      // create flag 7 and send to handle_src
      pack_len = CreateFlag7(handle_dest, data_buf);
      src_fd = getHandleFD(cli_sk_head, handle_src);
      if (send(src_fd, data_buf, pack_len, 0) < 0) {
         perror("send call");
         exit(-1);
      }
   }
}

void SendList(char *data_buf, int cli_sk_count, int src_fd, node *cli_sk_head) {
   char *handle12;
   char *temp = data_buf;
   uint16_t pack_len, pack_len_net;
   uint8_t flag;
   uint8_t handleLen;
   uint32_t handle_numb = htons((uint32_t) cli_sk_count);
   int i;
   node *nodeCur = cli_sk_head;

   // create flag 11
   pack_len = sizeof(pack_len) + sizeof(flag) + sizeof(handle_numb);
   flag = FLAG11;

   pack_len_net = htons(pack_len);
   memcpy(temp, &pack_len_net, sizeof(pack_len));
   temp += sizeof(pack_len);

   memcpy(temp, &flag, sizeof(flag));
   temp += sizeof(flag);

   memcpy(temp, &handle_numb, sizeof(handle_numb));
   // send it to handle_src
   if (send(src_fd, data_buf, pack_len, 0) < 0) {
      perror("send call");
      exit(-1);
   }

   // create flag 12 chat-header
   temp = data_buf;
   flag = FLAG12;
   pack_len = 0;
   pack_len_net = htons(pack_len_net);

   memcpy(temp, &pack_len_net, sizeof(pack_len));
   temp += sizeof(pack_len);

   memcpy(temp, &flag, sizeof(flag));
   temp += sizeof(flag);

   pack_len = sizeof(pack_len) + sizeof(flag);

   for (i = 0; i < cli_sk_count; i++) {
      // get handle of each clients
      handle12 = nodeCur->handle;
      handleLen = strlen(handle12);
      pack_len += sizeof(handleLen) + handleLen;

      memcpy(temp, &handleLen, sizeof(handleLen));
      temp += sizeof(handleLen);

      memcpy(temp, handle12, handleLen);
      temp += handleLen;

      nodeCur = nodeCur->next;
   }
   // send handle information
   if (send(src_fd, data_buf, pack_len, 0) < 0) {
      perror("send call");
      exit(-1);
   }
}

void EndConnection(char *data_buf, int src_fd, node **cli_sk_head, int *cli_sk_count) {
   char *temp = data_buf;
   //   node *cli_sk_cur = cli_sk_head;
   uint16_t pack_len, pack_len_net;
   uint8_t flag = 9;

   // create flag 9
   pack_len = sizeof(pack_len) + sizeof(flag);
   pack_len_net = htons(pack_len);

   memcpy(temp, &pack_len_net, sizeof(pack_len));
   temp += sizeof(pack_len);

   memcpy(temp, &flag, sizeof(flag));

   // send flag 9
   if (send(src_fd, data_buf, pack_len, 0) < 0) {
      perror("send call");
      exit(-1);
   }

   // remove connection
   if(close(src_fd) < 0) {
      perror("close call");
      exit(-1);
   }

   removeNode(cli_sk_head, src_fd);
   (*cli_sk_count)--;
}

void InterpretData(char *data_buf, node **cli_sk_head, int fd_src, int *cli_sk_count) {
   uint16_t pack_len, pack_len_net;
   uint8_t flag = 0;
   char *temp = data_buf;

   // get chat-header information
   memcpy(&pack_len_net, temp, sizeof(pack_len_net));
   pack_len = ntohs(pack_len_net);
   temp += sizeof(pack_len);

   flag = *temp;

   if (flag == 4) {
      // broadcast
      Broadcast(data_buf, *cli_sk_head, pack_len, fd_src);
   }
   else if (flag == 5) {
      // direct message
      DirectMessage(data_buf, *cli_sk_head, pack_len);
   }
   else if (flag == 10) {
      // list
      SendList(data_buf, *cli_sk_count, fd_src, *cli_sk_head);
   }
   else if (flag == 8) {
      // exit
      EndConnection(data_buf, fd_src, cli_sk_head, cli_sk_count);
   }
}

void IssetCheckClients(node **cli_sk_head, int *cli_sk_count, fd_set *fdvar, char **data_buf) {
   node *cli_sk_cur = *cli_sk_head;
   int i = 0;
   int message_len = 0;

   for (i = 0; i < *cli_sk_count; i++) {
      if (FD_ISSET(cli_sk_cur->fd, fdvar)) {
         // recv disconnection or error
         if ((message_len = recv(cli_sk_cur->fd, *data_buf, MAX_BUFFER, 0)) <= 0) {

            if (message_len < 0) {
               perror("recv call");
               exit(-1);
            }

            // remove disconnected cli from the linked list
            if(close(cli_sk_cur->fd) < 0) {
               perror("close call");
               exit(-1);
            }

            removeNode(cli_sk_head, cli_sk_cur->fd);
            (*cli_sk_count)--;
         }
         // recv data
         else {
            InterpretData(*data_buf, cli_sk_head, cli_sk_cur->fd, cli_sk_count);
         }
      }
      cli_sk_cur = cli_sk_cur->next;
   }
}

void ConnectSendRecv(int max_sk, fd_set *fdvar, int server_socket, node **cli_sk_head, int *cli_sk_count, char **data_buf) {
   //   int new_client_sk = 0;

   // select
   if (select(max_sk + 1, fdvar, NULL, NULL, NULL) < 0) { // blocking here
      perror("select error");
      exit(-1);
   }

   // if new client connection
   if (FD_ISSET(server_socket, fdvar)) {
      AcceptNewClient(server_socket, cli_sk_head, cli_sk_count);
   }

   // FD_ISSET check client servers
   IssetCheckClients(cli_sk_head, cli_sk_count, fdvar, data_buf);
}


int main(int argc, char *argv[])
{
   char *data_buf = NULL;        //buffer for receiving from client
   fd_set fdvar;
   node *cli_sk_head = malloc(sizeof(node));
   int cli_sk_count = 0, server_socket = 0, max_sk = server_socket;

   server_socket = CreateServer(argc, argv, &data_buf);

   while (1) {
      max_sk = ResetFD(&fdvar, &cli_sk_head, cli_sk_count, server_socket);
      ConnectSendRecv(max_sk, &fdvar, server_socket, &cli_sk_head, &cli_sk_count, &data_buf);
   }

   free(data_buf);
   free(cli_sk_head);

   // close the sockets
   close(server_socket);

   return 0;
}
