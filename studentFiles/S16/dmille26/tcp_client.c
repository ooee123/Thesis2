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
#include "chat.h"

#define NUM_ARGS 4

int lenOfStrTilChar(char* str, char c) {
   // Returns Length Of String With a Null. Counts the Null
   int i = 0;
   // add constraints for null within 255 but not part of the name
   while(str[i] != c && i < MAX_STR)
      i++;
   if (str[i] == c)
      return i;
   else 
      return -1;
}

int lenOfStr(char * str) {
   // Returns Length Of String With a Null. Counts the Null
   int i = 0;
   while(str[i] != '\0' && i < MAX_STR)
      i++;
   if (str[i] == '\0') {
      return i;
   }
   else 
      return -1;
}
void parseArgs(int argc, char* argv[], cliArgs* opts) {
   int lenPort = 0;
   int lenHandle = 0;
   int lenServer = 0;
   if(argc!= NUM_ARGS) {
      printf("usage: %s handle server-name port-number\n", argv[0]);
      exit(1);
   }
   lenHandle = strlen(argv[1]);
   if (lenHandle > MAX_STR - 1) {
      printf("Illegal Hanlde Length. Must be less than 256 Characters.\n");
      exit(1);
   }
   strncpy(opts->handle, argv[1], lenHandle);
   opts->handle[lenHandle] = '\0';
   lenServer = strlen(argv[2]);
   if (lenServer > MAX_STR - 1) {
      printf("Illegal ServerName Length. Must be less than 256 Characters.\n");
      exit(1);
   }
   strncpy(opts->serverName, argv[2], lenServer);
   opts->serverName[lenServer] = '\0';
   int tmp = atoi(argv[3]);
   if (tmp > MIN_PORT && tmp < MAX_PORT) {
      lenPort = strlen(argv[3]);
      strncpy(opts->port, argv[3], lenPort);
      opts->port[lenPort] = '\0';
   } else {
      printf("Invalid Port Number. Must be in range (%d : %d)\n",MIN_PORT, MAX_PORT);
      exit(1);
   }
}

int claimHandleRequest(cliArgs* meta, int client_socket) {
   struct msgPkt msg;
   uint16_t pktLen = NORMAL_HDR_LEN;
   msg.flags = C_S_INIT;
   pktLen += setClientHandle(&msg, meta);
   // + 1 for field indicating length og src handle
   int pkt_size = NORMAL_HDR_LEN + msg.lenSrcHandle + 1; 

   msg.pktLenHost = pkt_size;
   u_char* packet = calloc(1, pkt_size);
   int offset = cpyHdr(&msg, packet);
   cpyCli(&msg, packet, offset);
   int sent =  send(client_socket, packet, pkt_size, 0);
   free(packet);
   if(sent < 0) {
      perror("send call");
      exit(-1);
   }
   return 1;
}
int claimHandleResponse(cliArgs * meta, int client_socket) {
   int message_len = 0;
   u_char* buf = calloc(1, NORMAL_HDR_LEN);
      if ((message_len = recv(client_socket, buf, NORMAL_HDR_LEN, 0)) < 0) {
         perror("recv call");
         exit(-1);
      }
      if (message_len != NORMAL_HDR_LEN) {
         printf("Server Response to claiming handle error.\n");
         exit(-1);
      }
      if (buf[2] == S_C_GOOD_HNDL) {
         return 1;
      } else if ( buf[2] == S_C_ERR_HNDL_EXISTS) {
         printf("\nHandle already in use: %s\n", meta->handle);
         exit(1);
      } else {
         perror("Unexpected flag, in response to claiming handle.");
      }
   free(buf);
}
int claimHandle(cliArgs* meta, int client_socket) {
   claimHandleRequest(meta, client_socket);
   claimHandleResponse(meta, client_socket);
}
void serverTerminated() {
   printf("\nServer Terminated\n");
   exit(2);
}

void errorDstHandle(u_char* data) {
   int lenOfHandle = data[0];
   char handle[lenOfHandle + 1];
   memcpy(handle, data + 1, lenOfHandle);
   handle[lenOfHandle] = '\0';
   printf("\nClient with handle '%s' does not exist.\n", handle);
}
int printHandle(u_char* data, int offset) {
   int len = data[offset];
   char tmpHandle[len + 1];
   memcpy(tmpHandle, data + offset + 1, len);
   tmpHandle[len] = '\0';
   printf("\t%s\n",tmpHandle);
   return offset + len + 1;
}
void incomingListOfHandles(u_char* data, int client_socket, int len) {
   int offset = 4;
   uint32_t numHandles;
   memcpy(&numHandles, data, sizeof(uint32_t));
   numHandles = ntohl(numHandles);
   printf("\nNumber of Clientst: %d\n", numHandles);
   while(numHandles) {
      offset = printHandle(data, offset);
      numHandles--;
   }
}
      
