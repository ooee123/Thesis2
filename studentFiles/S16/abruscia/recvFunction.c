// Add to makefile**** and library to the client and server files

#include "tcp.h"

// Function for client to recv one packet 
// Pass buf malloced as NORMAL_HEAD_LEN
// Returns length of buffer in host order
uint16_t recvFunc(int socketNum, char *buf) {
   int bytesReceived = 0;
   int index = 0;
   uint16_t mesgLen = 0;
   
   // put header into buffer
   while (index < NORMAL_HEAD_LEN) {
      bytesReceived = recv(socketNum, buf + index, NORMAL_HEAD_LEN - 
       bytesReceived, 0); //get 3 bytes of header
      index += bytesReceived;

      if (bytesReceived < 0) {
         perror("Error in bytes Received");
         exit(-1);
      }

      if (bytesReceived == 0) {
         printf("Server Terminated\n");
         close(socketNum);
         exit(0);
      }
   }

   memcpy(&mesgLen, buf, FLAG_OFFSET);
   mesgLen = ntohs(mesgLen);

   buf = realloc(buf, mesgLen);
   
   // Retrieve message len worth
   while (index < mesgLen) {
      bytesReceived = recv(socketNum, buf + index, mesgLen - bytesReceived, 0); //get 3 bytes of header
      index += bytesReceived;

      if (bytesReceived < 0) {
         perror("Error in bytes Received");
         exit(-1);
      }

      if (bytesReceived == 0) {
         printf("Server Terminated\n");
         close(socketNum);
         exit(0);
      }
   }


   return mesgLen;  
}
