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

#define MAXBUFF 2048 
#define MAX_HANDLE_LEN 256
#define MAX_MESS_LEN 1000
#define START_OF_PACKET 3
#define CHDR_SIZE 3
#define ACK_MESS_FLAG 2
#define ERR_MESS_FLAG 3
#define MESS_FLAG 5
#define BROADCAST_FLAG 4
#define MSG_ERR_FLAG 7
#define EXIT_FLAG 8
#define ACK_EXIT_FLAG 9
#define LIST_FLAG 10
#define HAND_NUM_FLAG 11
#define RECV_HAND_FLAG 12

struct chatHeader {
   uint16_t length;
   uint8_t flag;
};

struct messagePacket {
   struct chatHeader chdr;
   uint8_t destLen;
   char *destHandle;
   uint8_t srcLen;
   char *srcHandle;
   char *message;
};

static uint32_t num_handles;

void sourceInit(char **argv, char **srcName, int *srcLen) {

   *srcLen = strlen(argv[1]);
   
   if ((*srcName = (char *) malloc(*srcLen + 1)) == NULL) {
      perror("malloc() error");
      exit(-1);
   }
 
   memcpy(*srcName, argv[1], *srcLen);
   (*srcName)[*srcLen] = '\0';
}

void buffInit(char **buf) {

   if ((*buf = (char *) malloc(MAXBUFF)) == NULL) {
      perror("malloc() error");
      exit(-1);
   } 

}

int parseMessage(struct messagePacket *msg, int tag, 
 int mesLen, char *send_buf) {
   char *destName, *message;
   int index = START_OF_PACKET, start = index;

   if (tag) {
      while (send_buf[index] != ' ') {
         index++;
      }

      if (index - start < MAX_HANDLE_LEN && index < mesLen) {
         if ((destName = (char *) malloc(index - start)) == NULL) {
            perror("malloc() error");
            exit(-1);
         }
      }
      else {
         printf("Invalid Command\n");
         return -1;
      }
      memcpy(destName, send_buf + start, index - start);
      destName[index - start] = '\0';
      msg->destHandle = destName;
      msg->destLen = strlen(destName);
      msg->chdr.length = msg->chdr.length + msg->destLen + 1;
      index++;
   }

   if ((mesLen - index) < (MAX_MESS_LEN - msg->chdr.length)) {
      if ((message = (char *) malloc(mesLen - index + 1)) == NULL) {
         perror("malloc() error");
         exit(-1);
      } 
      memcpy(message, send_buf + index, mesLen - index);
      msg->message = message;
      msg->message[mesLen - index] = '\0';   
      msg->chdr.length = msg->chdr.length + (mesLen - index) + 1;
      
      index = mesLen;
   }
   else {
      if ((message = (char *) malloc(MAX_MESS_LEN - msg->chdr.length)) == NULL) {
         perror("malloc() error");
         exit(-1);
      }
      memcpy(message, send_buf + index, (MAX_MESS_LEN - msg->chdr.length - 1));
      msg->message = message;
      msg->message[MAX_MESS_LEN - msg->chdr.length] = '\0';
      msg->chdr.length = MAX_MESS_LEN;
      index += (MAX_MESS_LEN - msg->chdr.length);
   }
   return index;
}

char* packer(struct messagePacket *msg) {
   char *packet;
   int index = 2;
   int16_t length;

   if ((packet = (char *) malloc(msg->chdr.length)) == NULL) {
      perror("Packer malloc() error");
      exit(-1);
   }

   length = htons(msg->chdr.length);   

   if (msg->chdr.flag == MESS_FLAG) {
      memcpy(packet, &(length), 2);
      memcpy(packet + index, &(msg->chdr.flag), 1);
      index++;
      memcpy(packet + index, &(msg->destLen), 1);
      index++;
      memcpy(packet + index, msg->destHandle, msg->destLen);
      index = index + msg->destLen;
      memcpy(packet + index, &(msg->srcLen), 1);
      index++;
      memcpy(packet + index, msg->srcHandle, msg->srcLen); 
      index = index + msg->srcLen;
      memcpy(packet + index, msg->message, (msg->chdr.length) - index);          
   }      
   else if (msg->chdr.flag == BROADCAST_FLAG) {
      memcpy(packet, &length, 2);
      memcpy(packet + index, &(msg->chdr.flag), 1);
      index++;
      memcpy(packet + index, &(msg->srcLen), 1);
      index++;
      memcpy(packet + index, msg->srcHandle, msg->srcLen);
      index = index + msg->srcLen;
      memcpy(packet + index, msg->message, (msg->chdr.length) - index);
   }

   return packet;
}