void displayMessage(u_char * data, int len) {
   int lenSrcHandle = data[0];
   int dataOffset = lenSrcHandle + 1;
   char message[len + 3];
   memcpy(message, data + 1, lenSrcHandle);
   message[lenSrcHandle] = ':';
   message[lenSrcHandle + 1] = ' ';
   memcpy(message + lenSrcHandle + 2, data + dataOffset, len - dataOffset);
   message[len + 2] = '\0';
   int lenData = strlen (data + dataOffset);
   printf("\n%s\n", message);
}
void sameLinePrompt() {
   printf("$: ");
   fflush(stdout);
}
void processResponse(int flag, u_char* data, int client_socket, int len) {
   static uint32_t numHandles;
   if (flag == C_S_C_DIR_MSG) {
      displayMessage(data, len); 
      sameLinePrompt();
   } else if (flag == C_A_BROADCAST) {
      displayMessage(data, len);
      sameLinePrompt();
   } else if (flag == S_C_ERR_DST_NEXISTS) {
      errorDstHandle(data);
      sameLinePrompt();
   } else if (flag == S_C_ACK_EXT) {
      exit(1);
   } else if (flag == S_C_NUM_HNDLS) {
      memcpy(&numHandles, data, sizeof(uint32_t));
      numHandles = ntohl(numHandles);
   } else if (flag == S_C_VALID_HNDL) {
      incomingListOfHandles(data, client_socket, len);
      sameLinePrompt();
   } else {
      printf("Unexpected Flag from Server Packet.\n");
   }
}

int recieveData(int client_socket) {
   u_char* buf = calloc(1, NORMAL_HDR_LEN);
   int message_len = 0;
   int dataBufLen = 0;
   int flag = 0;
      if ((message_len = recv(client_socket, buf, NORMAL_HDR_LEN, 0)) < 0) {
         perror("recv call");
         exit(-1);
      }
      if (message_len == 0)
         serverTerminated();
      else {
         dataBufLen = parseHdr(buf, &flag);
         free(buf);
         buf = calloc(1, dataBufLen);
         if (dataBufLen > 0) {
            if((message_len = recv(client_socket, buf, dataBufLen, 0)) < 0) {
               perror("recv call");
               exit(-1);
            }
         }
         processResponse(flag, buf, client_socket, dataBufLen);
      }
   return dataBufLen;
}
int getMsgData(char* buf, int* done) {
   fgets(buf, MAX_MSG_LEN, stdin);
   int len = strlen(buf);
   if (buf[len - 1] == '\n') {
      *done =1;
      return len -1;
   }
   return len;
}
//From StandardIn
int getDstHandle(struct msgPkt* msg) {
   char buf [MAX_MSG_LEN];   
   scanf("%s", buf);
   msg->lenDstHandle = strlen(buf);
   if (msg->lenDstHandle > MAX_STR - 1) {
      return -1;
   }
   memcpy(msg->dstHandle, buf, msg->lenDstHandle);
   return msg->lenDstHandle;
}
int staticBroadcastMsgSetup(struct msgPkt* msg, u_char* packet, cliArgs* meta) {
   int offset = NORMAL_HDR_LEN;
   setClientHandle(msg, meta);
   offset = cpyCli(msg, packet, offset);
   msg->flags = C_A_BROADCAST;
   return offset;
}
int staticMsgSetup(struct msgPkt* msg, u_char* packet, cliArgs* meta) {
   int offset = NORMAL_HDR_LEN;
   setClientHandle(msg, meta);
   if (getDstHandle(msg) == -1) {
      printf("\nHandle exceeds allowed limit.\n");
      return -1;
   }
   offset = cpyDst(msg, packet, offset);
   offset = cpyCli(msg, packet, offset);
   msg->flags = C_S_C_DIR_MSG; 
   return offset;
}
void dynamicMsgSetup(struct msgPkt* msg, u_char* packet, int offset, int* done) {
      char buf[MAX_MSG_LEN];
      int dataOffset = getMsgData(buf, done);
      setMsgLen(msg, offset + dataOffset + 1);
      cpyHdr(msg, packet);
      cpyData(buf, packet, dataOffset, offset);
}
 
