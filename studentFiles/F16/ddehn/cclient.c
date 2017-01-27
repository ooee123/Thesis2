/******************************************************************************
* tcp_client.c
*
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
#include "testing.h"

#define MAXDATA 1000
#define MAXBUF 1515
#define MAXINBUF 65535


int socketNum = 0;         //socket descriptor
char *sendBuffer = NULL;    
char *inputBuffer = NULL; 
char *origInputBuffer = NULL;
struct chatHdr myHdr; 
char *myHandle; 
uint8_t myHandleLength; 
int numClients = 0; 

void sendInitPacket() {
   int sent = 0; 
   myHdr.flag = 1; 
   myHdr.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint8_t) + 
    myHandleLength);

   //chatHeader
   memcpy(sendBuffer, &myHdr, sizeof(struct chatHdr));
   //src handle Length
   memcpy(sendBuffer + sizeof(struct chatHdr), &myHandleLength, sizeof(uint8_t));
   //src handle
   memcpy(sendBuffer + sizeof(struct chatHdr) + sizeof(uint8_t), 
    myHandle, myHandleLength);

   sent = send(socketNum, sendBuffer, ntohs(myHdr.packetLength), 0);
   if (sent < 0) {
     perror("send call");
     exit(-1); 
   }   
}

void initClientConnection (int argc, char *argv[]) {
   //create packet buffer
   if ((sendBuffer = (char *) malloc(MAXBUF)) == NULL){
      perror("malloc() error");
      exit(-1);
   }
   if ((inputBuffer = (char *) malloc(MAXINBUF)) == NULL){
      perror("malloc() error");
      exit(-1);
   }
   origInputBuffer = inputBuffer;
         
   /* check command line arguments  */
   if (argc != 4){
      printf("usage: %s host-name port-number \n", argv[0]);
      exit(-1);
   }

   myHandle = argv[1];
   myHandleLength = strlen(argv[1]); 

   /* set up the TCP Client socket  */
   socketNum = tcpClientSetup(argv[2], argv[3]);
   sendInitPacket(); 
}

void initSet(fd_set *mySet) {
   FD_ZERO(mySet); 
   FD_SET(STDIN_FILENO, mySet);
   FD_SET(socketNum, mySet); 
}

int iterSet(fd_set *mySet){
   if (FD_ISSET(STDIN_FILENO, mySet)) {
      return STDIN_FILENO;
   }else if (FD_ISSET(socketNum, mySet)) {
      return socketNum;
   }else {
      perror("iter set failed");
      exit(-1);   
   }
}

void makeMessage(uint8_t destHandleLength, char *client, int messageLength) {
   int sent = 0; 
   
   myHdr.packetLength = htons(sizeof(struct chatHdr) + 2*sizeof(uint8_t) + 
    destHandleLength + myHandleLength + messageLength); 

   //chatHeader
   memcpy(sendBuffer, &myHdr, sizeof(struct chatHdr));
   //dest Handle Length
   memcpy(sendBuffer + sizeof(struct chatHdr), &destHandleLength, sizeof(uint8_t)); 
   //dest Handle
   memcpy(sendBuffer + sizeof(struct chatHdr)+ sizeof(uint8_t), client, 
    destHandleLength);
   //src handle Length
   memcpy(sendBuffer + sizeof(struct chatHdr) + sizeof(uint8_t) + destHandleLength, 
    &myHandleLength, sizeof(uint8_t));
   //src handle
   memcpy(sendBuffer + sizeof(struct chatHdr) + 2*sizeof(uint8_t) + destHandleLength, 
    myHandle, myHandleLength);
   //text message
   memcpy(sendBuffer + sizeof(struct chatHdr) + 2*sizeof(uint8_t) +
    destHandleLength + myHandleLength, inputBuffer, messageLength); 

   sent = mySend(socketNum, sendBuffer, ntohs(myHdr.packetLength), 0);
   if (sent < 0) {
      perror("send call");
      exit(-1); 
   }

}


