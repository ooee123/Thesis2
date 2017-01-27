/******************************************************************************
* tcp_client.c
*author: James Green
* large portions of code written by Dr Smith's TA in the 90s
* use at own risk
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
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
#include "client.h"

#define MAXBUF 1024
#define CH_HD_LEN 3

char * gClientHandle;
char * gServerName;
int gSocks[2];
int gSocketNum;
int gLength;
char * gCBuff;

int main(int argc, char * argv[])
{
   gCBuff = (char *) calloc(MAXBUF, sizeof(char));
	if (argc != 4){
		printf("usage: %s [handle] [host] [port] \n", argv[0]);
		exit(1);
	}
   getHandle(argc, argv);
   getServer(argc, argv);
   /* set up the TCP Client socket  */
   gSocketNum = tcpClientSetup(argv[2], argv[3]);
   startupClient();
	return 0;
}

void startupClient()
{
   gSocks[0] = 2; //stdin
   gSocks[1] = gSocketNum; //server socket
   sendInit(); //send intial packet
   printf("$: "); //print prompt (initial packet success)
   fflush(stdout);
   while(1) //loop until ctrl+c or %E
   {
      clientSelect(); //listen
      free(gCBuff); //dont forget to free your vars
      gCBuff = (char*) calloc(MAXBUF, sizeof(char)); //or else you will suffer for three days straight
   }
   close(gSocketNum); //clean up 
}

void sendInit()
{
   char *packet; //holds the packet for sending
   char *ptr;  //ptr to point to packet for traversing
   uint8_t handleLen = strlen(gClientHandle); //length of my handle
   //uint8_t length = htons(gLength); 
   uint16_t packLength = 3 + 1 + handleLen;
   
   packet = malloc(packLength);
   ptr = packet;

   *ptr = htons(packLength);
   ptr += 2;
   memset(ptr, C_INIT, 1);
   ptr += 1;
   memset(ptr, handleLen, 1);
   ptr += 1;
   memcpy(ptr, gClientHandle, handleLen);
   ptr += handleLen;
   packet[1] = htons(packLength) & 0xff; //pack lower eight bits
   packet[0] = htons(packLength) >> 8; //pack upper eight bits
   
   safeSend(packet, packLength); //safely send packet
   initGet(); //check for initial packet
}

void initGet()
{
   int messlen;
   char flag;
   // printf("reading the initial packet\n");
   if ( (messlen = recv(gSocketNum, gCBuff, MAXBUF, 0)) < 0)
   {
      perror("recv");
      exit(-1);
   }
   // printf("handle read.\n");
   flag = gCBuff[2];
   

   if (flag == S_INIT_ERR)
   {
      printf("Handle already in use: %s\n", gClientHandle);
      close(gSocketNum);
      exit(-1);
   }
   else if (flag != S_INIT_GOOD) //debug
   {
      printf("bad flag: %d\n", flag);
      exit(-1);
   }
   
}

//function to safely send on the socket
void safeSend(char *packet, int packetLength)
{
   int ack;
   
   ack = send(gSocketNum, packet, packetLength, 0);
   
   if (ack < 0)
   {
      perror("send");
      exit(-1);   
   }
}

//sunction for listening to server
void clientSelect()
{
   char * inbuff;
   fd_set fdx;
   int in_len;
   
   FD_ZERO(&fdx); //reset
   FD_SET(gSocks[0], &fdx);
   FD_SET(gSocks[1], &fdx);
   
   if ( select(gSocketNum + 1, (fd_set *) &fdx, NULL, NULL, NULL) < 0)
   {
      perror("select");
      exit(-1);
   }

   if (FD_ISSET(gSocks[0], &fdx)) //if we're listening to client input
   {
      inbuff = (char *)malloc(MAXBUF);
      in_len = 0;
   
      while ((inbuff[in_len] = getchar()) != '\n') //user hasn't pressed enter
      {
         in_len++;
      }
      inbuff[in_len] = '\0'; //null term for string functions
      
      if (inbuff[0] != '%')
      {
         printf("Invalid command\n");
         printf("$: ");
         fflush(stdout);
      }
      else if (inbuff[0] == '%' && !isspace(inbuff[1])) 
      {
         if ((inbuff[1] == 'B' || inbuff[1] == 'b') && isspace(inbuff[2]))
         {
            broadcast(inbuff);
            printf("$: ");
            fflush(stdout);
         }
         if ((inbuff[1] == 'E' || inbuff[1] == 'e'))
         {
            c_exit(inbuff);
            printf("$: ");
            fflush(stdout);
         }
         if ((inbuff[1] == 'M' || inbuff[1] == 'm'))
         {
            message(inbuff);
            printf("$: ");
            fflush(stdout);
         }
         if ((inbuff[1] == 'L' || inbuff[1] == 'l'))
         {
            c_list(inbuff);
            printf("$: ");
            fflush(stdout);
         }
         
      }
      memset(&inbuff[0], 0, sizeof(inbuff));
      
   }
   if (FD_ISSET(gSocks[1], &fdx))
   {
      serverListen(); //receive messages from server
   }
}

