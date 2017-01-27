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

void messagePackets(int socket_num, char *myHandle) {
   char** buffers = malloc(sizeof(char*)); //start w/ single buffer
   int numbuffs = 1; // number of buffers
   
   
   char *send_buf = NULL;         //data buffer
   int sent = 0;            //actual amount of data sent

   char *destHandle = malloc(255);
   char *text = malloc(1000);
   *buffers = text;
   int textlen = 0;
   
   scanf("%s", destHandle); //this is the handle of the client I want to talk to
   
   //char extraspace = getchar();
   
   while((text[textlen] = getchar()) != '\n') {
      textlen++;
      if (textlen == 999) {
         numbuffs++;
         text[textlen] = '\0';
         buffers = (char **) realloc(buffers, sizeof(char*) * numbuffs);
         text = (char *) malloc (1000);
         *(buffers + (numbuffs-1)) = text;
         textlen = 0;
      }
   }
   text[textlen] = '\0';
   uint16_t packetLen = 5 + strlen(destHandle) + 1000 + strlen(myHandle) + 1; //host to network
   uint16_t npacketLen = htons(packetLen);
   
   //create packet buffer
   if ((send_buf = malloc(packetLen)) == NULL)
   {
      perror("malloc() error");
      exit(-1);
   }
   
   char *mover = send_buf;
   memcpy(mover, &npacketLen, 2);
   mover+=2;
   
   uint8_t flag = 5;
   memcpy(mover, &flag, 1);
   mover +=1;
   
   uint8_t destLen = strlen(destHandle);
   memcpy(mover, &destLen, 1);
   mover += 1;
   
   memcpy(mover, destHandle, strlen(destHandle));
   mover += strlen(destHandle);
   
   uint8_t myLen = strlen(myHandle);
   memcpy(mover, &myLen, 1);
   mover += 1;
   
   memcpy(mover, myHandle, strlen(myHandle));
   mover += strlen(myHandle);
   
   int i;
   for(i = 0; i < numbuffs; i++) {
      memcpy(mover, buffers[i], strlen(buffers[i]));
      sent = send(socket_num, send_buf, packetLen, 0);
   }
}