void makeBroadcast(char *sendBuffer, char *message, int messageLength) {
   int sent = 0; 
   
   //chatHeader
   memcpy(sendBuffer, &myHdr, sizeof(struct chatHdr));
   //src handle Length
   memcpy(sendBuffer + sizeof(struct chatHdr), &myHandleLength, sizeof(uint8_t));
   //src handle
   memcpy(sendBuffer + sizeof(struct chatHdr) + sizeof(uint8_t), 
    myHandle, myHandleLength);
   //text message
   memcpy(sendBuffer + sizeof(struct chatHdr) + sizeof(uint8_t) + 
    myHandleLength, message, messageLength);

   sent = mySend(socketNum, sendBuffer, ntohs(myHdr.packetLength), 0);
   if (sent < 0) {
      perror("send call");
      exit(-1); 
   }   
}

void createMessagePacket(int length) {
   char *temp; 
   uint8_t handleLength = 0; 
   inputBuffer += 2; 
   myHdr.flag = 5;  
   char *client = (char *) malloc(255); 
   int totalMessageLength = 0;
   int leftLength = 0;  

   //need a copy because string delimiter modifies orig inputBuffer
   char *message = (char *)malloc(strlen(inputBuffer)); 
   memcpy(message, inputBuffer, strlen(inputBuffer));

   //grabbing the client handle
   temp = strtok(message, " ");
   handleLength = strlen(temp);

   //copying client handle into malloc space
   memcpy(client, temp, handleLength); 
   
   //ptr to data payload and calculating length of it
   inputBuffer += handleLength + 1;
   totalMessageLength = length - handleLength - 4;

   if (totalMessageLength > MAXDATA) {
      leftLength = totalMessageLength - MAXDATA;
      totalMessageLength = MAXDATA;
      makeMessage(handleLength, client, totalMessageLength);
      inputBuffer += MAXDATA;
      makeMessage(handleLength, client, leftLength); 
   } else {
      makeMessage(handleLength, client, totalMessageLength); 
   }
      
   free(client);
}

void createBroadcastPacket() {
   inputBuffer += 2; 
   myHdr.flag = 4;  
   int totalMessageLength = strlen(inputBuffer);

   if (totalMessageLength > MAXDATA) {
      myHdr.packetLength = htons(MAXDATA);
      makeBroadcast(sendBuffer, inputBuffer, MAXDATA);
      inputBuffer += MAXDATA; 
      myHdr.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint8_t) + 
       myHandleLength + strlen(inputBuffer)); 
      makeBroadcast(sendBuffer, inputBuffer, totalMessageLength - MAXDATA);   
   } else {
      myHdr.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint8_t) + 
       myHandleLength + totalMessageLength);
      makeBroadcast(sendBuffer, inputBuffer, totalMessageLength); 
   }  
}

void killClient(){
   close(socketNum);
   free(origInputBuffer);
   free(sendBuffer);
   exit(1); 
}

void iterClients(int grabbed, int grabLength) {
   int count = 0; 
   int secondPass = 0;
   int messageLength = grabLength;
   uint8_t handleLength;
   char *handle = (char *)malloc(255);
   struct chatHdr myHdr;
 
   if (!grabbed) {
      if ((messageLength = recv(socketNum, inputBuffer, MAXBUF, 0)) < 0) {
         perror("recv call");
         exit(-1);
      }
   }

   while (count != numClients){
      if (secondPass) { 
         if ((messageLength = recv(socketNum, inputBuffer, MAXBUF, 0)) < 0) {
            perror("recv call");
            exit(-1);
         }
      }

      memcpy(&myHdr, inputBuffer, sizeof(struct chatHdr));

      while (messageLength >=  ntohs(myHdr.packetLength)) {
         memcpy(&handleLength, inputBuffer + sizeof(struct chatHdr), sizeof(uint8_t));
         memcpy(handle, inputBuffer + sizeof(struct chatHdr) + sizeof(uint8_t), 
          handleLength);
         handle[handleLength] = '\0';         

         printf("  %s\n", handle);
         count++;

         messageLength -= ntohs(myHdr.packetLength);
         inputBuffer += ntohs(myHdr.packetLength);
         if (messageLength >= 3) { //remaining message has at least a chat Header
            memcpy(&myHdr, inputBuffer, sizeof(struct chatHdr));
         }
      }
      secondPass = 1;
      if (messageLength >= 0) {
         memcpy(origInputBuffer, inputBuffer, messageLength);
         inputBuffer = origInputBuffer;
         inputBuffer += messageLength;
      }
   }

   free(handle);

}