//function to pull handle from cmdline args
void getHandle(int argc, char * argv[])
{
   gClientHandle = malloc(strlen(argv[1]) + 1);
   strcpy(gClientHandle, argv[1]);
}

//function to get servername from cmdline args
void getServer(int argc, char * argv[])
{
   gServerName = malloc(strlen(argv[2]) + 1);
   strcpy(gServerName, argv[2]);
}

//function to listen to server for packets
void serverListen()
{
   int messlen;
   int flag;
   char * handle;
   //char * tmpBuff = gCBuff;
   int handleLen = 0;
   
   if ((messlen = recv(gSocketNum, gCBuff, MAXBUF, 0)) < 0)
   {
      perror("recv");
      exit(-1);
   }
   
   if (messlen == 0)
   {
      printf("Server terminated\n");
      exit(-1);
   }
   
   flag = gCBuff[2];
   
   if (flag == S_MSG_ERR)
   {
      //printf("errrrrrrr\n");
      handleLen = (int) *(gCBuff + 3);
      handle = malloc(handleLen + 1);
      memcpy(handle, gCBuff + 4, handleLen);
      handle[handleLen] = '\0';
      printf("Client with handle %s does not exist\n", handle);
      printf("$: ");
      fflush(stdout);
   }
   else if (flag == C_BROADCAST || flag == 0) //flag == 0 is bug, but this works
   {
      //printf("YOU GOT MAIL\n");
      getBroadcast(gCBuff, messlen);
      printf("$: ");
      fflush(stdout);
   }
   else if (flag == S_EXIT_ACK)
   {
      //printf("It's okay to die.\n");
      close(gSocketNum);
      exit(1);
   }
   else if (flag == C_MESSAGE)
   {
      //printf("We got a message\n");
      getMessage(gCBuff, messlen);
      printf("$: ");
      fflush(stdout);
      memset(&gCBuff[0], 0, sizeof(gCBuff));
   }
   else if (flag == S_LIST)
   {
      getListCount(gCBuff, messlen);
      //dont need to print prompt until all clients are in
   }
   else if (flag == 12)
   { 
      listHandles();
      printf("$: ");
      fflush(stdout);
   }
   else //debug
   {
      printf("bad flag is %d\n", flag);
   }
   memset(&gCBuff[0], 0, sizeof(gCBuff));
}

//get the broacast packet from server using ptr arithmetic
void getBroadcast(char * buffer, int messlen)
{
   //printf("getting broadcast\n");
   char * handle;
   char * msg;
   //char * temp = buffer;
   int handleLen = (int) *(buffer + 3);
   handle = malloc(handleLen + 1);
   memcpy(handle, buffer + 4, handleLen);
   handle[handleLen] = '\0'; //null terminate for string functions
   buffer += 3 + handleLen;
   msg = malloc(messlen);
   strcpy(msg, buffer);
   printf("\n%s: %s\n", handle, msg + 1);
}

//send the exit packet to server
void c_exit(char * buffer)
{
   //printf("sending exit req...\n");
   char *packet, *ptr;
   //int packetLen = htons(CH_HD_LEN);
   int packetLen = CH_HD_LEN;

   packet = malloc(packetLen);
   ptr = packet;
   
   *ptr = htons(packetLen);
   ptr += 2;
   memset(ptr, C_EXIT, 1);
   packet[1] = htons(packetLen) & 0xff; //pack lower eight bits
   packet[0] = htons(packetLen) >> 8; //pack upper eight bits
   safeSend(packet, packetLen);
}

//send list packet to server
void c_list(char * buffer)
{
   //printf("sending list packet\n");
   char * packet, *ptr;
   int packetLen = CH_HD_LEN;
   
   packet = malloc(packetLen);
   ptr = packet;
   
   *ptr = htons(packetLen);
   ptr += 2;
   memset(ptr, C_LIST, 1);
   packet[1] = htons(packetLen) & 0xff; //pack lower eight bits
   packet[0] = htons(packetLen) >> 8; //pack upper eight bits
   safeSend(packet, packetLen);
}

