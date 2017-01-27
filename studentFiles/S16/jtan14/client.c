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

#include "testing.h"
#include "packet.h"
#include "networks.h"

int main(int argc, char** argv) {

   int socketNum;

   chkCmdLine(argc, argv);

   setupSocket(&socketNum, argv[2], argv[3]);
   checkHandle(argv[1], socketNum);

   runClient(socketNum, argv[1]);

   return 0;
}

void checkHandle(char *handleName, int sockNum) {

   char sendBuff[BUFFER_SIZE];
   char rcvBuff[BUFFER_SIZE];
   struct packetHeader temp;

   int sendLength = 0;
   u_char handleLength = strlen(handleName);
   sendLength += sizeof(struct packetHeader) + sizeof(u_char) 
                 + handleLength;

   //Copies info into packet temp
   temp.flag = 1;
   temp.packLength = htons(sendLength);
   
   memcpy(sendBuff, &temp, sizeof(struct packetHeader));
   memcpy(sendBuff + sizeof(struct packetHeader), 
          &handleLength, sizeof(u_char));
   memcpy(sendBuff + sizeof(struct packetHeader) + sizeof(u_char),
          handleName, handleLength);

   

   if (send(sockNum, sendBuff, sendLength, 0) < 0) {
      perror("Send Call Error");
      exit(1);
   }

   if (recv(sockNum, rcvBuff, BUFFER_SIZE, 0) < 0) {
      perror("Receive Call Error");
      exit(1);
   }

   memcpy(&temp, rcvBuff, sizeof(struct packetHeader));
   if (temp.flag == 3) {
      printf("Handle already in use: %s\n", handleName);
      close(sockNum);
      exit(1);
   }
}

void chkCmdLine(int argc, char** argv) {

   /*Checks the command line Args*/
   if (argc != 4) {
      printf("Usage: %s handle server-name server-port\n", argv[0]);
      exit(1);
   }
}

