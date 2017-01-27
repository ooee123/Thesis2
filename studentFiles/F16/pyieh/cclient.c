#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "testing.h"
#include "networks.h"
#include "cclient.h"

static char myHandle[HANDLE_MAX_LEN];
static int numHandles = 0;
packet readPacket(int socketNum);

// creates packet to be sent, if needFree == 1, frees data (after it's been copied into packet)
char *createPacket(char *data, uint8_t flag, uint16_t size, int needFree) {
   char *packet = calloc(size, sizeof(char));
   chat_header *pckt_hdr = (chat_header *)packet;
   
   pckt_hdr->len = htons(size);
   pckt_hdr->flag = flag;
   if (data) {
      memcpy(packet + CHAT_HEADER_LEN, data, size - CHAT_HEADER_LEN + 1);
      
      if (needFree) {
         free(data);
      }
   }
   
   return packet;
} 

int sendPacket(char *pkt, int socketNum, int needFree) {
   int totalSent;
   uint16_t packetLen = ntohs(((chat_header *)pkt)->len);
   uint8_t flag = ((chat_header *)pkt)->flag;
   
   uint8_t handleLen;
   
   if (flag == S_HANDLE_FLAG) {
      memcpy(&handleLen, pkt + CHAT_HEADER_LEN, sizeof(uint8_t));
      packetLen = CHAT_HEADER_LEN + sizeof(uint8_t) + handleLen;
   }
      
   totalSent = send(socketNum, pkt, packetLen, 0);
   
   if (needFree) {
      free(pkt);
   }
   return (packetLen - totalSent);
}

void newLine() {
   printf("$: ");
   fflush(stdout);
}

void clearStdIn() {
   while (getchar() != '\n') {}
}

/* Helper to check which stream has data ready */
int checkSelect(int socketNum) {
   fd_set rfds;
   int selectSocket;
   
   FD_ZERO(&rfds);
   FD_SET(0, &rfds);
   FD_SET(socketNum, &rfds);
   
   selectSocket = select(FD_SETSIZE, &rfds, NULL, NULL, NULL);
   
   if (selectSocket < 0) {
      perror("check_select:select");
      exit(-1);
   }
   else if (selectSocket) {
      if (FD_ISSET(0, &rfds)) {
         return 0;
      }
      else if (FD_ISSET(socketNum, &rfds)) {
         return socketNum;
      }
   }
   return -1;
}

void printClientMessage(packet pkt) {
   int offset;
   char srcHandle[HANDLE_MAX_LEN];
   uint8_t hLen1, hLen2;
   
   memcpy(&hLen1, pkt.data, sizeof(hLen1));
   memcpy(&hLen2, pkt.data + sizeof(hLen1) + hLen1, sizeof(hLen2));
   
   memcpy(srcHandle, pkt.data + sizeof(hLen1) + hLen1 + sizeof(hLen2), hLen2);
   srcHandle[hLen2] = '\0';  
   
   // data format: hLen1 (1) + DEST (hLen1) + hLen2 (1) + SRC (hLen2) + msg
   offset = 1 + hLen1 + 1 + hLen2;
   
   printf("\n%s: %s\n", srcHandle, pkt.data + offset);
}

void printClientBroadcast(packet pkt) {
   int offset;
   char srcHandle[HANDLE_MAX_LEN];
   uint8_t hLen;
   
   // copy SRC handle len
   memcpy(&hLen, pkt.data, sizeof(hLen));
   // copy SRC handle 
   memcpy(srcHandle, pkt.data + sizeof(hLen), hLen);
   srcHandle[hLen] = '\0';
   
   
   // data format: hLen (1) + SRC (hLen) + msg
   offset = 1 + hLen;
   
   printf("\n%s: %s\n", srcHandle, pkt.data + offset);
}

uint32_t getNumHandles(packet pkt) {
   uint32_t numHandles;
   
   memcpy(&numHandles, pkt.data, sizeof(numHandles));
   
   return ntohl(numHandles);
}