//send broadcast to server
void broadcast(char* buffer)
{
   char * input;
   char *packet, *ptr;
   int handleLen = 0, textLen = 0, packetLen = 0;
   input = malloc (strlen(buffer) + 1);
   strcpy(input, buffer);
   input += 3; //get rid of "% "
   if (strlen(input) > MAXBUF)
   {
      printf("ERR: Message to long.\n");
   }
   else
   {
      handleLen = strlen(gClientHandle);
      textLen = strlen(input) + 1;
      packetLen = CH_HD_LEN + 1 + handleLen + textLen;
      packet = malloc(packetLen);
      uint16_t packetLen2BYTES = packetLen;
      
      ptr = packet;
      *ptr = htons(packetLen2BYTES);
      ptr += 2;
      memset(ptr, C_BROADCAST, 1);
      ptr++;
      memset(ptr, handleLen, 1);
      ptr++;
      memcpy(ptr, gClientHandle, handleLen);
      ptr += handleLen;
      strcpy(ptr, input);
      packet[1] = htons(packetLen) & 0xff; //pack lower eight bits
      packet[0] = htons(packetLen) >> 8; //pack upper eight bits
      //printf("broadcasted %s\n", input);
      safeSend(packet, packetLen);
   }
}

//send message to server
void message(char * buffer)
{
   char * input, *handle, *msg;
   char *packet, *ptr;
   int myHandleLen = 0, dstHandleLen = 0, textLen = 0, packetLen = 0;
   input = malloc (strlen(buffer) + 1);
   strcpy(input, buffer);


   strtok(input, " "); //get rid of the %M 
   handle = strtok(NULL, " "); //this should be the handle

   if (handle == NULL)
   {
      printf("ERR: Must provide handle\n");
      return;
   }
   else
   {
      msg = buffer;
   }
   msg += 3; //get rid of %M
   if (strlen(msg) > MAXBUF)
   {
      printf("ERR: Message to long.\n");
   }
   else
   {
      myHandleLen = strlen(gClientHandle);
      dstHandleLen = strlen(handle);
      input += dstHandleLen; //get rid of the dst handle
      textLen = strlen(msg) + 1;
      packetLen = CH_HD_LEN + 1 + myHandleLen + textLen + dstHandleLen;
      packet = malloc(packetLen);
      uint16_t packetLen2BYTES = packetLen;
      ptr = packet;
      *ptr = htons(packetLen2BYTES);
      ptr += 2;
      memset(ptr, C_MESSAGE, 1);
      ptr++;
      memset(ptr, dstHandleLen, 1);
      ptr++;
      memcpy(ptr, handle, dstHandleLen);
      ptr += dstHandleLen;
      memset(ptr, myHandleLen, 1);
      ptr++;
      memcpy(ptr, gClientHandle, myHandleLen);
      ptr += myHandleLen;
      strcpy(ptr, msg + myHandleLen);
      packet[1] = htons(packetLen) & 0xff; //pack lower eight bits
      packet[0] = htons(packetLen) >> 8; //pack upper eight bits
      safeSend(packet, packetLen);
      memset(&msg[0], 0, sizeof(msg));

   }
}

//read message from server
void getMessage(char * buffer, int messlen)
{
   char *cliHandle, *dstHandle;
   char * msg = NULL;
   //char * temp = buffer;
   int cliHandleLen;
   int dstHandleLen = (int) *(buffer + 3);
   dstHandle = malloc(dstHandleLen + 1);
   dstHandle[dstHandleLen] = '\0'; //null term for print
   buffer += 4 + dstHandleLen;
   cliHandleLen = (int) *buffer;
   cliHandle = malloc(cliHandleLen + 1);
   memcpy(cliHandle, buffer + 1, cliHandleLen);
   cliHandle[cliHandleLen] = '\0';
   buffer += 1 + cliHandleLen;
   //printf("clihandle  IS %s\n", cliHandle);
   msg = malloc(messlen);
   strcpy(msg, buffer);
   buffer = NULL;
   //strtok(msg, " "); //work around to get rid of tokens left from handle (bug)
   //memcpy(msg, buffer, messlen - 5 - cliHandleLen - dstHandleLen);
   printf("\n%s: %s\n", cliHandle, msg);
   free(msg);
}

//get number of clients on server
void  getListCount(char * buff, int messlen)
{
   int handlecount = 0, ndx;
   buff += 3;
   memcpy(&handlecount, buff, 4);
   printf("There are %d handles\n", handlecount);
   
   for (ndx = 0; ndx < handlecount; ndx++)
   {
     // listHandles();
   }   
}

//list all handles on server
void listHandles()
{
   int handleLen;
   char * handle;
   
   handleLen = *(gCBuff + 3);
   handle = malloc(handleLen + 1);
   memcpy(handle, gCBuff + 4, handleLen);
   //printf("TEST\n");
   printf("   %s\n", handle);
}
