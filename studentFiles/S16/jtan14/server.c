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

#define BUFFER_SIZE 1024
#define TRUE 1
#define FALSE 0

int main(int argc, char** argv) {

   int servSock = 0;

   //Create Server socket
   servSock = rcvSetup(argc, argv);

   //Listens for a client
   sockListen(servSock, 5);

   loopClients(servSock);

   return 0;
}

void loopClients(int servSock) {
   fd_set fds, curFds;
   int curClient;
   char rcvBuff[BUFFER_SIZE];
   int msgLen = 0;

   //initializes the list values.
   struct handleList list;
   list.curSize = 0;
   list.first = calloc(1, sizeof(struct handle));

   if (!list.first) {
      perror("Calloc Call Err");
      exit(1);
   }

   list.maxSize = 10;

   FD_ZERO(&fds);
   FD_SET(servSock, &fds);

   while (TRUE) {
      curFds = fds;

      if (select(FD_SETSIZE, &curFds, NULL, NULL, NULL) < 0) {
         perror("Select Call Err");
         exit(1);
      }

      if (FD_ISSET(servSock, &curFds)) {
         curClient = acceptSock(servSock);
      }
      else {
         curClient = nextSock(servSock, &curFds, &fds);
      }

      if ((msgLen = recv(curClient, rcvBuff, BUFFER_SIZE, 0)) < 0) {
         perror("Rcv Call Err");
         exit(1);
      }

      sendResponse(servSock, rcvBuff, curClient, &list, msgLen, &fds);
   }
}

void sendResponse(int servSock, char* rcvBuff, int curClient,
                  struct handleList* list, int msgLen, fd_set* fds) {

   struct packetHeader* temp;   
   temp = (struct packetHeader*) rcvBuff;

   if (msgLen == 0) {
      processExit(list, rcvBuff, curClient, fds);
   }
   else {
      switch(temp->flag) {
         case 1:
            validHandle(list, rcvBuff, fds, curClient);
            break;

         case 4:
            broadCast(list, rcvBuff, curClient);
            break;

         case 5:
            directMsg(list, rcvBuff, curClient);
            break;

         case 8:
            processExit(list, rcvBuff, curClient, fds);
            break;

         case 10:
            reqHandles(list, rcvBuff, curClient);
            break;

         default:
            printf("%s\n", rcvBuff);
            break;

      }
   }
}

void processExit(struct handleList* list, char* rcvBuff, int curClient, 
                 fd_set* fds) {

   char sendBuff[1000];

   struct packetHeader temp;
   temp.flag = 9;
   temp.packLength = sizeof(struct packetHeader);
   temp.packLength = htons(temp.packLength);

   removeFromList(list, curClient, fds);

   memcpy(sendBuff, &temp, sizeof(struct packetHeader));
   if (send(curClient, sendBuff, sizeof(struct packetHeader), 0) < 0) {
      perror("Send Call");
      exit(1);
   }
   FD_CLR(curClient, fds);
   close(curClient);
}

void removeFromList(struct handleList* list, int curClient, fd_set* fds) {

   int i;
   int removed = 1;
   for (i = 0; i < list->maxSize && removed; i++) {
      if (list->first[i].inUse && list->first[i].fd == curClient) {
         list->first[i].inUse = 0;
         removed = 0;

         list->curSize -= 1;
      }
   }
}

void reqHandles(struct handleList* list, char* rcvBuff, int curClient) {

   char sendBuff[1000]; 
   struct packetHeader* sending = (struct packetHeader*)sendBuff;
   uint16_t size = sizeof(struct packetHeader) + 1;
   sending->packLength = htons(size);
   sending->flag = 11;
   sendBuff[sizeof(struct packetHeader)] = list->curSize;

   if (send(curClient, sendBuff, sizeof(struct packetHeader) + 1, 0) < 0) {
      perror("Send Call");
      exit(1);
   }
   int i, count = 0;
   uint16_t curPackSize = 0;
   sending->flag = 12;
   for (i = 0; count < list->curSize; i++) {
      if (list->first[i].inUse) {
         count++;
         if (curPackSize + sizeof(struct packetHeader) + list->first[i].len >=
             BUFFER_SIZE) {
            sending->packLength = htons(curPackSize);

            memcpy(sendBuff, sending, sizeof(struct packetHeader));
            sendBuff[curPackSize] = '\0';
            if (send(curClient, sendBuff, BUFFER_SIZE, 0) < 0) {
               perror("Send Call");
               exit(1);
            }
            curPackSize = 0;
         }
         sendBuff[sizeof(struct packetHeader) + curPackSize] 
                  = list->first[i].len;
         memcpy(sendBuff + sizeof(struct packetHeader) + curPackSize + 1, 
                list->first[i].name, list->first[i].len);
         curPackSize += 1 + list->first[i].len;
      }
   }
   sending->packLength = curPackSize + sizeof(struct packetHeader);
   sending->packLength = htons(sending->packLength);
   memcpy(sendBuff, sending, sizeof(struct packetHeader));

   if(send(curClient, sendBuff, curPackSize + 
      sizeof(struct packetHeader), 0) < 0) {
      perror("Send Call");
      exit(1);
   }
}