// format: 1b = handleLen, rest = handle
void makeHandleData(char *buf, char *handle, uint8_t len) {
   memcpy(buf, &len, sizeof(len));
   memcpy(buf + sizeof(len), handle, len);
}

void printClientHandle(packet pkt) {
   uint8_t handleLen;
   char handle[HANDLE_MAX_LEN];
   
   memcpy(&handleLen, pkt.data, sizeof(handleLen));
   memcpy(handle, pkt.data + sizeof(uint8_t), handleLen);
   handle[handleLen] = '\0';
   
   printf("\t%s\n", handle);
   
   if (!(--numHandles)) {
      newLine();
   }
}

void sendHandleListRequest(int socketNum) {
   char *pkt = createPacket(NULL, C_LIST_HANDLES_FLAG, CHAT_HEADER_LEN, 0);
               
   if (sendPacket(pkt, socketNum, 1)) {
      perror("runClient:L:send");
      exit(-1);
   }
}

void sendExitRequest(int socketNum) {
   char *pkt = createPacket(NULL, C_EXIT_FLAG, CHAT_HEADER_LEN, 0);
   
   if (sendPacket(pkt, socketNum, 1)) {
      perror("runClient:L:send");
      exit(-1);
   }
}


void action(packet pkt) {
   packet *pkt_ptr = (packet *)(&pkt);
   int flag = (pkt_ptr->header).flag; 
   uint8_t handleLen;
   char handle[HANDLE_MAX_LEN];
      
   if (flag == S_BAD_MSG_DEST_FLAG) { // Sent packet with unknown DEST handle
      // get DEST handle len 
      memcpy(&handleLen, (pkt.data), sizeof(handleLen));
      // get DEST handle
      memcpy(handle, pkt.data + sizeof(handleLen), handleLen);
      *(handle + handleLen) = '\0';
      
      fprintf(stderr, "\nClient with handle %s does not exist.\n", handle);
      newLine();
   }
   else if (flag == C_MESSAGE_FLAG) { // Received message from (other) client delivered by server
      printClientMessage(pkt);
      newLine();
   }
   else if (flag == C_BROADCAST_FLAG) {
      printClientBroadcast(pkt);
      newLine();
   }
   else if (flag == S_NUM_HANDLES_FLAG) {      
      numHandles = getNumHandles(pkt);
      
      printf("Number of clients: %d\n", numHandles);
   }
   else if (flag == S_HANDLE_FLAG) {      
      printClientHandle(pkt);
   }
   else if (flag == S_ACK_EXIT_FLAG) {
      free(pkt.data);
      exit(0);
   }
   else {
      fprintf(stderr, "Received packet with unknown flag: %d\n", flag);
   }
   // free up packet data
   free(pkt.data);
}

