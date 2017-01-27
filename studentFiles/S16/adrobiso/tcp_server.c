/******************************************************************************
* tcp_server.c
*
* CPE 464 - Program 1
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "chat.h"

static struct handleFd *clientHandles;
static int numHandles, activeHandles;
static char buff[BUFFER_SIZE];
static struct packetHeader *pktHead;

void close_client(struct handleFd *client) {
   close(client->fd);
   client->fd = 0;
   client->handle[0] = '\0';
}

int recieve_packet(struct handleFd *client) {
   int message_len= 0;     //length of the received message
   int remaininglen;
   char *buffloc;

   buffloc = buff;
   if ((message_len = recv(client->fd, buffloc, 2, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   buffloc += message_len;
   if (message_len == 0) {
      printf("\nclient %s exited without notification\n", client->handle);
      close_client(client);
      activeHandles--;
      return 0;
   }
   else if (message_len == 1) {
      if ((message_len = recv(client->fd, buffloc, 1, 0)) < 0) {
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
      if ((message_len = recv(client->fd, buffloc, remaininglen, 0)) < 0) {
         perror("recv call");
         exit(-1);
      }
      buffloc += message_len;
      remaininglen -= message_len;
   }
   
   return 1;
}

void send_packet(int socket_num, int length) {
   int sent = 0;            //actual amount of data sent

   sent = send(socket_num, buff, length, 0);
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

void send_ack(int client_socket) {
   pktHead->length = htons(sizeof(struct packetHeader));
   pktHead->flag = 9;
   send_packet(client_socket, ntohs(pktHead->length));
}

void send_handle_list(int client_socket) {
   int i;

   // Send number of active handles
   pktHead->length = sizeof(struct packetHeader);
   pktHead->flag = 11;
   memcpy(buff + pktHead->length, &activeHandles, sizeof(int));
   pktHead->length += sizeof(int);
   pktHead->length = htons(pktHead->length);
   send_packet(client_socket, ntohs(pktHead->length));

   // Send flag 12
   pktHead->length = 0;
   pktHead->flag = 12;
   send_packet(client_socket, sizeof(struct packetHeader));

   // Send handles one at a time with noe header
   for (i = 0; i < numHandles; i++) {
      if (clientHandles[i].fd > 0) {
         *buff = strlen(clientHandles[i].handle);
         memcpy(buff + 1, clientHandles[i].handle, *buff);
         send_packet(client_socket, *buff + 1);
      }
   }
}

void good_handle(int client_socket) {
   pktHead->length = htons(sizeof(struct packetHeader));
   pktHead->flag = 2;
   send_packet(client_socket, ntohs(pktHead->length));
}

void handle_exists(int client_socket) {
   pktHead->length = htons(sizeof(struct packetHeader));
   pktHead->flag = 3;
   send_packet(client_socket, ntohs(pktHead->length));
}

void dst_no_exist(int client_socket) {
   pktHead->length = htons(sizeof(struct packetHeader));
   pktHead->flag = 7;
   send_packet(client_socket, ntohs(pktHead->length));
}

void broadcast_msg(struct handleFd *client) {
   int i;

   for (i = 0; i < numHandles; i++) {
      if (clientHandles[i].fd > 0 && strcmp(clientHandles[i].handle, client->handle)) {
         send_packet(clientHandles[i].fd, ntohs(pktHead->length));
      }
   }
}

void send_msg(struct handleFd *client) {
   char *dsthandlelen, *dsthandle;
   char dst[HANDLE_LEN + 1];
   int i;

   dsthandlelen = buff + sizeof(struct packetHeader);
   dsthandle = dsthandlelen + 1;

   memcpy(dst, dsthandle, *dsthandlelen);
   *(dst + *dsthandlelen) = '\0';

   for (i = 0; i < numHandles && strcmp(clientHandles[i].handle, dst); i++)
   ;

   if (i < numHandles) {
      send_packet(clientHandles[i].fd, ntohs(pktHead->length));
   }
   else {
      dst_no_exist(client->fd);
   }
}

void new_handle_request(struct handleFd *client) {
   char src[HANDLE_LEN + 1];
   char *srchandlelen, *srchandle;
   int i;
   
   srchandlelen = buff + sizeof(struct packetHeader);
   srchandle = srchandlelen + 1;
   memcpy(src, srchandle, *srchandlelen);
   *(src + *srchandlelen) = '\0';

   for (i = 0; i < numHandles && strcmp(clientHandles[i].handle, src); i++)
      ;
   if (i == numHandles) {
      strcpy(client->handle, src);
      good_handle(client->fd);
   }
   else {
      handle_exists(client->fd);
      close_client(client);
   }
}

void handle_client(struct handleFd *client) {
   if (recieve_packet(client) > 0) {
      switch (pktHead->flag) {
      case 1:
         new_handle_request(client);
         break;

      case 4:
         broadcast_msg(client);
         break;

      case 5:
         send_msg(client);
         break;

      case 8:
         send_ack(client->fd);
         close_client(client);
         activeHandles--;
         break;

      case 10:
         send_handle_list(client->fd);
         break;

      default:
         printf("\nunknown flag\n");
         break;
      }
   }
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */
int tcp_recv_setup()
{
   int server_socket= 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */

   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0)
   {
      perror("socket call");
      exit(1);
   }

   local.sin_family= AF_INET;         //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= htons(0);                 //let system choose the port

   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
   {
      perror("bind call");
      exit(-1);
   }
   
   //get the port name and print it out
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0)
   {
      perror("getsockname call");
      exit(-1);
   }

   printf("Server is using port %d \n", ntohs(local.sin_port));

   if (listen(server_socket, 5) < 0) {
      perror("listen call");
      exit(-1);
   }
   
   return server_socket;
}

