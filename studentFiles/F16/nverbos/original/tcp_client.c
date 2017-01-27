//Nick Verbos
//tcp client for chat program

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
#include <ctype.h>
#include <signal.h>

#include "testing.h"
#include "tcp_client.h"
#include "networks.h"


int main(int argc, char *argv[]){
   CClient cc;

   setup(argc, argv, &cc);
   sendInitialPacket(&cc);

   printf("$: ");
   fflush(stdout);
   while(1){
      handleNext(&cc);
   }
	
   close(cc.serverSocket);
	
	return SUCCESS;
}

void setup(int argc, char *argv[], CClient *cc){
   //check command line arguments
	if (argc != 4)
	{
		printf("usage: %s handle host-name port-number \n", argv[0]);
		exit(1);
	}
   
   //Set the handle length and handle
   cc->handleLen = strlen(argv[1]);
   cc->handle = malloc(cc->handleLen);
   memcpy(cc->handle, argv[1], cc->handleLen);

	//set up the TCP Client socket
	cc->serverSocket = tcpClientSetup(argv[2], argv[3]);

	//create packet buffer
	if ((cc->buf = (char *) malloc(MAXBUF)) == NULL)
	{
		perror("malloc() error");
		exit(-1);
	}
   FD_ZERO(&cc->fdSet);
   FD_SET(0, &cc->fdSet);
   FD_SET(cc->serverSocket, &cc->fdSet);
}

void handleNext(CClient *cc){
   FD_SET(cc->serverSocket, &cc->fdSet);
   FD_SET(0, &cc->fdSet);
   if(select(cc->serverSocket + 1, &cc->fdSet, NULL, NULL, NULL) == -1){
      perror("select call");
   }

   if(FD_ISSET(cc->serverSocket, &cc->fdSet)){
      printPacket(cc);
   } else {
      handleNextCommand(cc);
   }

}

void printPacket(CClient *cc){
   ChatHeader *ch;
   waitForResponse(cc);
   u_char *handleLenPtr;
   char *handle;

   ch = (ChatHeader *)cc->buf;
   
   switch (ch->flag) {
      case MESSAGE :
         printIncomingMessage(cc);
         break;

      case MESSAGE_ERROR :
         handleLenPtr = (u_char *)(cc->buf + sizeof(ChatHeader));
         handle = (char *)(handleLenPtr + sizeof(u_char));
         printf("Client with handle %.*s does not exist.\n", *handleLenPtr, handle);
         break;
         
      case BROADCAST :
         printIncomingBroadcast(cc);
         break;

      case NUMBER_HANDLES :
         handleIncomingHandles(cc);
         break;

      case HANDLE :
         printHandle(cc);
         break;

      case ACK :
         exit(0);
         break;

      default :
         puts("Unknown command");
         break;
	}
   if(ch->flag != NUMBER_HANDLES && ch->flag != HANDLE){
      printf("$: ");
      fflush(stdout);
   }
}

void handleIncomingHandles(CClient *cc){
   uint32_t *numHandlesPtr, numHandles;
   
   numHandlesPtr = (uint32_t *)(cc->buf + sizeof(ChatHeader));
   numHandles = ntohl(*numHandlesPtr);

   printf("\nNumber of clients: %zu\n", numHandles);
   cc->handles = numHandles;
}

void printHandle(CClient *cc){
   char *handle;
   u_char *handleLenPtr, handleLen;

   handleLenPtr = (u_char *)(cc->buf + sizeof(ChatHeader));
   handleLen = *handleLenPtr;
   handle = cc->buf + sizeof(ChatHeader) + sizeof(u_char);

   printf("\t%.*s\n", handleLen, handle);
   cc->handles--;

   if(cc->handles == 0){
      printf("$: ");
      fflush(stdout);
   }
}

