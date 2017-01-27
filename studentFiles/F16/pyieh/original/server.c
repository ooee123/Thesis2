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
#include "cclient.h"
#include "server.h"

static int numHandles = 0;
static int maxNumHandles = INITIAL_HANDLES;

/* Adds handle+socketNum pair to list of handles, adds socketNum to FD_SET */   
void add_handle(my_socket **head_ptr, char *handle, uint32_t socketNum, fd_set *rFds) {
   my_socket *head = *head_ptr;
   my_socket *next = (head + numHandles);
   next->handle = calloc(strlen(handle) + 1, sizeof(char));
   strcpy(next->handle, handle);
   
   memcpy(&(next->socketNum), &socketNum, sizeof(socketNum));
   
   // realloc if necessary
   if (++numHandles >= maxNumHandles) {
      *head_ptr = realloc(head, (maxNumHandles*2)*sizeof(my_socket));
      maxNumHandles = maxNumHandles*2;
   }
   
   FD_SET(socketNum, rFds);
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

char *createPacket(char *text, uint8_t flag, uint16_t size, int needFree) {
   char *packet = calloc(size, sizeof(char));
   chat_header *pckt_hdr = (chat_header *)packet;
   
   pckt_hdr->len = htons(size);
   pckt_hdr->flag = flag;
   if (text) {
      memcpy(packet + CHAT_HEADER_LEN, text, size - CHAT_HEADER_LEN + 1);
      
      if (needFree) {
         free(text);
      }
   }
   return packet;
} 

int exists_handle(my_socket *head, char *handle) {
   int i;
   for (i = 0; i < numHandles; i++) {
      if (strcmp((head + i)->handle, handle) == 0) {
         return i;
      }
   }
   return -1;
}

void remove_handle(my_socket *head, char *handle, fd_set *rFds) {
   int i;
   for (i = 0 ; i < numHandles; i++) {
      if (strcmp((head + i)->handle, handle) == 0) {
         FD_CLR((head + i)->socketNum, rFds);
         memmove(head + i, head + i + 1, sizeof(my_socket)*(numHandles - i));
         numHandles--;
         return;
      }
   } 
}

void remove_handle_by_socket(my_socket *head, int socketNum, fd_set *rFds) {
   int i;
   for (i = 0 ; i < numHandles; i++) {
      if ((head + i)->socketNum == socketNum) {
         FD_CLR((head + i)->socketNum, rFds);
         memmove(head + i, head + i + 1, sizeof(my_socket)*(numHandles - i));
         numHandles--;
         return;
      }
   } 
}

// format: 1b = handleLen, rest = handle
void makeHandleData(char *buf, char *handle, uint8_t len) {
   memcpy(buf, &len, sizeof(len));
   memcpy(buf + sizeof(len), handle, len);
}

int listenSockets(fd_set *readFds, int server_socket) {
   int i;
   if (select(FD_SETSIZE, readFds, NULL, NULL, NULL) < 0) {
      perror("select");
   }

      if (FD_ISSET(server_socket, readFds)) {
         return tcpAccept(server_socket);
      }
      else {
         for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, readFds) && i != server_socket) {
               return i;
            }
         }
      }
   
   return -1;
}

void sockListen(int socketNum) {
   if (listen(socketNum, 5) < 0) {
      perror("listen call");
      exit(-1);
   }
}

void broadcastMessage(char *buf, my_socket *all_handles, int client_socket) {
   char srcHandle[HANDLE_MAX_LEN];
   uint8_t handleLen;
   int msgLen, i;
   
   // get SRC handle len
   memcpy(&handleLen, (buf + CHAT_HEADER_LEN), sizeof(handleLen));
   // get SRC handle 
   memcpy(&srcHandle, buf + CHAT_HEADER_LEN + sizeof(handleLen), handleLen);
   srcHandle[handleLen] = '\0';
         
   msgLen = strlen(buf + CHAT_HEADER_LEN + sizeof(handleLen) + handleLen);
   
   for (i = 0; i < numHandles; i++) {
      if (strcmp((all_handles + i)->handle, srcHandle) != 0) { // Not original sender
         if (sendPacket(buf, (all_handles + i)->socketNum, 0)) {
            perror("send()");
            exit(-1);
         }
      }
   }
}

void sendReadyToListHandlesAck(int client_socket) {
   char *pkt;
   uint32_t *totalHandles = malloc(sizeof(uint32_t));
   *totalHandles = htonl(numHandles);
   pkt = createPacket((char *)totalHandles, S_NUM_HANDLES_FLAG, CHAT_HEADER_LEN + sizeof(uint32_t), 0);
   if (sendPacket(pkt, client_socket, 1)) {
      perror("sendReadyToListHandlesAck:send");
      exit(-1);
   }
}
      
void sendListHandles(int client_socket, my_socket *all_handles) {
   char *pkt, buf[HANDLE_MAX_LEN + 1];
   int i;
   uint8_t handleLen;
   for (i = 0; i < numHandles; i++) {
      // make packet data: handleLen (1) + handle (handleLen)
      handleLen = strlen((all_handles + i)->handle);
      makeHandleData(buf, (all_handles + i)->handle, handleLen);
      
      // create to-be-sent packet
      pkt = createPacket(buf, S_HANDLE_FLAG, CHAT_HEADER_LEN + sizeof(uint8_t) + handleLen, 0);
      // set chat header's packetLen to 0
      ((chat_header *)pkt)->len = 0;
      // send packet
      sendPacket(pkt, client_socket, 1);
   }   
}

