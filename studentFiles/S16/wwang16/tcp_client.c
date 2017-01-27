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

int tcp_send_setup(char *host_name, char *port);
void registerWithServer(int socket);
void transmit(int socket, char *packet, int packetLength);
void receive(char *handle, int socket);
void client(int socket_num);
int receiveServerMessage(int socket_num);
void processDestinationDNE(char *msg);
void processReceivePrivateMessage(char *msg);
void processReceiveBroadcastMessage(char *msg);
void processClientCount(char *msg);
void processClientList(char *msg, int socket, int msgLen);
void parseClientInput(int socket_num);
void processClientInput(char type, char *msg, int msgSize,
                        char *handle, int handleSize, int socket_num);
int parseMessage(char *msg);
void sendPrivateMessage(char *msg, int msgSize, char *destHandle,
                        int handleSize, int socket);
void sendBroadcastMessage(char *msg, int msgSize, int socket_num);
void sendUserListRequest(int socket_num);
void sendExitPacket(int socket_num);

static char* myHandle = NULL;

int main(int argc, char * argv[])
{
   int socket_num;         //socket descriptor
   
   /* check command line arguments  */
   if(argc!= 4) {
      printf("usage: %s <handle> <host-name> <port-number> \n", argv[0]);
      exit(1);
   }
   
   if (strlen(argv[1]) > 255) {
      printf("\nYour handle of %zu characters is too long.\n\
             Must be within 255 characters\n", strlen(argv[1]));
      exit(1);
   }
   
   /* set up the socket for TCP transmission  */
   //tested
   socket_num = tcp_send_setup(argv[2], argv[3]);
   
   /* loop to continue program as long as not %e */
   myHandle = argv[1];
   client(socket_num);
   
   /* clean up socket */
   close(socket_num);
   
   return 0;
}

void client(int socket_num) {
   fd_set activeSockets, readFromSockets;
   FD_ZERO(&activeSockets);
   FD_SET(socket_num, &activeSockets);
   FD_SET(STDIN_FILENO, &activeSockets);
   
   //Register with server
   registerWithServer(socket_num);
   printf("$: ");
   fflush(stdout);
   
   //loop to prevent client program exit
   while (TRUE) {
      readFromSockets = activeSockets;
      
      if (select(socket_num + 1, &readFromSockets, NULL, NULL, NULL) < 0) {
         perror("Select call");
         exit(-1);
      }
      else {
         if (FD_ISSET(socket_num, &readFromSockets)) {
            if (receiveServerMessage(socket_num) == -1) {
               return;
            }
         }
         if (FD_ISSET(STDIN_FILENO, &readFromSockets)) {
            parseClientInput(socket_num);
         }
      }
   }
   
}

int receiveServerMessage(int socket_num) {
   int msgLen = 0, retVal;
   enum STATE {START, PROCESS_LEN, GETDATA};
   static int currentState = START;
   static int currentLen = 0;
   static int totalLen = 0;
   static char *msgBuff = NULL;
   
   switch (currentState) {
      case START:
         if (msgBuff) {
            free (msgBuff);
         }
         msgBuff = calloc(1, SERVER_BUFF_SIZE);
         if ((msgLen = recv(socket_num, msgBuff, SERVER_BUFF_SIZE, 0)) < 0) {
            perror("recv call");
            exit(-1);
         }
         else if (msgLen == 0) {
            printf("\nServer closed, will exit now\n");
            exit(0);
         }
         else if (msgLen == 1) {
            currentLen = 1;
            currentState = PROCESS_LEN;
         }
         else if (msgLen < ntohs((uint16_t)*msgBuff)) {
            currentLen = msgLen;
            totalLen = ntohs((uint16_t)*msgBuff);
            currentState = GETDATA;
         }
         retVal = 0;
         break;
      case PROCESS_LEN:
         if ((msgLen = recv(socket_num, msgBuff, SERVER_BUFF_SIZE, 0)) < 0) {
            perror("recv call");
            exit(-1);
         }
         else if (msgLen == 0) {
            printf("\nServer closed, will exit now\n");
            exit(0);
         }
         else if ((currentLen += msgLen) >= 2) {
            totalLen = ntohs((uint16_t)*msgBuff);
            currentState = GETDATA;
         }
         break;
      case GETDATA:
         if ((msgLen = recv(socket_num, msgBuff, SERVER_BUFF_SIZE, 0)) < 0) {
            perror("recv call");
            exit(-1);
         }
         else if (msgLen == 0) {
            printf("\nServer closed, will exit now\n");
            exit(0);
         }
         else if ((currentLen += msgLen) < totalLen) {
            retVal = 0;
         }
         break;
      default:
         printf("undected error occured.\n");
         exit(-1);
         break;
   }

   Header *flagCheck = (Header *) msgBuff;
   if (flagCheck->flag == FLAG_STC_CLIENT_LIST) {
      processClientList(msgBuff, socket_num, msgLen);
      return 0;
   }
   
   if (currentLen == totalLen) {
      retVal = parseMessage(msgBuff);
      currentState = START;
   }
   
   return retVal;
}

