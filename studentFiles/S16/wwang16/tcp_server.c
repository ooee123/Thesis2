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

/*function declarations*/
void initializeClient(char *handle, int pkgLen,
                      HandleList **head, int socket);
void parseMessage(int socket, char *buffer,
 HandleList **clients, fd_set *activeSockets);
void server(int serverSocket);
void receiveMessage(int socket, HandleList **clientList,
                    fd_set *activeSockets);
void clientExitCleanup(int socket,
 fd_set *activeSockets, HandleList **clientList, int sendAck);
void processPrivateMessage(char *buffer, HandleList **clientList);
void processBroadcastMessage(char *buffer, HandleList **clientList);
uint32_t processClientCount(int socket, HandleList **clients);
void processClientList(int socket, HandleList **clients, uint32_t numClients);
void sendFailedMsgPkt(int socket, int handleLength, char *handle);
void removeClientFromList(HandleList **head, int socket);
HandleList *clientLookup(HandleList **head, char *handle);
void sendFirstAck(int socket, int success);
void sendCloseAck(int socket);
void transmit(int socket, char *packet, int packetLength);

int main(int argc, char *argv[])
{
   int server_socket= 0;   //socket descriptor for the server socket
   int client_socket= 1;   //socket descriptor for the client socket
   
   if (argc == 2) {
      printf("Trying to get port %s\n", argv[1]);
      server_socket = strtol(argv[1], NULL, 10);
   }
   
   //create the server socket and pass it to main server loop
   server(server_socket = tcp_recv_setup(server_socket));
   
   /* close the sockets */
   close(server_socket);
   close(client_socket);
   
   return 0;
}

void server(int serverSocket) {
   int client_socket, numSocketsToRead;
   int fdSize = serverSocket + 1;
   char *buffer;
   int iterator;
   HandleList *clients = NULL;
   fd_set activeSockets, readFromSockets;
   
   buffer = calloc(1, SERVER_BUFF_SIZE);
   FD_ZERO(&activeSockets);
   FD_SET(serverSocket, &activeSockets);
   
   if (listen(serverSocket, 5) < 0)
   {
      perror ("listen call");
      exit (EXIT_FAILURE);
   }
   
   while (TRUE) {
      FD_ZERO(&readFromSockets);
      readFromSockets = activeSockets;
      
      /* exit if select fails */
      if ((numSocketsToRead = select(fdSize, &readFromSockets,
       NULL, NULL, NULL)) < 0) {
         perror("select call");
         exit(1);
      }
      
      /* we received a message*/
      for (iterator = 0; iterator < fdSize; iterator++) {
         if (FD_ISSET(iterator, &readFromSockets)) {
            if (iterator == serverSocket) {
               client_socket = tcp_listen(serverSocket, 5);
               FD_SET(client_socket, &activeSockets);
               if(client_socket >= fdSize)  {
                  //printf("adding client %d\n", client_socket);
                  fdSize = client_socket + 1;
               }
            }
            else {
               //printf("read message from socket %d\n", iterator);
               receiveMessage(iterator, &clients, &activeSockets);
               if(--numSocketsToRead == 0) {
                  break;
               }
            }
         }
      }
   }
}