void printIncomingMessage(CClient *cc){
   u_char *flag, *destHandleLen, *fromHandleLen;
   char *destHandle, *fromHandle, *message;
   u_short *packetLenPtr, packetLen;
   
   packetLenPtr = (u_short *)cc->buf;
   packetLen = *packetLenPtr;
   flag = (u_char *)cc->buf + sizeof(short);
   destHandleLen = flag + 1;
   destHandle = (char *)destHandleLen + 1;
   fromHandleLen = (u_char *)destHandle + *destHandleLen;
   fromHandle = (char *)fromHandleLen + 1;
   message = (char *)fromHandle + *fromHandleLen;
   
   printf("\n%.*s: %s\n", *fromHandleLen, fromHandle, message);
}

void printIncomingBroadcast(CClient *cc){
   u_char *flag, *fromHandleLen;
   char *fromHandle, *message;
   u_short *packetLenPtr;
   
   packetLenPtr = (u_short *)cc->buf;
   flag = (u_char *)cc->buf + sizeof(short);
   fromHandleLen = flag + 1;
   fromHandle = (char *)fromHandleLen + 1;
   message = (char *)fromHandle + *fromHandleLen;
   
   printf("\n%.*s: %s\n", *fromHandleLen, fromHandle, message);
}



void handleNextCommand(CClient *cc){
   char command, handle[256], *message, *input;
   
   input = getUserInput();
   message = malloc(strlen(input));
   sscanf(input, "%%%c", &command);
   
   command = toupper(command);

   switch (command) {
      case 'M' :
         sscanf(input, "%%%c %s %[^\n]", &command, handle, message);
         sendMessagePacket(message, handle, cc);
         break;

      case 'B' :
         sscanf(input, "%%%c %[^\n]",&command, message);
         sendBroadcastPacket(message, cc);
         break;
         
      case 'L' :
         requestHandleList(cc);
         break;
      
      case 'E' :
         exitServer(cc);
         break;

      default :
         puts("Invalid command");
         break;
	}
   if(command != 'E'){
      printf("$: ");
      fflush(stdout);
   }
   free(input);
   free(message);
}

char *getUserInput(){
   char *string;
   int ch;
   size_t len = 0, size = 1000;

   if(feof(stdin)){
      puts("EOF reached even though client did not exit.  Exiting.");
      exit(1);
   }

   string = realloc(NULL, sizeof(char)*size);

   if (!string){
      return string;
   }

   while(EOF!=(ch=fgetc(stdin)) && ch != '\n'){
      string[len++]=ch;
      if(len==size){
         string = realloc(string, sizeof(char)*(size+=16));
         if(!string)return string;
      }
   }
   string[len++]='\0';

   return realloc(string, sizeof(char)*len);
}

void sendInitialPacket(CClient *cc){
   ChatHeader ch, *chPtr;
   
   //Copy appropriate data into buffer
   ch.packetLen = htons(sizeof(ChatHeader) + cc->handleLen + sizeof(u_char));
   ch.flag = INITIAL_PACKET;
   memcpy(cc->buf, &ch, sizeof(ChatHeader));
   memcpy(cc->buf + sizeof(ChatHeader), &cc->handleLen, sizeof(u_char));
   memcpy(cc->buf + sizeof(ChatHeader) + sizeof(u_char), cc->handle, cc->handleLen);
   

   //send the initial packet to the server
   send(cc->serverSocket, cc->buf, ntohs(ch.packetLen), 0);

   waitForResponse(cc);

   chPtr = (ChatHeader *)cc->buf;

   if(chPtr->flag == INITIAL_SUCCESS){
      //successful connection
   } else if(chPtr->flag == INITIAL_ERROR){
      printf("Handle already in use: %.*s\n", cc->handleLen, cc->handle);
      exit(1);
   } else {
      puts("Unexpected flag received. Exiting.");
      exit(1);
   }

}

