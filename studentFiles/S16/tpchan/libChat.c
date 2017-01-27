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

#include "libChat.h"
#include "testing.h"

int recv_e(int socket, void *buffer, size_t length, int flags)
{
   int numBytes;
   if ((numBytes = recv(socket, buffer, length, flags)) < 0)
   {
      perror("recv call");
      exit(-1);
   }
   return numBytes;
}

void send_e(int socket, const void *buf, size_t len, int flags)
{
   if (send(socket, buf, len, flags) < 0)
   {
      perror("send call");
      exit(-1);
   }
}
char *createHeader(uint16_t length, uint8_t flag)
{
   char *buf = calloc(sizeof(char), HEADER_SIZE);
   
   length = htons(length);
   
   memcpy(buf, &length, 2);
   memcpy(buf + 2, &flag, 1);
   return buf;
}

char *getPacket(int socketNum, int packLength)
{
   char *buf = calloc(sizeof(char), packLength);
   int numBytes;
   int temp;
   
   numBytes = recv_e(socketNum, buf, packLength, 0);
   if (numBytes == 0) {
      free(buf);
      return NULL;
   }
   
   while (numBytes < packLength)
   {
      temp = recv_e(socketNum, buf + numBytes, packLength - numBytes, 0);
      if (temp == 0) {
         free(buf);
         return NULL;
      }
      numBytes += temp;
   }
   return buf;
}

struct myHeader *tcp_recieve_header(int socketNum)
{
   struct myHeader *myHead = malloc(sizeof(myHead));
   uint16_t temp;
   char *buf = getPacket(socketNum, HEADER_SIZE);
   
   if (!buf)
      return NULL;
   
   memcpy(&temp, buf, 2);
   memcpy(&myHead->flag, buf + 2, 1);
   myHead->length = ntohs(temp);
   free(buf);
   return myHead;
}