void parseServerInput() {
   struct chatHdr serverHdr;
   uint8_t handleLength;
   char *handle = (char *)malloc(255); 
   char *message = (char *)malloc(MAXDATA); 
   int messageLength, netNumClients;

   if ((messageLength = recv(socketNum, inputBuffer, MAXBUF, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }

   memcpy(&serverHdr, inputBuffer, sizeof(struct chatHdr));
   
   switch (serverHdr.flag) {
      case 0: //receiving message -- broadcast or personal message
         memcpy(&handleLength, inputBuffer + sizeof(struct chatHdr), sizeof(uint8_t));
         memcpy(handle, inputBuffer + sizeof(struct chatHdr) + sizeof(uint8_t), 
          handleLength);
         handle[handleLength] = '\0';
         messageLength = ntohs(serverHdr.packetLength) - sizeof(struct chatHdr) - 
          sizeof(uint8_t) - handleLength; 
         
         memcpy(message, inputBuffer + sizeof(struct chatHdr) + sizeof(uint8_t)
          + handleLength, messageLength);
         message[messageLength] = '\0'; 
         printf("\n%s: %s\n", handle, message);
      case 2: //positive acknowledgement of handle 
         break;
      case 3: //error handle
         printf("\nHandle already in use: %s\n", myHandle);
         exit(-1);         
         break;
      case 7: //client you asked for doesn't exist
         memcpy(&handleLength, inputBuffer + sizeof(struct chatHdr), sizeof(uint8_t));
         memcpy(handle, inputBuffer + sizeof(struct chatHdr) + sizeof(uint8_t), 
          handleLength);
         handle[handleLength] = '\0';
         printf("\nClient with handle %s does not exist\n", handle); 
         break;
      case 9:
         killClient();
         break;
      case 11:
         memcpy(&netNumClients, inputBuffer + sizeof(struct chatHdr), sizeof(uint32_t));
         numClients = ntohl(netNumClients);
         printf("Number of clients: %d\n", numClients);
         if (messageLength > ntohs(serverHdr.packetLength)){
            inputBuffer += ntohs(serverHdr.packetLength);
            iterClients(1, messageLength - ntohs(myHdr.packetLength));
         } else {
            iterClients(0, messageLength); 
         }
         break;
      default: 
         break;
   }

}

void makeListPacket() {
   inputBuffer += 2; 
   int sent = 0;
   myHdr.flag = 10;  

   myHdr.packetLength = htons(sizeof(struct chatHdr)); 
   memcpy(sendBuffer, &myHdr, sizeof(struct chatHdr)); 
   
   //send flag 10
   sent = mySend(socketNum, sendBuffer, ntohs(myHdr.packetLength), 0);
   if (sent < 0) {
      perror("send call");
      exit(-1); 
   }
}

void createEndPacket() {
   inputBuffer += 2; 
   int sent = 0;
   myHdr.flag = 8;  

   myHdr.packetLength = htons(sizeof(struct chatHdr)); 
   memcpy(sendBuffer, &myHdr, sizeof(struct chatHdr)); 

   sent = mySend(socketNum, sendBuffer, ntohs(myHdr.packetLength), 0);
   if (sent < 0) {
      perror("send call");
      exit(-1); 
   }
   
   parseServerInput(); 
}

void parseInput(){
   int length = 0;
   
   while ((inputBuffer[length] = getchar()) != '\n'){
      length++;
   }

   inputBuffer[length] = '\0';

   if (*inputBuffer != '%') {
      printf("Command with incorrect format\n");
      return; 
   } else {
      switch (*(++inputBuffer)) {
         case 'M':
         case 'm':
            createMessagePacket(length); 
            break;
         case 'B':
         case 'b': 
            createBroadcastPacket();
            break; 
         case 'L':
         case 'l':
            makeListPacket();
            break;
         case 'E':
         case 'e': 
            createEndPacket();
            break;
         default: 
            break;
      }
   }
   inputBuffer = origInputBuffer;
}

int main(int argc, char * argv[])
{
   fd_set mySet;
   int activeSocket = -1;   

   initClientConnection(argc, argv); 
   initSet(&mySet); 
   
   while (mySelect(socketNum+1, &mySet, NULL, NULL, NULL)) {
      activeSocket = iterSet(&mySet);
      if (activeSocket == STDIN_FILENO) {
         parseInput();
      } else {
         parseServerInput(); 
      }
      initSet(&mySet);
      printf("$: ");
      fflush(stdout);   
   }

   return 0;
}


