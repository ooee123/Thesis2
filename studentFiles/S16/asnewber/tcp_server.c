/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 2
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
//#define DEBUG

#include "networks.h"

int serverSocket = 0;            //socket descriptor for the server socket
char *buffer;                    //buffer for receiving from client
int messageLen = 0;              //length of the received message

int portNum = 0;                 //port number
char *numEnd;                    //use for arg2 of strtol
int clientCount = 0;             //total number of clients
int maxClients = 5;              //clients we can have until we have to realloc
Client *clientArray;             //array of clients

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup() {
   struct sockaddr_in local;           /* socket address for local side  */
   socklen_t length = sizeof(local);   /* length of local address        */

   /* create the socket  */
   serverSocket = socket(AF_INET, SOCK_STREAM, 0);
   if(serverSocket < 0) {
      perror("socket call");
      exit(1);
   }

   local.sin_family= AF_INET;                //internet family
   local.sin_addr.s_addr= INADDR_ANY;        //wild card machine address
   local.sin_port= htons(portNum);           //choose the port, if specified

   /* bind the name (address) to a port */
   if (bind(serverSocket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
      exit(EXIT_FAIL);
   }
    
   //get the port name and print it out
   if (getsockname(serverSocket, (struct sockaddr*)&local, &length) < 0) {
      perror("getsockname call");
      exit(EXIT_FAIL);
   }

   printf("Server is using port %d\n", ntohs(local.sin_port));
           
   return serverSocket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

void tcp_call_listen() {
   if (listen(serverSocket, BACK_LOG) < 0) {
      perror("listen call");
      exit(EXIT_FAIL);
   }
}

void tcp_call_send(char *packet, int packetLen, int clientSocket) {
   int sentLen;
   if ((sentLen = send(clientSocket, packet, packetLen, 0)) < 0) {
      perror("send error");
      exit(EXIT_FAIL);
   }
   #ifdef DEBUG
   printf("Sent %d bytes from %d to %d\n", sentLen, serverSocket, clientSocket);
   #endif
}

void add_client(int socket) {
   clientArray[clientCount].socket = socket;
   clientCount++;
   int i = 0;

   if (clientCount == maxClients) {
      Client *temp = clientArray;
      maxClients += 5;
      clientArray = realloc(clientArray, maxClients * sizeof(Client));
      for (i = 0; i < clientCount; i++) {
         clientArray[i] = temp[i];
      }
   }
}

int get_index_by_socket(int socket) {
   int i = 0;
   int index = -1;

   for (i = 0; i < clientCount; i++) {
      if (clientArray[i].socket == socket) {
         index = i;
      }
   }

   return index;
}

int get_index_by_handle(char *handle) {
   int i = 0;
   int index = -1;

   for (i = 0; i < clientCount; i++) {
      if (strcmp(handle, clientArray[i].handle) == 0) {
         index = i;
      }
   }

   return index;
}

void drop_client(int socket) {
   int i = 0;
   int index;

   index = get_index_by_socket(socket);

   for (i = index; i < clientCount; i++) {
      clientArray[i] = clientArray[i + 1];
   }

   clientCount--;
}

int client_exists(char *handle) {
   int exists = 0;
   int i = 0;

   for (i = 0; i < clientCount; i++) {
      if (clientArray[i].handle && strcmp(handle, clientArray[i].handle) == 0) {
         exists = 1;
      }
   }

   return exists;
}

void print_clients() {
   int i;
   printf("~~~~~~~~~~~~~~~~~~~~~~~~\n");
   for (i = 0; i < clientCount; i++) {
      if (clientArray[i].handle) {
         printf("%s: %d", clientArray[i].handle, clientArray[i].handleLen);
      }
      printf(" socket: %d\n", clientArray[i].socket);
   }
   printf("~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

void send_init_err(int length, int clientSocket) {
   char *packet;
   char *packetPointer;
   uint16_t packetLen;

   packetLen = sizeof(struct NormalHeader);
   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);

   *packetPointer = SERVER_BAD;

   tcp_call_send(packet, packetLen, clientSocket);
}

void send_init_good(int clientSocket) {
   char *packet;
   char *packetPointer;
   int packetLen = sizeof(struct NormalHeader);

   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);

   *packetPointer = SERVER_GOOD;

   tcp_call_send(packet, packetLen, clientSocket);
}

void init_receive(int packetLen, int clientSocket) {
   uint8_t handleLen;
   char *handle;
   int i;
   handleLen = *(buffer + sizeof(struct NormalHeader));
   handle = malloc(handleLen + 1);
   memcpy(handle, buffer + sizeof(struct NormalHeader) + sizeof(uint8_t), handleLen);
   handle[handleLen] = '\0';

   if (client_exists(handle)) {
      send_init_err(packetLen, clientSocket);
   }
   else {
      i = get_index_by_socket(clientSocket);
      clientArray[i].handle = handle;
      clientArray[i].handleLen = handleLen;
      send_init_good(clientSocket);
   }
}

void server_broadcast(int packetLen, int clientSocket) {
   int i = 0;

   for (i = 0; i < clientCount; i++) {
      if (clientArray[i].socket != clientSocket) {
         tcp_call_send(buffer, packetLen, clientArray[i].socket);
      }
   }
}

void send_msg_err(int clientSocket, uint8_t receiverLen, char *receiverHandle) {
   char *packet;
   char *packetPointer;
   uint16_t packetLen;

   packetLen = sizeof(struct NormalHeader) + sizeof(uint8_t) + receiverLen;
   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);

   *packetPointer = MESSAGE_ERR;
   packetPointer += sizeof(uint8_t);

   *packetPointer = receiverLen;
   packetPointer += sizeof(uint8_t);

   memcpy(packetPointer, receiverHandle, receiverLen);

   tcp_call_send(packet, packetLen, clientSocket);
}

void server_message(int length, int clientSocket) {
   char *senderHandle;
   char *receiverHandle;
   char *message = NULL;
   uint8_t receiverLen;
   uint8_t senderLen;
   int index;
   int receiverSocket;
   //int packetsToSend = length / MAX_MSG_LEN + 1;
   //int i;

   receiverLen = *(buffer + sizeof(struct NormalHeader));
   receiverHandle = malloc(receiverLen + 1);
   memcpy(receiverHandle, buffer + sizeof(struct NormalHeader) + sizeof(uint8_t), receiverLen);
   receiverHandle[receiverLen] = '\0';

   senderLen = *(buffer + sizeof(struct NormalHeader) + sizeof(uint8_t) + receiverLen);
   senderHandle = malloc(senderLen + 1);
   memcpy(senderHandle, buffer + sizeof(struct NormalHeader) + sizeof(uint8_t) + receiverLen + sizeof(uint8_t), senderLen);
   senderHandle[senderLen] = '\0';
   message = buffer + sizeof(struct NormalHeader) + sizeof(uint8_t) + receiverLen + sizeof(uint8_t) + senderLen;

   if (client_exists(receiverHandle)) {
      index = get_index_by_handle(receiverHandle);
      receiverSocket = clientArray[index].socket;
      //for (i = 0; i < packetsToSend; i++) {
         #ifdef DEBUG
         printf("Sending a %d sized message to: %d\n", length, receiverSocket);
         print_clients();
         #endif
         tcp_call_send(buffer, length, receiverSocket);
      //   strcpy(message, message + MAX_MSG_LEN - 1 - receiverLen - 1 - senderLen - sizeof(struct NormalHeader));
      //   length -= MAX_MSG_LEN - 1 - receiverLen - 1 - senderLen - sizeof(struct NormalHeader);
      //}
   }
   else {
      send_msg_err(clientSocket, receiverLen, receiverHandle);
   }
}

void exit_client(int length, int clientSocket) {
   char *packet;
   char *packetPointer;
   uint16_t packetLen;

   packetLen = sizeof(struct NormalHeader);
   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);

   *packetPointer = SERVER_ACK;

   tcp_call_send(packet, packetLen, clientSocket);
}

