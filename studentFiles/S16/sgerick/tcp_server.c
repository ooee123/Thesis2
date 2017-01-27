/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>

#include "networks.h"
#include "testing.h"

int backupSocket= 0;
client* clients;
int numClients;
void cleanup(int signo) {
   printf("Successfully closed server.\n");
   close(backupSocket);
   char buffer[3];
   buffer[0] = 0;
   buffer[1] = 3;
   buffer[FLAG_IDX] = KICKED;
   int index;
   for (index = 0; index < numClients; index++) {
      send(clients[index].socket, buffer, 3, 0);
   }
   exit(0);
}
void sendAll(char* buffer, int numBytes, int exception) {
   int index;
   for (index = 0; index < numClients; index++) {
      if (clients[index].status == ONLINE && index != exception) {
         int sent = send(clients[index].socket, buffer, numBytes, 0);
         printf("Server sent %d bytes to \"%s\"\n", sent, clients[index].handle);
      }
   }
}

int findName(char* name) {
   int index;
   for (index = 0; index < numClients; index++) {
      if (clients[index].handle != NULL && strcmp(clients[index].handle, name) == 0
            && clients[index].status == ONLINE) {
         return index;
      }
   }
   return -1;
}

void acceptMessage(int index) {
   char src[MAX_NAME_SIZE], dest[MAX_NAME_SIZE], buf[BUFFER_LEN];
   int flag, destIdx = -1;
   uint16_t message_len = receiveMessage(clients[index].socket, src, dest, buf, &flag, numClients);
   if(flag == JOIN) {
      flag = 2 + (findName(src) != -1);
      buf[0] = 0;
      buf[1] = 3;
      buf[FLAG_IDX] = flag;
      send(clients[index].socket, buf, 3, 0);
      if (flag == ACCEPTED) {
         clients[index].status = ONLINE;
         clients[index].handle = malloc(1 + strlen(src));
         strcpy(clients[index].handle, src);                  
         printf("\"%s\" has connected.\n", clients[index].handle);
      }
      else {
         printf("A client attempted to connect with the exiting handle \"%s\"\n", src);
         clients[index].status = OFFLINE;
         close(clients[index].socket);
         free(clients[index].handle);
         numClients--;
         //HACK. Assumes client being rejected is last in the list.
      }
   }
   else if (flag == BROADCAST) { 
      printf("Received broadcast of %04d bytes from \"%s\": \"%s\"\n",
         message_len, clients[index].handle, buf);
      formatMessage(src, dest, buf, message_len - strlen(src) - 4, BROADCAST, buf);
      sendAll(buf, message_len, index);
   }
   else if (flag == MESSAGE) { 
      printf("\"%s\" sending message to \"%s\" : \"%s\"\n", src, dest, buf);
      destIdx = findName(dest);
      if (destIdx == -1) {
         message_len = formatMessage(src, dest, buf, 0, INVALID_NAME, buf);
         printf("Sending %d bytes to client for invalid\n", message_len);
         send(clients[index].socket, buf, message_len, 0);
      }
      else {
         message_len = formatMessage(src, dest, buf,
            message_len - strlen(src) - strlen(dest) - 5, MESSAGE, buf);
         send(clients[destIdx].socket, buf, message_len, 0);
      }
   }
   else if (flag == QUIT) {
      printf("\"%s\" has quit\n", clients[index].handle);
      message_len = htons(3);
      memcpy(buf, &message_len, 2);
      buf[FLAG_IDX] = KICKED;
      send(clients[index].socket, buf, 3, 0);
      close(clients[index].socket);
      numClients--;
      clients[index].status = OFFLINE;
      if (index != numClients) {
         memcpy(clients + index, clients + index + 1, (numClients - index) * sizeof(client));
      }
   }
   else if (flag == GET_HANDLES) {
      message_len = htons(7);
      memcpy(buf, &message_len, 2);
      buf[FLAG_IDX] = NUM_HANDLES;
      destIdx = htonl(numClients);
      memcpy(buf + 3, &destIdx, 4);
      send(clients[index].socket, buf, 7, 0);
      message_len = 3;
      buf[FLAG_IDX] = HANDLES;
      for (destIdx = 0; destIdx < numClients; destIdx++) {
         int handleLen = strlen(clients[destIdx].handle);
         buf[message_len++] = handleLen;
         memcpy(buf + message_len, clients[destIdx].handle, handleLen);
         message_len += handleLen;
      }
      message_len = htons(message_len);
      send(clients[index].socket, buf, ntohs(message_len), 0);
   }
}

void run(int server_socket) {
   fd_set fdset;
   int maxClients = DEF_CLIENTS;
   clients = malloc(DEF_CLIENTS * sizeof(client));
   int index;
   for (index = 0; index < DEF_CLIENTS; index++) {
      clients[index].status = OFFLINE;
   }
   struct sigaction action;
   action.sa_handler = cleanup;
   sigaction(SIGINT, &action, NULL);
   while (1) {
      FD_ZERO(&fdset);
      FD_SET(server_socket, &fdset);
      for (index = 0; index < numClients; index++) {
         if (clients[index].status != OFFLINE) {
            FD_SET(clients[index].socket, &fdset);
         }
      }
      select(FD_SETSIZE, &fdset, NULL, NULL, NULL);

      for (index = 0; index < numClients; index++) {
         if (FD_ISSET(clients[index].socket, &fdset)) {
            acceptMessage(index);
         }
      }
      if (FD_ISSET(server_socket, &fdset)) {
         clients[numClients].status = JOINING;
         clients[numClients++].socket = tcp_listen(server_socket, 5);
         if (numClients == maxClients) {
            clients = realloc(clients, maxClients * 2 * sizeof(client));
            memset(clients + maxClients, 0, maxClients * sizeof(client));
            maxClients *= 2;
         }
      }
   }
}

int main(int argc, char *argv[]) {
   int server_socket = 0;
   numClients = 0;
   if (argc > 1) {
      server_socket = atoi(argv[1]);
   }
   printf("sockaddr: %u sockaddr_in %u\n",
         sizeof(struct sockaddr), sizeof(struct sockaddr_in));
   server_socket = tcp_recv_setup(server_socket);
   backupSocket =  server_socket;

   run(server_socket);
   return 0;
}