int parseMessage(char *msg) {
   Header *header = (Header *)msg;
   //u_short pkLen = ntohs(header->packetLength);
   
   switch (header->flag) {
      case FLAG_STC_CONFIRM_EXIT:
         return -1;
      case FLAG_STC_GOOD_HANDLE:
         break;
      case FLAG_STC_BAD_HANDLE:
         printf("Error: Handle already exist on server. ");
         printf("Please pick another one.\n");
         return -1;
         break;
      case FLAG_STC_DEST_NOT_EXIST:
         processDestinationDNE(msg);
         printf("$: ");
         fflush(stdout);
         break;
      case FLAG_BROADCAST:
         processReceiveBroadcastMessage(msg);
         printf("$: ");
         fflush(stdout);
         break;
      case FLAG_STC_CLIENT_COUNT:
         processClientCount(msg);
         break;
      case FLAG_MESSAGE:
         processReceivePrivateMessage(msg);
         printf("$: ");
         fflush(stdout);
         break;
      default:
         printf("Not recognized packet, flag = %d\n", header->flag);
         break;
   }
   return 0;
}

void processDestinationDNE(char *msg) {
   char *handle = NULL;
   int handleLength = *(msg + sizeof(Header));
   
   handle = calloc(handleLength + 1, sizeof(char));
   
   memcpy(handle, msg + sizeof(Header) + 1, handleLength);
   *(handle + handleLength) = '\0';
   
   printf("Client with handle %s does not exist.\n", handle);
   
   free(handle);
}

void processReceivePrivateMessage(char *msg) {
   int msgLen = 0, handleLength, srcHandleLen;
   char *srcHandle = NULL, *message = NULL;
   
   srcHandleLen = *(msg + sizeof(Header));
   msgLen = ntohs(*((uint16_t *)msg)) - 1 - srcHandleLen; //skip dest
   handleLength = *(msg + sizeof(Header) + 1 + srcHandleLen);
   srcHandle = calloc(handleLength + 1, sizeof(char));
   
   memcpy(srcHandle, msg + sizeof(Header) + 1 + srcHandleLen + 1, handleLength);
   *(srcHandle + handleLength) = '\0';
   
   msgLen -= (1 + handleLength);
   message = calloc(msgLen + 1, sizeof(char));
   memcpy(message,
          msg + sizeof(Header) + 2 + srcHandleLen + handleLength,
          msgLen);
   *(message + msgLen) = '\0';
   
   printf("%s: %s\n", srcHandle, message);
   
   free(srcHandle);
   free(message);
}

void processReceiveBroadcastMessage(char *msg) {
   int msgLen = 0, srcHandleLen;
   char *srcHandle = NULL, *message = NULL;
   srcHandleLen = *(msg + sizeof(Header));
   srcHandle = calloc(srcHandleLen + 1, sizeof(char));
   memcpy(srcHandle, msg + sizeof(Header) + 1, srcHandleLen);
   
   msgLen = ntohs(*((uint16_t *)msg)) - 1 - srcHandleLen;
   message = calloc(msgLen + 1, sizeof(char));
   memcpy(message,
          msg + sizeof(Header) + 1 + srcHandleLen,
          msgLen);
   *(message + msgLen) = '\0';
   
   printf("\n%s: %s\n", srcHandle, message);
   
   free(srcHandle);
   free(message);
}

