/*
* Jeffrey Nunez
* CPE 464, 9am lab section
* 4/25/16
*
* Project 2 - cclient
*
* Builds off of the provided tcp_client.c
*/

#include "networks.h"
#include "testing.h"

#pragma pack(1)

int TcpSendSetup(char *hostName, char *port) {
   int socketNum;
   struct sockaddr_in remote;       /* socket address for remote side */
   struct hostent *hp;              /* address of remote host */

   /* create the socket */
   if ((socketNum = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket call error");
      exit(-1);
	}
    
   /* designate the addressing family */
   remote.sin_family = AF_INET;

   /* get the address of the remote host and store */
   if ((hp = gethostbyname(hostName)) == NULL) {
      printf("Error getting hostname: %s\n", hostName);
      exit(-1);
	}
    
   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   /* get the port used on the remote side and store */
   remote.sin_port = htons(atoi(port));

   if (connect(socketNum, (struct sockaddr*)&remote,
    sizeof(struct sockaddr_in)) < 0) {
      perror("connect call");
      exit(-1);
   }

   return socketNum;
}

/* Function is called when flag 12 has just been identified. */
void receiveAllHandles(int socketNum, int numHandles) {
   uint8_t currHdlLen;
   char thisHandle[UCHAR_MAX + 1];

   while (numHandles > 0) {
      safeRecv(socketNum, (char *)&currHdlLen, 1);

      safeRecv(socketNum, thisHandle, currHdlLen);
      thisHandle[currHdlLen] = '\0';
      printf("%s\n", thisHandle);

      --numHandles;
   }
}

void receiveFromServer(int socketNum) {
   normalHeader justReceived;
   uint8_t dstHdlLen, srcHdlLen;
   char *dstHandle, *srcHandle;
   char *message;
   static int numHandles;
   
   safeRecv(socketNum, (char *)&justReceived, sizeof(normalHeader));
   justReceived.pktLen = ntohs(justReceived.pktLen);

   if (justReceived.flg == LIST_NUM_FLG) {
      safeRecv(socketNum, (char *)&numHandles, justReceived.pktLen
       - sizeof(normalHeader));
      numHandles = ntohl(numHandles);
   }

   else if (justReceived.flg == LIST_FLG) {
      receiveAllHandles(socketNum, numHandles);
   }

   else if (justReceived.flg == EXIT_ACK_FLG) {
      close(socketNum);
      exit(1);
   }

   else if (justReceived.flg == BRDCST_FLG) {
      safeRecv(socketNum, (char *)&srcHdlLen, 1);

      srcHandle = malloc(srcHdlLen + 1);
      safeRecv(socketNum, srcHandle, srcHdlLen);
      srcHandle[srcHdlLen] = '\0';

      message = malloc(MAX_MSG_SIZE);
      safeRecv(socketNum, message, justReceived.pktLen - sizeof(normalHeader)
       - sizeof(srcHdlLen) - srcHdlLen);
      message[justReceived.pktLen - sizeof(normalHeader) - sizeof(srcHdlLen)
       - srcHdlLen] = '\0';

      printf("\n%s: %s\n", srcHandle, message);
      free(srcHandle);
      free(message);
   }

   else {
      if (justReceived.flg == MSG_FLG || justReceived.flg == BAD_HDL_FLG) {
         /* Destination Handle Length */
         safeRecv(socketNum, (char *)&dstHdlLen, 1);

         /* Destination Handle */
         dstHandle = malloc(dstHdlLen + 1);
         safeRecv(socketNum, dstHandle, dstHdlLen);
         dstHandle[dstHdlLen] = '\0';
      }

      if (justReceived.flg == BAD_HDL_FLG)
         printf("Client with handle %s does not exist.\n", dstHandle);
      else if (justReceived.flg == MSG_FLG) {
         /* Source Handle Length */
         safeRecv(socketNum, (char *)&srcHdlLen, 1);

         /* Source Handle */
         srcHandle = malloc(srcHdlLen + 1);
         safeRecv(socketNum, srcHandle, srcHdlLen);
         srcHandle[srcHdlLen] = '\0';

         /* Message stuff */
         if (justReceived.pktLen - dstHdlLen - srcHdlLen - 2 == 0) {
            message = malloc(1);
            *message = '\0';
         }
         else {
            message = malloc(MAX_MSG_SIZE);
            safeRecv(socketNum, message, justReceived.pktLen
             - sizeof(normalHeader) - sizeof(dstHdlLen) - dstHdlLen
             - sizeof(srcHdlLen) - srcHdlLen);
            message[justReceived.pktLen - sizeof(normalHeader)
             - sizeof(dstHdlLen) - dstHdlLen - sizeof(srcHdlLen)
             - srcHdlLen] = '\0';
         }

         printf("\n%s: %s\n", srcHandle, message);
         free(srcHandle);
         free(message);
      }
      free(dstHandle);
   }
}

void requestExit(int socketNum) {
   normalHeader hdr;

   hdr.pktLen = sizeof(normalHeader);
   hdr.pktLen = htons(hdr.pktLen);
   hdr.flg = EXIT_REQ_FLG;

   safeSend(socketNum, (char *)&hdr, sizeof(normalHeader));
}

void sendListRequest(int socketNum) {
   normalHeader hdr;

   hdr.pktLen = sizeof(normalHeader);
   hdr.pktLen = htons(hdr.pktLen);
   hdr.flg = LIST_REQ_FLG;

   safeSend(socketNum, (char *)&hdr, sizeof(hdr));
}

void initialPacket(char *sourceHandle, int socketNum) {
   normalHeader hdr, response;
   uint8_t sourceHandleLength;

   sourceHandleLength = (uint8_t)strlen(sourceHandle);

   hdr.pktLen = sizeof(hdr) + sizeof(sourceHandleLength) +
    sourceHandleLength;
   hdr.pktLen = htons(hdr.pktLen);
   hdr.flg = INIT_PKT_FLG;

   safeSend(socketNum, (char *)&hdr, sizeof(hdr));
   
   safeSend(socketNum, (char *)&sourceHandleLength, sizeof(char));

   safeSend(socketNum, sourceHandle, sourceHandleLength);
   
   safeRecv(socketNum, (char *)&response, sizeof(normalHeader));

   if (response.flg == BAD_REPLY_FLG) {
      printf("Handle already in use: %s\n", sourceHandle);
      exit(-1);
   }
}

void sendRemainingBroadcast(int socketNum, char firstChar,
 uint8_t sourceHandleLength, char *sourceHandle) {
   normalHeader hdr;
   unsigned short packetLength;
   char sendBuf[MAX_MSG_SIZE];
   int sendLen = 0;
   char tmpChar;

   sendBuf[sendLen] = firstChar;
   sendLen++;

   if (EOF == (tmpChar = getchar()))
      sendBuf[sendLen] = '\n';
   else {
      sendBuf[sendLen] = tmpChar;
      sendLen++;

      while ((sendBuf[sendLen] = getchar()) != '\n' &&
       sendLen < MAX_MSG_SIZE - 1)
         sendLen++;
      ungetc(sendBuf[sendLen], stdin);

      sendBuf[sendLen] = '\0';
   }

   packetLength = sizeof(normalHeader) + sizeof(sourceHandleLength)
    + sourceHandleLength + sendLen;

   hdr.pktLen = packetLength;
   hdr.pktLen = htons(hdr.pktLen);
   hdr.flg = BRDCST_FLG;        

   safeSend(socketNum, (char *)&hdr, sizeof(hdr));

   safeSend(socketNum, (char *)&sourceHandleLength, sizeof(char));

   safeSend(socketNum, sourceHandle, sourceHandleLength);

   safeSend(socketNum, sendBuf, sendLen);

   if (sendLen < MAX_MSG_SIZE - 1)
      ungetc('\n', stdin);
}

void sendRemainingMessage(int socketNum, char firstChar,
 uint8_t destinationHandleLength, char *destinationHandle,
 uint8_t sourceHandleLength, char *sourceHandle) {
   normalHeader hdr;
   unsigned short packetLength;
   char sendBuf[MAX_MSG_SIZE];
   int sendLen = 0;
   char tmpChar;

   sendBuf[sendLen] = firstChar;
   sendLen++;

   if (EOF == (tmpChar = getchar()))
      sendBuf[sendLen] = '\n';
   else {
      sendBuf[sendLen] = tmpChar;
      sendLen++;

      while ((sendBuf[sendLen] = getchar()) != '\n'
       && sendLen < MAX_MSG_SIZE - 1)
         sendLen++;
      ungetc(sendBuf[sendLen], stdin);

      sendBuf[sendLen] = '\0';
   }

   packetLength = sizeof(normalHeader) + sizeof(destinationHandleLength)
    + destinationHandleLength + sizeof(sourceHandleLength)
    + sourceHandleLength + sendLen;

   hdr.pktLen = packetLength;
   hdr.pktLen = htons(hdr.pktLen);
   hdr.flg = MSG_FLG;        

   safeSend(socketNum, (char *)&hdr, sizeof(hdr));

   safeSend(socketNum, (char *)&destinationHandleLength, sizeof(char));

   safeSend(socketNum, destinationHandle, destinationHandleLength);

   safeSend(socketNum, (char *)&sourceHandleLength, sizeof(char));

   safeSend(socketNum, sourceHandle, sourceHandleLength);

   safeSend(socketNum, sendBuf, sendLen);

   if (sendLen < MAX_MSG_SIZE - 1)
      ungetc('\n', stdin);
}

void sendPacket(char *sourceHandle, int socketNum) {
   normalHeader hdr;
   unsigned short packetLength;
   uint8_t destinationHandleLength = 0;
   char destinationHandle[UCHAR_MAX + 1];
   uint8_t sourceHandleLength = 0;
   char sendBuf[MAX_MSG_SIZE];
   int sendLen = 0;
   char tmpChar;

   sourceHandleLength = (uint8_t)strlen(sourceHandle);

   if (getchar() == '%') {
      tmpChar = getchar();
     
      if (tmpChar == 'E' || tmpChar == 'e') {
         requestExit(socketNum);
      }

      else if (tmpChar == 'L' || tmpChar == 'l') {
         sendListRequest(socketNum); 
      }

      else if (tmpChar == 'B' || tmpChar == 'b') {
         getchar(); /* process (ignore) the whitespace character following %B */
         if (EOF == (tmpChar = getchar()))
            sendBuf[sendLen] = '\n';
         else {
            sendBuf[sendLen] = tmpChar;
            sendLen++;

            while ((sendBuf[sendLen] = getchar()) != '\n' &&
             sendLen < MAX_MSG_SIZE - 1)
               sendLen++;
            ungetc(sendBuf[sendLen], stdin);

            sendBuf[sendLen] = '\0';
         }
         
         packetLength = sizeof(normalHeader) + sizeof(sourceHandleLength)
          + sourceHandleLength + sendLen;

         hdr.pktLen = packetLength;
         hdr.pktLen = htons(hdr.pktLen);
         hdr.flg = BRDCST_FLG;        

         safeSend(socketNum, (char *)&hdr, sizeof(hdr));

         safeSend(socketNum, (char *)&sourceHandleLength, sizeof(char));

         safeSend(socketNum, sourceHandle, sourceHandleLength);

         safeSend(socketNum, sendBuf, sendLen);

         while ((tmpChar = getchar()) != '\n') {
            sendRemainingBroadcast(socketNum, tmpChar,
             sourceHandleLength, sourceHandle);
         }
      }

      else if (tmpChar == 'M' || tmpChar == 'm') {
      getchar(); /* process (ignore) the whitespace character following %M */
         while (destinationHandleLength < UCHAR_MAX
          && (destinationHandle[destinationHandleLength] = getchar()) != ' '
          && destinationHandle[destinationHandleLength] != '\n')
            destinationHandleLength++;

         destinationHandle[destinationHandleLength] = '\0';

         if (EOF == (tmpChar = getchar()))
            sendBuf[sendLen] = '\n';
         else {
            sendBuf[sendLen] = tmpChar;
            sendLen++;

            while ((sendBuf[sendLen] = getchar()) != '\n' &&
             sendLen < MAX_MSG_SIZE - 1)
               sendLen++;
            ungetc(sendBuf[sendLen], stdin);

            sendBuf[sendLen] = '\0';
         }
         
         packetLength = sizeof(normalHeader) + sizeof(destinationHandleLength)
          + destinationHandleLength + sizeof(sourceHandleLength)
          + sourceHandleLength + sendLen;
         
         hdr.pktLen = packetLength;
         hdr.pktLen = htons(hdr.pktLen);
         hdr.flg = MSG_FLG;        

         safeSend(socketNum, (char *)&hdr, sizeof(hdr));

         safeSend(socketNum, (char *)&destinationHandleLength, sizeof(char));

         safeSend(socketNum, destinationHandle, destinationHandleLength);

         safeSend(socketNum, (char *)&sourceHandleLength, sizeof(char));

         safeSend(socketNum, sourceHandle, sourceHandleLength);

         safeSend(socketNum, sendBuf, sendLen);

         while ((tmpChar = getchar()) != '\n') {
            sendRemainingMessage(socketNum, tmpChar, destinationHandleLength,
             destinationHandle, sourceHandleLength, sourceHandle);
         }
      }
   }
}

void waitForActivity(char *myHandle, int socketNum) {
   fd_set readFDs;

   while (1) {   
      FD_ZERO(&readFDs);
      FD_SET(socketNum, &readFDs);
      FD_SET(0, &readFDs);

      printf("$: ");
      fflush(stdout);
      select(socketNum + 1, &readFDs, NULL, NULL, NULL);

      if (FD_ISSET(0, &readFDs))
         sendPacket(myHandle, socketNum);
      if (FD_ISSET(socketNum, &readFDs))
         receiveFromServer(socketNum);
   }
}

int main(int argc, char **argv) {
   int socketNum;
   char *myHandle;

   /* check command line arguments  */
   if(argc != 4) {
      printf("usage: %s client-handle host-name port-number\n", argv[0]);
   	exit(1);
   }

   /* check to see if user-defined handle is too long (limit 255 char) */
   if (strlen(argv[1]) > UCHAR_MAX)
      exit(1);
   else
      myHandle = argv[1];

   /* set up the socket for TCP transmission  */
   socketNum = TcpSendSetup(argv[2], argv[3]); 

   /* Send the first packet to give client's handle to server. */
   initialPacket(myHandle, socketNum);

   waitForActivity(myHandle, socketNum); 

   close(socketNum);
   return 0;   
}
