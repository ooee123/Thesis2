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
#include <sys/select.h>
#include "testing.h"


#include "networks.h"

#define MAXBUF 1024

void parseMessage(char* buf, int client_socket, char*** handles, int* count,
                  uint16_t packetLen){
   int i;
   
   uint8_t destlen;
   memcpy(&destlen, buf + 3, 1);
   
   char* desthandle = malloc(destlen);
   memcpy(desthandle, buf + 4, destlen);
   
   uint8_t sendlen;
   memcpy(&sendlen, buf+4 + destlen, 1);
   
   char* sendhandle = malloc(sendlen);
   memcpy(sendhandle, buf + 5 + destlen, sendlen);

   char *text = malloc(1000);
   strcpy(text, buf + 5 + destlen + sendlen);
   
   int valid = 0;
   int socket_num;
   //check is desthandle is valid
   for(i = 0; i < *count; i++){
      if((*handles)[i] != NULL && (strcmp((*handles)[i], desthandle) == 0)) {
         valid = 1;
         socket_num = i;
      }
   }

   //destination handle exists
   if(valid){
      int dest_socket;
      for (i = 0; i < *count; i++) {
         if((*handles)[i] != NULL && strcmp((*handles)[i], desthandle) == 0)
            dest_socket = i;
      }
      send(dest_socket, buf, packetLen, 0);
   }
   
   else { // dest handle doesnt exist, send back error packet
      char* invalid = malloc(4 + destlen);
      char* mover = invalid;
      uint16_t invalidRes = 4 + destlen;
      
      uint16_t ninvalidRes = htons(invalidRes);
      memcpy(mover, &ninvalidRes, 2);
      mover+=2;
      
      uint8_t flag = 7;
      memcpy(mover, &flag, 1);
      mover +=1;

      memcpy(mover, &destlen, 1);
      mover +=1;
      
      memcpy(mover, desthandle, destlen);
      send(client_socket, invalid, invalidRes,0);
   }
      
}

void parseBroadcast(char* buf, int client_socket, char*** handles, int* count,
                    uint16_t packetLen) {
   
   int i;
   int sent;
   for (i = 0; i < *count; i++) {
      if((*handles)[i] != NULL && i != client_socket) {
         sent = send(i, buf, packetLen, 0);
      }
   }
   
}

void parseExit(char* buf, int client_socket, char*** handles, int* count,
               uint16_t packetLen) {
   //send back chat header with flag 9
   char* res = malloc(3);
   
   uint16_t len = htons(packetLen); //host to network
   memcpy(res, &len, 2);
   uint8_t flag = 9;
   memcpy(res+2, &flag, 1);
   
   send(client_socket, res, packetLen, 0);
   //remove handle from handle list at client_socket index
   (*handles)[client_socket] = NULL;
   close(client_socket); //??
}

void parseList(int client_socket, char*** handles, int *count) {
   char* buf = malloc(7);
   int i;
   int sent;
   
   uint16_t len = htons(7);
   memcpy(buf, &len, 2);
   
   uint8_t flag = 11;
   memcpy(buf+2, &flag, 1);
   
   uint32_t num = 0;
   
   for(i = 0; i < *count; i++) {
      if((*handles)[i] != NULL)
         num++;
   }
   uint32_t numHandles = htonl(num);
   memcpy(buf+3, &numHandles, 4);
   
   sent = send(client_socket, buf, 7, 0); // send first packet
   
   
   //send data stream of packets now
   char* list, *mover;
   len = htons(0);
   flag = 12;
   uint8_t handlelen;
   int total = 0;
   
   for (i = 0; i < *count; i++) {
      if((*handles)[i] != NULL) {
         total += strlen((*handles)[i]);
      }
   }
   
   list = malloc(3+ num+ total);
   mover = list;
   memcpy(mover, &len, 2);
   mover+= 2;
   memcpy(mover, &flag, 1);
   mover+=1;
   
   for(i = 0; i < *count; i++) {
      if((*handles)[i] != NULL) {
         handlelen = strlen((*handles)[i]);
         memcpy(mover, &handlelen, 1);
         mover +=1;
         memcpy(mover, (*handles)[i], handlelen);
         mover+= handlelen;
      }
   }
   
   sent = send(client_socket, list, 3 + num + total, 0);
}