void setupSocket(int *sockNum, char *host, char *port) {

   int num;
   struct sockaddr_in remote;
   struct hostent *hp;

   if ((num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket call");
      exit(1);
   }

   //Designates Addr Family
   remote.sin_family = AF_INET;

   //Gets address of remote host and store
   if ((hp = gethostbyname(host)) == NULL) {
      printf("Could not find hostname: %s\n", host);
      exit(1);
   }

   memcpy((char*) &remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   //get the port used on the remote side and store
   remote.sin_port = htons(atoi(port));

   if (connect(num, (struct sockaddr*)&remote, 
       sizeof(struct sockaddr_in)) < 0) {
      perror("Problem Connect Call");
      exit(1);
   }

   *sockNum = num;
}

void sendMsg(char *handle, int socketNum, char *locHandle) {

   int handleLen = strlen(handle);
   int locHandLen = strlen(locHandle);
   int length = 0;
   uint16_t size = 0;
   char *sendBuff = calloc(BUFFER_SIZE, 1);
   char a;

   struct packetHeader *header = (struct packetHeader *) sendBuff;
   header->flag = 5;

   sendBuff[3] = handleLen;
   memcpy(sendBuff + 4, handle, handleLen);
   sendBuff[sizeof(struct packetHeader) + 1 + handleLen] = locHandLen;
   memcpy(sendBuff + 4 + handleLen + 1, locHandle, locHandLen);

   while ((a = getchar()) != '\n') {
      sendBuff[5 + handleLen + locHandLen + length] = a;
      length++;

      if ((length + sizeof(struct packetHeader) + handleLen 
          + locHandLen + 2) == BUFFER_SIZE) {
         header->packLength = length + handleLen + locHandLen + 2 
                              + sizeof(struct packetHeader);
         printf("Message is %d bytes, this is too long. ", length);
         printf("Message truncated to 1000 bytes.\n");
         memcpy(sendBuff, header, sizeof(struct packetHeader));
         if (send(socketNum, sendBuff, BUFFER_SIZE, 0) < 0) {
            perror("Send Call");
            exit(1);
         }
         length = 0;
      }
   }
   size += length + sizeof(struct packetHeader) + 2 + handleLen + locHandLen;
   header->packLength = htons(size);
   memcpy(sendBuff, header, sizeof(struct packetHeader));

   if (send(socketNum, sendBuff, 5 + handleLen + 
            locHandLen + length, 0) < 0) {
      perror("Send Call");
      exit(1);
   }
   printf("$: "); fflush(stdout);
}

void runClient(int socketNum, char* handleName) {

   int stopSig = 1, numClients;
   fd_set curFds, fds;

   FD_ZERO(&fds);
   FD_SET(socketNum, &fds);
   FD_SET(STDIN_FILENO, &fds);
   printf("$: "); fflush(stdout);

   while (stopSig) {
      curFds = fds;
      if (select(FD_SETSIZE, &curFds, NULL, NULL, NULL) < 0) {
         perror("Select Error");
         exit(1);
      }
      if (FD_ISSET(socketNum, &curFds)) {
         getMsg(socketNum, &numClients);
      }
      if (FD_ISSET(0, &curFds)) {
         readInput(&stopSig, socketNum, handleName);
      }
   }
}

void getMsg(int socketNum, int *numClients) {
   int msgLen;
   char rcvBuff[BUFFER_SIZE];
   struct packetHeader* temp;

   if ((msgLen = recv(socketNum, rcvBuff, BUFFER_SIZE, 0)) < 0) {
      perror("Receive Error");
      exit(1);
   }
   if (msgLen == 0) {
      printf("Server Terminated\n");
      exit(1);
   }
   temp = (struct packetHeader*) rcvBuff;
   switch (temp->flag) {
      case 4:
         processBroadCast(rcvBuff);
         printf("$: "); fflush(stdout);
         break;

      case 5:
         processMsg(rcvBuff);
         printf("$: "); fflush(stdout);
         break;

      case 7:
         nonExist(rcvBuff);
         break;

      case 9:
         exit(0);
         break;

      case 11:
         processLength(rcvBuff, numClients);
         break;

      case 12:
         processHandleInfo(socketNum, rcvBuff, numClients);
         printf("$: "); fflush(stdout);

         break;

      default:
         break;
   }
}

void processHandleInfo(int socketNum, char *rcvBuff, int* numClients) {
   
   int size = *numClients;
   char curName[50];
   int curSpot; 
   int curSize;
   curSpot = sizeof(struct packetHeader);

   while (size) {
      curSize = rcvBuff[curSpot];
      if (curSize == 0) {
         if (recv(socketNum, rcvBuff, BUFFER_SIZE, 0) < 0) {
            perror("Recv Call");
            exit(1);
         }
         curSpot = sizeof(struct packetHeader);
         curSize = rcvBuff[curSpot];
      }

      memcpy(curName, rcvBuff + curSpot + 1, curSize);
      curSpot += 1 + curSize;
      curName[curSize] = '\0';
      printf("  %s\n", curName);
      size--;
   }
}

void processLength(char *rcvBuff, int* numClients) {
   printf("\n");

   int numC = rcvBuff[sizeof(struct packetHeader)];
   printf("Number of clients: %d\n", numC);

   *numClients = numC;
}

void processBroadCast(char *rcvBuff) {
   printf("\n");
   uint16_t size;
   memcpy(&size, rcvBuff, sizeof(uint16_t));
   size = ntohs(size);

   int srcLen = *(rcvBuff + sizeof(struct packetHeader));
   int msgLen = sizeof(struct packetHeader) + 1 + srcLen;
   char msg[1001];
   char srcName[51];

   memcpy(srcName, rcvBuff + sizeof(struct packetHeader) + 1, srcLen);
   srcName[srcLen] = '\0';
   memcpy(msg, rcvBuff + msgLen, size - msgLen);
   msg[size-msgLen] = '\0';
   printf("%s:%s\n", srcName, msg);

}

void nonExist(char *rcvBuff) {

   u_char dstLen = rcvBuff[sizeof(struct packetHeader)];
   char handleName[51];
   memcpy(handleName, rcvBuff + sizeof(struct packetHeader) + 1, dstLen);
   handleName[dstLen] = '\0';
   printf("Client with handle %s does not exist.\n", handleName);

   printf("$: "); fflush(stdout);
}

void processMsg(char *rcvBuff) {
   printf("\n");
   uint16_t size;
   memcpy(&size, rcvBuff, sizeof(uint16_t));
   size = ntohs(size);
   int dstLen = *(rcvBuff + sizeof(struct packetHeader));
   int srcLen = *(rcvBuff + sizeof(struct packetHeader) + 1 + dstLen);
   char* srcHandle = rcvBuff + sizeof(struct packetHeader) + 1 + dstLen + 1;
   int msgLen = sizeof(struct packetHeader) + 2 + dstLen + srcLen;

   char srcName[51];
   char msg[1001];

   memcpy(srcName, srcHandle, srcLen);
   srcName[srcLen] = '\0';

   memcpy(msg, rcvBuff + msgLen, size - msgLen);
   msg[size - msgLen] = '\0';

   printf("%s:%s\n", srcName, msg);

}

void broadCastMsg(int sockNum, char *handleName) {

   char *sendBuff = calloc(BUFFER_SIZE, 1);
   int handleLen = strlen(handleName);
   int length = 0;
   uint16_t size = 0;

   struct packetHeader *header = (struct packetHeader*) sendBuff;
   header->flag = 4;

   char a;
   sendBuff[sizeof(struct packetHeader)] = handleLen;
   memcpy(sendBuff + sizeof(struct packetHeader) + 1, handleName, handleLen);

   while ((a = getchar()) != '\n') {
      sendBuff[sizeof(struct packetHeader) + 1 + handleLen + length] = a;
      length++;

      if ((1 + length + handleLen + sizeof(struct packetHeader)) 
          == BUFFER_SIZE) {
         header->packLength = 1 + length + 
                              handleLen + sizeof(struct packetHeader);
         header->packLength = htons(header->packLength);
         printf("Message is %d bytes, this is too long.", size);
         printf("Message truncated to 1000 bytes.");
         memcpy(sendBuff, header, sizeof(struct packetHeader));
         sendBuff[BUFFER_SIZE] = '\0';
         if (send(sockNum, sendBuff, BUFFER_SIZE, 0) < 0) {
            perror("Send Call");
            exit(1);
         }
         length = 0;
      }
   }

   size = length + sizeof(struct packetHeader) + 1 + handleLen;
   //puts to network order.
   header->packLength = htons(size);
   memcpy(sendBuff, header, sizeof(struct packetHeader));
   if (send(sockNum, sendBuff, size, 0) < 0) {
      perror("Send Call");
      exit(1);
   }
   printf("$: "); fflush(stdout);
}

void printHandles(int sockNum, char* handleName) {
   struct packetHeader temp;
   temp.flag = 10;
   temp.packLength = sizeof(struct packetHeader);
   char sendBuff[1000];
   memcpy(sendBuff, &temp, sizeof(struct packetHeader));

   if (send(sockNum, sendBuff, sizeof(struct packetHeader), 0) < 0) {
      perror("Send Error");
      exit(1);
   }
}

void execExitCmd(int socketNum, char* handleName) {

   char sendBuff[1000];

   struct packetHeader temp;
   temp.flag = 8;
   temp.packLength = sizeof(struct packetHeader);
   memcpy(sendBuff, &temp, sizeof(struct packetHeader));

   if (send(socketNum, sendBuff, sizeof(struct packetHeader), 0) < 0) {
      perror("Send Call");
      exit(1);
   }
   printf("$: "); fflush(stdout);

}

void readInput(int *stopSig, int socketNum, char* handleName) {

   char* option = malloc(2);
   char* handle = malloc(50);

      scanf("%s", option);

      /*Options processing for msg*/
      if (!strcmp(option, "%m") || !strcmp(option, "%M")) {
         scanf("%s", handle);
         sendMsg(handle, socketNum, handleName);
      }
      else if (!strcmp(option, "%b") || !strcmp(option, "%B")) {
         broadCastMsg(socketNum, handleName);
      }
      else if (!strcmp(option, "%l") || !strcmp(option, "%L")) {
         printHandles(socketNum, handleName);
      }
      else if (!strcmp(option, "%e") || !strcmp(option, "%E")) {
         execExitCmd(socketNum, handleName);
      }
      else {
         printf("Sorry invalid option\n");
         printf("$: "); fflush(stdout);
      }
}