void messageSender(char *data, int socket_num, struct messagePacket msg) {
   
   if (send(socket_num, data, msg.chdr.length, 0) < 0) {
      perror("Error on send");
      exit(-1);
   }

   free(data);
}

//TODO: finish receive function
int ackServer(int socket_num) {
   int rec;
   char handle[START_OF_PACKET];
   struct chatHeader hdr;
   
   if ((rec = recv(socket_num, handle, START_OF_PACKET, 0)) < 0) {
      perror("Receive message error in getClientName\n");
      exit(-1);
   } 

   memcpy(&hdr, handle, START_OF_PACKET);
   
   if (hdr.flag == ACK_MESS_FLAG) {
      return 0;
   }
   else if (hdr.flag == ERR_MESS_FLAG) {
      return -1;  
   }
}

void printErrMsg(char buff[]) {
   char dest[MAX_HANDLE_LEN];
   int destLen, index = START_OF_PACKET;
   uint8_t length;
   struct chatHeader hdr;

   memcpy(&hdr, buff, START_OF_PACKET);

   memcpy(&length, buff + START_OF_PACKET, 1);
   index++;

   destLen = length;
   memcpy(dest, buff + index, destLen);
   dest[destLen] = '\0';

   printf("\nClient with handle %s does not exist\n", dest);
}

void printMessage(char buff[]) {
   char src[MAX_HANDLE_LEN], msg[MAX_MESS_LEN];
   int srcLen, index = START_OF_PACKET;
   uint16_t tLen;
   uint8_t length;
   struct chatHeader hdr;

   memcpy(&hdr, buff, START_OF_PACKET);
   tLen = ntohs(hdr.length);
   hdr.length = tLen;

   memcpy(&length, buff + START_OF_PACKET, 1);
   index += length + 1;

   memcpy(&length, buff + index, 1);
   srcLen = length;
   index++;
   
   memcpy(src, buff + index, srcLen);
   index += srcLen;

   memcpy(msg, buff + index, hdr.length - index);

   printf("\n%s: %s\n", src, msg);
}

void printBroadcast(char buff[]) {
   char dest[MAX_HANDLE_LEN], msg[MAX_MESS_LEN];
   int destLen, index = START_OF_PACKET;
   uint16_t tLen;
   uint8_t length;
   struct chatHeader hdr;

   memcpy(&hdr, buff, START_OF_PACKET);
   tLen = ntohs(hdr.length);
   hdr.length = tLen;

   memcpy(&length, buff + START_OF_PACKET, 1);
   index++;

   destLen = length;
   memcpy(dest, buff + index, destLen);
   dest[destLen] = '\0';
   
   index += destLen;
   memcpy(msg, buff + index, hdr.length - index);
   printf("\n%s: %s\n", dest, msg);
}

void listNumHandles(char buff[]) {
   uint32_t temp;

   memcpy(&temp, buff + CHDR_SIZE, sizeof(uint32_t));
   num_handles = ntohl(temp);
   printf("\nNumber of clients %d\n", num_handles);
}

void receiveList(char buff[], int socket_num) {
   char handle[MAX_MESS_LEN];
   uint32_t index = 1;
   uint8_t length, offset = CHDR_SIZE + 1;
  
   memcpy(&length, buff + CHDR_SIZE, 1);
   memcpy(handle, buff + offset, length);
   handle[length] = '\0';
   printf("\n   %s\n", handle);
 
   while (index < num_handles) {
      if (recv(socket_num, buff, MAX_MESS_LEN, 0) < 0) {
         perror("Error on Network receive");
         exit(-1);
      }
      memcpy(&length, buff + CHDR_SIZE, 1);
      memcpy(handle, buff + offset, length);
      handle[length] = '\0';
      printf("   %s\n", handle);
      index++;
      memset(handle, 0, MAX_MESS_LEN);
   }

}