void server_send_num_clients(int clientSocket) {
   char *packet;
   char *packetPointer;
   uint16_t packetLen;

   packetLen = sizeof(struct NormalHeader) + sizeof(int);
   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);

   *packetPointer = NUM_CLIENTS;
   packetPointer += sizeof(uint8_t);

   *packetPointer = clientCount;

   tcp_call_send(packet, packetLen, clientSocket);
}

void server_client_list(int packetLength, int clientSocket) {
   char *packet;
   char *packetPointer;
   uint16_t packetLen;
   int sum = 0;
   int i = 0;

   for (i = 0; i < clientCount; i++) {
      sum += clientArray[i].handleLen;
   }

   packetLen = sizeof(struct NormalHeader) + (sizeof(uint8_t) * clientCount) + sum;

   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);

   *packetPointer = LIST_CLIENTS;
   packetPointer += sizeof(uint8_t);


   for (i = 0; i < clientCount; i++) {
      *packetPointer = clientArray[i].handleLen;
      packetPointer += sizeof(uint8_t);
      memcpy(packetPointer, clientArray[i].handle, clientArray[i].handleLen);
      packetPointer += clientArray[i].handleLen;
   }

   tcp_call_send(packet, packetLen, clientSocket);
}

int tcp_call_accept() {
   int toAccept = 0;

   if ((toAccept = accept(serverSocket, NULL, NULL)) < 0) {
      perror("accept error");
      exit(EXIT_FAIL);
   }

   add_client(toAccept);

   return toAccept;
}
   
