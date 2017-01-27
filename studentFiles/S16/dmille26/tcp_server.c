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
#include "chat.h"
#include "tcp_server.h"


void parseArgs(int argc, char* argv[], int* port) {
   if (argc == 1) {
      port = 0; //let system decide port
   } else if (argc == 2) {
      *port = atoi(argv[1]);
   } else {
      printf("Incorrect Number of paramaters.\n");
      printf("%s server [port]\n", argv[0]);
   }
}

void setTimeStruct(struct timeval* t, int sec, int usec) {
   t->tv_sec = sec;
   t->tv_usec = usec;
}
void checkForData(int * status, int maxFd, fd_set* rfds, fd_set* wfds, fd_set* 
      efds, struct timeval* tv) {
   *status = select(maxFd, rfds, wfds, efds, NULL);
   if (*status == -1)
      perror("select call.");
}
int checkOtherHandles(clientTable* table, char* handle) {
   struct client* tmp = table->head;
   int i;
   int validHandle= 1;
   for (i = 0; i < table->count -1; i++) {
      if (strcmp(tmp->handle, handle) == 0) {
         validHandle = 0;
         break;
      }
      tmp = tmp->next;
   }
   strcpy(table->tail->handle, handle);
   return validHandle;
}
void sendHdrOnlyReply(struct client* client, int flags) {
   u_char packet[NORMAL_HDR_LEN];
   uint16_t tmp = NORMAL_HDR_LEN;
   u_char flg = flags;
   tmp = htons(tmp);
   memcpy(packet, &tmp, sizeof(uint16_t));
   packet[2] = flg;
   int sent =  send(client->socket, packet, NORMAL_HDR_LEN, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
}


   
void claimingHandle(struct client* client, clientTable* table) {
   
   int offset = 0;
   u_char lenOfHandle = client->buf[offset];
  
   offset++;
   memcpy(client->handle, client->buf + offset, lenOfHandle); 
   client->handle[lenOfHandle] = '\0';
   
 
   if (checkOtherHandles(table, client->handle) == 0) {
      sendHdrOnlyReply(client, S_C_ERR_HNDL_EXISTS);
      removeNewClient(table);
   } else {
      sendHdrOnlyReply(client, S_C_GOOD_HNDL);
   }
}      
int getPacketLen(u_char* buf) {
   uint16_t pktLen;
   memcpy(&pktLen, buf, sizeof(uint16_t));
   return ntohs(pktLen);
}
void sendDstNotFound(int socket, char* handle, int lenOfHandle) {
   int size = NORMAL_HDR_LEN + lenOfHandle+ 1;
   u_char packet[size];
   uint16_t pktLen = htons(size);
   memcpy(packet, &pktLen, sizeof(uint16_t));
   packet[2] = S_C_ERR_DST_NEXISTS;
   packet[3] = lenOfHandle;

   memcpy(packet + 4, handle, lenOfHandle);

   int sent =  send(socket, packet, size, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
}
int getSocketByHandle(clientTable* table, int src_socket, char* handle, 
                      int lenOfHandle)
{
   struct client* tmp = table->head;
   char lookUpHandle[lenOfHandle + 1];
   memcpy(lookUpHandle, handle, lenOfHandle);
   lookUpHandle[lenOfHandle] = '\0';
   int i;
   for (i = 0; i < table->count; i++, tmp = tmp->next) {
   
  
      if (strcmp(tmp->handle, lookUpHandle) == 0) {
         return tmp->socket;
      }
   }
   return -1;
}
int getBroadcastData(u_char* out, u_char* buf) {
   uint32_t lenSrcHandle = (int)buf[0];
   int offset = lenSrcHandle + 1;
   strcpy(out, buf + offset);
   return strlen(out) + 1;
}
int getData(u_char* out, u_char* buf) {
   int lenDstHandle = buf[0];
   int lenSrcHandle = buf[lenDstHandle + 1];
   int offset = lenDstHandle + 1 + lenSrcHandle + 1;
   strcpy(out, buf + offset);
   return strlen(out) + 1;
}
void msgAll(int src_socket, struct msgPkt msg, clientTable* table, u_char* packet) {
   int i;
   struct client* cur = table->head;
   for (i = 0; i < table->count; i++, cur = cur->next) {
      if (cur->socket != src_socket) {
         int sent =  send(cur->socket, packet, msg.pktLenHost, 0);
         if (sent < 0) {
            perror("send call");
            exit(-1);
         }
      }
   }
}


int broadcastMessage(struct client* client, clientTable* table, 
                  int bufLen) {
   struct msgPkt msg;
   msg.flags = C_S_C_DIR_MSG;
   msg.lenSrcHandle = client->buf[0];
   memcpy(msg.srcHandle, client->buf + 1, msg.lenSrcHandle);
   int sizeOfData = getBroadcastData(msg.msgData, client->buf);
   setMsgLen(&msg, NORMAL_HDR_LEN + msg.lenSrcHandle + 1 + sizeOfData);
   u_char* packet = calloc(1, msg.pktLenHost);
   int offset = cpyHdr(&msg, packet);
   offset = cpyCli(&msg, packet, offset); 
   strcpy(packet + offset, msg.msgData);
   msgAll(client->socket, msg, table, packet);
   
   return 1;
}
int directMessage(struct client* client, clientTable* table, 
                  int bufLen) {
   struct msgPkt msg;
   msg.flags = C_S_C_DIR_MSG;
   msg.lenDstHandle = client->buf[0];
   msg.lenSrcHandle = client->buf[msg.lenDstHandle +1];
   memcpy(msg.dstHandle, client->buf + 1, msg.lenDstHandle);
   memcpy(msg.srcHandle, client->buf + msg.lenDstHandle + 2, msg.lenSrcHandle);
   int sizeOfData = getData(msg.msgData, client->buf);
   int dstSocket = getSocketByHandle(table, client->socket, msg.dstHandle,
                                     msg.lenDstHandle); 
   if (dstSocket == -1) {
      sendDstNotFound(client->socket, msg.dstHandle, msg.lenDstHandle);
      return -1;
   }
   setMsgLen(&msg, NORMAL_HDR_LEN + msg.lenSrcHandle + 1 + sizeOfData);
   u_char* packet = calloc(1, msg.pktLenHost);
   int offset = cpyHdr(&msg, packet);
   offset = cpyCli(&msg, packet, offset); 
   strcpy(packet + offset, msg.msgData);
   int sent =  send(dstSocket, packet, msg.pktLenHost, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
   return 1;
}
void ackExit(int socket) {
   char packet[NORMAL_HDR_LEN];
   uint16_t size = NORMAL_HDR_LEN;
   size = htons(size);
   memcpy(packet, &size, sizeof(uint16_t));
   packet[2] = S_C_ACK_EXT;
   int sent =  send(socket, packet, NORMAL_HDR_LEN, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
}
void replyCntActive(int socket, int count) {
   char packet[NORMAL_HDR_LEN + 4];
   uint16_t size = NORMAL_HDR_LEN;
   size = htons(size);
   memcpy(packet, &size, sizeof(uint16_t));
   packet[2] = S_C_NUM_HNDLS;
   int numHandles = htonl(count);
   memcpy(packet + NORMAL_HDR_LEN, &numHandles, sizeof(numHandles));
   int sent =  send(socket, packet, NORMAL_HDR_LEN, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
}
int addHandleToPkt(u_char* packet, char* handle, int offset) {
   int len = strlen(handle);
   packet[offset] = len;
   offset++;
   memcpy(packet + offset, handle, len);
   return offset + len;
}
void replyListOfActive(clientTable* table, int socket) {
   int i;
   int offset = NORMAL_HDR_LEN;
   struct client* tmp = table->head;
   uint32_t numHandles = htonl(table->count);
   int size = 0;
   uint16_t pktLen;
   for (i = 0, tmp = table->head; i < table->count; i++, tmp = tmp->next) {
      size += strlen(tmp->handle) + 1;
   }
   char packet[size + NORMAL_HDR_LEN + 4];
   pktLen = size + NORMAL_HDR_LEN + 4;
   pktLen = htons(pktLen);
   memcpy(packet, &pktLen, sizeof(uint16_t));
   packet[2] = S_C_VALID_HNDL;
   memcpy(packet + offset, &numHandles, sizeof(uint32_t));
   offset += 4;
   for (i = 0, tmp = table->head; i < table->count; i++, tmp = tmp->next) {
      offset = addHandleToPkt(packet, tmp->handle, offset);
   }
   int sent =  send(socket, packet, size + NORMAL_HDR_LEN + 4, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
}

void parseData(struct client* client, clientTable* table, int bufLen, 
               int flags) {
   if (flags == C_S_INIT) {
      claimingHandle(client, table);
   } else if (flags == C_A_BROADCAST) {
      broadcastMessage(client, table, bufLen);
   } else if (flags == C_S_C_DIR_MSG) {
      directMessage(client, table, bufLen); 
   } else if (flags == C_S_REQ_EXT) {
      ackExit(client->socket);
   } else if (flags == C_S_REQ_HNDLS) {
      replyCntActive(client->socket, table->count);
      replyListOfActive(table, client->socket);

   } else {
      perror("Unexpected flag value.");
   }

}

void killDeadClients(clientTable* table, int* rmvClients, int deadClients, 
   fd_set* rfds) 
   {
   int i = 0;
   int j = 0;
   for (i = 0; i < deadClients; i++) {
      int rmvCli = removeClient(table, rmvClients[j]);
   }
   FD_ZERO(rfds);
}
int getPacketSize(u_char* buf) {
   uint16_t size;
   memcpy(&size, buf, sizeof(uint16_t));
   size = ntohs(size);
   return size;
}
void readMsgData(struct client* cur, int lenData) {
   int message_len = 0;
   if (lenData > NORMAL_HDR_LEN) {
      if ((message_len = recv(cur->socket, cur->buf,
           lenData - NORMAL_HDR_LEN, 0)) < 0) {
         perror("recv call");
         exit(-1);
      }
   }
}


void readAllData(fd_set* rfds, clientTable* table, int server_socket) {
   int i = 0;
   int message_len = 0;
   struct client* cur = table->head;
   u_char hdrBuf[NORMAL_HDR_LEN];
   int rmvClients[table->count];
   int deadClients = 0;
   int dataLen = 0;

   for (i = 0; i < table->count; i++) {
      if (FD_ISSET(cur->socket, rfds)) {
         if ((message_len = recv(cur->socket, hdrBuf, NORMAL_HDR_LEN, 0)) < 0) {
            perror("recv call");
            exit(-1);
         }
         if (message_len == NORMAL_HDR_LEN ) {
            dataLen = getPacketSize(hdrBuf); 
            readMsgData(cur, dataLen);
            parseData(cur, table, message_len + NORMAL_HDR_LEN, hdrBuf[2]);
         } else if (message_len == 0) {
            rmvClients[deadClients] = cur->socket;
            deadClients++;
         } else {
            printf("Malformed Packet. Message Len = %d\n", message_len);
            exit(-1);
         }
      }
      cur = cur->next;
   }
   killDeadClients(table, rmvClients, deadClients, rfds);
}
void resetFds(clientTable* table, fd_set* rfds, int server_socket) {
   int i = 0;
   struct client* cliPtr = table->head;
   FD_SET(server_socket, rfds);
   for (i = 0; i < table->count; i++) {
      FD_SET(cliPtr->socket, rfds);
      cliPtr = cliPtr->next;
   }
}
void runServer(int server_socket) {
   int status = 0;
   clientTable allClients = {0, NULL, NULL,  server_socket+1, ""};
   fd_set rfds;
   FD_ZERO(&rfds);
   while(1) {
      resetFds(&allClients, &rfds, server_socket); 
      checkForData(&status, allClients.maxFd, &rfds, NULL, NULL, NULL);
      checkServerSocket(&rfds, &allClients,  server_socket);
      readAllData(&rfds, &allClients, server_socket);
   }
}

int main(int argc, char *argv[])
{
   
   int port = 0;
   parseArgs(argc, argv, &port);

   int server_socket= 0;   //socket descriptor for the server socket
   //create the server socket
   server_socket= tcp_recv_setup(port);
   runServer(server_socket);
   /* close the sockets 
      close(server_socket);
      close(client_socket);
      Never close connection. */
   return 1;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup(int port)
{
   int server_socket= 0;
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
   local.sin_port= htons(port);                 //let system choose the port

   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
      exit(-1);
   }

   //get the port name and print it out
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0)
   {
      perror("getsockname call");
      exit(-1);
   }


   int  status = tcp_listen(server_socket, 5);
   return server_socket;
}


