/******************************************************************************
* tcp_client.c
*
*****************************************************************************/

#include <ctype.h>
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
#include "testing.h"
#include "chat.h"

static int server_socket;
static char buff[BUFFER_SIZE];
static struct packetHeader *pktHead;

void invalid_cmd () {
   fprintf(stderr, "Invalid command\n");
}

void send_packet() {
   int sent = 0;            //actual amount of data sent

   pktHead->length = htons(pktHead->length);

   sent = send(server_socket, buff, ntohs(pktHead->length), 0);
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

void recieve_packet() {
   int message_len= 0;     //length of the received message
   int remaininglen;
   char *buffloc;

   buffloc = buff;
   if ((message_len = recv(server_socket, buffloc, 2, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   buffloc += message_len;
   if (message_len == 0) {
      fprintf(stderr, "\nServer Terminated\n");
      exit(EXIT_FAILURE);
   }
   else if (message_len == 1) {
      if ((message_len = recv(server_socket, buffloc, 1, 0)) < 0) {
         perror("recv call");
         exit(-1);
      }
      buffloc += message_len;
   }

   if (pktHead->length == 0)
   remaininglen = 1;
   else
   remaininglen = ntohs(pktHead->length) - 2;

   while (remaininglen > 0) {
      if ((message_len = recv(server_socket, buffloc, remaininglen, 0)) < 0) {
         perror("recv call");
         exit(-1);
      }
      buffloc += message_len;
      remaininglen -= message_len;
   }
}

void send_text () {
   int charcount;
   
   charcount = 0;
   while((*(buff + pktHead->length) = getchar()) != '\n') {
      charcount++;
      pktHead->length++;
      if (charcount == MSG_LEN - 1) {
         *(buff + pktHead->length++) = '\0';
         send_packet();
         pktHead->length = ntohs(pktHead->length);
         
         pktHead->length -= MSG_LEN;
         charcount = 1;
         *(buff + pktHead->length++) = ' ';
      }
   }
   *(buff + pktHead->length++) = '\0';
   send_packet();
}

void send_broadcast (char *srchandle) {
   pktHead->length = sizeof(struct packetHeader);
   pktHead->flag = 4;
   
   *(buff + pktHead->length) = strlen(srchandle);
   pktHead->length++;
   strcpy(buff + pktHead->length, srchandle);
   pktHead->length += strlen(srchandle);
   
   send_text();
}

void send_message(char *srchandle) {
   char desthandle[HANDLE_LEN + 1];
   
   if (scanf(" %s", desthandle) > 0) {
      pktHead->length = sizeof(struct packetHeader);
      pktHead->flag = 5;
      
      *(buff + pktHead->length) = strlen(desthandle);
      pktHead->length++;
      strcpy(buff + pktHead->length, desthandle);
      pktHead->length += strlen(desthandle);
      
      *(buff + pktHead->length) = strlen(srchandle);
      pktHead->length++;
      strcpy(buff + pktHead->length, srchandle);
      pktHead->length += strlen(srchandle);
      
      send_text();
   }
   else {
      invalid_cmd();
   }
}

void recieve_handle() {
   char handlelen;
   char handle[HANDLE_LEN + 1];
   int message_len= 0;     //length of the received message

   if ((message_len = recv(server_socket, buff, 1, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   memcpy(&handlelen, buff, 1);
   if ((message_len = recv(server_socket, buff, handlelen, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   memcpy(handle, buff, handlelen);
   *(handle + handlelen) = '\0';
   printf("   %s\n", handle);
}

void broadcast_recieved() {
   char src[HANDLE_LEN + 1];
   char *srchandlelen, *srchandle, *msg;
   
   srchandlelen = buff + sizeof(struct packetHeader);
   srchandle = srchandlelen + 1;
   msg = srchandle + *srchandlelen;

   memcpy(src, srchandle, *srchandlelen);
   *(src + *srchandlelen) = '\0';

   printf("\n%s:%s\n", src, msg);
}

void message_recieved() {
   char src[HANDLE_LEN + 1];
   char *dsthandlelen, *srchandlelen, *dsthandle, *srchandle, *msg;
   
   dsthandlelen = buff + sizeof(struct packetHeader);
   dsthandle = dsthandlelen + 1;
   srchandlelen = dsthandle + *dsthandlelen;
   srchandle = srchandlelen + 1;
   msg = srchandle + *srchandlelen;

   memcpy(src, srchandle, *srchandlelen);
   *(src + *srchandlelen) = '\0';

   printf("\n%s:%s\n", src, msg);
}

void bad_handle () {
   char dst[HANDLE_LEN + 1];
   char *dsthandlelen, *dsthandle;
   
   dsthandlelen = buff + sizeof(struct packetHeader);
   dsthandle = dsthandlelen + 1;

   memcpy(dst, dsthandle, *dsthandlelen);
   *(dst + *dsthandlelen) = '\0';

   fprintf(stderr, "\nClient with handle %s does not exist.\n", dst);
}

void handle_user_input(char *srchandle) {
   char cmd[3];

   scanf("%s", cmd);

   if (cmd[0] != '%') {
      invalid_cmd();
   }
   else {
      switch (toupper(cmd[1])) {
      case 'M':
         send_message(srchandle);
         break;

      case 'B':
         send_broadcast(srchandle);
         break;

      case 'L':
         pktHead->length = sizeof(struct packetHeader);
         pktHead->flag = 10;
         send_packet();
         break;

      case 'E':
         pktHead->length = sizeof(struct packetHeader);
         pktHead->flag = 8;
         send_packet();
         break;

      default:
         invalid_cmd();
         break;
      }
   }
}

void handle_packet() {
   static int numHandles;
   int i;

   recieve_packet();

   switch (pktHead->flag) {
   case 4:
      broadcast_recieved();
      break;

   case 5:
      message_recieved();
      break;

   case 7:
      bad_handle();
      break;

   case 9:
      printf("\n");
      close(server_socket);
      exit(EXIT_SUCCESS);
      break;

   case 11:
      memcpy(&numHandles, buff + sizeof(struct packetHeader), sizeof(int));
      printf("\nNumber of clients: %d\n", numHandles);
      break;

   case 12:
      printf("\n");
      for (i = 0; i < numHandles; i++) {
         recieve_handle();
      }
      break;
   }
}

void check_handle_valid(char *handle) {
   recieve_packet();

   if (pktHead->flag == 2);
   else if(pktHead->flag == 3) {
      fprintf(stderr, "Handle already in use: %s\n", handle);
      exit(EXIT_FAILURE);
   }
   else {
      fprintf(stderr, "could not confirm handle\n");
      exit(EXIT_FAILURE);
   }
}

void request_handle(char *handle) {
   pktHead->length = sizeof(struct packetHeader);
   pktHead->flag = 1;
   *(buff + pktHead->length) = strlen(handle);
   pktHead->length++;
   memcpy(buff + pktHead->length, handle, strlen(handle));
   pktHead->length += strlen(handle);

   send_packet();
   check_handle_valid(handle);
}

int tcp_send_setup(char *host_name, char *port)
{
   int socket_num;
   struct sockaddr_in remote;       // socket address for remote side
   struct hostent *hp;              // address of remote host

   // create the socket
   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket call");
      exit(-1);
   }
   
   // designate the addressing family
   remote.sin_family = AF_INET;

   // get the address of the remote host and store
   if ((hp = gethostbyname(host_name)) == NULL)
   {
      fprintf(stderr, "Error getting hostname: %s\n", host_name);
      exit(-1);
   }
   
   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   // get the port used on the remote side and store
   remote.sin_port= htons(atoi(port));

   if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
   {
      perror("connect call");
      exit(-1);
   }

   return socket_num;
}

void run (char *srchandle) {
   static struct timeval timeout;
   static fd_set fdvar;
   
   timeout.tv_sec = 1;  // set timeout to 1 second
   timeout.tv_usec = 0; // set timeout (in micro-second)
   
   FD_ZERO(&fdvar);
   FD_SET(server_socket, &fdvar);
   FD_SET(0, &fdvar);

   select(server_socket + 1,(fd_set *) &fdvar, (fd_set *) 0,
   (fd_set *) 0, &timeout);

   if (FD_ISSET(server_socket, &fdvar)) {
      handle_packet(server_socket);
      printf("$: ");
      fflush(stdout);
   }

   if (FD_ISSET(0, &fdvar)) {
      handle_user_input(srchandle);
      printf("$: ");
      fflush(stdout);
   }
}

int main(int argc, char * argv[])
{
   /* check command line arguments  */
   if(argc != 4)
   {
      fprintf(stderr, "usage: %s handle host-name port-number \n", argv[0]);
      exit(1);
   }

   pktHead = (struct packetHeader*)buff;

   /* set up the socket for TCP transmission  */
   server_socket = tcp_send_setup(argv[2], argv[3]);
   request_handle(argv[1]);

   printf("$: ");
   fflush(stdout);
   
   while (1) {
      run(argv[1]);
   }

   return 0;
}