void tcp_call_receive(int clientSocket) {
   int length = 0;
   struct NormalHeader *temp;

   if ((length = recv(clientSocket, buffer, MAX_MSG_LEN, 0)) < 0) {
      perror("receive error");
      exit(EXIT_FAIL);
   }

   #ifdef DEBUG
      printf("Received %d bytes from %d\n", length, clientSocket);
   #endif

   temp = (struct NormalHeader *) buffer;

   if (length == 0) {
      drop_client(clientSocket);
   }
   else {
      #ifdef DEBUG
      printf("%d\n", temp->flag);
      #endif
      switch (temp->flag) {
         case CLIENT_INIT:
            init_receive(length, clientSocket);
            break;
         case BROADCAST:
            server_broadcast(length, clientSocket);
            break;
         case MESSAGE: 
            server_message(length, clientSocket);
            break;
         case CLIENT_EXIT:
            exit_client(length, clientSocket);
            break;
         case CLIENT_LIST:
            server_send_num_clients(clientSocket);
            server_client_list(length, clientSocket);
            break;
         default:
            printf("Not a valid flag... Weird.\n");
            break;
      }
   }
}

void tcp_call_select() {
   fd_set fds;
   int i = 0;

   FD_ZERO(&fds);
   FD_SET(serverSocket, &fds);

   for (i = 0; i < clientCount; i++) {
      FD_SET(clientArray[i].socket, &fds);
   }

   if (select(FD_SETSIZE, (fd_set *) &fds, NULL, NULL, NULL) < 0) { 
      perror("select error");
      exit(EXIT_FAIL);
   }

   if (FD_ISSET(serverSocket, &fds)) {
      tcp_call_accept();
   }

   for (i = 0; i < clientCount; i++) {
      if (FD_ISSET(clientArray[i].socket, &fds)) {
         tcp_call_receive(clientArray[i].socket);
      }
   }
   #ifdef DEBUG
      printf("this should repeat\n");
   #endif
}

int main(int argc, char *argv[]) {
   if (argc > 1) {
      portNum = strtol(argv[1], &numEnd, DECIMAL);
   }
   buffer = (char *) malloc(MAX_MSG_LEN);
   serverSocket = tcp_recv_setup();
   clientArray = malloc(sizeof(Client) * maxClients);
   tcp_call_listen();
   while (1) {                   // select until CTRL+C
      tcp_call_select();
   }
   close(serverSocket);
   return 0;
}