void processClientCount(char *msg) {
   uint32_t clientCount = ntohl(*((uint32_t *)(msg + sizeof(Header))));
   
   printf("Number of clients: %u\n", clientCount);
}

void processClientList(char *msg, int socket_num, int msgLen) {
   int currentLen = 0;
   int handleLength = 0;
   char handle[MAX_HANDLE_SIZE + 1];
   char *msgBuff= NULL;
   
   uint32_t count = ntohl(*((uint32_t *) (msg + sizeof(Header))));
   msg += sizeof(Header) + sizeof(uint32_t);
   currentLen += sizeof(Header) + sizeof(uint32_t);
   
   handleLength = *((u_char *)msg);
   msg++;
   currentLen++;
   
   while (count--) {
      if (handleLength + currentLen <= msgLen) { //within this packet
         memcpy(handle, msg, handleLength);
         handle[handleLength] = '\0';
      }
      else { //the handle split between two packets
         int readSoFar = msgLen - currentLen;
         memcpy(handle, msg, readSoFar);
         
         if (msgBuff) {
            free(msgBuff);
         }
         msgBuff = calloc(1, sizeof(SERVER_BUFF_SIZE));
         msgLen = 0;
         int remaining = handleLength - readSoFar;
         
         int readSize = 0;
         
         while (remaining > 0) {
            if ((msgLen = recv(socket_num, msgBuff, SERVER_BUFF_SIZE, 0)) < 0) {
               perror("recv call");
               exit(-1);
            }
            readSize = remaining > msgLen ? msgLen : remaining;
            
            memcpy(handle + readSoFar, msgBuff, readSize);
            readSoFar += readSize;
            remaining -= readSize;
         }
         handle[handleLength] = 0;
         
         msg = msgBuff;
         currentLen = 0;
         handleLength = readSize;
      }
      
      printf("\t%s\n", handle);
      msg += handleLength;
      currentLen += handleLength;
      
      handleLength = *((u_char *)msg);
      msg++;
      currentLen++;
   }
   printf("$: ");
   fflush(stdout);
}

void parseClientInput(int socket_num) {
   char msgBuffer[MAX_MSG_SIZE], handleBuffer[MAX_HANDLE_SIZE];
   char c = 0, type = 0;
   int handleCounter = 0, msgCounter = 0;
   
   c = fgetc(stdin);
   if (c == '%') {
      type = fgetc(stdin);
      if (type == 'e' || type == 'E' || type == 'L' || type == 'l') {
         while((c = fgetc(stdin)) != '\n' && c != EOF);
         processClientInput(type, NULL, 0, NULL, 0, socket_num);
         return;
      }
      
      if ((c = fgetc(stdin)) != ' ') {
         printf("Invalid command, please check usage.\n$: ");
         fflush(stdout);
         return;
      }
      
      if (type == 'M' || type == 'm' || type == 'B' || type == 'b') {
         if (type == 'M' || type == 'm') {
            while ((c = fgetc(stdin)) != ' ') {
               if (c == '\n') {
                  printf("Invalid command, please check usage again\n$: ");
                  fflush(stdout);
                  return;
               }
               handleBuffer[handleCounter++] = c;
               if(handleCounter > 255) {
                  printf("Target handle of %d characters is too long.\
                         Must be within 255 characters/n", handleCounter);
                  return;
               }
            }
         }
         
         while ((c = fgetc(stdin)) != '\n') {
            msgBuffer[msgCounter++] = c;
            if (msgCounter == MAX_MSG_SIZE - 1) {
               msgBuffer[msgCounter] = '\0';
               processClientInput(type, msgBuffer, msgCounter,
                                  handleBuffer, handleCounter, socket_num);
               msgCounter = 0;
            }
         }
         
         msgBuffer[msgCounter] = '\0';
         
         processClientInput(type, msgBuffer, msgCounter,
                            handleBuffer, handleCounter, socket_num);
      }
      else {
         while((c = fgetc(stdin)) != '\n' && c != EOF);
         printf("Invalid command, please check usage\n$: ");
         fflush(stdout);
      }
   }
   else {
      while((c = fgetc(stdin)) != '\n' && c != EOF);
      printf("Invalid command, please check usage\n$: ");
      fflush(stdout);
   }
}