int runClient(int socketNum, char **argv) {
   char c = 'a';
   char *pkt, text[MAX_MSG_LEN+1], dest_handle[HANDLE_MAX_LEN+1], *data;
   int d, t, ret_select;
   packet rec_packet;
   
   newLine();
   
   while(1) {
      // sees if socket from server has data, if not then go to stdIn
      while ((ret_select = checkSelect(socketNum)) < 0) {} 
      // stdIn 
      if (!ret_select) {
         if (getchar() == '%') {
            c = getchar();
            getchar();
            if (c == 'M' || c == 'm') { // Message
               // get dest handle
               for (d = 0; (c = getchar()) != ' '; d++) {
                  dest_handle[d] = c;
               }
               
               while (c != '\n') {
                  // get message
                  for (t = 0; ((c = getchar()) != '\n') && t <= MAX_MSG_LEN; t++) {
                     text[t] = c;
                  }
                  text[t] = '\0';
               
                  /* message data buffer: 1 (DEST handle len) + d (DEST handle) 
                                       + 1 (SRC handle len) + s (SRC handle) + t+1 (message) */
                  data = calloc(2*sizeof(uint8_t) + d + (t + 1) + strlen(myHandle), sizeof(char));
               
                  // copy DEST handle data into buffer
                  makeHandleData(data, dest_handle, d);
                  // copy SRC handle data into buffer 
                  makeHandleData(data + sizeof(uint8_t) + d, myHandle, strlen(myHandle));
                  // copy message data into buffer
                  strcpy(data + sizeof(uint8_t) + d + sizeof(uint8_t) + strlen(myHandle), text);
               
                  // create packet to be sent
                  pkt = createPacket(data, C_MESSAGE_FLAG, 
                                    CHAT_HEADER_LEN + 2*sizeof(uint8_t) +d+t+1+strlen(myHandle), 1);
                  if (sendPacket(pkt, socketNum, 1)) {
                     perror("runClient:M:send");
                     exit(-1);
                  }
              
               }
               newLine();
            } 
            else if (c == 'B' || c == 'b') { // Broadcast
               while (c != '\n') {
                  for (t = 0; ((c = getchar()) != '\n') && t <= MAX_MSG_LEN; t++) {
                     text[t] = c;
                  }
                  text[t] = '\0';
                  
                  /* broadcast data buffer: 1 (SRC handle len) + s (SRC handle) + t+1 (message) */
                  data = calloc(sizeof(uint8_t) + strlen(myHandle) + (t + 1), sizeof(char));
                  
                  //copy SRC handle data into buffer
                  makeHandleData(data, myHandle, strlen(myHandle));
                  //copy message data into buffer 
                  strcpy(data + sizeof(uint8_t) + strlen(myHandle), text);
                  
                  // create packet to be sent
                  pkt = createPacket(data, C_BROADCAST_FLAG, 
                                     CHAT_HEADER_LEN + sizeof(uint8_t) + strlen(myHandle) + (t+1), 1);
                  
                  if (sendPacket(pkt, socketNum, 1)) {
                     perror("runClient:B:send");
                     exit(-1);
                  }
               }
               newLine();
            }
            else if (c == 'L' || c == 'l') { // List of handles
               sendHandleListRequest(socketNum);
               //newLine();
            }
            else if (c == 'E' || c == 'e') { // Exit
               sendExitRequest(socketNum);
            }
            else {
               fprintf(stderr, "Invalid input\n");
               clearStdIn();
               newLine();

            }
            // Don't need to free, data ptr is freed after it's sent in sendPacket()
         }
         else {
            fprintf(stderr, "Invalid input\n");
            clearStdIn();
            newLine();

         }
      }
      else { // input from server
         rec_packet = readPacket(socketNum);
         
         //printf("Got packet from server\n");
         action((rec_packet));
         //newLine();
      }
   }
}