void sendMessageHelper(char *message, char *destHandle, CClient *cc){
   int toSend, stop = 0;
   char sendBuf[1000];
   sendBuf[999] = '\0';

   while(strlen(message) <= 999 && !stop){
      if(strlen(message) > 999){
         toSend = 999;
      } else {
         toSend = strlen(message);
         stop = 1;
      }
      memcpy(sendBuf, message, toSend);
      sendMessagePacket(sendBuf, destHandle, cc);
      message = message + toSend;   
   }
}

void sendMessagePacket(char *message, char *destHandle, CClient *cc){ 
   ChatHeader ch;
   u_char destHandleLen = strlen(destHandle);

   //Copy appropriate data into the buffer 
   ch.packetLen = htons(cc->handleLen + destHandleLen + strlen(message) + 6);
   ch.flag = MESSAGE;
   memcpy(cc->buf, &ch, sizeof(ChatHeader));
   memcpy(cc->buf + sizeof(ChatHeader), &destHandleLen, sizeof(u_char));
   memcpy(cc->buf + sizeof(ChatHeader) + sizeof(u_char), destHandle, destHandleLen);
   memcpy(cc->buf + sizeof(ChatHeader) + sizeof(u_char) + destHandleLen, &cc->handleLen, sizeof(u_char));
   memcpy(cc->buf + sizeof(ChatHeader) + sizeof(u_char) + destHandleLen + sizeof(u_char), cc->handle, cc->handleLen);
   memcpy(cc->buf + sizeof(ChatHeader) + sizeof(u_char) + destHandleLen + sizeof(u_char) + cc->handleLen, message, strlen(message) + 1);
   
   //Send the packet
   send(cc->serverSocket, cc->buf, ntohs(ch.packetLen), 0);
}

void sendBroadcastPacket(char *message, CClient *cc){ 
   ChatHeader ch;

   //Copy appropriate data into the buffer 
   ch.packetLen = htons(cc->handleLen + strlen(message) + 5);
   ch.flag = BROADCAST;
   memcpy(cc->buf, &ch, sizeof(ChatHeader));
   memcpy(cc->buf + sizeof(ChatHeader), &cc->handleLen, sizeof(u_char));
   memcpy(cc->buf + sizeof(ChatHeader) + sizeof(u_char), cc->handle, cc->handleLen);
   memcpy(cc->buf + sizeof(ChatHeader) + sizeof(u_char) + cc->handleLen, message, strlen(message)+ 1);
   
   //Send the packet
   send(cc->serverSocket, cc->buf, ntohs(ch.packetLen), 0);
}

void requestHandleList(CClient *cc){ 
   ChatHeader ch;

   ch.flag = REQUEST_HANDLES;
   ch.packetLen = htons(sizeof(ChatHeader));
   
   send(cc->serverSocket, &ch, sizeof(ChatHeader), 0);
}
   
void waitForResponse(CClient *cc){
   ChatHeader ch;
   int amtReceived;

   if ((amtReceived = recv(cc->serverSocket, &ch, sizeof(ChatHeader), 0)) < 0){
		perror("recv call");
		exit(-1);
	}

   if(amtReceived == 0){
      puts("Server Terminated");
      exit(1);
   }
   
   memcpy(cc->buf, &ch, sizeof(ChatHeader));
   ch.packetLen = ntohs(ch.packetLen);

   if(ch.packetLen != sizeof(ChatHeader)){
      if (recv(cc->serverSocket, cc->buf + sizeof(ChatHeader), ch.packetLen - sizeof(ChatHeader), 0) < 0){
		   perror("recv call");
		   exit(-1);
	   }

   }
}


void exitServer(CClient *cc){
   ChatHeader ch;

   ch.packetLen = htons(sizeof(ChatHeader));
   ch.flag = EXIT;
   memcpy(cc->buf, &ch, sizeof(ChatHeader));
   send(cc->serverSocket, cc->buf, sizeof(ChatHeader), 0);
   
   FD_CLR(0, &cc->fdSet);
   
}