void broadCast(struct handleList* list, char* rcvBuff, int curClient) {

   u_char rcvHandleLen = rcvBuff[sizeof(struct packetHeader)];

   uint16_t size;
   memcpy(&size, rcvBuff, sizeof(uint16_t));

   size = ntohs(size);

   int i;
   for (i = 0; i < list->maxSize; i++) {
      if (memcmp(list->first[i].name, rcvBuff + 
                 sizeof(struct packetHeader) + 1, rcvHandleLen) != 0 &&
          list->first[i].inUse) {
         if (send(list->first[i].fd, rcvBuff, size, 0) < 0){
            perror("Send Error");
            exit(1);
         }
      }
   }
}

int getHandle(struct handleList* list, char* rcvBuff) {
   int i;
   u_char length = *(rcvBuff + sizeof(struct packetHeader));
   char *start = rcvBuff + sizeof(struct packetHeader) + 1;

   int count;
   for (i = 0, count = 0; count < list->curSize; i++) {
      if (list->first[i].inUse) {
         if (memcmp(list->first[i].name, start, length) == 0 &&
             list->first[i].inUse) {
            return list->first[i].fd;
         }
         count++;
      }
   }

   return -1;
}

void directMsg(struct handleList* list, char* rcvBuff, int curClient) {
   int fd;
   int rcvHandleLen = rcvBuff[sizeof(struct packetHeader)];
   uint16_t size;
   memcpy(&size, rcvBuff, sizeof(uint16_t));
   size = ntohs(size);
   
   char sendBuff[1000];
   struct packetHeader temp;

   if ((fd = getHandle(list, rcvBuff)) < 0) {

      temp.flag = 7;
      memcpy(sendBuff, &temp, sizeof(struct packetHeader));
      sendBuff[sizeof(struct packetHeader)] = rcvHandleLen;
      memcpy(sendBuff + sizeof(struct packetHeader) + 1, 
             rcvBuff + sizeof(struct packetHeader) + 1, 
             rcvHandleLen);

      if (send(curClient, sendBuff, sizeof(struct packetHeader) + 1 
          + rcvHandleLen, 0) < 0) {
         perror("Send Error");
         exit(1);
      }
   }
   else {
      if (send(fd, rcvBuff, size, 0) < 0 ) {
         perror("Send Error");
         exit(1);
      }
   }
}

int handleExists(struct handleList* list, char* rcvBuff) {
   int i;
   int length = *(rcvBuff + sizeof(struct packetHeader));
   char *start = rcvBuff + sizeof(struct packetHeader) + 1;

   for (i = 0; i < list->curSize; i++) {
      if (memcmp(list->first[i].name, start, length) == 0 && 
          list->first[i].inUse) {
         return TRUE;
      }
   }
   return FALSE;
}

void addToList(struct handleList* list, char* rcvBuff, int curClient, 
               int handleLen) {

   int i = 0;
   int isFound = 1;

   if (list->curSize == list->maxSize) {
      list->first = realloc(list->first, 
                           sizeof(struct handle) * list->maxSize * 2);
      list->maxSize *= 2;
   }

   while (i <= list->curSize && isFound) {
      //Checks if there's a gap and fills in the gap otherwise
      //loops through till the end to fill in.
      if (list->first[i].inUse == 0) {
         list->first[i].inUse = 1;
         list->first[i].fd = curClient;
         list->first[i].len = handleLen;
         memcpy(list->first[i].name, rcvBuff + 
                sizeof(struct packetHeader) + 1, handleLen);
         isFound = 0;
         list->curSize += 1;
      }
      i++;
   }   
}

void validHandle(struct handleList* list, char* rcvBuff, fd_set* fds, 
                 int curClient) {

   struct packetHeader temp;
   char sendBuff[BUFFER_SIZE];

   int handleLen = *(rcvBuff + sizeof(struct packetHeader));
   temp.packLength = handleLen + sizeof(struct packetHeader) + 1;
   temp.packLength = htons(temp.packLength);

   if (handleExists(list, rcvBuff)) {
      temp.flag = 3;
   }
   else {
      temp.flag = 2;

      //Creates the new Handle and adds to list.
      addToList(list, rcvBuff, curClient, handleLen);
      FD_SET(curClient, fds);
   }

   memcpy(sendBuff, &temp, sizeof(struct packetHeader));

   if (send(curClient, sendBuff, sizeof(struct packetHeader), 0) < 0) {
      perror("Send call Error");
      exit(1);
   }
}

int nextSock (int servSock, fd_set *read, fd_set *fds) {
   int i;
   for (i = 0; i < FD_SETSIZE; i++) {
      if (FD_ISSET(i, read) && i != servSock) {
         return i;
      }
   }
   return 1;
}

int acceptSock(int servSock) {
   int client;

   if ((client = accept(servSock, (struct sockaddr*)0, 
       (socklen_t*)0)) < 0) {
      perror("Accept Call Err");
      exit(1);
   }
   return client;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */
int rcvSetup(int argc, char** argv) {
   int server_socket = 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */

   if (argc > 2) {
      printf("Invalid num of Params\n");
      exit(1);
   }

   /* create the socket  */
   server_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (server_socket < 0)
   {
      perror("socket call");
      exit(1);
   }

   local.sin_family = AF_INET;         //internet family
   local.sin_addr.s_addr = INADDR_ANY; //wild card machine address
   if (argc == 2) {
      local.sin_port = htons((int)argv[1]);
   }
   else {
      local.sin_port = htons(0);                 //let system choose the port
   }
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

   printf("socket has port %d \n", ntohs(local.sin_port));
  
   return server_socket;
}

void sockListen(int servSock, int back) {

   if (listen(servSock, back) < 0) {
      perror("listen call");
      exit(-1);
   }
}





