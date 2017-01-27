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

//3 bytes chatHdr, 1 byte dest length,
//255 bytes dest handle, 1 byte src length,
//255 bytes src handle, 1000 bytes message
#define MAXBUF 1515  
#define MAXFILES 65535

int serverSocket; 
char *dataBuffer;
char **clients; 
int numClients; 
int highestSocket; 
fd_set mySet;   

void initServerSetup() {
   //creating packet buffer
   if ((dataBuffer = (char *)malloc(MAXBUF)) == NULL) {
      perror("malloc() error");
      exit(-1);
   }

   if ((clients = (char **)calloc(MAXFILES, sizeof(char *))) == NULL) {
      perror("client calloc() error");
      exit(-1);
   }  

   //creating the server socket 
   serverSocket = tcpServerSetup(); 
   highestSocket = serverSocket; 
}

void initSet(fd_set *mySet) {
   int i; 
   
   FD_ZERO(mySet);
   FD_SET(serverSocket, mySet);
   
   //iterate through my clients and include in set
   for (i=3; i<= highestSocket; i++) {
      if (clients[i] != 0) {
         FD_SET(i, mySet); 
      }
   }
}

int iterSet(fd_set *mySet) {
   int i; 
   
   for (i = 3; i<=highestSocket; i++) {
      if (FD_ISSET(i, mySet)) {
         return i; 
      }
   }
   return -1; 
}

void makeResponsePacket(int clientSocket, uint8_t flag) {
   struct chatHdr myHdr;
   int sent = 0; 

   myHdr.packetLength = htons(sizeof(struct chatHdr));
   myHdr.flag = flag; 
   memcpy(dataBuffer, &myHdr, sizeof(struct chatHdr));

   sent = send(clientSocket, dataBuffer, sizeof(struct chatHdr), 0); 
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }

}

void addClient(int clientSocket, char *handle) {
   int i;

   //see if client name in use
   for (i=3; i<= highestSocket; i++) {
      if (clients[i] != 0) {
         if (strcmp(clients[i], handle) == 0) {
            makeResponsePacket(clientSocket, 3);
            return;
         }   
      }
   }

   makeResponsePacket(clientSocket, 2); 
   clients[clientSocket] = handle; 
   numClients++; 

   if (clientSocket > highestSocket) {
      highestSocket = clientSocket; 
   }   
}

void acceptClient(int activeSocket) {
   int clientSocket = -1; 
   int messageLength = -1;
   char *handle; 
   struct chatHdr myHdr; 
   uint8_t handleLength = 0; 

   //activeSocket should consistently be the server socket
   clientSocket = tcpAccept(activeSocket);
   if ((messageLength = recv(clientSocket, dataBuffer, MAXBUF, 0)) < 0) {
      perror("this one, recv call");
      exit(-1);
   }
   
   memcpy(&myHdr, dataBuffer, sizeof(struct chatHdr)); 
   memcpy(&handleLength, dataBuffer + sizeof(struct chatHdr), sizeof(uint8_t)); 
   handle = (char *)malloc(sizeof(char)*(handleLength + 1)); 
   memcpy(handle, dataBuffer + sizeof(struct chatHdr) + sizeof(uint8_t), sizeof(char) 
    * handleLength); 
   handle[handleLength] = '\0';
   addClient(clientSocket, handle);
}

void makeHandlePacket(int clientSocket, char *handle, int handleLength, int flag) {
   struct chatHdr myHdr;
   int sent = 0;
   char *temp = (char *)malloc(MAXBUF);  

   myHdr.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint8_t) + 
    handleLength);
   myHdr.flag = flag; 
   memcpy(temp, &myHdr, sizeof(struct chatHdr));
   memcpy(temp + sizeof(struct chatHdr), &handleLength, sizeof(uint8_t));
   memcpy(temp + sizeof(struct chatHdr) + sizeof(uint8_t), handle, 
    handleLength);

   sent = mySend(clientSocket, temp, ntohs(myHdr.packetLength), 0); 
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }

   free(temp);
}

void makeClientNumPacket(int clientSocket) {
   struct chatHdr myHdr;
   int i, sent = 0;
   uint32_t copyNumClients = htonl(numClients);  

   myHdr.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint32_t));
   myHdr.flag = 11; 
   memcpy(dataBuffer, &myHdr, sizeof(struct chatHdr));
   memcpy(dataBuffer + sizeof(struct chatHdr), &copyNumClients, sizeof(uint32_t)); 

   sent = send(clientSocket, dataBuffer, ntohs(myHdr.packetLength), 0); 
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
   
   for (i=3; i<= highestSocket; i++){
      if (clients[i] != 0) {
         makeHandlePacket(clientSocket, clients[i], strlen(clients[i]), 12);
      }
   }
}

void removeClient(int clientSocket) {
   clients[clientSocket] = 0; //clear from server's array
   FD_CLR(clientSocket, &mySet); //clear from network set
   numClients--;

   makeResponsePacket(clientSocket, 9);
   close(clientSocket); 
   initSet(&mySet);
   //respond to client
}

