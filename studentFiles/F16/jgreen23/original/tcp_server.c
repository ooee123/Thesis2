/******************************************************************************
* tcp_server.c
* author: James Green
*
* CPE 464 - Program 1
* Large portion of code written by Dr Smith's TA in the 90's
* use at own risk
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
#include <sys/select.h>
#include "testing.h"
#include "networks.h"
#include "server.h"

#define MAXBUF 1024
#define HANDLEBUFFER 5

typedef struct client
{
   char *handle; // e.g. "unix1"
   int sock; //socket number
} client; //"struct client"


int gMaxClients = 5;    //global variable for max number of clients able to support
int gClientCount = 0;   //global variable for number of clients connected
client *gClients;       //list of clients connected
char * gBuff;           //global receiving buffer
int gPort = 0;          //globl port server will use
int gServerSock = 0;    //global for the FD for the socket
   
int main(int argc, char **argv)
{
   gClients = (client *) malloc(sizeof(client) * gMaxClients); //allocate enough space for all the clients
   gBuff = (char*) calloc(sizeof(char) * MAXBUF, sizeof(char)); //set all spaces in the receiving buffer to 0 (lots of time wasted here)
 
   if (argc >= 2){
      gPort = atoi(argv[1]); //will be 0 otherwise
   }
   gServerSock = tcpSetup(gPort); //if 0, system chooses ports
   safeListen(); //wait for a client
   while(1) { //quit with ctrl+c
      safeSelect(); //main driver
   }
   return 1;
}

//function for checking return value of send
void safeSend(int sock, char * buff, int len)
{
   int ack = 0;
   ack = send(sock, buff, len, 0);
   if (ack < 0)
   {
      perror("send");
      exit(-1);
   }
}

int tcpSetup(int port) //connect a client
{
   int server_socket;
   struct sockaddr_in local;
   socklen_t len = sizeof(local);

   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0)
   {
      perror("socket call");
      exit(-1);
   }

   local.sin_family= AF_INET;         //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= htons(port);       //use port number
   

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
   
   printf("Server is using port %d \n", ntohs(local.sin_port));
   
   return server_socket;
   
}

//listen to server
void safeListen() 
{
   if (listen(gServerSock, BACKLOG) < 0)
   {
      perror("listen call");
      exit(-1);
   }
   // printf("listening...\n");
}

//adds clients and accepts their connections
int safeAccept()
{
   int clientSock = 0;
   int ndx = 0;
   
   if (((clientSock = accept(gServerSock, (struct sockaddr*)0, (socklen_t*) 0)) < 0))
   {
      perror("accept");
      exit(-1);
   }

   gClients[gClientCount++].sock = clientSock; 

   if (gClientCount == gMaxClients)
   {
      gMaxClients *= 2;
      client *tmp = gClients;
      gClients = realloc(gClients, sizeof(client) * gMaxClients);
      
      for (ndx = 0; ndx < gMaxClients; ndx++)
      {
         gClients[ndx] = tmp[ndx];
      }
   }
   
   return clientSock;
}

//listen to all clients
void safeSelect()
{
   fd_set fdx;
   int ndx = 0;
   int socketMax = 0;
   FD_ZERO(&fdx); //reset
   FD_SET(gServerSock, &fdx); //check server socket
   
   for (ndx = 0; ndx <= gClientCount; ndx++)
   {
      FD_SET(gClients[ndx].sock, &fdx); //check all client sockets
   }
   
   //find the max socket number to select
   socketMax = gServerSock; //start at server socket
   for (ndx = 0; ndx < gClientCount; ndx++)
   {
      if (gClients[ndx].sock > socketMax)
      {
         socketMax = gClients[ndx].sock;
      }
   }

   if (select(socketMax + 1, (fd_set *) &fdx, NULL, NULL, NULL) < 0)
   {
      perror("select");
      exit(-1);
   }
   if (FD_ISSET(gServerSock, &fdx))
   {
      safeAccept();
   }
   
   for (ndx = 0; ndx < gClientCount; ndx++)
   {
      if (FD_ISSET(gClients[ndx].sock, &fdx))
      {
         // printf("got a message from %d. handling...\n", gClients[ndx].sock);
         handle_message(gClients[ndx].sock);
         
      }
   }
}

//get the position of the client
int getClient(int socknum)
{
   int ndx;
   for (ndx = 0; ndx < gClientCount; ndx++)
   {
      if (gClients[ndx].sock == socknum)
      {
         return ndx;
      }
   }
   
   return -1;
}

//get the socket of the client
int clientExists(char *handle)
{
   int ndx;
   for (ndx = 0; ndx < gClientCount; ndx++)
   {
      if (gClients[ndx].handle && strcmp(gClients[ndx].handle, handle) == 0)
      {
         return gClients[ndx].sock;
      }
   }
   
   return 0;
}

//handle the packet from the server
void handle_message(int sock)
{
   int meslen = 0, ndx = 0;
   int clientsock = getClient(sock);
   bzero(gBuff, MAXBUF);
   if ( (meslen = recv(sock, gBuff, MAXBUF, 0)) < 0)
   {
      perror("recv");
      exit(-1);
   }
   if (meslen == 0) //client quit
   {
      //shift and delete client
      for (ndx = clientsock; ndx < gClientCount + 1; ndx++)
      {
         gClients[ndx] = gClients[ndx + 1];
      }
      gClientCount--;
   }
   else
   {
      int flg = gBuff[2];
      
      if (flg == C_INIT)
      {
         // printf("got an init from %d\n", sock);
         getInit(sock, gBuff, meslen);
      }
      if (flg == C_BROADCAST)
      {
         // printf("got a broadcast request\n");
         getBroad(sock, gBuff, meslen);
      }
      if (flg == C_EXIT)
      {
         // printf("got an exit request\n");
         s_exit(sock, gBuff, meslen);
      }
      if (flg == C_MESSAGE)
      {
         getMessage(sock, gBuff, meslen);
      }
      if (flg == C_LIST)
      {
         getList(sock, gBuff, meslen);
      }
      memset(&gBuff[0], 0, sizeof(gBuff));
   }
}

//get broadcast packet from client
void getBroad(int sock, char * buffer, int messlen)
{
   char * handle;
   char * outmsg;
   char * temp = buffer;
   int handleLen = strlen(buffer);
   int ndx = 0;
   
   handle = malloc(handleLen + 1);
   memcpy(handle, buffer + 3 + 1, handleLen); //3 for chat header, 1 for length
   handle[handleLen] = '\0'; //null terminate for string functions
   buffer += 4 + handleLen;
   outmsg = malloc(MAXBUF);
   strcpy(outmsg, buffer);
   
   for (ndx = 0; ndx < gClientCount; ndx++)
   {
      if (gClients[ndx].sock != sock) //send to everyone but the one who %B'd
      {
         safeSend(gClients[ndx].sock, temp, messlen);

      }
   }
   
}

//get list packet from client
void getList(int sock, char * buff, int messlen)
{
   int ndx = 0;
   //send list count first
   sendNumHandles(sock);
   //then send all packets in a row for the handles
   for (ndx = 0; ndx < gClientCount; ndx++)
   {
      sendHandle(sock, gClients[ndx].handle);
   }
   
}

//send the handle (flag = 12)
void sendHandle(int sock, char * handle)
{
   // printf("sending handle to %d\n", sock);
   char * packet;
   char * ptr;
   int packetLen = 0;
   packetLen = 3 + 1 + strlen(handle);
   packet = calloc(packetLen, sizeof(char));
   ptr = packet;
   *ptr = htons(packetLen);
   ptr += 2;
   memset(ptr, F_HANDLE, 1);
   ptr++;
   memset(ptr, strlen(handle), 1);
   ptr++;
   memcpy(ptr, handle, strlen(handle) + 1);
   packet[1] = htons(packetLen) & 0xff; //pack lower eight bits
   packet[0] = htons(packetLen) >> 8; //pack upper eight bits
   safeSend(sock, packet, packetLen);
   free(packet);

}

//send the number of handles
void sendNumHandles(int sock)
{
   char * packet, *ptr;
   int packLen = 3 + 4; //chat header + 4 byte number for how many clients
   packet = malloc(packLen);
   ptr = packet;
   *ptr = htons(packLen);
   ptr += 2;
   memset(ptr, S_LIST, 1);
   ptr++;   
   *ptr = gClientCount;
   packet[1] = htons(packLen) & 0xff; //pack lower eight bits
   packet[0] = htons(packLen) >> 8; //pack upper eight bits
   safeSend(sock, packet, packLen);
   
}

//get the message packet from the server
void getMessage(int sender_sock, char * buff, int messlen)
{
   char * dstHandle;
   int dstSock;
   int dstHandleLen = (int) *(buff + 3);
   dstHandle = malloc(dstHandleLen + 1);
   memcpy(dstHandle, buff + 4, dstHandleLen);
   dstSock = clientExists(dstHandle);
   
   if (dstSock != 0)
   {
      safeSend(dstSock, buff, messlen); //just have to forward message packet to client
   }
   else //client doesnt exist
   {
      int packetLen = 3 + 1 + dstHandleLen;
      char * packet = malloc(packetLen);
      char *ptr = packet;
      *ptr = htons(packetLen);
      ptr += 2;
      memset(ptr, S_MSG_ERR, 1);
      ptr ++;
      memset(ptr, dstHandleLen, 1);
      ptr++;
      memcpy(ptr, dstHandle, dstHandleLen);
      packet[1] = htons(packetLen) & 0xff; //pack lower eight bits
      packet[0] = htons(packetLen) >> 8; //pack upper eight bits
      safeSend(sender_sock, packet, packetLen);
   }
}

//sending exit ack to client
void s_exit(int sock, char * buff, int messlen)
{
   // printf("making %d exit\n", sock);

   char *packet, *ptr;
   int packetLen = htons(3);
   
   packet = malloc(4);
   ptr = packet;
   
   *ptr = packetLen;
   ptr += 2;
   ptr[0] = 9; // dont know why i did it this way but w/e

   safeSend(sock, packet, packetLen);
}

//get initial packet and check if client exists
void getInit(int sock, char *buffer, int messlen)
{
   char *clientHandle, *packet, *ptr;
   int packLen = 3;
   int ndx = 0;

   uint8_t handleLen = (uint8_t) *(buffer + 3);
   clientHandle = (char *) malloc(handleLen + 1);
   memcpy(clientHandle, gBuff + 4, handleLen);
   clientHandle[handleLen] = '\0'; //null terminate for string functions
   
   if (clientExists(clientHandle))
   {
      packet = (char *)malloc(packLen);
      ptr = packet;
      *ptr = packLen;
      ptr += 2;
      memset(ptr, S_INIT_ERR, 1);
      packet[1] = htons(packLen) & 0xff; //pack lower eight bits
      packet[0] = htons(packLen) >> 8; //pack upper eight bits
      safeSend(sock, packet, packLen);
   }
   else
   {
      // printf("NEW CLIENT!\n");
      ndx = getClient(sock);
      gClients[ndx].handle = clientHandle;
      strcpy(gClients[ndx].handle, clientHandle);
   
      packet = malloc(packLen);
      ptr = packet;
      *ptr = packLen;
      ptr += 2;
      memset(ptr, S_INIT_GOOD, 1);
      packet[1] = packLen & 0xff;
      packet[0] = packLen >> 8;
      safeSend(sock, packet, packLen);
   }
}