int reset_fdvars(fd_set *fdvar, int server_socket) {
   int maxfd, i;
   
   FD_ZERO(fdvar);
   FD_SET(server_socket, fdvar);
   maxfd = server_socket;  
   
   for (i = 0; i < numHandles; i++) {
      if (clientHandles[i].fd > 0)
      FD_SET(clientHandles[i].fd, fdvar);

      if (clientHandles[i].fd > maxfd)
      maxfd = clientHandles[i].fd;
   }
   
   return maxfd;
}

void new_client(int server_socket, int *maxHandles) {
   int newSocket, i;
   
   if ((newSocket = accept(server_socket, (struct sockaddr *)0, (socklen_t*)0))<0)
   {
      perror("accept");
      exit(EXIT_FAILURE);
   }

   for (i = 0; i < numHandles && clientHandles[i].fd != 0; i++)
      ;
   if (i == numHandles)
   numHandles++;
   if (numHandles > *maxHandles) {
      *maxHandles *= 2;
      clientHandles = realloc(clientHandles, *maxHandles * sizeof(struct handleFd));
      memset(clientHandles + numHandles, 0, (*maxHandles - numHandles) * sizeof(struct handleFd));
   }
   clientHandles[i].fd = newSocket;
   activeHandles++;
}

void select_fd (int server_socket, int *maxHandles) {
   static struct timeval timeout;   // select call 
   static fd_set fdvar;
   int maxfd, i;
   
   timeout.tv_sec = 1;  // set timeout to 1 second
   timeout.tv_usec = 0; // set timeout (in micro-second)
   
   maxfd = reset_fdvars(&fdvar, server_socket);

   select(maxfd + 1,(fd_set *) &fdvar, (fd_set *) 0, 
      (fd_set *) 0, &timeout);

   if (FD_ISSET(server_socket, &fdvar)) {
      new_client(server_socket, maxHandles);
   }

   for (i = 0; i < numHandles; i++) {
      if (FD_ISSET(clientHandles[i].fd, &fdvar)) {
         handle_client(&clientHandles[i]);
      }
   }
}

int main(int argc, char *argv[])
{
   int server_socket= 0;   //socket descriptor for the server socket
   int maxHandles;

   pktHead = (struct packetHeader*)buff;

   //create the server socket
   server_socket = tcp_recv_setup();

   //look for a client to serve
   maxHandles = 10;
   clientHandles = calloc(maxHandles, sizeof(struct handleFd));
   numHandles = 0;
   activeHandles = 0;

   while (1) {
      select_fd(server_socket, &maxHandles);
   }
      
   return EXIT_SUCCESS;
}