void parsePacket(char* buf, int client_socket, char*** handles, int *count) {
   int i;
   // need to update count every time I remalloc for handles
   char* stoc = malloc(3);
   char* mover = stoc;
   
   uint16_t len;
   uint8_t flag;
   memcpy(&len, buf, 2); //this is in network order
   uint16_t packetLen = ntohs(len);
   memcpy(&flag, buf + 2, 1);

   if (flag == (uint8_t)1) {
      //client initial packet to the server
      uint8_t handleLen;
      memcpy(&handleLen, buf+3, 1);
      char* handle = malloc(handleLen);
      memcpy(handle, buf + 4, handleLen);
      
      //check if valid handle
      int check = 0;
      for (i = 0; i < *count; i++) {
         if((*handles)[i] != NULL && strcmp((*handles)[i], handle) == 0){
            check = 1;
         }
      }
      uint16_t pLen = 3;//host to network
      uint16_t npLen = htons(pLen);
      memcpy(mover, &npLen, 2);
      mover+=2;

      if(check) {
         //handle already exists
         uint8_t pFlag = 3;
         memcpy(mover, &pFlag, 1);
         
      }
      else {
         //valid handle
         uint8_t pFlag = 2;
         memcpy(mover, &pFlag, 1);
         
         //add to array of handles
         uint8_t handleLen;
         memcpy(&handleLen, buf+3, 1);
         
         (*handles)[client_socket] = malloc(handleLen);
         memcpy((*handles)[client_socket], buf+4, handleLen);
      }
      
      int sent;
      sent =  send(client_socket, stoc, pLen, 0);
      if (check)
         close(client_socket);
      if(sent < 0)
      {
         perror("send call");
         exit(-1);
      }
   }
   
   else if (flag == (uint8_t)5) {
      parseMessage(buf, client_socket, handles, count, packetLen);
   }
   else if (flag == 4) {
      parseBroadcast(buf, client_socket, handles, count, packetLen);
   }
   else if (flag == 8) {
      parseExit(buf, client_socket, handles, count, packetLen);
      
   }
   else if (flag == 10) {
      parseList(client_socket, handles, count);
   }
   
}

int findLargest(fd_set* fdvar, int count, char*** handles){
   int i;
   int maxfd = 3;
   for(i = 3; i < count; i++) {
      if ((*handles)[i] != NULL && i > maxfd){
         maxfd = i;
      }
      
   }
   return maxfd;
}
void testClear(char *** handles, int count, fd_set* fdvar){
   int i;
   FD_ZERO(fdvar);
   FD_SET(3, fdvar);
   
   for(i = 0; i < count; i++) {
      if((*handles)[i] != NULL) {
         FD_SET(i, fdvar);
      }
   }
}
void initHandles(char*** handles, int count) {
   int i;
   *handles = malloc(sizeof(char*) * count);
   for (i = 0; i < count; i++)
      (*handles)[i] = NULL;
}

void init(char* argv) {
   int server_socket, client_socket, i, message_len = 0;   //socket descriptor for the server socket
   //socket descriptor for the client socket
   char *buf = NULL;        //buffer for receiving from client
   
   fd_set fdvar;
   
   
   int count = 5;
   char **handles;
   initHandles(&handles, count);
   
   
   //create packet buffer
   buf = (char *) malloc(MAXBUF);
   
   
   //create the server socket
   server_socket = tcpServerSetup(argv);
   FD_ZERO(&fdvar);
   FD_SET(server_socket, &fdvar);
   
   while (1) {
      //find largest fd
      int largest = findLargest(&fdvar, count, &handles);
      
      if(largest== count-1){
         handles = (char**)realloc(handles, sizeof(char* ) * count *2);
         count = count *2;
      }
      
      select(largest + 1, &fdvar, NULL, NULL, NULL);
      if(FD_ISSET(server_socket, &fdvar)) {
         client_socket = tcpAccept(server_socket);
         FD_SET(client_socket, &fdvar);
      }
      
      
      for(i = 4; i <= largest + 1; i++) {
         if(FD_ISSET(i, &fdvar)) {
            if ((message_len = recv(i, buf, MAXBUF, 0)) < 0)
            {
               perror("recv call");
               exit(-1);
            }
            parsePacket(buf, i, &handles, &count);
         }
      }
      
      
      testClear(&handles, count, &fdvar);
   }

}

int main(int argc, char *argv[])
{
   init(argv[1]);
   return 0;
}
