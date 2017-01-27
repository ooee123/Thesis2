 /****************************************************************************
 * networks.c
 *
 * CPE 464 - Program 1
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include "networks.h"

int formatMessage(char* src, char* dest, char* message, uint16_t length, int flag, char* buffer) {
   uint16_t slen;
   uint16_t index = 3;
   uint8_t handleLen;
   char* backupBuf = malloc(length);
   memcpy(backupBuf, message, length);
   message = backupBuf;
   buffer[FLAG_IDX] = flag;
   if(flag == BROADCAST) {
      handleLen = strlen(src);
      buffer[index++] = handleLen;
      memcpy(buffer + index, src, handleLen);
      index += handleLen;
      memcpy(buffer + index, message, length);
      index += length;
   }
   else if (flag == MESSAGE) {
      handleLen = strlen(dest);
      buffer[index++] = handleLen;
      memcpy(buffer + index, dest, handleLen);
      index += handleLen;
      handleLen = strlen(src);
      buffer[index++] = handleLen;
      memcpy(buffer + index, src, handleLen);
      index += handleLen;
      memcpy(buffer + index, backupBuf, length);
      index += length;
   }
   slen = htons(index);
   memcpy(buffer, &slen, 2);
   free(backupBuf);
   return index;
}

void printHandles(int socket, int handles) {
   uint8_t handleLength;
   char handle[MAX_NAME_SIZE];
   while (handles-- > 0) {
      printf("\33[2K\r");
      recv(socket, &handleLength, 1, 0);
      recv(socket, handle, handleLength, 0);
      printf("\t%.*s\n", handleLength, handle);
   }
}

int receiveMessage(int socket, char* src, char* dest, char* message, int* flag,
      int handles) { 
   uint16_t length = 0;
   uint8_t handleLength, handleLength2;
   int messageLen;
   recv(socket, &length, 2, 0);
   length = ntohs(length);
   recv(socket, flag, 1, 0);
   switch (*flag) {
      case JOIN:
         recv(socket, &handleLength, 1, 0);
         recv(socket, src, handleLength, 0);
         src[handleLength] = '\0';
         break;
      case BROADCAST:
         recv(socket, &handleLength, 1, 0);
         recv(socket, src, handleLength, 0);
         src[handleLength] = '\0';
         messageLen = length - 4 - handleLength;
         if (messageLen > 0) {
            recv(socket, message, messageLen, 0);
         }
         message[messageLen] = '\0';
         break;
      case MESSAGE:
         recv(socket, &handleLength, 1, 0);
         recv(socket, dest, handleLength, 0);
         dest[handleLength] = '\0';
         recv(socket, &handleLength2, 1, 0);
         recv(socket, src, handleLength2, 0);
         src[handleLength2] = '\0';
         messageLen = length - 5 - handleLength - handleLength2;
         if (messageLen > 0) {
            recv(socket, message, length - 5 - handleLength - handleLength2, 0);
         }
         message[messageLen] = '\0';
         break;
      case QUIT:
         break;
      case NUM_HANDLES:
         recv(socket, message, 4, 0);
         break;
      case HANDLES:
         printHandles(socket, handles);
         break;
   }
   return length;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup(int given) {
   int server_socket = socket(AF_INET, SOCK_STREAM, 0);
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len = sizeof(local);  /* length of local address        */

   /* create the socket  */
   if(server_socket < 0) {
      perror("socket call");
      exit(1);
   }

   local.sin_family = AF_INET;         //internet family
   local.sin_addr.s_addr = INADDR_ANY; //wild card machine address
   local.sin_port = htons(given);                 //let system choose the port

   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
      exit(-1);
   }
    
   //get the port name and print it out
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
      perror("getsockname call");
      exit(-1);
   }

   printf("socket has port %d \n", ntohs(local.sin_port));
           
   return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

int tcp_listen(int server_socket, int back_log) {
   int client_socket;
   if (listen(server_socket, back_log) < 0) {
      perror("listen call");
      exit(-1);
   }
  
   if ((client_socket = accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
      perror("accept call");
      exit(-1);
   }

   return client_socket;
}

int tcp_send_setup(char *host_name, char *port)
{
    int socket_num;
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host

    // create the socket
    if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
       perror("socket call");
       exit(-1);
    }
    

    // designate the addressing family
    remote.sin_family= AF_INET;

    // get the address of the remote host and store
    if ((hp = gethostbyname(host_name)) == NULL) {
       printf("Error getting hostname: %s\n", host_name);
       exit(-1);
    }
    
    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

    // get the port used on the remote side and store
    remote.sin_port= htons(atoi(port));

    if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0) {
       perror("connect call");
       exit(-1);
    }

    return socket_num;
}
