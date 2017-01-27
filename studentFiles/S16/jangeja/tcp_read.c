#include "testing.h"
#include "tcp_read.h"

void increaseSocketBin(SocketBin *socketBin) {
   socketBin->size += 10;
   socketBin->sockets = realloc(socketBin->sockets, socketBin->size * sizeof(Socket));
}

void addSocket(SocketBin *socketBin, char *handle, uint32_t socketNum) {
   if (socketBin->numSockets == socketBin->size) {
      increaseSocketBin(socketBin);
   }
   if (handle != NULL) {
      (socketBin->sockets + socketBin->numSockets)->handle = calloc(sizeof(char) * strlen(handle) + 1, sizeof(char));
      strcpy((socketBin->sockets + socketBin->numSockets)->handle, handle);
   }
   (socketBin->sockets + socketBin->numSockets)->socketNum = socketNum;
   (socketBin->numSockets)++;
   //send good handle message
}

Socket *listenSockets(fd_set *readFds,  int max, SocketBin *socketBin) {
   int selReturn, i;
   selReturn = select(max + 1, readFds, NULL, NULL, NULL);
   if (selReturn < 0 && (errno != EINTR)) {
      printf("select error");
   }
   else{
      for (i = 0; i < socketBin->numSockets; i++) {
         if (FD_ISSET((socketBin->sockets + i)->socketNum, readFds)) {
            return socketBin->sockets + i; 
         } 
      }
   }
   return NULL;
   
}

char *genPacket(char *message, uint8_t flag, int length, int clear) {
   char *packet = calloc(length, sizeof(char));
   
   ((Header *)packet)->flag = flag;
   ((Header *)packet)->length = htons(length);
   if (message) {
      memcpy(packet + SIZE_HEADER, message, length - SIZE_HEADER);
      if (clear) {
         free(message);
      }
   }
   return packet;
}




char *genMessage(uint8_t flag, int length, char *srcHandle, char *buff, int more) {
   char *message;
   static char *dstHandle;
   uint8_t srcLen;
   static uint8_t dstLen;
   int i = 0, start, totalLen = 0, textLen = 0;
   
   if (flag == 5) {
      start = 0;
      if (dstHandle == NULL) {
         while ((i < length) && (buff[i] != ' ') && (i < MAX_USER_INPUT)) {
            i++;
         }  
         dstHandle = calloc(i, sizeof(char));
         dstLen = i;
         i++;
         start = i;
         memcpy(dstHandle, buff, dstLen);
      }
      totalLen += dstLen + 1;
      totalLen += strlen(srcHandle) + 1;
      while ((i < length) && ((i - start)  < MAX_TEXT_SIZE)) {
         i++;
      }
      totalLen += (i - start + 1) ;
      message = calloc(totalLen, sizeof(char));
      *message = dstLen; //Set Destination Handle Length
      memcpy(message + 1, dstHandle, dstLen); //Set Destination handle
      *(message + 1 + dstLen) = strlen(srcHandle); //Set Src Handle Length
      memcpy(message + 2 + dstLen, srcHandle, strlen(srcHandle)); //Set SrcHandle
      memcpy(message + 2 + dstLen + strlen(srcHandle), buff + start, i - start);
      message[totalLen - 1] = 0;
      if (more == 0) {
         free(dstHandle);
         dstHandle = NULL;
         dstLen = 0;
      }
   }
   else if (flag == 4) { //Broadcast
      totalLen = strlen(srcHandle) + 1;
      totalLen += length + 1;
      textLen = length;
      message = calloc(totalLen, sizeof(char));
      *message = strlen(srcHandle);
      memcpy(message + 1, srcHandle, strlen(srcHandle));
      memcpy(message + 1 + strlen(srcHandle), buff, textLen);
      message[totalLen - 1] = 0;
   }
   else if (flag == 10) { //Request list of handles
      message = NULL;
   }
   else if (flag == 8) {   //Exit request
      message = NULL;
   }
   return message;
}