void receiveMessage(int socket,
                    HandleList **clientList, fd_set *activeSockets) {
   int msgLen = 0;
   char *msgBuffer = calloc(1, SERVER_BUFF_SIZE);
   if ((msgLen = recv(socket, msgBuffer, SERVER_BUFF_SIZE, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   if (msgLen == 0) {
      clientExitCleanup(socket, activeSockets, clientList, FALSE);
   }
   else {
      parseMessage(socket, msgBuffer, clientList, activeSockets);
   }
   
   free(msgBuffer);
   
//   HandleList *temp;
//   for (temp = *clientList; temp != NULL; temp = temp->next) {
//      printf("handle length: %d\n", temp->length);
//   }
}

void parseMessage(int socket, char *buffer,
 HandleList **clients, fd_set *activeSockets) {
   Header *header = (Header *)buffer;
   u_short pkLen = ntohs(header->packetLength);
   
   switch (header->flag) {
      case FLAG_CTS_INIT:
         initializeClient(buffer + sizeof(Header), pkLen, clients, socket);
         break;
      case FLAG_BROADCAST:
         processBroadcastMessage(buffer, clients);
         break;
      case FLAG_MESSAGE:
         processPrivateMessage(buffer, clients);
         break;
      case FLAG_CTS_EXIT:
         clientExitCleanup(socket, activeSockets, clients, TRUE);
         break;
      case FLAG_CTS_REQ_LIST_CLIENTS:
         processClientList(socket, clients,
                           processClientCount(socket, clients));
         break;
      default:
         printf("Not recognized packet, flag = %d\n", header->flag);
         break;
   }
   
}

void initializeClient(char *handle, int pkgLen,
                      HandleList **head, int socket) {
   int handleLength = *((u_char *)handle);
   //printf("initializing client\n");
   handle++;
   
   HandleList *newNode = clientLookup(head, handle);
   
   if (newNode != NULL) {
      sendFirstAck(socket, FALSE);
      return;
   }
   
   //printf("handlelen: %d\n", handleLength);
   
   newNode = calloc(1, sizeof(HandleList));
   newNode->handle = calloc(handleLength, sizeof(char));
   
   memcpy(newNode->handle, handle, handleLength);
   
   newNode->length = handleLength;
   newNode->socket = socket;
   newNode->next = *head;
   *head = newNode;
   
   sendFirstAck(socket, TRUE);
}

void clientExitCleanup(int socket, fd_set *activeSockets,
 HandleList **clientList, int sendAck) {
   removeClientFromList(clientList, socket);
   FD_CLR(socket, activeSockets);
   
   if (sendAck) {
      sendCloseAck(socket);
   }
   
   close(socket);
}

void processPrivateMessage(char *buffer, HandleList **clientList) {
   int handleLength = *(buffer + sizeof(Header));
   char *destHandle = buffer + sizeof(Header) + 1;
   HandleList *destination = clientLookup(clientList, destHandle);
   
   if (!destination) {
      char *srcHandle = buffer + sizeof(Header) + 1 + handleLength + 1;
      handleLength = *(buffer + sizeof(Header) + 1 + handleLength);
      HandleList *source = clientLookup(clientList, srcHandle);
      
      handleLength = *(buffer + sizeof(Header));
      char *destHandle = buffer + sizeof(Header) + 1;
      sendFailedMsgPkt(source->socket, handleLength, destHandle);
   }
   else {
      Header *header = (Header *)buffer;
      u_short pkLen = ntohs(header->packetLength);
      transmit(destination->socket, buffer, pkLen);
   }
}

void processBroadcastMessage(char *buffer, HandleList **clientList) {
   HandleList *cur;
   Header *header = (Header *)buffer;
   u_short pkLen = ntohs(header->packetLength);
   int handleLength = *(buffer + sizeof(Header));
   char *destHandle = buffer + sizeof(Header) + 1;
   
   for (cur = *clientList; cur != NULL; cur = cur->next) {
      if (strncmp(cur->handle, destHandle, handleLength) != 0) {
         transmit(cur->socket, buffer, pkLen);
      }
   }
}

void sendFailedMsgPkt(int socket, int handleLength, char *handle) {
   Header init;
   int packetSize;
   char *packet = NULL;
   
   packetSize = sizeof(Header) + 1 + handleLength;
   
   init.flag = FLAG_STC_DEST_NOT_EXIST;
   init.packetLength = htons(packetSize);
   
   packet = calloc(1, packetSize);
   
   memcpy(packet, &init, sizeof(Header));
   memcpy(packet + sizeof(Header), &handleLength, 1);
   memcpy(packet + sizeof(Header) + 1, handle, handleLength);
   
   transmit(socket, packet, packetSize);
   
   free(packet);
}

uint32_t processClientCount(int socket, HandleList **clients) {
   Header init;
   HandleList *temp;
   int packetSize;
   uint32_t clientCount = 0;
   char *packet = NULL;
   
   packetSize = sizeof(Header) + sizeof(uint32_t);
   
   init.flag = FLAG_STC_CLIENT_COUNT;
   init.packetLength = htons(packetSize);
   
   for (temp = *clients; temp != NULL; temp = temp->next, clientCount++) {};
   clientCount = htonl(clientCount);
   
   packet = calloc(1, packetSize);
   
   memcpy(packet, &init, sizeof(Header));
   memcpy(packet + sizeof(Header), &clientCount, sizeof(uint32_t));
   
   transmit(socket, packet, packetSize);
   
   free(packet);
   
   return clientCount;
}

void processClientList(int socket, HandleList **clients, uint32_t numClients) {
   Header init;
   HandleList *temp;
   int packetSize;
   char *packet = NULL, *writePtr = NULL;
   
   packetSize = sizeof(Header) + sizeof(uint32_t);
   writePtr = packet = calloc(1, packetSize);
   
   init.packetLength = 0;
   init.flag = FLAG_STC_CLIENT_LIST;
   memcpy(writePtr, &init, sizeof(Header));
   writePtr += sizeof(Header);
   
   memcpy(writePtr, &numClients, sizeof(uint32_t));
   writePtr += sizeof(uint32_t);

   for (temp = *clients; temp != NULL; temp = temp->next) {
      packetSize += (1 + temp->length);
      packet = realloc(packet, packetSize);

      memcpy(writePtr, &(temp->length), 1);
      writePtr += 1;
      
      memcpy(writePtr, temp->handle, temp->length);
      writePtr += temp->length;
   }
   
   transmit(socket, packet, packetSize);
   free(packet);
}

void removeClientFromList(HandleList **head, int socket) {
   HandleList *cur = NULL, *prev = NULL;
   
   for (cur = *head; cur != NULL; prev = cur, cur = cur->next) {
      if (cur->socket == socket) {
         if (prev == NULL) {
            *head = cur->next;
         }
         else {
            prev->next = cur->next;
         }
         
         free(cur->handle);
         free(cur);
      }
   }
}
                   
void sendFirstAck(int socket, int success) {
   Header init;
   int packetSize;
   char *packet = NULL;
   
   packetSize = sizeof(Header);
   
   init.flag = success ? FLAG_STC_GOOD_HANDLE : FLAG_STC_BAD_HANDLE;
   init.packetLength = htons(packetSize);
   
   packet = calloc(1, packetSize);
   
   memcpy(packet, &init, sizeof(Header));
   
   transmit(socket, packet, packetSize);
   
   free(packet);
}

void sendCloseAck(int socket) {
   Header init;
   int packetSize;
   char *packet = NULL;
   
   packetSize = sizeof(Header);
   
   init.flag = FLAG_STC_CONFIRM_EXIT;
   init.packetLength = htons(packetSize);
   
   packet = calloc(1, packetSize);
   
   memcpy(packet, &init, sizeof(Header));
   
   transmit(socket, packet, packetSize);
   
   free(packet);
}

HandleList *clientLookup(HandleList **head, char *handle) {
   HandleList *cur;
   
   for (cur = *head;
        cur != NULL && memcmp(cur->handle, handle, cur->length) != 0;
        cur = cur->next) {};
   
   return cur;
}

/* This function sets the server socket.  It lets the system
 determine the port number.  The function returns the server
 socket number and prints the port number to the screen.  */

int tcp_recv_setup(int socket_num)
{
   int server_socket = 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */
   
   
   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0)
   {
      perror("socket call");
      exit(1);
   }
   
   local.sin_family= AF_INET;         //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= htons(socket_num);  //let system choose the port
   
   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
   {
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

/* This function waits for a client to ask for services.  It returns
 the socket number to service the client on.    */

int tcp_listen(int server_socket, int back_log)
{
   //printf("tcp_listen\n");
   int client_socket= 0;
   
   if ((client_socket =
        accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
      perror("accept call");
      exit(-1);
   }
   
   return(client_socket);
}

void transmit(int socket, char *packet, int packetLength) {
   int sent = send(socket, packet, packetLength, 0);
   
   if(sent < 0) {
      perror("send call");
      exit(-1);
   }
}

