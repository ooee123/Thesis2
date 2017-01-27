/******************************************************************************
 * tcp_client.c
 * Alexander DeMello (ademello)
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

#include "testing.h"
#include "networks.h"

int handleLen;
char * handle;
int socket_num = 0;
char* ackBuf;
char* inBuf;
char* send_buf;
char* recv_buf;

void sendPacket(char *toSend, int sendSize) {
   if(send(socket_num, toSend, sendSize, 0) < 0) {
      perror("sending packet");
      exit(-1);
   }
}

void printMessage() {
   char * srcHandle;
   uint8_t destHandleLen = recv_buf[3];
   uint8_t srcHandleLen = recv_buf[3 + destHandleLen + 1];
   
   srcHandle = calloc(1, srcHandleLen + 1);
   memcpy(srcHandle, recv_buf + 4 + destHandleLen + 1, srcHandleLen);
  
   srcHandle[srcHandleLen] = '\0';
   printf("\n%s: %s\n", srcHandle, 
         recv_buf + destHandleLen + 1 + srcHandleLen + 1 + 3);
   printf("$: ");
   fflush(stdout);
}

void printBroadcast() {
   char * srcHandle;
   uint8_t srcHandleLen = recv_buf[3];

   srcHandle = calloc(1, srcHandleLen + 1);
   memcpy(srcHandle, recv_buf + 4 , srcHandleLen);

   srcHandle[srcHandleLen] = '\0';
   printf("\n%s: %s\n", srcHandle,
         recv_buf + srcHandleLen + 1 + 3);
   printf("$: ");
   fflush(stdout);
}

void recievePacket( ) {
   int message_len = 0;
   int flag;
   uint16_t buffLen;

   if((message_len = recv(socket_num, &buffLen, 2, 0)) < 1) {
      perror("recv header");
      exit(-1);
   }
   buffLen = ntohs(buffLen);

   if((message_len = recv(socket_num, recv_buf + 2, buffLen, 0))  < 1) {
       perror("recv from server");
       exit(-1);
   }
   flag = recv_buf[2];

   switch(flag) {
      
      case FLAG_BROADCAST:
         printBroadcast();
         break;
      
      case FLAG_MSG:
         printMessage();
         break;

      case FLAG_MSG_ERROR:
         printf("Got Msg error packet\n");
         break;

      case FLAG_CLIENT_CLOSE:
         printf("got client close packet\n");
         break;

      case FLAG_CLIENT_COUNT:
         printf("got client count packet\n");
         break;
   }

}

void messageBuilder(char* header) {
   int i, j;
   uint16_t netPackLen = 0, packLen = 4;
   char* temp = calloc(1, 256);

   //move the pointer to the destHandle
   inBuf += 3;
   memcpy(send_buf, header, 3);
   
   for(i =0; (inBuf[i] != ' ') && (inBuf[i] != '\0'); i++) {
      temp[i] = inBuf[i];
   }
   //adds the length of dest Handle and src Handle
   packLen += i + handleLen + 1;
   memset(send_buf + 3, i, 1); //put lenth of dest Handle into packet
   memcpy(send_buf + 4, temp, i); // put dest handle in packet
   memset(send_buf + 4 + i, handleLen, 1); //src Handle length
   memcpy(send_buf + 4 + i + 1, handle, handleLen); //the src handle
   inBuf += (i + 1);
   
   for(j = 0; inBuf[j] != '\0'; j++) {
      send_buf[packLen + j] = inBuf[j];  
   }
   send_buf[packLen +j +1] = '\0';
   packLen += j + 1;
   
   netPackLen = htons(packLen);
   memcpy(send_buf, &netPackLen, 2);
  
   sendPacket(send_buf, packLen);
   printf("$: ");
   fflush(stdout);
}

void broadcastBuilder(char * header) {
   int j;
   uint16_t netPackLen = 0, packLen = 4;
   
   //move the pointer to the destHandle
   
   memcpy(send_buf, header, 3);
   memset(send_buf + 3, handleLen, 1); //src Handle length
   memcpy(send_buf + 4, handle, handleLen); //the src handle
   inBuf += 4 + handleLen;
   
   for(j = 0; inBuf[j] != '\0'; j++) {
      send_buf[packLen + j] = inBuf[j];
   }
   send_buf[packLen +j +1] = '\0';
   packLen += j + 1;
   
   netPackLen = htons(packLen);
   memcpy(send_buf, &netPackLen, 2);
   
   sendPacket(send_buf, packLen);
   printf("$: ");
   fflush(stdout);
}

void packetBuilder() {
   int send_len = 0;
   char* header = calloc(1, 3);

   while ((inBuf[send_len] = getchar()) != '\n')
      send_len++;
   inBuf[send_len] = '\0';

   if(inBuf[0] != '%') {
      printf("\nInvalid command\n");
   }

   if(inBuf[1] == 'M' || inBuf[1] == 'm') {
      memset(header + 2, FLAG_MSG, 1);
      messageBuilder(header);
   }

   if(inBuf[1] == 'B' || inBuf[1] == 'b') {
      memset(header + 2, FLAG_BROADCAST, 1);
      broadcastBuilder(header);
   }

   if(inBuf[1] == 'L' || inBuf[1] == 'l') {
      memset(header, htons(3), 2);
      memset(header + 2, FLAG_REQ_CLIENT_LIST, 1);
      sendPacket(header, 3);
   }

   if(inBuf[1] == 'E' || inBuf[1] == 'e') {
      memset(header, htons(3), 2);
      memset(header + 2, FLAG_SERVER_EXIT, 1);
      sendPacket(header, 3);
   } 
}

void clientMainLoop() {
   fd_set socketList, tempList;
 
   FD_ZERO(&socketList);
   //stdin is first, socket is second
   FD_SET(2, &socketList);
   FD_SET(socket_num, &socketList);
   
   while(1) {
      tempList = socketList;
      if(select(socket_num +1, &tempList, NULL, NULL, NULL) < 0) {
         perror("select call");
         exit(-1);
      }

      if (FD_ISSET(2, &tempList)) {
         packetBuilder(); 
      }

      if (FD_ISSET(socket_num, &tempList)) {
         recievePacket();
      }
   }
}

void initialPacket() {
   uint16_t packetSize = 3 + 1 + strlen(handle);
   uint16_t net_pckSize = htons(packetSize);
   char *packet = calloc(1, packetSize);

   memcpy(packet, &net_pckSize, 2);
   memset(packet + 2, FLAG_INITIAL, 1);
   memset(packet + 3, strlen(handle), 1);
   memcpy(packet + 4, handle, strlen(handle));
   
   sendPacket(packet, packetSize);

      recv(socket_num, ackBuf, 3, 0);
      if(ackBuf[2] == 2);
      else if (ackBuf[2] == 3){
         perror("initial packet ack");
         exit(-1);
      }
}


int main(int argc, char * argv[]) {
   ackBuf = (char *) malloc(32);
   recv_buf = (char *) malloc(1100);
   send_buf = (char *) malloc(1100);
   inBuf = (char *) malloc(1100);

   if(argc != 4) {
      printf("incorrect parameters\n");
   }
   handle = argv[1];
   if(strlen(handle) > 255) {
      printf("Handle too long\n");
      exit(-1);
   }
   handleLen = strlen(handle);

   /* set up the socket for TCP transmission  */
   socket_num= tcp_send_setup(argv[2], argv[3]);
   printf("$: ");
   fflush(stdout);
   initialPacket(socket_num);
   clientMainLoop(socket_num);
   
   close(socket_num);
   return 0;   
}



int tcp_send_setup(char *host_name, char *port)
{
    int socket_num;
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host

    // create the socket
    if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    perror("socket call");
	    exit(-1);
	}
    

    // designate the addressing family
    remote.sin_family= AF_INET;

    // get the address of the remote host and store
    if ((hp = gethostbyname(host_name)) == NULL)
	{
	  printf("Error getting hostname: %s\n", host_name);
	  exit(-1);
	}
    
    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

    // get the port used on the remote side and store
    remote.sin_port= htons(atoi(port));

    if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
    {
	perror("connect call");
	exit(-1);
    }

    return socket_num;
}

