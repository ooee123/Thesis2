/******************************************************************************
 * Timothy Chu 3PM Lab
 * CPE464 Program2
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
#include <ctype.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "testing.h"
#include "clientserver.h"

int main(int argc, char *argv[]) {
   int socket_num;               
   char *handle = argv[1];      

   /* connect to server */
   socket_num = setUpClient(argc, argv, handle);
   /* start chat service */
   clientChat(socket_num, handle);
   close(socket_num);
   return 0;
}

int setUpClient(int argc, char **argv, char *handle) {
   int socket_num;
   int buf_size = sizeof(normalHeader);
   char recv_buf[buf_size];
   int recvd;

   /* check command line arguments  */
   if (argc != 4) {
      printf("usage: %s handle host-name port-number\n", argv[0]);
	   exit(-1);
   }

   /* set up the socket for TCP transmission */
   socket_num = tcp_send_setup(argv[2], argv[3]);
   sendInitialPacket(socket_num, handle);
   /* wait for server to respond */
   recvd = recv(socket_num, recv_buf, buf_size, MSG_WAITALL);
   checkReceivedServerData(recvd, socket_num);
   if (recvd != buf_size) {
      printf("Did not correctly receive header\n");
      exit(-1);
   } 

   normalHeader nh;
   memcpy(&nh, recv_buf, sizeof(normalHeader));
   if (nh.flag == S_TO_C_BAD_HANDLE) {
      printf("Handle already in use: %s\n", handle);
      exit(-1);
   } 

   return socket_num;
}

int tcp_send_setup (char *host_name, char *port) {
   int socket_num;
   struct sockaddr_in remote;       // socket IP address for remote side
   struct hostent *hp;              // address of remote host

   /* create the socket */
   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	   perror("socket call");
	   exit(-1);
	}

   /* designate the addressing family */
   remote.sin_family = AF_INET;
   /* get the address of the remote host and store */
   if ((hp = gethostbyname(host_name)) == NULL) {
	   printf("Error getting hostname: %s\n", host_name);
	   exit(-1);
   }
    
   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);
   /* get the port used on the remote side and store */
   remote.sin_port = htons(atoi(port));

   if (connect(socket_num, (struct sockaddr *) &remote, 
    sizeof(struct sockaddr_in)) < 0) {
	   perror("connect call");
	   exit(-1);
   }

   return socket_num;
}

void sendInitialPacket(int socket_num, char *handle) {
   int sent;
   char *send_buf;
   uint8_t handleLen = strlen(handle);
   int send_len = sizeof(normalHeader) + sizeof(uint8_t) + handleLen;
   normalHeader nh = {htons(send_len), C_TO_S_INITIAL_PACKET};

   send_buf = malloc(send_len * sizeof(char));
   memcpy(send_buf, &nh, sizeof(normalHeader));
   memcpy(send_buf + sizeof(normalHeader), &handleLen, sizeof(uint8_t));
   memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t), handle, 
    handleLen);
      
   sent = send(socket_num, send_buf, send_len, 0);
   if (sent < 0) {
      perror("send call");
	   exit(-1);
   }
   free(send_buf);
}

void checkReceivedServerData(int recvd, int socket_num) {
   if (recvd < 0) {
      perror("recv call");
      exit(-1);
   }
   if (recvd == 0) {
      printf("Server Terminated. Exiting\n");
      close(socket_num);
      exit(0);
   }
}

void clientChat(int socket_num, char *handle) {
   while (1) {
      printf("$: ");
      fflush(stdout);

      /* blocking for input by using select */
      serviceClientFDS(socket_num, handle);
   }
}

void serviceClientFDS(int socket_num, char *handle) {
   fd_set read_fd_set;
   int sel;   
   int highestFD = socket_num + 1;

   FD_ZERO(&read_fd_set);
   FD_SET(0, &read_fd_set);
   FD_SET(socket_num, &read_fd_set);

   sel = select(highestFD, &read_fd_set, NULL, NULL, NULL);
   if (sel < 0) {
      perror("select call");
      exit(-1);
   }
   
   if (FD_ISSET(socket_num, &read_fd_set)) {
      processServerData(socket_num);
   }
   if (FD_ISSET(0, &read_fd_set)) {
      processSTDIN(socket_num, handle);
   }
}

void processServerData(int socket_num) {
   int recvd;
   char *recv_buf = NULL;
   uint16_t dataLen;

   while((recvd = receiveServerData(socket_num, &recv_buf, &dataLen)) == 0);
   printf("\n");
   printReceivedServerData(socket_num, recv_buf, recvd); 
   if (recv_buf != NULL) {
      free(recv_buf);
   }
}

