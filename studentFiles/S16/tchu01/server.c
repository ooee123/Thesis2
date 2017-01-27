/******************************************************************************
 * Timothy Chu 3pm Lab
 * CPE464 Program2
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
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#include "testing.h"
#include "networks.h"
#include "clientserver.h"

int main(int argc, char *argv[]) {
   int server_socket = 0;   //socket descriptor for the server socket

   printf("sockaddr: %d sockaddr_in %d\n", sizeof(struct sockaddr), 
    sizeof(struct sockaddr_in));
    
   /* create the server socket */
   server_socket = setUpServer(argc, argv);
   /* start chat service */
   serverChat(server_socket);
   /* close the sockets */
   close(server_socket);
   return 0;
}

int setUpServer(int argc, char **argv) {
   int portNum = 0;

   if (argc > 2) {
      printf("usage: %s <optional: port-number>\n", argv[0]);
      exit(-1);
   }
   if (argc == 2) 
      portNum = atoi(argv[1]);

   return tcp_recv_setup(portNum);
}

/* This function sets the server socket. The function returns the server
   socket number and prints the port number to the screen.  */
int tcp_recv_setup(int portNum) {
   int server_socket= 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */

   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_STREAM, 0);
   if (server_socket < 0) {
      perror("socket call");
      exit(-1);
   }

   local.sin_family= AF_INET;          //internet family
   local.sin_addr.s_addr= INADDR_ANY;  //wild card machine address
   local.sin_port= htons(portNum);     
   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
   	perror("bind call");
	   exit(-1);
   }
    
   //get the port name and print it out
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
	   perror("getsockname call");
	   exit(-1);
   }
   printf("Server is using port %d\n", ntohs(local.sin_port));
	        
   return server_socket;
}

void serverChat(int server_socket) {
   char **handles = NULL;
   int numHandles = 0;
   int highestFD = server_socket + 1;

   while (1) { 
      serviceServerFDS(server_socket, &handles, &numHandles,
       &highestFD);
   } 
}

void serviceServerFDS(int server_socket, char ***handles, 
 int *numHandles, int *highestFD) {
   fd_set read_fd_set;
   int sel, i;

   FD_ZERO(&read_fd_set);
   FD_SET(server_socket, &read_fd_set);

   for (i = 0; i < *numHandles; i++) {
      int32_t tmpFD;
      memcpy(&tmpFD, (*handles)[i], sizeof(int32_t));
      FD_SET(tmpFD , &read_fd_set);
   } 
 
   /* listen to server socket for incoming connections */
   if (listen(server_socket, 5) < 0) {
      perror("listen call");
      exit(-1);
   }

   sel = select(*highestFD, &read_fd_set, NULL, NULL, NULL);
   if (sel < 0) {
      perror("select call");
      exit(-1);
   }

   if (FD_ISSET(server_socket, &read_fd_set)) {
      processConnections(server_socket, highestFD, handles, numHandles);
   } else {
      processClientData(handles, numHandles, highestFD, read_fd_set);
   }
}