void processClientInput(char type, char *msg, int msgSize,
                        char *handle, int handleSize, int socket_num) {
   switch (type) {
      case 'E':
      case 'e':
         sendExitPacket(socket_num);
         break;
      case 'M':
      case 'm':
         sendPrivateMessage(msg, msgSize, handle, handleSize, socket_num);
         break;
      case 'B':
      case 'b':
         sendBroadcastMessage(msg, msgSize, socket_num);
         break;
      case 'L':
      case 'l':
         sendUserListRequest(socket_num);
         break;
      default:
         break;
   }
}

void sendPrivateMessage(char *msg, int msgSize, char *destHandle,
                        int handleSize, int socket) {
   Header init;
   u_char myHandleLength;
   uint16_t packetSize;
   char *packet = NULL;
   
   myHandleLength = strlen(myHandle);
   packetSize = sizeof(Header) + 1 + handleSize + 1 + myHandleLength + msgSize;
   
   init.flag = FLAG_MESSAGE;
   init.packetLength = htons(packetSize);
   
   packet = calloc(1, packetSize);
   
   memcpy(packet, &init, sizeof(Header));
   memcpy(packet + sizeof(Header), &handleSize, 1);
   memcpy(packet + sizeof(Header) + 1, destHandle, handleSize);
   memcpy(packet + sizeof(Header) + 1 + handleSize, &myHandleLength, 1);
   memcpy(packet + sizeof(Header) + 1 + handleSize + 1,
          myHandle, myHandleLength);
   memcpy(packet + sizeof(Header) + 1 + handleSize + 1 + myHandleLength,
          msg, msgSize);
   
   transmit(socket, packet, packetSize);
   
   free(packet);
   
   printf("$: ");
   fflush(stdout);
}

void sendBroadcastMessage(char *msg, int msgSize, int socket_num) {
   Header init;
   int packetSize, myHandleLength;
   char *packet = NULL;
   
   myHandleLength = strlen(myHandle);
   packetSize = sizeof(Header) + 1 + myHandleLength + msgSize;
   
   init.flag = FLAG_BROADCAST;
   init.packetLength = htons(packetSize);
   
   packet = calloc(packetSize, sizeof(char));
   
   memcpy(packet, &init, sizeof(Header));
   memcpy(packet + sizeof(Header), &myHandleLength, 1);
   memcpy(packet + sizeof(Header) + 1, myHandle, myHandleLength);
   memcpy(packet + sizeof(Header) + 1 + myHandleLength, msg, msgSize);
   
   transmit(socket_num, packet, packetSize);
   
   free(packet);
   
   printf("$: ");
   fflush(stdout);
}

void sendExitPacket(int socket_num) {
   Header init;
   int packetSize;
   char *packet = NULL;
   
   packetSize = sizeof(Header);
   
   init.flag = FLAG_CTS_EXIT;
   init.packetLength = htons(packetSize);
   
   packet = calloc(1, packetSize);
   
   memcpy(packet, &init, sizeof(Header));
   
   transmit(socket_num, packet, packetSize);
   
   free(packet);
}

void sendUserListRequest(int socket_num) {
   Header init;
   int packetSize;
   char *packet = NULL;
   
   packetSize = sizeof(Header);
   
   init.flag = FLAG_CTS_REQ_LIST_CLIENTS;
   init.packetLength = htons(packetSize);
   
   packet = calloc(1, packetSize);
   
   memcpy(packet, &init, sizeof(Header));
   
   transmit(socket_num, packet, packetSize);
   
   free(packet);
}

void registerWithServer(int socket) {
   Header init;
   u_char handleLength;
   int packetSize;
   char *packet = NULL;
   
   handleLength = strlen(myHandle);
   packetSize = sizeof(Header) + 1 + handleLength;
   
   init.flag = FLAG_CTS_INIT;
   init.packetLength = htons(packetSize);
   
   packet = calloc(1, packetSize);
   
   memcpy(packet, &init, sizeof(Header));
   memcpy(packet + sizeof(Header), &handleLength, 1);
   memcpy(packet + sizeof(Header) + 1, myHandle, handleLength);
   
   transmit(socket, packet, packetSize);
   
   free(packet);
}

void transmit(int socket, char *packet, int packetLength) {
   int sent = send(socket, packet, packetLength, 0);
   
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
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

