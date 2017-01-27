/******************************************************************************
* formerly tcp_client.c
* now much better
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
#include <errno.h>

#include "testing.h"
#include "networks.h"

int sock;
int bodySize;
uint8_t handleLen;
uint8_t *handle;
char *sendBuf;
uint8_t headBuf[3];
uint8_t *bodyBuf;
char *readBuf;
char commandBuf[3];
char *handleBuf;
int hlen;

int sendInit() {
   size_t sendLen = 4 + handleLen;

   sendBuf = (char *)calloc(sendLen, sizeof(char));
   ((uint16_t *)sendBuf)[0] = htons(sendLen);
   sendBuf[2] = 1;
   sendBuf[3] = handleLen;
   memcpy(sendBuf+4, handle, handleLen);
   
   send(sock, sendBuf, sendLen, 0);
   free(sendBuf);
   
   if (recv(sock, headBuf, 3, MSG_WAITALL) < 3) {
      perror("recv call");
      exit(-1);
   }

   if (headBuf[2] == 2) {
      return 1;
   } else if (headBuf[2] == 3) {
      printf("Handle already in use: %s\n", handle);
      return 0;
   } else {
      perror("wrong flag");
      exit(-1);
   }
}

int newCommand() {
   scanf("%s", commandBuf);
   commandBuf[2] = '\0';
   if(commandBuf[0] != '%') {
      printf("Invalid command\n");
      return 0;
   }

   if (commandBuf[1] == 'e' || commandBuf[1] == 'E') {

      sendBuf = (char *)calloc(3, sizeof(char));
      ((uint16_t *)sendBuf)[0] = htons(3);
      sendBuf[2] = 8;
      send(sock, sendBuf, 3, 0);
      free(sendBuf);

   } else if (commandBuf[1] == 'b' || commandBuf[1] == 'B') {

      readBuf = (char *)calloc(1001, sizeof(char));
      fgets(readBuf, 1000, stdin);
      readBuf[strlen(readBuf)-1] = '\0';
      int sendLen = 4 + handleLen + strlen(readBuf);
      sendBuf = (char *)calloc(sendLen, sizeof(char));
      ((uint16_t *)sendBuf)[0] = htons(sendLen);
      sendBuf[2] = 4;
      sendBuf[3] = handleLen;
      memcpy(sendBuf+4, handle, handleLen);
      memcpy(sendBuf+4+handleLen, readBuf, strlen(readBuf));

      send(sock, sendBuf, sendLen, 0);
      free(sendBuf);
      free(readBuf);

   } else if (commandBuf[1] == 'm' || commandBuf[1] == 'M') {

      handleBuf = (char *)calloc(256, sizeof(char));
      scanf("%s", handleBuf);
      hlen = strlen(handleBuf);
      readBuf = (char *)calloc(1001, sizeof(char));
      fgets(readBuf, 1000, stdin);
      readBuf[strlen(readBuf)-1] = '\0';
      int sendLen = 4 + hlen + 1 + handleLen + strlen(readBuf);
      sendBuf = (char *)calloc(sendLen, sizeof(char));
      ((uint16_t *)sendBuf)[0] = htons(sendLen);
      sendBuf[2] = 5;
      sendBuf[3] = hlen;
      memcpy(sendBuf+4, handleBuf, hlen);
      sendBuf[4+hlen] = handleLen;
      memcpy(sendBuf+5+hlen, handle, handleLen);
      memcpy(sendBuf+5+hlen+handleLen, readBuf, strlen(readBuf));

      send(sock, sendBuf, sendLen, 0);
      free(sendBuf);
      free(readBuf);
      
   } else if (commandBuf[1] == 'l' || commandBuf[1] == 'L') {

      sendBuf = (char *)calloc(3, sizeof(char));
      ((uint16_t *)sendBuf)[0] = htons(3);
      sendBuf[2] = 10;
      send(sock, sendBuf, 3, 0);
      free(sendBuf);

   } else {
      printf("Invalid command\n");
   }

   return 0;
}

int readIn() {
   int slen;
   int itr;
   int nHandles;
   int exitCheck = recv(sock, headBuf, 3, MSG_DONTWAIT | MSG_PEEK);
   
   if (exitCheck < 0) {
      perror("recv exit check");
      exit(-1);
   } else if (!exitCheck) {
      printf("Server Terminated\n");
      free(handle);
      close(sock);
      return 0;
   } else {
      if (recv(sock, headBuf, 3, MSG_WAITALL) < 3) {
         perror("recv header");
         exit(-1);
      }

      bodySize = ntohs(*((uint16_t *)headBuf)) - 3;

      if (bodySize > 0) {
         bodyBuf = (uint8_t *)calloc(bodySize, sizeof(uint8_t));

         if (recv(sock, bodyBuf, bodySize, MSG_WAITALL) < bodySize) {
            perror("recv body");
            exit(-1);
         }
      }

      switch (headBuf[2]) {
         case 4:
            hlen = bodyBuf[0];
            handleBuf = (char *)calloc(hlen+1, sizeof(char));
            memcpy(handleBuf, bodyBuf+1, hlen);
            readBuf = (char *)calloc(bodySize-hlen, sizeof(char));
            memcpy(readBuf, bodyBuf+1+hlen, bodySize-hlen-1);
            printf("\n%s:%s\n", handleBuf, readBuf);
            free(handleBuf);
            break;

         case 5:
            slen = bodyBuf[0];
            hlen = bodyBuf[slen+1];
            handleBuf = (char *)calloc(hlen+1, sizeof(char));
            memcpy(handleBuf, bodyBuf+slen+2, hlen);
            readBuf = (char *)calloc(bodySize-hlen-slen-1, sizeof(char));
            memcpy(readBuf, bodyBuf+slen+2+hlen, bodySize-hlen-2-slen);
            printf("\n%s:%s\n", handleBuf, readBuf);
            free(handleBuf);
            break;

         case 7:
            hlen = bodyBuf[0];
            handleBuf = (char *)calloc(hlen+1, sizeof(char));
            memcpy(handleBuf, bodyBuf+1, hlen);
            printf("\nClient with handle %s does not exit.\n", handleBuf);
            break;

         case 9:
            printf("\n");
            return 1;
            break;

         case 11:
            nHandles = ntohl(*((uint32_t *)bodyBuf));
            printf("\nNumber of clients: %d\n", nHandles);
            for(itr = 0; itr < nHandles; itr++) {
               if (recv(sock, headBuf, 3, MSG_WAITALL) < 3) {
                  perror("recv header");
                  exit(-1);
               }
               bodySize = ntohs(*((uint16_t *)headBuf)) - 3;
               bodyBuf = (uint8_t *)calloc(bodySize, sizeof(uint8_t));
               if (recv(sock, bodyBuf, bodySize, MSG_WAITALL) < bodySize) {
                  perror("recv body");
                  exit(-1);
               }
               hlen = bodyBuf[0];
               handleBuf = (char *)calloc(hlen+1, sizeof(char));
               memcpy(handleBuf, bodyBuf+1, hlen);
               printf("  %s\n", handleBuf);
            }
            break;

         default:
            break;
      }
   }

   return 0;
}

int runClient() {
   fd_set set;
   fd_set staticSet;

   FD_ZERO(&staticSet);
   FD_SET(0, &staticSet);
   FD_SET(sock, &staticSet);
   while(1) {
      printf("$: ");
      fflush(stdout);

      memcpy(&set, &staticSet, sizeof(staticSet));
      select(4, &set, NULL, NULL, NULL);
      
      if (FD_ISSET(0, &set)) {
         newCommand();        
      } else {
         if (readIn()) {
            return 0;
         }
      }
   }

   return 0;
}

int main(int argc, char * argv[]) {
   /* check command line arguments  */
   if (argc != 4)
   {
      printf("usage: %s handle server-name server-port \n", argv[0]);
      exit(1);
   }

   handleLen = strlen(argv[1]);
   handle = (uint8_t *)calloc(handleLen + 1, sizeof(char));
   memcpy(handle, argv[1], handleLen); 
   
   /* set up the TCP Client socket  */
   sock = tcpClientSetup(argv[2], argv[3]);

   if (sendInit()) {
      runClient();
   }

   return 0;
}