void sendExitAck(int client_socket) {
   char *pkt = createPacket(NULL, S_ACK_EXIT_FLAG, CHAT_HEADER_LEN, 0);
   
   if (sendPacket(pkt, client_socket, 1)) {
      perror("runClient:L:send");
      exit(-1);
   }
}

/* buf -> packet data */
void action(char *buf, my_socket **all_handles_ptr, int client_socket, fd_set *readFds) {
   packet *pkt = (packet *)buf;
   my_socket *all_handles = *all_handles_ptr;
   uint8_t flag = (pkt->header).flag, handleLen;
   char *sending_pkt;
   char handle[HANDLE_MAX_LEN];
   int  handleOffset;
      
   if (flag == C_HANDLE_FLAG) { // 1: New client trying to connect 
      memcpy(&handleLen, (buf + CHAT_HEADER_LEN), sizeof(handleLen));
      memcpy(handle, buf + CHAT_HEADER_LEN + sizeof(handleLen), handleLen);
      handle[handleLen] = '\0';

      if (exists_handle(all_handles, handle) > -1) { // Handle already exists
         // re-flip bits before sending
         ((chat_header *)buf)->len = htons(((chat_header *)buf)->len);
         sending_pkt = createPacket(NULL, 3, CHAT_HEADER_LEN, 0);
      }
      else { // Handle doesn't already exist, adding handle-socket pair to list
         sending_pkt = createPacket(NULL, 2, CHAT_HEADER_LEN, 0);
         add_handle(all_handles_ptr, handle, client_socket, readFds);
      }
   
      if (sendPacket(sending_pkt, client_socket, 1)) {
         perror("send()");
         exit(-1);
      }
   }
   else if (flag == C_BROADCAST_FLAG) { // 4: Broadcast      
      broadcastMessage(buf, all_handles, client_socket);
   }
   else if (flag == C_MESSAGE_FLAG) { // 5: Message       
      // get DEST handle len 
      memcpy(&handleLen, (buf + CHAT_HEADER_LEN), sizeof(handleLen));
      // get DEST handle
      memcpy(handle, buf + CHAT_HEADER_LEN + sizeof(handleLen), handleLen);
       *(handle + handleLen) = '\0';
       
      if ((handleOffset = exists_handle(all_handles, handle)) > -1) { // DEST Handle exists -> send message to DEST
         sendPacket(buf, (all_handles + handleOffset)->socketNum, 0);
      } 
      else { // DEST Handle does NOT exist -> send error message
         makeHandleData(buf, handle, handleLen);
         //format: chat header (3) + dest handle len (1) + dest handle (d) 
         sending_pkt = createPacket(buf, S_BAD_MSG_DEST_FLAG, CHAT_HEADER_LEN + sizeof(uint8_t) + handleLen, 0);
         
         if (sendPacket(sending_pkt, client_socket, 1)) {
         perror("send()");
         exit(-1);
         }
      }
      
   }
   else if (flag == C_EXIT_FLAG) { // 8: Client request exit       
      sendExitAck(client_socket);
   }
   else if (flag == C_LIST_HANDLES_FLAG) { // 10: Client request list of handles      
      sendReadyToListHandlesAck(client_socket);
      
      sendListHandles(client_socket, all_handles);
   }
   // don't need to free sending_pkt, sencPacket() frees it
}

int runServer(int argc, char **argv) {
   int server_socket; 
	int client_socket; 
	char *buf = NULL; 
	int message_len = 0; 
   my_socket *all_handles = calloc(INITIAL_HANDLES, sizeof(my_socket));
   fd_set readFds;
   int i;
   
	// create buffer to store receiving packets
	if ((buf = (char *) malloc(MAXBUF)) == NULL){
		perror("malloc");
		exit(-1);
	}
	
	// create the server socket
	server_socket = tcpServerSetup();
   
   sockListen(server_socket);
	
   
   while(1) {      
      /* Reset sockets */
      FD_ZERO(&readFds);
      FD_SET(server_socket, &readFds);
      
      for (i = 0; i < numHandles; i++) {
         FD_SET((all_handles + i)->socketNum, &readFds);
      }
      
      client_socket = listenSockets(&readFds, server_socket);
      
      // get data on chosen client's socket
      if ((message_len = recv(client_socket, buf, MAXBUF, 0)) < 0) {
         perror("recv call");
         exit(-1);
      } 
      else if (message_len == 0) { // client closed with Ctrl-C
         remove_handle_by_socket(all_handles, client_socket, &readFds);         
      }
      else {         
         action(buf, &all_handles, client_socket, &readFds);
      }
   }
   
	// free data  
   free(all_handles);
   free(buf);
   
   // close sockets
	close(server_socket);
	close(client_socket);
   
   return 0;
}

int main(int argc, char *argv[]) {	
   runServer(argc, argv);
	return 0;
}