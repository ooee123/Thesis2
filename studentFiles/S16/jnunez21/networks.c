#include <stdlib.h>

#pragma pack(1)

typedef struct normalHeader {
   unsigned short pktLen;
   unsigned char flg;
} normalHeader;

/* Encapsulates send() and error checking to shorten code
 * in caller function */
int safeSend(int socketNum, char *buf, int bytesToSend) {
   int rtn = 0;
   int currentBytes = 0;

   while (rtn < bytesToSend) {
      currentBytes = send(socketNum, buf, bytesToSend - currentBytes, 0);
      if (currentBytes < 0) {
         perror("send call error");
         exit(EXIT_FAILURE);
      }
      rtn += currentBytes;
   }

   return rtn;
}

/* Encapsulates recv() and error checking to shorten code
 * in caller function.
 *
 * Returns 0 on success (received as many bytes as desired/expected,
 * returns -1 when the length of the data received is 0 (which occurs
 * in the event of the client quitting unexpectedly) */
int safeRecv(int socketNum, char *buf, int bytesToReceive) {
   int rtn = 0;
   int totalBytesReceived = 0;
   int currentBytes = 0;
   
   while (totalBytesReceived < bytesToReceive) {
      if ((currentBytes = (recv(socketNum, buf, bytesToReceive - currentBytes,
       0))) < 0) {
         perror("recv call error");
         exit(EXIT_FAILURE);
      }
      if (!currentBytes && !totalBytesReceived) {
         rtn = -1;
         break;   
      }
      totalBytesReceived += currentBytes;
   }

   return rtn;
}
