/******************************************************************************
 * tcp_client.c
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>

#include "networks.h"
#include "testing.h"


int socketBackup;
char handle[255];
int knownClients = 0;

void cleanup(int signo) {
   char buffer[3];
   buffer[1] = 0x03;
   buffer[FLAG_IDX] = QUIT;
   send(socketBackup, buffer, 3, 0);
   printf("interrupt handler called successfully\n");
   close(socketBackup);
   exit(0);
}

int getInput(char* buf) {
   int index;
   while ((buf[index] = getchar()) != '\n') {
      index++;
   }
   buf[index++] = '\0';
   buf[index] = '\0';
   return (buf[0] != '%' || index < 3);
}

int getFlag(char code) {
   switch (code) {
      case 'B': case 'b':
         return BROADCAST;
      case 'M': case 'm':
         return MESSAGE;
      case 'L': case 'l':
         return GET_HANDLES;
      case 'E': case 'e':
         return QUIT;
      default:
         return 0;
   }
}
      
void sendInput(int socket, char* buf) {
   uint16_t size = 0;
   int args, flag;
   char destHandle[MAX_NAME_SIZE],  finalBuffer[BUFFER_LEN + 2 * MAX_NAME_SIZE + 5];
   if (getInput(buf)) {
      return;
   }
   flag = getFlag(buf[1]);
   switch (buf[1]) {
      case 'B': case 'b':
         args = sscanf(buf + 3, "%[^\n]", finalBuffer);
         size = strlen(finalBuffer);
         strcpy(buf, finalBuffer);
         break;
      case 'M': case 'm':
         args = sscanf(buf, "%*s %s %[^\n]", destHandle, buf);
         if (args == 0) {
            printf("Please specify a recipient.\n");
            return;
         }
         else if (args == 1) {
            size = 0;
         }
         else {
            size = strlen(buf);
         }
         break;
      case 'L': case 'l': case 'E': case 'e':
         buf += 2;
         size = 0;
         break;
      default:
         printf("Invalid command\n");
         return;
   }
   size = formatMessage(handle, destHandle, buf, size, flag, finalBuffer);
   if (send(socket, finalBuffer, size, 0) < 0) {
      perror("send call");
      exit(-1);
   }
}

int receiveInput(int socket, char* buf, int* status, int suppress) {
   int flag = 0;
   char src[MAX_NAME_SIZE], dest[MAX_NAME_SIZE];
   receiveMessage(socket, src, dest, buf, &flag, knownClients);
   if (flag == INVALID_NAME) {
      printf("Client with handle %s does not exist.\n", dest);
   }
   else if (flag == KICKED) {
      close(socket);
      exit(0);
   }
   else if (flag == NUM_HANDLES) {
      memcpy(&knownClients, buf, 4); 
      knownClients = ntohl(knownClients);
      printf("Number of clients: %d\n", knownClients);
   }
   else if (flag != HANDLES && !suppress) {
      printf("%s: %s\n", src, buf);
   }
   return flag;
}

void sendUsername(int socket) {
   char writeBuffer[BUFFER_LEN], readBuffer[BUFFER_LEN];
   uint8_t handleLength;
   int state;
   uint16_t length;
   fd_set fdset;
   handleLength = strlen(handle);
   memcpy(writeBuffer + 3, &handleLength, 1);
   writeBuffer[FLAG_IDX] = JOIN;
   length = htons(handleLength + 5);
   memcpy(writeBuffer, &length, 2);
   memcpy(writeBuffer + 4, handle, handleLength);
   send(socket, writeBuffer, strlen(handle) + 4, 0);

   do {
      FD_ZERO(&fdset);
      FD_SET(socket, &fdset);
      select(FD_SETSIZE, &fdset, NULL, NULL, NULL);
      state = receiveInput(socket, readBuffer, NULL, 1);
   }
   while (state != ACCEPTED && state != REJECTED);
   if (state == REJECTED) {
      printf("Handle already in use: %s\n", handle);
      exit(-4);
   }
}

void run(int socket) {
   char readBuffer[BUFFER_LEN], writeBuffer[BUFFER_LEN];
   fd_set fdset;
   int status = ONLINE;
   socketBackup = socket;
   sendUsername(socket);
   while (1) {
       printf("$: ");
       fflush(stdout);
       FD_ZERO(&fdset);
       FD_SET(0, &fdset);
       FD_SET(socket, &fdset);
       select(FD_SETSIZE, &fdset, NULL, NULL, NULL);
       if (FD_ISSET(0, &fdset) && status == ONLINE ) {
           sendInput(socket, writeBuffer);
       }
       if (FD_ISSET(socket, &fdset)) {
          printf("\33[2K\r");
          receiveInput(socket, readBuffer, &status, 0);
       }
   }
}

int main(int argc, char * argv[]) {
   struct sigaction action;
   action.sa_handler = cleanup;
   sigaction(SIGINT, &action, NULL);
   if (argc < 4) {
      printf("Usage: cclient <server IP> <server port> <username>\n");
      exit(-1);
   }
   if (strlen(argv[3]) > MAX_NAME_SIZE) {
      printf("Handle must be at most %d characters.\n", MAX_NAME_SIZE);
   }
   int socket_num = tcp_send_setup(argv[1], argv[2]);
   strcpy(handle, argv[3]);
   run(socket_num);
   return close(socket_num);
}