void broadcastMessage(int socket_num, char* myHandle) {
   char *send_buf = NULL;         //data buffer
   int sent = 0;            //actual amount of data sent
   int textlen = 0;
   
   char *text = malloc(1000);
   while((text[textlen] = getchar()) != '\n') {
      textlen++;
   }
   //scanf(" %[^\n]", text); //this is the message I want to send

   text[textlen] = '\0';
   
   uint16_t packetLen = 4 + strlen(myHandle) + strlen(text) +1; //host to network
   uint16_t npacketLen = htons(packetLen);
   //create packet buffer
   if ((send_buf = malloc(packetLen)) == NULL)
   {
      perror("malloc() error");
      exit(-1);
   }
   char *mover = send_buf;
   
   memcpy(mover, &npacketLen, 2);
   mover += 2;
   
   uint8_t flag = 4;
   memcpy(mover, &flag, 1);
   mover+=1;
   
   uint8_t sendlen = strlen(myHandle);
   memcpy(mover, &sendlen, 1);
   mover+= 1;
   

   memcpy(mover, myHandle, strlen(myHandle));
   mover += strlen(myHandle);

   strcpy(mover, text);
   
   sent =  send(socket_num, send_buf, packetLen, 0);
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

void listHandles(int socket_num, char* myHandle) {
   char* buf = malloc(3);
   
   uint16_t len = htons(3);
   memcpy(buf, &len, 2);
   uint8_t flag = 10;
   memcpy(buf+2, &flag, 1);
   
   send(socket_num, buf, 3, 0);
   
}

void exitProgram(int socket_num) {
   char* buf = malloc(3);
   
   uint16_t packetLen = 3;//host to network order
   uint16_t npacketLen = htons(packetLen);
   memcpy(buf, &npacketLen, 2);
   uint8_t flag = 8;
   memcpy(buf+2, &flag, 1);
   
   send(socket_num, buf, packetLen, 0);
   
}


void parseCommands(int socket_num, char* handle) {
   fd_set fdvar;
   FD_ZERO(&fdvar);
   FD_SET(0, &fdvar);
   FD_SET(socket_num, &fdvar);
   
   uint32_t numListHandles;
   
   char* buf = malloc(MAXBUF);
   char* command = malloc(3);
   while(1) {
      printf("$:");
      fflush(stdout);
      
      select(4, &fdvar, NULL,NULL, NULL);
      if(FD_ISSET(0, &fdvar)) {
         
         scanf("%s", command);
         
         if ((*command != '%') || ((*(command + 1) != 'M') && (*(command + 1) != 'm') && (*(command + 1) != 'B') && (*(command + 1) != 'b') && (*(command + 1) != 'L') && (*(command + 1) != 'l') && (*(command + 1) != 'E') && (*(command + 1) != 'e'))) {
            printf("Invalid Command\n");
         }
         
         else if(strcmp(command, "%M") == 0 || strcmp(command, "%m") == 0)
            messagePackets(socket_num, handle);
         
         
         else if(strcmp(command, "%B") ==0 || strcmp(command, "%b") == 0)
            broadcastMessage(socket_num, handle);
         
         else if(strcmp(command, "%L") ==0 || strcmp(command, "%l") == 0)
            listHandles(socket_num, handle);
         
         else if(strcmp(command, "%E") ==0 || strcmp(command, "%e") == 0)
            exitProgram(socket_num);
       }
      
       else if(FD_ISSET(socket_num, &fdvar)){
          int rec = recv(socket_num, buf, 3, 0);
          
          
          //determine if message or broadcast
          uint16_t packetLen;
          memcpy(&packetLen, buf, 2);//this will be in network order
          uint16_t pLen = ntohs(packetLen);
      
          uint8_t flag;
          memcpy(&flag, buf + 2, 1);
          
          if(flag != 12)
             rec = recv(socket_num, buf, pLen- 3, 0); // recieve rest of packet
                
          // all flag checks
          
          if(flag == 5) {
                   //message
                   uint8_t senderHandle;
                   memcpy(&senderHandle, buf+strlen(handle) +1, 1);
                   
                   char* sender = malloc(senderHandle);
                   memcpy(sender, buf + 2 + strlen(handle), senderHandle);
                   
                   char *text = malloc(1000);
                   strcpy(text, buf+ 2+ strlen(handle) + senderHandle);
                   
                   printf("\n%s: %s\n", sender, text);
             
          }
          
         
          
          else if(flag == 4) {
                   //broadcast
                   uint8_t sendlen;
                   memcpy(&sendlen, buf, 1);
                   
                   char *s = malloc(sendlen);
                   memcpy(s, buf+1, sendlen);
                   
                   char* text = malloc(1000);
                   strcpy(text, buf+1+sendlen);
                   
                   printf("\n%s: %s\n", s, text);
             
          }
                
          
          else if (flag == 7)
             
          {
                   uint8_t handlelen;
                   memcpy(&handlelen, buf, 1);
                   char *toprint = malloc(handlelen);
                   memcpy(toprint, buf+1, handlelen);
                   //error response from message
                   printf("\nClient with handle %s does not exist.\n", toprint);
             
          }
                
          
          else if (flag == 9) {
                   //okay to terminate
                   exit(0);
             
          }
                
          
          else if(flag == 11){
                   uint32_t num;
                   memcpy(&num, buf, 4);
                   numListHandles = ntohl(num);
          }
                
          
          else if(flag == 12) {
             //print results of handles
             printf("\nNumber of clients: %lu\n", (unsigned long)numListHandles);
             uint8_t len;
             char* string;
             int i;
             
             for (i = 0; i < numListHandles; i++) {
                recv(socket_num, &len, 1, 0);
                string = malloc(len);
                recv(socket_num, string, len, 0);
                printf("\t%s\n", string);
             }
             
          }
       
       }
      
      FD_ZERO(&fdvar);
      FD_SET(0, &fdvar);
      FD_SET(socket_num, &fdvar);
      fflush(stdout);

    }
}

void sendInitialPacket(char* handle, int socket_num) {
   //chat-header, one byte handle length, handle
   uint8_t handleLen = strlen(handle);
   int send_len = handleLen + 4;
   int sent, message_len;
   
   char* send_buf = malloc(send_len);
   char* mover = send_buf;
   
   uint16_t var = htons(send_len);//host to network
   memcpy(mover, &var, 2);
   mover +=2;
   uint8_t flag = 1;
   memcpy(mover, &flag, 1);
   mover+=1;
   memcpy(mover, &handleLen, 1);
   mover+=1;
   memcpy(mover, handle, handleLen);
   
   sent =  send(socket_num, send_buf, send_len, 0);
   
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
   
   char* buf = (char *) malloc(MAXBUF);
   //client blocks until it recieves a 2 or 3 flag back
   if ((message_len = recv(socket_num, buf, MAXBUF, 0)) < 0)
   {
      perror("recv call");
      exit(-1);
   }
   
   uint8_t responseFlag;
   memcpy(&responseFlag, buf+2, 1);
   
   if(responseFlag == 2) {
      parseCommands(socket_num, handle);
   }
   

   else if(responseFlag == 3) {
      printf("Handle already in use: %s\n", handle);
      exit(0);
   }
   
   
}

int main(int argc, char** argv) {
   int socket_num = 0;         //socket descriptor
   
   if(argc < 4) {
      printf("Invalid command line arguments.\n");
      return 0;
   }
      
   /* set up the TCP Client socket  */
   socket_num = tcpClientSetup(argv[2], argv[3]);
   sendInitialPacket(argv[1], socket_num); //send initial packet and continue program
   
   
   return 0;
}