void receiveMsg(int socket_num) {
   char buff[MAX_MESS_LEN];
   struct chatHeader hdr;
   int rec = 0;

   if ((rec = recv(socket_num, buff, MAX_MESS_LEN, 0)) < 0) {
      perror("Error on Network receive");
      exit(-1);
   }
   else if (rec == 0){
      close(socket_num);
      printf("\n");
      exit(0);
   }
   memcpy(&hdr, buff, START_OF_PACKET);
   if (hdr.flag == BROADCAST_FLAG) {
      printBroadcast(buff);
   }
   else if (hdr.flag == MESS_FLAG) {
      printMessage(buff);
   }
   else if (hdr.flag == MSG_ERR_FLAG) {
      printErrMsg(buff);
   }
   else if (hdr.flag == ACK_EXIT_FLAG) {
      close(socket_num);
      printf("\n");
      exit(0);
   }
   else if (hdr.flag == HAND_NUM_FLAG) {
      listNumHandles(buff);     
   }
   else if (hdr.flag == RECV_HAND_FLAG) {
      receiveList(buff, socket_num);
   }
}

void messagePacketInit(struct messagePacket *msg) {

   msg->chdr.length = 3;
   msg->destHandle = NULL;
   msg->destLen = 0;
   msg->srcHandle = NULL;
   msg->srcLen = 0;
   msg->message = NULL;
}

void exiting(int socket_num) {
   char packet[CHDR_SIZE];
   struct chatHeader hdr;

   hdr.flag = EXIT_FLAG;
   hdr.length = htons(CHDR_SIZE);

   memcpy(packet, &hdr, CHDR_SIZE);

   if (send(socket_num, packet, CHDR_SIZE, 0) < 0) {
      perror("Error on Exit Send");
      exit(-1);
   } 

}

void listHandles(int socket_num) {
   char packet[CHDR_SIZE];
   struct chatHeader hdr;

   hdr.flag = LIST_FLAG;
   hdr.length = htons(CHDR_SIZE);

   memcpy(packet, &hdr, CHDR_SIZE);

   if (send(socket_num, packet, CHDR_SIZE, 0) < 0) {
      perror("Error on List Send");
      exit(-1);
   } 
}

void messageChoice(char *send_buf, char pktType[], 
 char *srcName, int srcLen, int socket_num, int mesLen) {
   struct messagePacket msg;
   int destFlag = 0, index = 0; //destflag
   char *data = NULL;
   uint8_t flag;
   
   messagePacketInit(&msg);
   msg.srcHandle = srcName;
   msg.srcLen = srcLen;
   
   if ((strcmp(pktType, "%m") == 0) || (strcmp(pktType, "%M") == 0)) {      
      msg.chdr.flag = MESS_FLAG;
      destFlag = 1;
      msg.chdr.length = msg.chdr.length + msg.srcLen + 1;

      while (index != mesLen) {
         index = parseMessage(&msg, destFlag, mesLen, send_buf);
         if (index < 0) {
            index = mesLen;
         }
         else {
            //Possible reason for not sending correctly
            data = packer(&msg);
            memcpy(&flag, data + 2, 1);
            messageSender(data, socket_num, msg);
            free(msg.message);
            msg.message = NULL;
         }
      }
      free(msg.destHandle);
      msg.destHandle = NULL;
      msg.destLen = 0;
   }
   else if ((strcmp(pktType, "%b") == 0) || (strcmp(pktType, "%B") == 0)) {
      msg.chdr.flag = BROADCAST_FLAG;
      msg.chdr.length = msg.chdr.length + msg.srcLen + 1;
 
      while (index != mesLen) {
         index = parseMessage(&msg, destFlag, mesLen, send_buf);
         if (index < 0) {
            index = mesLen;
         }
         else {
            data = packer(&msg);
            messageSender(data, socket_num, msg);
            free(msg.message);
            msg.message = NULL;
         }
      } 
   }
   else if ((strcmp(pktType, "%l") == 0) || (strcmp(pktType, "%L") == 0)) {
      printf("List flag\n");
      listHandles(socket_num);
   }
   else if ((strcmp(pktType, "%e") == 0) || (strcmp(pktType, "%E") == 0)) {
      exiting(socket_num);
   }
}