void removeSocket(SocketBin *socketBin, int clientSocket) {
   int i;
   Socket *temp;
   for (i = 0; i < socketBin->numSockets; i++) {
      if((socketBin->sockets + i)->socketNum == clientSocket) {
         temp = (socketBin->sockets + i);
         if (temp->handle) {
            free(temp->handle);
            temp->handle = NULL;
         }
         close(clientSocket);
         if (i != (socketBin->numSockets - 1)) {
            memmove((socketBin->sockets + i), (socketBin->sockets + i + 1), (socketBin->numSockets - i - 1) * sizeof(Socket));
         }
         socketBin->numSockets--;
         break;
      } 
   }
}

int sendPacket(char *packet, int socketNum, int freeFlag) {
   int sent;
   uint16_t length = ntohs(((Header *)packet)->length);
   if (length == 0) {
      length = freeFlag;
   }
   sent = send(socketNum, packet, length, 0);
   if (freeFlag) {
      free(packet);
   }
   return (sent - length); 
}


char *readDataFromSocket(int socket_num, int numBytesLen) {
   typedef enum {START, PROCESS_LENGTH, PROCESS_DATA} State;
   static State state = START;
   static uint16_t length = 0;
   static uint8_t handleLen;
   static char *buffer;
   static int bytesRead;
   static int getHandles;
   if (state == START) {
      buffer = NULL;
      length = 0;
      handleLen = 0;
      bytesRead = recv(socket_num, &length, numBytesLen, 0);
      if (bytesRead < numBytesLen) {
         state = PROCESS_LENGTH;   
      }
      else {
         if (numBytesLen == 2) {
            length = ntohs(length);
         }
         else {
            handleLen = (uint8_t)length;
            length = handleLen;
         }
         if (length != 0) {
            if (handleLen != 0) {
               length++;
            }
            buffer = calloc(length, sizeof(char));
            memcpy(buffer, &length, 2);
            getHandles = 0;
         }
         else {
            buffer = calloc(SIZE_HEADER, sizeof(char));
            getHandles = 1;  
         }
         state = PROCESS_DATA;
      }
   }
   else if (state == PROCESS_LENGTH) {
      bytesRead += recv(socket_num, &length + bytesRead, LENGTH_BYTES - bytesRead, 0);
      if (bytesRead <= 0) {
         length = 7;
         buffer = calloc(length, sizeof(char));
         *((uint16_t *)buffer) = (uint16_t)length;
         *(buffer + sizeof(uint16_t)) = 0;
         strcpy(buffer + SIZE_HEADER, "END");
         return buffer;   
      }
      if (bytesRead < numBytesLen) {
         state = PROCESS_LENGTH;   
      }
      else {
         if (numBytesLen == 2) {
            length = ntohs(length);
         }
         else {
            handleLen = (uint8_t)length;
            length = handleLen;
         }
         if (length != 0) {
            if (handleLen != 0) {
               length++;
            }
            buffer = calloc(length, sizeof(char));
            memcpy(buffer, &length, 2);
            getHandles = 0;
         }
         else {
            buffer = calloc(SIZE_HEADER, sizeof(char));
            getHandles = 1;
         }
         state = PROCESS_DATA;
      }
   }
   else if (state == PROCESS_DATA) {
      if (getHandles) {
         bytesRead += recv(socket_num, buffer + bytesRead, SIZE_HEADER - bytesRead, 0);
      }
      else {
         bytesRead += recv(socket_num, buffer + bytesRead, length - bytesRead, 0);
      }
      if ((getHandles && bytesRead < SIZE_HEADER) || ((bytesRead < length) && (!getHandles))) {
         state = PROCESS_DATA;
      }
      else {
         state = START;
         bytesRead = 0;
         getHandles = 0;
         length = 0;
         handleLen = 0;
         return buffer;
      }
   }
   return NULL;
   
}