int message(cliArgs* meta, int client_socket) {
   struct msgPkt msg; 
   int done = 0;
   u_char packet[2 * MAX_MSG_LEN]; //won't transmit extrabytes

   int offset;
   if ((offset = staticMsgSetup(&msg, packet, meta)) == -1) {
      return -1;
   }
   getchar(); // eat up single space
   while(done == 0) {
      dynamicMsgSetup(&msg, packet, offset, &done);
      int sent =  send(client_socket, packet, msg.pktLenHost, 0);
      if(sent < 0) {
         perror("send call");
         exit(-1);
      }
   }
   return 1;
}


void broadcast(cliArgs* meta, int client_socket) {
struct msgPkt msg; 
   int done = 0;
   u_char packet[2 * MAX_MSG_LEN]; //won't transmit extrabytes

   int offset = staticBroadcastMsgSetup(&msg, packet, meta);
   while(done == 0) {
      dynamicMsgSetup(&msg, packet, offset, &done);
      int sent =  send(client_socket, packet, msg.pktLenHost, 0);
      if(sent < 0) {
         perror("send call");
         exit(-1);
      }
   }
   return 1;

}
void listActiveHandles(int client_socket) {
   char packet[NORMAL_HDR_LEN];
   uint16_t pktLen = NORMAL_HDR_LEN;
   pktLen = htons(pktLen);
   memcpy(packet, &pktLen, sizeof(uint16_t));
   packet[2] = C_S_REQ_HNDLS; 
   int sent = send(client_socket, packet, NORMAL_HDR_LEN, 0);
   if(sent < 0) {
      perror("send call");
      exit(-1);
   }
}

void exitSec(int client_socket) {
   char packet[NORMAL_HDR_LEN];
   uint16_t pktLen = NORMAL_HDR_LEN;
   pktLen = htons(pktLen);
   memcpy(packet, &pktLen, sizeof(uint16_t));
   packet[2] = C_S_REQ_EXT;
   int sent = send(client_socket, packet, NORMAL_HDR_LEN, 0);
   if(sent < 0) {
      perror("send call");
      exit(-1);
   }
}
void invalidCmd() {
   printf("Invalid command.\n%%M - Message <handle> [text]\t %%B - Broadcast [text]\n");
   printf("%%L - List Handles\t\t %%E - exit\nCoMmAnDs ArE CaSe InSeNsItVe\n");
}
void controlClient(cliArgs* meta, int client_socket) {
   char cmd[4];
   char* trash = calloc(1, MAX_STR);
   fgets(cmd, 4, stdin);
         switch(cmd[1]) {
            case 'm':
               message(meta, client_socket);
               break;
            case 'M':
               message(meta, client_socket);
               break;
            case 'b':
               broadcast(meta, client_socket);
               break;
            case 'B':
               broadcast(meta, client_socket);
               break;
            case 'l':
               listActiveHandles(client_socket);
               break;
            case 'L':
               listActiveHandles(client_socket);
               break;
            case 'e':
               exitSec(client_socket);
               break;
            case 'E':
               exitSec(client_socket);
               break;
            default:
               fgets(trash, MAX_STR, stdin);
               invalidCmd();
               break;
         } 
         sameLinePrompt();
         free(trash);
}

int checkForData(int maxFd, fd_set* rfds, fd_set* wfds, fd_set* efds, 
                 struct timeval* tv) {
      return select(maxFd, rfds, wfds, efds, NULL);
}
int resetFds(fd_set* fds_set, int socket) {
   FD_ZERO(fds_set);
   FD_SET(0, fds_set);
   FD_SET(socket, fds_set);
   FD_CLR(1, fds_set);
   FD_CLR(2, fds_set);
   return socket + 1;
}
void prompt() {
   printf("\n$: ");
   fflush(stdout);
}
void runClient(int socket_num, cliArgs* meta) {
   claimHandle(meta, socket_num);
   fd_set rfds;
   int maxFd = resetFds(&rfds, socket_num);
   sameLinePrompt();
   while(1) {
      checkForData(maxFd, &rfds, NULL, NULL, NULL);
      if (FD_ISSET(socket_num, &rfds)) {
         recieveData(socket_num);
      } if (FD_ISSET(0, &rfds)) {
         controlClient(meta, socket_num);
      }
      maxFd = resetFds(&rfds, socket_num);
   }
}

int main(int argc, char * argv[]) {
   int socket_num;         //socket descriptor
   cliArgs meta;
   /* check command line arguments  */
   parseArgs(argc, argv, &meta);
   /* set up the socket for TCP transmission  */
   socket_num= tcp_send_setup(meta.serverName, meta.port);
   runClient(socket_num, &meta);

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
   int status = connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in));
   if (status < 0)
   {
      perror("connect call");
      exit(-1);
   }

   return socket_num;
}

