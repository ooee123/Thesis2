
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>
#include <string.h>
#include "testing.h"
#include "packet.h"

int safeSend(int socket, const void* buffer, size_t length, int flags) {
   int sent;
   if((sent = send(socket, buffer, length, flags)) < 0) {
      perror("Send failed");
      exit(1);
   }  
   return sent;
} 

int safeRecv(int socket, void* buffer, size_t length, int flags) {
   int recvd;
   if((recvd = recv(socket, buffer, length, flags)) < 0) {
      perror("Recieve failed");
      exit(1);
   } 
   return recvd;
} 

// creates and sends just a header packet with the given flag
int sendHeader(int socket, int flag) {
   struct header head = {htons(HEADER_LEN), flag};
   return safeSend(socket, &head, HEADER_LEN, 0);
} 

// receives just a 3 byte header packet, returns just the flag
int recvHeader(int socket) {
   struct header* head = NULL;
   while((head = (struct header*)receive_packet(socket)) == NULL);
   return head->flags;
} 

uint8_t* receive_packet(int socket) {
   
   // States of receiving a packet
   enum {
      START,
      PROCESS_HEADER,
      GET_DATA
   };
   
   // Tracks all of the data of the packet as it is created 
   struct partial_packet {
      int who;       // who this partial packet belongs to
      int haveLen;   // how much data (bytes) we have so far
      int packetLen; // how much data (bytes) we are looking for
      uint8_t* data; // the raw data we've received so far
   };

   static struct partial_packet partial;
   static int state = START;

   switch(state) {
      case START: {
         // setup the partial packet, assigning it to the socket
         partial.who = socket;
         partial.haveLen = 0;
         partial.packetLen = HEADER_LEN; 
         partial.data = malloc(sizeof(uint8_t) * HEADER_LEN); 
         
         // attempt to recieve two bytes
         int recvLen = safeRecv(socket, partial.data, HEADER_LEN, 0);
         
         // socket closed
         if(recvLen <= 0) {
            free(partial.data);
            return EMPTY_HEADER;
         } 

         partial.haveLen += recvLen;

         if(partial.haveLen == partial.packetLen) {
            int totalLen = (ntohs(((struct header*)partial.data)->len));

            if(partial.haveLen == totalLen) {
               state = START;
               return partial.data;
            } 

            partial.packetLen = totalLen;
            partial.data = realloc(partial.data, totalLen);
            

            state = GET_DATA; 
         } 
         else {
            state = PROCESS_HEADER;
         } 
         
         return NULL;
      }

      case PROCESS_HEADER: {
         // don't corrupt our current partial packet with another socket's data
         if(partial.who != socket)
            return NULL;

         uint8_t* data_dst = partial.data + partial.haveLen;
         int wanted = partial.packetLen - partial.haveLen;
         int recvLen = safeRecv(socket, data_dst, wanted, 0);
 
         // socket closed
         if(recvLen <= 0) {
            free(partial.data);
            state = START;
            return EMPTY_HEADER;
         }         
         
         partial.haveLen += recvLen;

         if(partial.haveLen == partial.packetLen) {
            // update the partial packet
            int totalLen = (ntohs(((struct header*)partial.data)->len));
            if(partial.haveLen == totalLen) {
               state = START;
               return partial.data;
            } 

            partial.packetLen = totalLen;
            partial.data = realloc(partial.data, totalLen);
            state = GET_DATA; 
         } 
         
         return NULL;
      }

      case GET_DATA: {
         // don't corrupt our current partial packet with another socket's data
         if(partial.who != socket)
            return NULL;
         
         uint8_t* data_dst = partial.data + partial.haveLen;
         int wanted = partial.packetLen - partial.haveLen;

         int recvLen = safeRecv(socket, data_dst, wanted, 0);
         
         // socket closed
         if(recvLen <= 0) {
            free(partial.data);
            state = START;
            return EMPTY_HEADER;
         }         
         
         partial.haveLen += recvLen;
         if(partial.haveLen == partial.packetLen) {
            state = START;
            return partial.data;
         } 
         
         return NULL;
      }

      default:
         printf("ERROR: In an invalid state!\n");
         return NULL;
   } 
} 

// creates a new packet with the given flag
int createPacket(int flag, uint8_t** packet) {
   *packet = malloc(sizeof(struct header));
   struct header* head = (struct header*) *packet;
   head->len = htons(HEADER_LEN);
   head->flags = flag;
   return HEADER_LEN;
} 

// adds the given address to the packet
int addAddress(struct unpacked_address* addr, uint8_t** packet) {
   // truncate address that are too long
   if(addr->len > MAX_HANDLE_LEN) {
      printf("\n\nTruncating address\n\n");
      addr->len = MAX_HANDLE_LEN;
   }

   struct header* head = (struct header*) *packet;
   uint16_t current_len = ntohs(head->len);
   uint16_t new_len = current_len + addr->len + 1;
   // expand the packet for the new data
   *packet = realloc(*packet, new_len);
   // add the new data
   uint8_t* dst = *packet + current_len;
   *dst = addr->len;
   memcpy(dst+1, addr->handle, addr->len);
   // update the packet len
   head->len = htons(new_len);
   return new_len;
} 

// adds the given message to the packet
int addMessage(char* message, uint8_t** packet) {
   // truncate messages that are too long (split packets else where)
   if(strlen(message) > MAX_MSG_CHAR_LEN)
      message[MAX_MSG_CHAR_LEN] = '\0';

   struct header* head = (struct header*) *packet;
   uint16_t current_len = ntohs(head->len);
   uint16_t new_len = current_len + strlen(message) + 1;
   //expand the packet for the new data
   *packet = realloc(*packet, new_len);
   // add the new data
   uint8_t* dst = *packet + current_len;
   memcpy(dst, message, strlen(message) + 1);
   // update the packet len
   head->len = htons(new_len);
   return new_len;
} 