// Possible problem with pktType declaration
void getRead(char **send_buf, char pktType[][3], int *mesLen) {

   while (((*send_buf)[*mesLen] = getchar()) != '\n' && *mesLen < MAXBUFF - 1) {
      *mesLen = *mesLen + 1;
   }

   (*send_buf)[*mesLen] = '\0';
   *mesLen = *mesLen + 1;
   
   memcpy(*pktType, *send_buf, 2);
   (*pktType)[2] = '\0';
}

int msgSystem(char **argv, int socket_num) {
   fd_set active, current;
   int maxFd = socket_num + 1, fdIdx, srcLen = 0, mesLen = 0; 
   char *send_buf = NULL, *srcName = NULL, pktType[3];

   FD_ZERO(&active);
   FD_SET(STDIN_FILENO, &active);
   FD_SET(socket_num, &active);
   
   sourceInit(argv, &srcName, &srcLen);
   buffInit(&send_buf);
   
   while(1) {
      printf("$: ");
      fflush(stdout);
      current = active;
      if (select(maxFd, &current, NULL, NULL, NULL) < 0) {
         perror("select");
         exit(-1);
      }
      
      fdIdx = 0;
      while (fdIdx < maxFd) {
         if (FD_ISSET(fdIdx, &current)) {
            if (fdIdx == STDIN_FILENO) {
               getRead(&send_buf, &pktType, &mesLen);
               messageChoice(send_buf, pktType, srcName, srcLen, socket_num, mesLen);
            }
            else if(fdIdx == socket_num) {
               receiveMsg(fdIdx);  
            }
         }
         fdIdx++;
      }
      memset(send_buf, 0, MAXBUFF);
      memset(pktType, 0, 2);
      mesLen = 0;
   }
}

int ccSetup(int argc, char **argv) {
   int socket_num, index = 0;
   struct sockaddr_in remote;
   struct hostent *hp;
   char *initPacket;
   uint16_t packetlen = 0;
   uint8_t handlen = 0, flag = 1;

   if (argc != 4) {
      printf("cclient handle server-name server-port\n");
      exit(1);
   }

   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket call\n");
      exit(-1);
   }
   
   remote.sin_family = AF_INET;

   if ((hp = gethostbyname(argv[2])) == NULL) {
      printf("Error getting hostname: %s\n", argv[2]);
      exit(-1);
   }

   memcpy((char *)&remote.sin_addr, (char *)hp->h_addr, hp->h_length);

   remote.sin_port = htons(atoi(argv[argc - 1]));

   if (connect(socket_num, (struct sockaddr *)&remote, sizeof(struct sockaddr_in)) < 0) {
      perror("connect call\n");
      exit(-1);
   }

   if ((initPacket = (char *) malloc(3 + 1 + 1 + strlen(argv[1]))) == NULL) {
      perror("Initial Packet malloc() error");
      exit(-1);
   }
   
   packetlen = htons(3 + 1 + 1 + strlen(argv[1]));
   handlen = strlen(argv[1]);
   memcpy(initPacket, &packetlen, 2);
   index += 2;
   memcpy(initPacket + index, &flag, 1);
   index++; 
   memcpy(initPacket + index, &handlen, 1);
   index++;
   memcpy(initPacket + index, argv[1], handlen);
   index += handlen;
   initPacket[index] = '\0';
   
   if (send(socket_num, initPacket, ntohs(packetlen), MSG_CONFIRM) < 0) {
       perror("Error on Send");
       exit(-1);
   }   
 
   if (ackServer(socket_num) < 0) {
      printf("Handle already in use: %s\n", argv[1]);
      exit(-1);
   }
   free(initPacket);
 
   return socket_num;
}

int main(int argc, char **argv) {
   int socket_num = 0, flag = 0;

   socket_num = ccSetup(argc, argv);
   
   flag = msgSystem(argv, socket_num); //returns 0 upon exit

   if (!flag) {
      close(socket_num);
      return 0; 
   }
}
