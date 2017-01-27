/******************************************************************************
* formerly tcp_server.c
* now much better
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "testing.h"
#include "networks.h"

fd_set set;
fd_set staticSet;
char **handles;
char *handleBuf;
int maxHandles;
int curHandles;
int bodySize;
int server_socket; //socket descriptor for the server socket
uint8_t headBuf[3];
uint8_t *bodyBuf;
uint8_t *sendBuf;

int handle1(int fd) {
   int itr;

   handleBuf = (char *)calloc(bodyBuf[0] + 1, sizeof(uint8_t));
   memcpy(handleBuf, bodyBuf+1, bodyBuf[0]);

   int found = 0;
   for(itr = 4; itr < maxHandles; itr++) {
      if (handles[itr] && !strcmp(handles[itr], handleBuf)) {
         found = 1;
         printf("%s : %s\n", handles[itr], handleBuf);
      }
   }
   *((uint16_t *)headBuf) = htons(3);
   if (found) {
      headBuf[2] = 3;
      send(fd, headBuf, 3, 0);
      FD_CLR(fd, &staticSet);
      free(handleBuf);
      close(fd);
   } else {
      headBuf[2] = 2;
      handles[fd] = handleBuf;
      curHandles++;
      send(fd, headBuf, 3, 0);
   }

   return 0;
}

int handle4(int fd) {
   int itr;

   sendBuf = (uint8_t *)calloc(bodySize + 3, sizeof(uint8_t));
   memcpy(sendBuf, headBuf, 3);
   memcpy(sendBuf + 3, bodyBuf, bodySize);

   for(itr = 4; itr < maxHandles; itr++) {
      if (handles[itr] && itr != fd) {
         send(itr, sendBuf, bodySize + 3, 0);
      }
   }

   free(sendBuf);

   return 0;
}

int handle5(int fd) {
   int itr;
   handleBuf = (char *)calloc(bodyBuf[0] + 1, sizeof(uint8_t));
   memcpy(handleBuf, bodyBuf+1, bodyBuf[0]);

   for(itr = 4; itr < maxHandles; itr++) {
      if (handles[itr] && !strcmp(handles[itr], handleBuf)) {
         sendBuf = (uint8_t *)calloc(bodySize + 3, sizeof(uint8_t));
         memcpy(sendBuf, headBuf, 3);
         memcpy(sendBuf + 3, bodyBuf, bodySize);
         send(itr, sendBuf, bodySize + 3, 0);
         free(sendBuf);
         return 0;
      }
   }

   sendBuf = (uint8_t *)calloc(4+bodyBuf[0], sizeof(uint8_t));
   *((uint16_t *)sendBuf) = htons(4+bodyBuf[0]);
   sendBuf[2] = 7;
   sendBuf[3] = bodyBuf[0];
   memcpy(sendBuf+4, handleBuf, bodyBuf[0]);
   send(fd, sendBuf, 4+bodyBuf[0], 0);
   free(sendBuf);

   return 0;
}

int handle8(int fd) {
   *((uint16_t *)headBuf) = htons(3);
   headBuf[2] = 9;
   send(fd, headBuf, 3, 0);
   free(handles[fd]);
   curHandles--;
   handles[fd] = 0;
   FD_CLR(fd, &staticSet);
   close(fd);

   return 0;
}

int handle10(int fd) {
   int itr;

   sendBuf = (uint8_t *)calloc(7, sizeof(uint8_t));
   *((uint16_t *)sendBuf) = htons(7);
   sendBuf[2] = 11;
   *((uint32_t *)(sendBuf + 3)) = htonl(curHandles);
   send(fd, sendBuf, 7, 0);
   free(sendBuf);

   for (itr = 4; itr < maxHandles; itr++) {
      if (handles[itr]) {
         int sendLen = 4+strlen(handles[itr]);
         sendBuf = (uint8_t *)calloc(sendLen, sizeof(char));
         *((uint16_t *)sendBuf) = htons(sendLen);
         sendBuf[2] = 12;
         sendBuf[3] = strlen(handles[itr]);
         memcpy(sendBuf + 4, handles[itr], strlen(handles[itr]));
         send(fd, sendBuf, sendLen, 0);
         free(sendBuf);
      }
   }

   return 0;
}

int processSocket(int fd) {
   int exitCheck = recv(fd, headBuf, 3, MSG_DONTWAIT | MSG_PEEK);
   
   if (exitCheck < 0) {
      perror("recv exit check");
      exit(-1);
   } else if (!exitCheck) {
      free(handles[fd]);
      curHandles--;
      handles[fd] = 0;
      FD_CLR(fd, &staticSet);
      close(fd);
   } else {
      if (recv(fd, headBuf, 3, MSG_WAITALL) < 3) {
         perror("recv header");
         exit(-1);
      }

      bodySize = ntohs(*((uint16_t *)headBuf)) - 3;
               
      if (bodySize > 0) {
         bodyBuf = (uint8_t *)calloc(bodySize, sizeof(uint8_t));

         if (recv(fd, bodyBuf, bodySize, MSG_WAITALL) < 0) {
            perror("recv body");
            exit(-1);
         }
      }

      switch (headBuf[2]) {
         case 1:
            handle1(fd);
            break;

         case 4:
            handle4(fd);
            break;

         case 5:
            handle5(fd);
            break;

         case 8:
            handle8(fd);
            break;

         case 10:
            handle10(fd);
            break;
         
         default:
            break;
      }

      if (bodySize > 0) {
         free(bodyBuf);
      }
   }

   return 0;
}

int runServer() {
   int itr;
   int temp;

   FD_ZERO(&staticSet);
   FD_SET(server_socket, &staticSet);

   while(1) {

      memcpy(&set, &staticSet, sizeof(staticSet));   
      select(maxHandles+1, &set, NULL, NULL, NULL);

      if (FD_ISSET(server_socket, &set)) {
         temp = tcpAccept(server_socket);
         FD_SET(temp, &staticSet);
         if (temp == maxHandles) {
            maxHandles *= 2;
            handles = (char **)realloc(handles, maxHandles*sizeof(char *));
         }
      }
   
      for(itr = 4; itr < maxHandles; itr++) {
         if (FD_ISSET(itr, &set)) {
            processSocket(itr);
         }
      }
   }

   return 0;
}

int main(int argc, char *argv[]) {
   server_socket = 0;
	
   //create the server socket
   server_socket = tcpServerSetup();

   handles = (char**)calloc(10, sizeof(char*));
   maxHandles = 10;
   curHandles = 0;

   runServer();

   return 0;
}