void processConnections(int server_socket, int *highestFD, char ***handles, int *numHandles) {
   int recvd, message_len;   
   char *recv_buf;

   //accept the connection
   int32_t client_socket = tcp_listen(server_socket, 5);
   if (client_socket + 1 > *highestFD) {
      *highestFD = client_socket + 1;
   }

   //get the normal header
   normalHeader nh;
   recvd = recv(client_socket, &nh, sizeof(normalHeader), MSG_WAITALL);
   checkClientInitialPacket(recvd, client_socket);

   if (recvd != sizeof(normalHeader)) {
      printf("Did not correctly receive initial packet normal header\n");
      close(client_socket);
      return;
   }
   
   if(nh.flag != C_TO_S_INITIAL_PACKET) {
      printf("Did not receive correct (initial) packet from client!\n");
      close(client_socket);
      return;
   } else {
      message_len = ntohs(nh.packetLen) - sizeof(normalHeader);
      recv_buf = malloc(message_len);
      recvd = recv(client_socket, recv_buf, message_len, 
       MSG_WAITALL);
      checkClientInitialPacket(recvd, client_socket);

      if (recvd != message_len) {
         printf("Did not correctly receive rest of packet\n");
         close(client_socket);
         return;
      }
      initialPacketResponse(client_socket, recv_buf, handles, 
       numHandles);
      free(recv_buf);
   }
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */
int tcp_listen(int server_socket, int back_log) {
   int client_socket = 0;

   if ((client_socket = accept(server_socket, (struct sockaddr *) 0, 
    (socklen_t *)0)) < 0) {
      perror("accept call");
      exit(-1);
   }
  
   return(client_socket);
}

void checkClientInitialPacket(int recvd, int client_socket) {
   if (recvd < 0) {
      perror("recv call");
      exit(-1);
   }
   if (recvd == 0) {
      printf("Client Terminated.\n");
      close(client_socket);
   }
}

/*
 * Server to client response when it receives a packet with flag = 1.
 */
void initialPacketResponse(int client_socket, char *recv_buf, 
 char ***handles, int *numHandles) {
   /* check handle and do necessary set up */
   int exist = verifyHandle(client_socket, recv_buf, handles, numHandles);
   
   /* reply to client */
   ackHandle(client_socket, exist);
}

/*
 * The handles list keeps track of the associated FD as well. First 4 bytes is the fd,
 * the next 1 byte is the handle length, and the rest is the handle.
 */
int verifyHandle(int client_socket, char *recv_buf, char ***handles, int *numHandles) {
   uint8_t handleLen;
   memcpy(&handleLen, recv_buf, sizeof(uint8_t));
   char *handle = malloc(sizeof(int32_t) + sizeof(uint8_t) + handleLen);

   /* embed fd */
   memcpy(handle, &client_socket, sizeof(int32_t));
   /* embed recv_buf */
   memcpy(handle + sizeof(int32_t), recv_buf, handleLen + sizeof(uint8_t));

   /* check for existing handle */
   int i;
   int exist = 0;
   for (i = 0; i < *numHandles; i++) {
      int temp = compareHandle(handle, handles, i);
      if (temp == 1) {
         exist = 1;
         break;
      }
   }

   if (exist == 1) {
   } else {
      /* add to handles list */
      *handles = realloc(*handles, sizeof(char *) * (*numHandles + 1));
      (*handles)[*numHandles] = handle;
      (*numHandles)++;
   }

   return exist;
}

int compareHandle(char *handle, char ***handles, int i) {
   uint8_t handleLen;
   char handleBuf[256];
   uint8_t chkLen;
   char chkBuf[256];
   
   memcpy(&handleLen, handle + sizeof(int32_t), sizeof(uint8_t));
   memcpy(handleBuf, handle + sizeof(int32_t) + sizeof(uint8_t), handleLen);
   handleBuf[handleLen] = '\0';
   memcpy(&chkLen, (*handles)[i] + sizeof(int32_t), sizeof(uint8_t));
   memcpy(chkBuf, (*handles)[i] + sizeof(int32_t) + sizeof(uint8_t), chkLen);
   chkBuf[chkLen] = '\0';
   
   if (strcmp(handleBuf, chkBuf) == 0) {
      return 1;
   } else {
      return 0;
   }
}

void ackHandle(int client_socket, int exist) {
   int sent = 0;

   /* reply with flag 2 */
   if (exist == 0) {
      normalHeader nhreply = {htons(sizeof(normalHeader)), 
       S_TO_C_GOOD_HANDLE};
      sent = send(client_socket, &nhreply, sizeof(normalHeader), 0);
      
      if (sent < 0) {
         perror("send call");
         exit(-1);
      }
   /* reply with flag 3 */
   } else { 
      normalHeader nhreply = {htons(sizeof(normalHeader)), S_TO_C_BAD_HANDLE};
      sent = send(client_socket, &nhreply, sizeof(normalHeader), 0);
      
      if (sent < 0) {
         perror("send call");
         exit(-1);
      }
      close(client_socket);
   }
}

void processClientData(char ***handles, int *numHandles, int *highestFD, fd_set read_fd_set) {
   int recvd;
   char *recv_buf = NULL;
   uint16_t dataLen;
   int i = 0;

   for (i = 0; i < *numHandles; i++) {
      int32_t tempfd;
      memcpy(&tempfd, (*handles)[i], sizeof(int32_t));
      if (FD_ISSET(tempfd, &read_fd_set)) {
         while((recvd = receiveClientData(handles, numHandles, tempfd, &recv_buf, &dataLen)) == 0);
         if (recvd != -1) 
            replyToClient(tempfd, recv_buf, recvd, handles, numHandles);
         if(recv_buf != NULL) {
            free(recv_buf);
         }
      }
   }
}

int receiveClientData(char ***handles, int *numHandles, int socket_num, 
 char **recv_buf, uint16_t *packetLen) {
   enum STATES {START_STATE, PROCESS_LENGTH, GET_DATA};
   static enum STATES cur = START_STATE;
   static uint8_t msgLen[2] = {0};
   int recvd = 0;
   static int dataLen = 0;
   int status;

   switch (cur) {
      case START_STATE:
         recvd = recv(socket_num, msgLen, sizeof(uint16_t),
          MSG_WAITALL);
         status = checkReceivedClientData(handles, numHandles, recvd, socket_num);
         if (status == -1) {
            return -1;
         }

         if (recvd == 1) {
            printf("MSG_WAITALL did not block for the full message\n");
            cur = PROCESS_LENGTH;
         } else { //receivedBytes == 2
            memcpy(packetLen, msgLen, sizeof(uint16_t));
            *packetLen = ntohs(*packetLen);
            *recv_buf = malloc(*packetLen - sizeof(uint16_t));
            cur = GET_DATA;
         }
         break;
      case PROCESS_LENGTH:
         recvd = recv(socket_num, msgLen + 1, sizeof(uint8_t),
          MSG_WAITALL);
         status = checkReceivedClientData(handles, numHandles, recvd, socket_num);
         if (status == -1) {
            return -1;
         }

         memcpy(packetLen, msgLen, sizeof(uint16_t));
         *packetLen = ntohs(*packetLen);
         *recv_buf = malloc(*packetLen - sizeof(uint16_t));
         cur = GET_DATA;
         break;
      case GET_DATA:
         recvd = recv(socket_num, *recv_buf, *packetLen - sizeof(uint16_t), MSG_WAITALL);
         checkReceivedClientData(handles, numHandles, recvd, socket_num);

         if (recvd + dataLen != *packetLen - sizeof(uint16_t)) {
            printf("MSG_WAITALL did not block for the full message\n");
            dataLen += recvd;
         } else {
            cur = START_STATE;
            dataLen = 0;
            return *packetLen;
         }
         break;
   }
   return 0;
}

int checkReceivedClientData(char ***handles, int *numHandles, int recvd, int client_socket) {
   if (recvd < 0) {
      perror("recv call");
      exit(-1);
   }
   if (recvd == 0) {
      printf("Client Terminated.\n");

      int i, tmpfd;
      /* remove from handles list */
      for(i = 0; i < *numHandles; i++) {
         memcpy(&tmpfd, (*handles)[i], sizeof(int32_t));
         if(tmpfd == client_socket) {
            free((*handles)[i]);
            removeHandle(tmpfd, handles, numHandles, i);
            break;
         }
      }
      return -1;     
   }
   return 0;
}

/*
 * 1st byte = flag. Next byte = handle length. Handle. Msg.
 * message_len = packet length
 */
void replyToClient(int client_socket, char *buf, int message_len, 
 char ***handles, int *numHandles) {
   uint8_t flag;

   memcpy(&flag, buf, sizeof(uint8_t));
   if (flag == C_TO_S_BROADCAST) {
      forwardBroadcast(client_socket, buf, message_len, handles, numHandles);
   } else if (flag == C_TO_S_MESSAGE) {
      forwardMessage(client_socket, buf, message_len, handles, numHandles);
   } else if (flag == C_TO_S_LIST_HANDLES) {
      ackListNum(client_socket, numHandles);
      ackListHandles(client_socket, handles, numHandles);
   } else if (flag == C_TO_S_EXIT) {
      ackExit(client_socket, handles, numHandles);
   } else {
   }
}

void forwardBroadcast(int client_socket, char *buf, int message_len, 
 char ***handles, int *numHandles) {
   int sent, pkt_len, data_len, i;
   int32_t tmpfd;
   char *send_buf;
   uint8_t srcLen;
   char srcHandle[MAX_HANDLE_LEN];

   memcpy(&srcLen, buf + sizeof(uint8_t), sizeof(uint8_t));
   memcpy(srcHandle, buf + (2 * sizeof(uint8_t)), srcLen);
   srcHandle[srcLen] = '\0';

   data_len = message_len - sizeof(normalHeader) - sizeof(uint8_t) - srcLen;
   pkt_len = sizeof(normalHeader) + sizeof(uint8_t) + srcLen + data_len;
   normalHeader nh = {htons(pkt_len), 0};
   send_buf = malloc(pkt_len);
   
   memcpy(send_buf, &nh, sizeof(normalHeader));
   memcpy(send_buf + sizeof(normalHeader), &srcLen, sizeof(uint8_t));
   memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t), srcHandle, srcLen);
   memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t) + srcLen,
    buf + (2 * sizeof(uint8_t)) + srcLen, data_len);

   for (i = 0; i < *numHandles; i++) {
      memcpy(&tmpfd, (*handles)[i], sizeof(int32_t));
      if (tmpfd != client_socket) {
         sent = send(tmpfd, send_buf, pkt_len, 0);
         if (sent < 0) {
            perror("send call");
            exit(-1);
         }
      }
   }
   free(send_buf);
}