int receiveServerData(int socket_num, char **recv_buf, uint16_t *packetLen) { 
   enum STATES {START_STATE, PROCESS_LENGTH, GET_DATA};
   static enum STATES cur = START_STATE;
   static uint8_t msgLen[2] = {0};
   static int dataLen = 0;
   int recvd = 0;

   switch (cur) {
      case START_STATE: 
         recvd = recv(socket_num, msgLen, sizeof(uint16_t), MSG_WAITALL);
         checkReceivedServerData(recvd, socket_num);

         if (recvd == 1) {
            printf("MSG_WAITALL did not block for the full message\n");
            cur = PROCESS_LENGTH;   
         } else { //receivedBytes == 2
            memcpy(packetLen, msgLen, sizeof(uint16_t));
            *packetLen = ntohs(*packetLen);
            *recv_buf = malloc(*packetLen - sizeof(uint16_t));
            cur = GET_DATA;
         }
         break;
      case PROCESS_LENGTH:
         recvd = recv(socket_num, msgLen + 1, sizeof(uint8_t), MSG_WAITALL);
         checkReceivedServerData(recvd, socket_num);

         memcpy(packetLen, msgLen, sizeof(uint16_t));
         *packetLen = ntohs(*packetLen);
         *recv_buf = malloc(*packetLen - sizeof(uint16_t));
         cur = GET_DATA;
         break;
      case GET_DATA:
         recvd = recv(socket_num, *recv_buf, *packetLen - sizeof(uint16_t), MSG_WAITALL);
         checkReceivedServerData(recvd, socket_num);
 
         if (dataLen + recvd != *packetLen - sizeof(uint16_t)) {
            printf("MSG_WAITALL did not block for the full message\n");
            dataLen += recvd;
         } else {
            cur = START_STATE;
            dataLen = 0;
            return *packetLen;
         }
         break;
   }
   return 0;
}

void printReceivedServerData(int socket_num, char *recv_buf, int packetLen) {
   uint8_t flag;
   uint8_t handleLen = 0;
   char handle[MAX_HANDLE_LEN + 1];
   static uint32_t numHandles= 0;
   memcpy(&flag, recv_buf, sizeof(uint8_t));

   if (flag == S_TO_C_NO_HANDLE) {
      memcpy(&handleLen, recv_buf + sizeof(uint8_t), sizeof(uint8_t)); 
      memcpy(handle, recv_buf + (2 * sizeof(uint8_t)), handleLen);
      handle[handleLen] = '\0';
      printf("Client with handle %s does not exist.\n", handle); 
   } else if (flag == S_TO_C_LIST_HANDLES_ACK) {
      memcpy(&numHandles, recv_buf + sizeof(uint8_t), sizeof(uint32_t));
      numHandles = ntohl(numHandles);
      printf("Number of clients: %d", numHandles);
      processServerData(socket_num);
   } else if (flag == S_TO_C_LIST_HANDLES) {
      int ptr = sizeof(uint8_t);
      while(numHandles > 0) {
         memcpy(&handleLen, recv_buf + ptr, sizeof(uint8_t));
         ptr += sizeof(uint8_t);
         memcpy(&handle, recv_buf + ptr, handleLen);
         handle[handleLen] = '\0';
         printf("   %s\n", handle);
         ptr += handleLen;
         numHandles--;
      }
   } else if (flag == S_TO_C_ACK_EXIT) {
      close(socket_num);
      printf("System acknowledged exit. Goodbye\n");
      exit(0);
   } else {
      memcpy(&handleLen, recv_buf + sizeof(uint8_t), sizeof(uint8_t));
      memcpy(handle, recv_buf + (2 * sizeof(uint8_t)), handleLen);
      handle[handleLen] = '\0';
      printf("%s: %s\n", handle, recv_buf + (2 * sizeof(uint8_t)) + handleLen);
   }
}

void processSTDIN(int socket_num, char *handle) {
   int i, noMoreInput = 0;
   char opcode[OPCODE_LEN];                  //opcode buffer
   char send_buf[MAX_MSG_LEN];                  //data buffer
   char destHandle[MAX_HANDLE_LEN];
   int send_len = 0;
     
   /* get opcode */
   for (i = 0; i < OPCODE_LEN; i++) {
      opcode[i] = getchar();
   }

   /* skip expected space */
   if(getchar() != '\n') {
      if (opcode[1] == 'M' || opcode[1] == 'm') {      
         i = 0;
         while (1) {
            destHandle[i] = getchar();
            if (destHandle[i] == ' ') {
               destHandle[i] = '\0';
               break;
            }
            if (destHandle[i] == '\n') {
               destHandle[i] = '\0';
               noMoreInput = 1;
               fprintf(stdin, "\n");
               break;
            }
            i++;
         }
      }
      if(noMoreInput == 0) {
         /* get message */
         while ((send_buf[send_len] = getchar()) != '\n') {
            if (send_len == MAX_MSG_LEN - 2) {
               send_buf[send_len + 1] = '\0';
               sendClientRequest(socket_num, handle, opcode, destHandle, send_buf, send_len + 1);
               send_len = -1;
            }
	         send_len++;
         }
      }
   }
   send_buf[send_len] = '\0';
   sendClientRequest(socket_num, handle, opcode, destHandle, send_buf, send_len + 1); 
}

