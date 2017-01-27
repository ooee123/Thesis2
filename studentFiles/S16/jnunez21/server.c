/*
* Jeffrey Nunez
* CPE 464, 9am lab section
* 4/25/16
*
* Program 2 - server
*
* Builds off of the provided tcp_server.c
*/

#include "networks.h"

#pragma pack(1)

typedef struct Handle {
   char thisHandle[UCHAR_MAX];
   int thisFD;
   struct Handle *nextHandle;
} Handle;

/* This function sets the server socket.  It lets the system
   determine the port number, unless the user specifies
   a port number via commandline argument.  The function returns the server
   socket number and prints the port number to the screen.  */
int TcpRecvSetup(int optionalPortNumber) {
   int serverSocket = 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len = sizeof(local);  /* length of local address        */

   /* create the socket  */
   serverSocket = socket(AF_INET, SOCK_STREAM, 0);
   if(serverSocket < 0)
   {
      perror("socket call error");
      exit(1);
   }

   local.sin_family = AF_INET;         //internet family
   local.sin_addr.s_addr = INADDR_ANY; //wild card machine address
   local.sin_port = htons(optionalPortNumber);

   /* bind the name (address) to a port */
   if (bind(serverSocket, (struct sockaddr *)&local, sizeof(local)) < 0) {
      perror("bind call error");
      exit(-1);
   }
    
   /* get the port name and print it out */
   if (getsockname(serverSocket, (struct sockaddr *)&local, &len) < 0) {
      perror("getsockname call");
      exit(-1);
   }

   printf("socket has port %d \n", ntohs(local.sin_port));
	        
   return serverSocket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */
int TcpListen(int serverSocket, int backlog) {
   int clientSocket = 0;
  
   if (listen(serverSocket, backlog) < 0) {
      perror("listen call error");
      exit(-1);
   }
  
   if ((clientSocket = accept(serverSocket, (struct sockaddr*)0,
    (socklen_t *)0)) < 0) {
      perror("accept call error");
      exit(-1);
   }
  
   return(clientSocket);
}

Handle *addHandle(Handle *oldHead, char *handleToAdd, int socketNumber) {
   Handle *newHead = malloc(sizeof(Handle));

   memset(newHead->thisHandle, 0, UCHAR_MAX);
   memcpy(newHead->thisHandle, handleToAdd, strlen(handleToAdd));
   newHead->thisFD = socketNumber;
   
   newHead->nextHandle = oldHead;
   
   return newHead;
}

Handle *removeHandle(Handle *head, int sktNumToRemove) {
   Handle *temp, *toRemove, *rtn;

   /*if the first handle in LL is a match*/
   if (head != NULL && head->thisFD == sktNumToRemove) {
      if (head->nextHandle != NULL) {
         temp = head->nextHandle;
         memset(head->thisHandle, 0, UCHAR_MAX);
         free(head);
         rtn = temp;
      }
      else {
         memset(head->thisHandle, 0, UCHAR_MAX);
         free(head);
         rtn = NULL;
      }
   }

   /*general case*/
   else {
      for (temp = head; temp != NULL && temp->nextHandle != NULL &&
       temp->nextHandle->thisFD != sktNumToRemove; temp = temp->nextHandle)
         ;
      /*if the last handle in LL is a match*/
      if (temp->nextHandle == NULL && temp->thisFD == sktNumToRemove) {
         memset(temp->thisHandle, 0, UCHAR_MAX);
         free(temp);
         rtn = head;
      }
      /*general case*/
      else {
         toRemove = temp->nextHandle;
         temp->nextHandle = temp->nextHandle->nextHandle;
         memset(toRemove->thisHandle, 0, UCHAR_MAX);
         free(toRemove);
         rtn = head;
      }
   }

   return rtn;
}

/* Walk through linked list, checking to see if newest Handle is
 * already on the list.
 *
 * Returns 1 if it's already on the list, or 0 if it's NOT on the list */
int isHandleTaken(Handle *head, char *toCheck) {
   Handle *temp;

   for (temp = head; temp != NULL && strcmp(temp->thisHandle, toCheck);
    temp = temp->nextHandle)
      ;
   
   return (temp != NULL);
}

/* Returns the socket number of the requested handle.
 * If the requested name isn't on the list, return -1 */
int findSocketNumber(Handle *head, char *handleToFindNumberFor) {
   Handle *temp;
   
   for (temp = head; temp != NULL &&
    strcmp(temp->thisHandle, handleToFindNumberFor); temp = temp->nextHandle)
      ;

   return temp != NULL ? temp->thisFD : -1;
}

int getNumberOfHandles(Handle *head) {
   Handle *temp;
   int rtn = 0;

   for (temp = head; temp != NULL; temp = temp->nextHandle)
      ++rtn;

   return rtn;
}

/* Response to initial packet when the client's handle is good (available) */
void goodReply(int clientSocket) {
   normalHeader goodFlagHeader;

   goodFlagHeader.pktLen = sizeof(normalHeader);
   goodFlagHeader.pktLen = htons(goodFlagHeader.pktLen);
   goodFlagHeader.flg = GOOD_REPLY_FLG;

   safeSend(clientSocket, (char *)&goodFlagHeader, sizeof(normalHeader));
}

/* Response to initial packet when the client's handle is bad (already taken) */
void badReply(int clientSocket) {
   normalHeader badFlagHeader;

   badFlagHeader.pktLen = sizeof(normalHeader);
   badFlagHeader.pktLen = htons(badFlagHeader.pktLen);
   badFlagHeader.flg = BAD_REPLY_FLG;

   safeSend(clientSocket, (char *)&badFlagHeader, sizeof(normalHeader));
}

void sendAllHandles(int clientSocket, Handle *head) {
   normalHeader hdr;
   Handle *temp;
   uint8_t currHdlLen;

   hdr.pktLen = 0;
   hdr.pktLen = htons(hdr.pktLen);
   hdr.flg = LIST_FLG;

   safeSend(clientSocket, (char *)&hdr, sizeof(normalHeader));

   for (temp = head; temp != NULL; temp = temp->nextHandle) {
      currHdlLen = strlen(temp->thisHandle);
      safeSend(clientSocket, (char *)&currHdlLen, 1);

      safeSend(clientSocket, temp->thisHandle, currHdlLen);
   }
}

void sendNumberOfHandles(int clientSocket, Handle *head) {
   normalHeader hdr;
   int numHandles;

   hdr.pktLen = sizeof(normalHeader) + sizeof(int);
   hdr.pktLen = htons(hdr.pktLen);
   hdr.flg = LIST_NUM_FLG;
   numHandles = htonl(getNumberOfHandles(head));

   safeSend(clientSocket, (char *)&hdr, sizeof(normalHeader));

   safeSend(clientSocket, (char *)&numHandles, sizeof(int));
}

void broadcastToClients(Handle *head, unsigned short packetLength,
 uint8_t sourceHandleLength, char *sourceHandle, char *message) {
   normalHeader hdr;
   Handle *temp;
   int dstSckNum;
   int messageLen;

   hdr.pktLen = packetLength;
   hdr.pktLen = htons(hdr.pktLen);
   hdr.flg = BRDCST_FLG;

   messageLen = packetLength - sizeof(normalHeader) - sizeof(sourceHandleLength)
    - sourceHandleLength;

   for (temp = head; temp != NULL; temp = temp->nextHandle) {
      if (strcmp(temp->thisHandle, sourceHandle)) {
         dstSckNum = temp->thisFD;

         safeSend(dstSckNum, (char *)&hdr, sizeof(normalHeader));

         safeSend(dstSckNum, (char *)&sourceHandleLength, sizeof(char));

         safeSend(dstSckNum, sourceHandle, sourceHandleLength);

         safeSend(dstSckNum, message, messageLen);
      }
   }
}

void relayMessagePacket(Handle *head, unsigned short packetLength,
 uint8_t destinationHandleLength, char *destinationHandle,
 uint8_t sourceHandleLength, char *sourceHandle, char *message) {
   normalHeader hdr;
   int dstSckNum;
   int messageLen;

   hdr.pktLen = packetLength;
   hdr.pktLen = htons(hdr.pktLen);
   hdr.flg = MSG_FLG;

   messageLen = packetLength - sizeof(normalHeader)
    - sizeof(destinationHandleLength) - destinationHandleLength
    - sizeof(sourceHandleLength)- sourceHandleLength;

   /* If the dest handle doesn't exist, send error packet to source */
   if ((dstSckNum = findSocketNumber(head, destinationHandle)) < 0) {
      hdr.flg = BAD_HDL_FLG;
      dstSckNum = findSocketNumber(head, sourceHandle);
   }

   safeSend(dstSckNum, (char *)&hdr, sizeof(hdr));

   safeSend(dstSckNum, (char *)&destinationHandleLength, sizeof(char));

   safeSend(dstSckNum, destinationHandle, destinationHandleLength);

   if (hdr.flg == MSG_FLG) {
      safeSend(dstSckNum, (char *)&sourceHandleLength, sizeof(char));

      safeSend(dstSckNum, sourceHandle, sourceHandleLength);

      safeSend(dstSckNum, message, messageLen);
   }
}

void processExitRequest(int clientSocket) {
   normalHeader hdr;

   hdr.pktLen = sizeof(normalHeader);
   hdr.pktLen = htons(hdr.pktLen);
   hdr.flg = EXIT_ACK_FLG;

   safeSend(clientSocket, (char *)&hdr, sizeof(normalHeader));
}

/*  returns -1 (RECV_FAILED) on flag-processing error, 0 on success,
 * -2 (BAD_HANDLE) if the FD of the newest client needs to be
 *  removed from clientSockets array (because of dup handle),
 *  and -3 (DROPPED_CLIENT) if the FD of current client needs
 *  to be removed because the client exited (unexpectedly). */
int receiveFromClient(int clientSocket) {
   int rtn = 0;
   normalHeader justReceived;
   static Handle *head = NULL;
   uint8_t dstHdlLen, srcHdlLen;
   char *dstHandle, *srcHandle;
   char *message;
   int messageLen = 0;

   //Normal Header stuff
   if (-1 == safeRecv(clientSocket, (char *)&justReceived,
    sizeof(normalHeader))) {
      head = removeHandle(head, clientSocket);
      rtn = DROPPED_CLIENT;
   }
   else {
      justReceived.pktLen = ntohs(justReceived.pktLen);
      /* 1: Initial Packet */
      if (justReceived.flg == INIT_PKT_FLG) {
         //Source Handle Length
         safeRecv(clientSocket, (char *)&srcHdlLen, 1);

         //Source Handle
         srcHandle = malloc(srcHdlLen + 1);
         safeRecv(clientSocket, srcHandle, srcHdlLen);
         srcHandle[srcHdlLen] = '\0';

         if (isHandleTaken(head, srcHandle)) {
            badReply(clientSocket);
            rtn = DUP_HANDLE;
         }
         else {
            head = addHandle(head, srcHandle, clientSocket);
            goodReply(clientSocket);
         }
         free(srcHandle);
      }

      /* 4: Broadcast Packet */
      else if (justReceived.flg == BRDCST_FLG) {
         
         safeRecv(clientSocket, (char *)&srcHdlLen, 1);

         srcHandle = malloc(srcHdlLen + 1);
         safeRecv(clientSocket, srcHandle, srcHdlLen);
         srcHandle[srcHdlLen] = '\0';

         message = malloc(MAX_MSG_SIZE);
         safeRecv(clientSocket, message,
          justReceived.pktLen - sizeof(normalHeader)
          - sizeof(srcHdlLen) - srcHdlLen);
         
         message[justReceived.pktLen - sizeof(normalHeader)
          - sizeof(srcHdlLen) - srcHdlLen] = '\0';

         broadcastToClients(head, justReceived.pktLen, srcHdlLen, srcHandle,
          message);

         memset(message, 0, MAX_MSG_SIZE);

         free(srcHandle);
         free(message);     
      }

      /* 5: Message Packet */
      else if (justReceived.flg == MSG_FLG) {
         /* Destination Handle Length */
         safeRecv(clientSocket, (char *)&dstHdlLen, 1);

         /* Destination Handle */
         dstHandle = malloc(dstHdlLen + 1);
         safeRecv(clientSocket, dstHandle, dstHdlLen);
         dstHandle[dstHdlLen] = '\0';

         /* Source Handle Length */
         safeRecv(clientSocket, (char *)&srcHdlLen, 1);

         /* Source Handle */
         srcHandle = malloc(srcHdlLen + 1);
         safeRecv(clientSocket, srcHandle, srcHdlLen);
         srcHandle[srcHdlLen] = '\0';

         /* Message stuff */
         message = malloc(MAX_MSG_SIZE);
         messageLen = justReceived.pktLen - sizeof(normalHeader)
          - sizeof(dstHdlLen) - dstHdlLen - sizeof(srcHdlLen) - srcHdlLen;

         safeRecv(clientSocket, message, messageLen);
         
         if (messageLen > 1)
            message[messageLen] = '\0';

         relayMessagePacket(head, justReceived.pktLen, dstHdlLen,
          dstHandle, srcHdlLen, srcHandle, message);

         free(dstHandle);
         free(srcHandle);
         free(message);
      }
      /* 8: Exit Request */
      else if (justReceived.flg == EXIT_REQ_FLG)
         processExitRequest(clientSocket);
      /* 10: Requesting list of handles */
      else if (justReceived.flg == LIST_REQ_FLG) {
         sendNumberOfHandles(clientSocket, head);
         sendAllHandles(clientSocket, head);
      }
      else
         rtn = RECV_FAILED;
   }
   return rtn;
}

void selectLoop(int serverSocket) {
   int *clientSockets;
   int currMaxNumClients = STARTING_MAX_NUM_CLIENTS;
   int numClients = 0, clientIndex = 0, removalIndex = 0;
   int receiveResult;
   int maxFD;
   fd_set readFDs;
      
   clientSockets = malloc(sizeof(int) * currMaxNumClients);

   clientSockets[numClients] = TcpListen(serverSocket, 5);
   FD_SET(clientSockets[numClients], &readFDs);
   ++numClients;

   while (1) {
      if (numClients == currMaxNumClients) {
         clientSockets = realloc(clientSockets, currMaxNumClients * 2);
         currMaxNumClients *= 2;
      }

      maxFD = serverSocket;
      FD_ZERO(&readFDs);
      FD_SET(serverSocket, &readFDs); /* so we know when to call accept */
      for (clientIndex = 0; clientIndex < numClients; clientIndex++) {
         if (clientSockets[clientIndex] > maxFD)
            maxFD = clientSockets[clientIndex];
         if (clientSockets[clientIndex] > 0)
            FD_SET(clientSockets[clientIndex], &readFDs);
      }
      
      select(maxFD + 1, &readFDs, NULL, NULL, NULL);
      
      if (FD_ISSET(serverSocket, &readFDs)) {
         if ((clientSockets[numClients] = accept(serverSocket,
          (struct sockaddr*)0, (socklen_t *)0)) < 0) {
            perror("accept call error");
            exit(-1);
         }
         else {
            ++numClients;
         }   
      }

      for (clientIndex = 0; clientIndex < numClients; clientIndex++) {
         if (FD_ISSET(clientSockets[clientIndex], &readFDs)) {
            receiveResult = receiveFromClient(clientSockets[clientIndex]);
            if (receiveResult == RECV_FAILED) {
               fprintf(stderr,
                "Error receiving packet from client with index %u.\n",
                clientIndex);
               exit(EXIT_FAILURE);
            }
            else if (receiveResult == DUP_HANDLE)
               --numClients;
            else if (receiveResult == DROPPED_CLIENT) {
               close(clientSockets[clientIndex]);
               for (removalIndex = clientIndex; removalIndex < numClients;
                removalIndex++) {
                  clientSockets[removalIndex] = clientSockets[removalIndex + 1];
               }
               --numClients;
            }
         }
      }
   }
}

int main(int argc, char **argv) {
   int serverSocket = 0;
   int optionalPortNumber = 0;

   if (argc == 2)
      optionalPortNumber = strtol(argv[1], NULL, 10);
   
   /* create the server socket */
   serverSocket = TcpRecvSetup(optionalPortNumber);

   /* begin the "wait for activity" loop */
   selectLoop(serverSocket);

   return 0;
}
