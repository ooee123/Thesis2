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
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#define IN_CMD_ARGS 2
#define BACKLOG 5
#define TABLE_SIZE 20
#define BUFF_SIZE 1000
#define HANDLE_LEN 256
#define CHDR_SIZE 3
#define MSG_FLAG 5
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

struct NameTable {
   char *name;
};

int serverSetup(int argc, char **argv) {
   int server_socket = 0;
   unsigned short portnum;
   struct sockaddr_in local;
   socklen_t len = sizeof(local);
   
   server_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (server_socket < 0) {
      perror("socket call\n");
      exit(1);
   }

   local.sin_family = AF_INET;
   local.sin_addr.s_addr = INADDR_ANY;

   if (argc == IN_CMD_ARGS) {
      portnum = atoi(argv[IN_CMD_ARGS - 1]);       
      local.sin_port = htons(portnum);
   }
   else {
      local.sin_port = htons(0);
   } 

   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call\n");
      exit(-1);
   }

   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
      perror("getsockname call\n");
      exit(-1);
   }
   
   if (listen(server_socket, BACKLOG) < 0) {
      perror("listen call");
      exit(-1);
   } 
   
   printf("socket has port %d \n", ntohs(local.sin_port));

   return server_socket;
}

int tcpAccept(int server_socket) {
   int client_socket = 0;

   if ((client_socket = accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
   {
      perror("accept call");
      exit(-1);
   }

   return(client_socket);
}

struct NameTable* tableInit() {
   struct NameTable *table;
   int index = 0;

   if ((table = malloc(TABLE_SIZE)) == NULL) {
      perror("malloc() error\n");
      exit(-1);
   } 

   while (index < TABLE_SIZE) {
      table[index++].name = NULL;
   }
   
   return table;
}

int getClientName(struct NameTable *table, int *tableIndex, int socket) {
   int tempIndex = 0, rec, srclen;
   uint8_t pos = 2, err = 3;
   char *handle, *src, rsp[CHDR_SIZE];

   if ((handle = (char *) calloc(1, HANDLE_LEN + CHDR_SIZE)) == NULL) {
      perror("Realloc() error\n");
      exit(-1);
   }
   
   if ((rec = recv(socket, handle, HANDLE_LEN + CHDR_SIZE, 0)) < 0) {
      perror("Receive message error in getClientName\n");
      exit(-1);
   }

   srclen = handle[CHDR_SIZE];
   if ((src = (char *) malloc(srclen + 1)) == NULL) {
      perror("malloc get client name");
      exit(-1);
   }

   memcpy(src, handle + CHDR_SIZE + 1, srclen);
   src[srclen] = '\0';
 
   while (tempIndex < *tableIndex) {
      if (table[tempIndex].name != NULL && 
         strcmp(src, table[tempIndex].name) == 0) {
         memcpy(rsp + 2, &err, 1);
         if (send(socket, rsp, CHDR_SIZE, MSG_CONFIRM) < 0) {
            perror("Error on sending error message");
            exit(-1);
         }
         free(handle);
         free(src);
         return -1;
      }
      tempIndex++;
   }

   memcpy(rsp + 2, &pos, 1);
   if (send(socket, rsp, CHDR_SIZE, MSG_CONFIRM) < 0) {
      perror("Error on sending pos message");
      exit(-1);
   }

   tempIndex = 0;
   while (table[tempIndex].name != NULL) {
      tempIndex++;
   }

   if (tempIndex < *tableIndex) {
      if ((table[tempIndex].name = (char *) malloc(srclen + 1)) == NULL) {
         perror("Handle name malloc\n");
         exit(-1);
      }
      memcpy(table[tempIndex].name, src, srclen + 1);
   }
   else {
      if ((table[*tableIndex].name = (char *) malloc(srclen + 1)) == NULL) {
         perror("Handle name malloc\n");
         exit(-1);
      }
      memcpy(table[*tableIndex].name, src, srclen + 1);
      *tableIndex = *tableIndex + 1;

      if (*tableIndex == sizeof(table)) {
         if (realloc(table, sizeof(table) * 2) == NULL) {
            perror("Realloc() error\n");
            exit(-1);
         }
      }
   }

   free(handle); 

   return 0; 
}

int tableLookUp(struct NameTable *table, int tblIdx, int startFd, 
 char message[]) {
   char destHandle[HANDLE_LEN];
   int index = CHDR_SIZE + 1;
   uint8_t destLen = 0;

   memcpy(&destLen, message + CHDR_SIZE, 1); 
   memcpy(destHandle, message + index, destLen);
   destHandle[destLen] = '\0';
     
   index = 0;
   while (index < tblIdx) {
      if (table[index].name != NULL && 
       strcmp(destHandle, table[index].name) == 0) {
         return (startFd + index);
      }
      index++;
   }
    
   return -1;
}

void broadcastMsg(int tblIdx, int client_soc, int startFd, 
 char message[], uint16_t msgLen) {
   int index = 0, fd;

   while (index < tblIdx) {
      if (index != (client_soc - startFd)) {
         fd = index + startFd;
         if (send(fd, message, msgLen, MSG_CONFIRM) < 0) {
            perror("Error on send");
            exit(-1);
         }
      }
      index++;
   }
}

void forwardMsg(int handleFd, char message[], uint16_t msgLen) {

   if (send(handleFd, message, msgLen, MSG_CONFIRM) < 0) {
      perror("Error on send");
      exit(-1);
   }
}

void sendErrorMsg(char message[], int client_soc) {
   char destHandle[HANDLE_LEN], *errPacket;
   int index = CHDR_SIZE + 1;
   uint8_t destLen = 0, flag;
   uint16_t pktLen, temp;

   memcpy(&destLen, message + CHDR_SIZE, 1);
   memcpy(destHandle, message + index, destLen);
   destHandle[destLen] = '\0';

   if ((errPacket = malloc(destLen + CHDR_SIZE + 1)) == NULL) {
      perror("err Msg malloc");
      exit(-1);
   } 
   pktLen = destLen + CHDR_SIZE + 1;
   temp = htons(pktLen);
   pktLen = temp;

   flag = MSG_ERR_FLAG;
   memcpy(errPacket, message, destLen + CHDR_SIZE + 1);
   memcpy(errPacket, &pktLen, 2);
   memcpy(errPacket + 2, &flag, 1);

   if (send(client_soc, errPacket, ntohs(pktLen), MSG_CONFIRM) < 0) {
      perror("Error on send");
      exit(-1);
   } 

}

void ackClientExit(int client_soc) {
   char packet[CHDR_SIZE];
   struct chatHeader hdr;

   hdr.flag = ACK_EXIT_FLAG;
   hdr.length = htons(CHDR_SIZE);

   memcpy(packet, &hdr, CHDR_SIZE);
   if (send(client_soc, packet, CHDR_SIZE, 0) < 0) {
      perror("Error on Exit Send");
      exit(-1);
   }
}

void list(struct NameTable *table, int tblIdx, int client_soc) {
   char numHandles[CHDR_SIZE + sizeof(uint32_t)];
   char *data;
   struct chatHeader hdr;
   uint32_t index = 0, handles;
   uint16_t length, handleLen;

   hdr.length = htons(CHDR_SIZE + sizeof(uint32_t));
   hdr.flag = HAND_NUM_FLAG;
   handles = htonl((unsigned int) tblIdx);
   memcpy(numHandles, &hdr, CHDR_SIZE);
   memcpy(numHandles + CHDR_SIZE, &handles, sizeof(uint32_t));

   if (send(client_soc, numHandles, CHDR_SIZE + sizeof(uint32_t), 0) < 0) {
      perror("Error on Exit Send");
      exit(-1);
   }

   hdr.flag = RECV_HAND_FLAG;

   while (index < tblIdx) {
      if (table[index].name != NULL) {
         handleLen = strlen(table[index].name);
         length = CHDR_SIZE + 1 + handleLen;
         
         if ((data = malloc(length)) == NULL) {
            perror("List malloc");
            exit(-1);
         }              
         hdr.length = htons(length);
         memcpy(data, &hdr, CHDR_SIZE);
         memcpy(data + CHDR_SIZE, &handleLen, 1);
         memcpy(data + CHDR_SIZE + 1, table[index].name, handleLen);

         if (send(client_soc, data, length, 0) < 0) {
            perror("Error on Exit Send");
            exit(-1);
         }
         free(data);
      }
      index++;
   }
   
}

void messageOpt(struct NameTable *table, int tblIdx, int client_soc, 
 int startFd, char message[]) {
   uint8_t flag;
   uint16_t length, temp;
   int handleFd;

   memcpy(&length, message, 2);
   temp = ntohs(length);
   length = temp;
   memcpy(&flag, message + 2, 1);

   if (flag == BROADCAST_FLAG) {
      broadcastMsg(tblIdx, client_soc, startFd, message, length);
   }
   else if (flag == MSG_FLAG) {
      handleFd = tableLookUp(table, tblIdx, startFd, message);
      if (handleFd < 0) {
         sendErrorMsg(message, client_soc); 
      }
      else {
         forwardMsg(handleFd, message, length);
      }
   }
   else if (flag == EXIT_FLAG) {
      ackClientExit(client_soc); 
   }
   else if (flag == LIST_FLAG) {
      list(table, tblIdx, client_soc);
   }
    
} 

void receiveMsg(struct NameTable *table, fd_set *act, int *tblIdx, int *idx, 
 int *maxFd, int *preFd, int startFd){
   char message[BUFF_SIZE];
   int rec;
   
   if ((rec = recv(*idx, message, BUFF_SIZE, 0)) < 0) {
      perror("Receive message error\n");
      exit(-1);
   }
   else if (rec == 0) {
      if (*idx == *maxFd - 1) {
         *maxFd = *preFd;
         *preFd = *maxFd - 1;
      }
      free(table[*idx - startFd].name);
      table[*idx - startFd].name = NULL;
      FD_CLR(*idx, act);
      close(*idx);
      if (*maxFd == startFd) {
         *tblIdx = 0;
      }
      else if (*idx - startFd == *tblIdx - 1) {
         *tblIdx = *tblIdx - 1;
      }
   }
   else {
      messageOpt(table, *tblIdx, *idx, startFd, message);   
      memset(message, 0, BUFF_SIZE);
      *idx = *maxFd;
   }
}

int systemControl(int server_socket) {
   fd_set active, current;
   int maxFd = server_socket + 1, index, temp_socket, tableIndex = 0;
   int preFd = maxFd, startFd = server_socket + 1;
   struct NameTable *table;

   FD_ZERO(&active);
   FD_SET(server_socket, &active);

   table = tableInit();

   while (1) {
      current = active;
      if (select(maxFd, &current, NULL, NULL, NULL) < 0) {
         perror("select\n");
         exit(-1);
      }
      
      index = 0;
      while (index < maxFd) { 
         if (FD_ISSET(index, &current)) {
            if (index == server_socket) {
               temp_socket = tcpAccept(server_socket);
               if (getClientName(table, &tableIndex, temp_socket) < 0) {
                  close (temp_socket);
               }
               else {
                  FD_SET(temp_socket, &active);
                  if (temp_socket == maxFd) {
                     preFd = maxFd;
                     maxFd = temp_socket + 1;
                     index = maxFd;
                  }
               }
               
            }
            else {
               receiveMsg(table, &active, &tableIndex, &index, &maxFd, 
                &preFd, startFd);
            }
         }
         index++;
      }
   }
   return 0;
} 

int main(int argc, char **argv) {
   int server_socket = 0;

   server_socket = serverSetup(argc, argv);   
   systemControl(server_socket);
   return 0;
}