void broadcast() {
   int i, sent;
   char *srcHandle;
   uint8_t handleLength; 
   struct chatHdr myHdr;

   memcpy(&myHdr, dataBuffer, sizeof(struct chatHdr)); 
   memcpy(&handleLength, dataBuffer + sizeof(struct chatHdr), sizeof(uint8_t)); 
   srcHandle = (char *)malloc(handleLength); 
   memcpy(srcHandle, dataBuffer + sizeof(struct chatHdr) + sizeof(uint8_t), 
    handleLength);

   for (i=3; i<=highestSocket; i++) {
      if (clients[i] != 0 && (strncmp(srcHandle, clients[i], handleLength) != 0)) {
         //set flag to zero for client side
         memset(dataBuffer + sizeof(uint16_t), 0, sizeof(uint8_t)); 
         sent = send(i, dataBuffer, ntohs(myHdr.packetLength), 0); 
         if (sent < 0) {
            perror("send call");
            exit(-1);
         }
      }
   }
}

void sendMessage(int client, char *srcHandle, int srcHandleLength, char *message, 
      int messageLength) {
  
   char *temp = (char *)malloc(MAXBUF);
   struct chatHdr myHdr;
   int sent = 0; 

   myHdr.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint8_t) + 
    srcHandleLength + messageLength); 
   myHdr.flag = 0;
   memcpy(temp, &myHdr, sizeof(struct chatHdr)); 
   memcpy(temp + sizeof(struct chatHdr), &srcHandleLength, sizeof(uint8_t));
   memcpy(temp + sizeof(struct chatHdr) + sizeof(uint8_t), srcHandle, 
    srcHandleLength);
   memcpy(temp + sizeof(struct chatHdr) + sizeof(uint8_t) + 
    srcHandleLength, message, messageLength);

   sent = send(client, temp, ntohs(myHdr.packetLength), 0); 
   if (sent < 0) {
      perror("send call");
      exit(-1);
   } 
   free(temp);
}

void parsePacket(int client) {
   int messageLength = -1; 
   struct chatHdr myHdr; 
   uint8_t destHandleLength = 0; 
   int found = 0;
   char *destHandle;
   int i, destSocket = 0;
   char *srcHandle;
   uint8_t srcHandleLength;
   char *message;

   if ((messageLength = recv(client, dataBuffer, MAXBUF, 0)) < 0) {
      perror("parsePacket recv call");
      exit(-1);
   }

   memcpy(&myHdr, dataBuffer, sizeof(struct chatHdr)); 
   switch (myHdr.flag) {
      case 4:
         broadcast(); 
         break;
      
      case 5:
         //destination info
         memcpy(&destHandleLength, dataBuffer + sizeof(struct chatHdr), sizeof(uint8_t)); 
         destHandle = (char *)malloc(destHandleLength); 
         memcpy(destHandle, dataBuffer + sizeof(struct chatHdr) + sizeof(uint8_t), 
          destHandleLength);
         
         //source info
         memcpy(&srcHandleLength, dataBuffer + sizeof(struct chatHdr) + sizeof(uint8_t)
          + destHandleLength, sizeof(uint8_t)); 
         srcHandle = (char *)malloc(srcHandleLength); 
         memcpy(srcHandle, dataBuffer + sizeof(struct chatHdr) + 2*sizeof(uint8_t) +
          destHandleLength, srcHandleLength);
         
         messageLength = ntohs(myHdr.packetLength) - 2*sizeof(uint8_t) - srcHandleLength 
          - destHandleLength - sizeof(struct chatHdr);
         message = dataBuffer + 2*sizeof(uint8_t) + sizeof(struct chatHdr) + 
          srcHandleLength + destHandleLength; 

         for (i=3; i<= highestSocket; i++) {
            if (clients[i] != 0) {
               if (strncmp(clients[i], destHandle, destHandleLength) == 0) {
                  found = 1;
                  destSocket = i;  
               }
            }
         }
         if (!found) {
            makeHandlePacket(client, destHandle, destHandleLength, 7);  
         } else {
            sendMessage(destSocket, srcHandle, srcHandleLength, message, messageLength); 
         }   
         break;
     
      case 10:
         makeClientNumPacket(client);
         break;
      
      case 8:
         removeClient(client); 
         break;
      
      default:
         break;
   }

}

int main (int argc, char *argv[]) {
   int activeSocket; 
   numClients = 0; 
   highestSocket = -1;   

   initServerSetup(); 
   initSet(&mySet);
    
   while (mySelect(highestSocket + 1, &mySet, NULL, NULL, NULL)) {
      activeSocket = iterSet(&mySet);
      if (activeSocket < 0) {
         perror("iterSet failed");
      } else if (activeSocket == serverSocket) {
         acceptClient(activeSocket);
      } else {
         parsePacket(activeSocket); 
      }
      initSet(&mySet); 
   }

   return 0; 
}