void sendClientRequest(int socket_num, char *srcHandle, char *opcode, char *destHandle,
 char *buf, int send_len) {
   char *msg = NULL;
   uint16_t msgLen = 0;

   /* check for valid input */
   if (opcode[0] != '%') {  
      printf("Invalid command\n");
   } else {
      if (tolower(opcode[1]) == 'b') {
         int success = getBroadcastMessage(buf, &msg, &msgLen);
         if (success) {
            sendBroadcast(socket_num, srcHandle, msg, msgLen);
         }
      } else if (tolower(opcode[1]) == 'm') {
         int success = checkHandleAndMessage(destHandle, buf, &msgLen);
         if (success) {
            sendMessage(socket_num, srcHandle, destHandle, buf, msgLen);
         }
      } else if (tolower(opcode[1]) == 'l') {
         sendList(socket_num);
      } else if (tolower(opcode[1]) == 'e') {
         sendExit(socket_num);
      } else {
         printf("Invalid command\n");
      }
   }
}

int getBroadcastMessage(char *buf, char **msg, uint16_t *msgLen) {
   *msg = buf;

   while (buf[*msgLen] != '\0') {
      (*msgLen)++;
      if (*msgLen == 0) {
         printf("Message too big for normal header\n");
         return 0;
      }
   }
   (*msgLen)++;
   return 1;
}

void sendBroadcast(int socket_num, char *srcHandle, char *msg, 
 uint16_t msgLen) {
   int sent = 0;
   char *send_buf;
   uint8_t srcHandleLen = strlen(srcHandle);
   int send_len = sizeof(normalHeader) + sizeof(uint8_t) + srcHandleLen + 
    msgLen;
   normalHeader nh = {htons(send_len), C_TO_S_BROADCAST};

   send_buf = malloc(send_len * sizeof(char));
   memcpy(send_buf, &nh, sizeof(normalHeader));
   memcpy(send_buf + sizeof(normalHeader), &srcHandleLen, sizeof(uint8_t));
   memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t), srcHandle, 
    srcHandleLen);
   memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t) + srcHandleLen,
    msg, msgLen);
      
   /* now send the data */
   sent = send(socket_num, send_buf, send_len, 0);
   if (sent < 0) {
      perror("send call");
	   exit(-1);
   }
   free(send_buf);
}

int checkHandleAndMessage(char *destHandle, char *buf, uint16_t *msgLen) {
   if (strlen(destHandle) > 255) {
      printf("Client with handle %s does not exist.\n", destHandle);
      return 0;
   }

   while (buf[*msgLen] != '\0') {
      (*msgLen)++;
      if(*msgLen == 0) {
         printf("Message too big for normal header\n");
         return 0;
      }
   }
   (*msgLen)++;
   return 1;
}

void sendMessage(int socket_num, char *srcHandle, char *destHandle, 
 char *msg, uint16_t msgLen) {
   int sent;
   char *send_buf;
   uint8_t srcHandleLen = strlen(srcHandle);
   uint8_t destHandleLen = strlen(destHandle);
   int send_len = sizeof(normalHeader) + sizeof(uint8_t) + destHandleLen + 
    sizeof(uint8_t) + srcHandleLen + msgLen;
   normalHeader nh = {htons(send_len), C_TO_S_MESSAGE};

   send_buf = malloc(send_len * sizeof(char));
   memcpy(send_buf, &nh, sizeof(normalHeader));
   memcpy(send_buf + sizeof(normalHeader), &destHandleLen, sizeof(uint8_t));
   memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t), destHandle, destHandleLen);
   memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t) + destHandleLen, 
    &srcHandleLen, sizeof(uint8_t));
   memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t) + destHandleLen +
    sizeof(uint8_t), srcHandle, srcHandleLen);
   memcpy(send_buf + sizeof(normalHeader) + sizeof(uint8_t) + destHandleLen +
    sizeof(uint8_t) + srcHandleLen, msg, msgLen);

   sent = send(socket_num, send_buf, send_len, 0);
   if (sent < 0) {
      perror("send call");
	   exit(-1);
   }
   free(send_buf);
}

void sendExit(int socket_num) {
   int sent;
   normalHeader nh = {htons(sizeof(normalHeader)), C_TO_S_EXIT};

   sent = send(socket_num, &nh, sizeof(normalHeader), 0);
   if (sent < 0) {
      perror("send call");
	   exit(-1);
   }
}

void sendList(int socket_num) {
   int sent;
   normalHeader nh = {htons(sizeof(normalHeader)), C_TO_S_LIST_HANDLES};

   sent = send(socket_num, &nh, sizeof(normalHeader), 0);
   if (sent < 0) {
      perror("send call");
	   exit(-1);
   }
}