// Reads in a packet from given socketNum, does conversions from Network->Host order for chat_header fields
packet readPacket(int socketNum) {
   packet pkt;
   chat_header hdr;
   int bytesRead;
   uint8_t handleLen;
   
   // reads in size of packet
   if ((bytesRead = recv(socketNum, &(hdr.len), PACKET_LEN_BYTES, 0)) < 0) {
      fprintf(stderr, "readPacket1:recv");
      exit(-1);
   }
   else if (bytesRead == 0) {
      fprintf(stderr, "\nServer is closed\n");
      exit(-1);
   }
   hdr.len = ntohs(hdr.len);
   
   // reads in flag
   if ((bytesRead = recv(socketNum, &(hdr.flag), 1, 0)) < 0) {
      fprintf(stderr,"readPacket2:recv");
      exit(-1);
   }
   else if (bytesRead == 0) {
      fprintf(stderr, "\nServer is closed\n");
      exit(-1);
   }
   
   // copy data from hdr into chat_header in packet struct
   memcpy(&pkt, &hdr, CHAT_HEADER_LEN);
   
   if (hdr.flag == S_HANDLE_FLAG) { // flag=12, list of handles, special case b/c header.len = 0
      // read in handle len
      if ((bytesRead = recv(socketNum, &handleLen, sizeof(uint8_t), 0)) < 0) {
         fprintf(stderr, "readPacket4:recv");
         exit(-1);
      }
      else if (bytesRead == 0) {
         fprintf(stderr, "\nServer is closed\n");
         exit(-1);
      }
      // create space for data [format: handleLen (1) + handle (handleLen)]
      pkt.data = calloc(sizeof(uint8_t) + handleLen, sizeof(char));
      // copy handle data
      memcpy(pkt.data, &handleLen, sizeof(uint8_t));
      // copy handle
      if ((bytesRead = recv(socketNum, pkt.data + sizeof(uint8_t), handleLen, 0)) < 0) {
         fprintf(stderr, "readPacket5:recv");
         exit(-1);
      }
      else if (bytesRead == 0) {
         fprintf(stderr, "\nServer is closed\n");
         exit(-1);
      }
      
   }
   else {
      if ((hdr.len - CHAT_HEADER_LEN) > 0) {
         pkt.data = calloc(hdr.len - CHAT_HEADER_LEN, sizeof(char));
  
         if((bytesRead = recv(socketNum, pkt.data, hdr.len - CHAT_HEADER_LEN, 0)) < 0) {
            fprintf(stderr,"readPacket3:recv");
            exit(-1);
         }
         else if (bytesRead == 0) {
            fprintf(stderr, "\nServer is closed\n");
            exit(-1);
         }
      }
      else {
         pkt.data = NULL;
      }
   }
   return pkt;
}

int checkHandle(int socketNum, char *handle) {
   int selectSocket;
   uint8_t handleLen = strlen(handle);
   char *handlePacket;
   packet pkt;
   fd_set readFDs;
   
   // handlePacketData: 1b = handle length, rest = handle (no null/padding)
   char *handlePacketData = calloc(1 + handleLen, sizeof(char));
   memcpy(handlePacketData, &handleLen, sizeof(handleLen));
   memcpy(handlePacketData + sizeof(handleLen), handle, handleLen);
   
   // creating packet flag=1 to send
   handlePacket = createPacket(handlePacketData, C_HANDLE_FLAG, CHAT_HEADER_LEN + handleLen + sizeof(handleLen),1);
   
   FD_ZERO(&readFDs);
   FD_SET(socketNum, &readFDs);
   
   if (sendPacket(handlePacket, socketNum, 1)) {
      perror("send()");
      exit(-1);
   }
   
   selectSocket = select(socketNum + 1, &readFDs, NULL, NULL, NULL);
   if (selectSocket < 0) {
      perror("select()");
      exit(-1);
   }
   else if (selectSocket) {
         while ((pkt = readPacket(socketNum)).data == NULL && (pkt.header.len > CHAT_HEADER_LEN)) {}
         
         if (pkt.header.flag == 3) { // Handle already in use
            fprintf(stderr, "Handle already in use: %s\n", handle);
            free(pkt.data);
            exit(0);
         }
         else if (pkt.header.flag != 2) { // Error, received a non 2 or 3 flag packet
            fprintf(stderr, "Received packet with flag=%d while waiting for flag=2\n", pkt.header.flag);
            free(pkt.data);
            exit(-1);
         }
         // Good handle
         free(pkt.data);
         memcpy(myHandle, handle, strlen(handle));
   }
   return 0;
}

int main(int argc, char *argv[]) {
   int socketNum; 
   
   if (argc!= 4) {
      fprintf(stderr, "Usage: %s handle server-name server-port\n", argv[0]);
	   exit(1);
   }
   
   if (strlen(argv[1]) > HANDLE_MAX_LEN) {
      fprintf(stderr, "Handle must be less than 256 characters\n");
      exit(1);
   }
   
   socketNum = tcpClientSetup(argv[2], argv[3]);
   checkHandle(socketNum, argv[1]);
   runClient(socketNum, argv);
   return 0;
}