void forwardMessage(int client_socket, char *buf, int message_len, 
 char ***handles, int *numHandles) {
   int sent, pkt_len, data_len, destFD;
   char *send_buf, destHandle[MAX_HANDLE_LEN], srcHandle[MAX_HANDLE_LEN];
   uint8_t destHandleLen, srcHandleLen;
   int error = destinationError(buf, handles, numHandles, &destFD);
   memcpy(&destHandleLen, buf + sizeof(uint8_t), sizeof(uint8_t));
   memcpy(destHandle, buf + (2 * sizeof(uint8_t)), destHandleLen);

   if (error == S_TO_C_NO_HANDLE) {
      pkt_len = sizeof(normalHeader) + sizeof(uint8_t) + 
       destHandleLen;
      send_buf = malloc(pkt_len);
      
      normalHeader nh = {htons(pkt_len), error};
      memcpy(send_buf, &nh, sizeof(normalHeader));
      memcpy(send_buf + sizeof(normalHeader), &destHandleLen, sizeof(uint8_t));
      memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t), destHandle, 
       destHandleLen);

      sent = send(client_socket, send_buf, pkt_len, 0);
      if (sent < 0) {
         perror("send call");
         exit(-1);
      }
      free(send_buf);
   } else {
      memcpy(&srcHandleLen, buf + (2 * sizeof(uint8_t)) + destHandleLen, sizeof(uint8_t));
      memcpy(srcHandle, buf + (2 * sizeof(uint8_t)) + destHandleLen + sizeof(uint8_t), 
       srcHandleLen);
      data_len = message_len - sizeof(normalHeader) - (2 * sizeof(uint8_t)) - 
       destHandleLen - srcHandleLen;
      pkt_len = sizeof(normalHeader) + sizeof(uint8_t) + srcHandleLen + data_len;
      send_buf = malloc(pkt_len);

      normalHeader nh = {htons(pkt_len), error};
      memcpy(send_buf, &nh, sizeof(normalHeader));
      memcpy(send_buf + sizeof(normalHeader), &srcHandleLen, sizeof(uint8_t));
      memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t), srcHandle, srcHandleLen);
      memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t) + srcHandleLen, buf + (2 * 
       sizeof(uint8_t)) + destHandleLen + sizeof(uint8_t) + srcHandleLen, data_len);

      sent = send(destFD, send_buf, pkt_len, 0);
      if (sent < 0) {
         perror("send call");
         exit(-1);
      }
      free(send_buf);
   }
}

int destinationError(char *buf, char ***handles, 
 int *numHandles, int *destFD) {
   int i;
   uint8_t handleLen;
   char handle[MAX_HANDLE_LEN];
   uint8_t chkLen;
   char chkHandle[MAX_HANDLE_LEN];
   memcpy(&handleLen, buf + sizeof(uint8_t), sizeof(uint8_t));
   memcpy(handle, buf + (2 * sizeof(uint8_t)), handleLen); 
   handle[handleLen] = '\0';

   for (i = 0; i < *numHandles; i++) {
      memcpy(&chkLen, (*handles)[i] + sizeof(int32_t), sizeof(uint8_t));
      memcpy(chkHandle, (*handles)[i] + sizeof(int32_t) + sizeof(uint8_t), chkLen);
      chkHandle[chkLen] = '\0';
      if (strcmp(handle, chkHandle) == 0) {
         memcpy(destFD, (*handles)[i], sizeof(int32_t));
         return 0;
      }
   }
   return S_TO_C_NO_HANDLE;
}

void ackListNum(int socket_num, int *numHandles) {
   int sent, sendNum;
   char *send_buf = malloc(sizeof(normalHeader) + sizeof(int32_t));
   normalHeader nh = {htons(sizeof(normalHeader) + sizeof(int32_t)), S_TO_C_LIST_HANDLES_ACK};
   memcpy(send_buf, &nh, sizeof(normalHeader));
   sendNum = htonl(*numHandles);
   memcpy(send_buf + sizeof(normalHeader), &sendNum, sizeof(int32_t));

   sent = send(socket_num, send_buf, sizeof(normalHeader) + sizeof(int32_t), 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
   free(send_buf);
}

void ackListHandles(int socket_num, char ***handles, int *numHandles) {
   int sent, i;
   uint8_t tmpLen;
   char handle[MAX_HANDLE_LEN];
   uint16_t pkt_len = sizeof(normalHeader); //packetLen
   int offset = sizeof(normalHeader); //for memcpy
   char *send_buf = malloc(sizeof(normalHeader));

   for(i = 0; i < *numHandles; i++) {
      memcpy(&tmpLen, (*handles)[i] + sizeof(int32_t), sizeof(uint8_t));
      memcpy(handle, (*handles)[i] + sizeof(int32_t) + sizeof(uint8_t), tmpLen);
      pkt_len += sizeof(uint8_t) + tmpLen; 

      send_buf = realloc(send_buf, pkt_len);
      memcpy(send_buf + offset, &tmpLen, sizeof(uint8_t));
      offset += sizeof(uint8_t);
      memcpy(send_buf + offset, handle, tmpLen);
      offset += tmpLen;
   }
   normalHeader nh = {htons(pkt_len), S_TO_C_LIST_HANDLES};
   memcpy(send_buf, &nh, sizeof(normalHeader));
   
   sent = send(socket_num, send_buf, pkt_len, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
   free(send_buf);
}

void ackExit(int socket_num, char ***handles, int *numHandles) {
   int sent, i;
   int32_t tmpfd;
   normalHeader nh = {htons(sizeof(normalHeader)), S_TO_C_ACK_EXIT};

   sent = send(socket_num, &nh, sizeof(normalHeader), 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
   /* remove from handles */
   for(i = 0; i < *numHandles; i++) {
      memcpy(&tmpfd, (*handles)[i], sizeof(int32_t));
      if(tmpfd == socket_num) {
         free((*handles)[i]);
         removeHandle(tmpfd, handles, numHandles, i);
         break;
      }
   }
}

void removeHandle(int socket_num, char ***handles, int *numHandles, int i) {
   int j;
   for (j = 0; j < *numHandles - i - 1; j++) {
      (*handles)[i + j] = (*handles)[i + j + 1];
   }
   *handles = realloc(*handles, sizeof(char *) * (*numHandles - 1));
   (*numHandles)--;
   close(socket_num); 
}
