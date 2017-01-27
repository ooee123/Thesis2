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

struct handle {
   int socket;
   char name[256];
};

struct handle *handle_list = NULL;
int handle_list_size = 0;
int num_handles = 0;
int max_handles = 0;

void forward_message(char *buf, int size, int socket) {

   char *handle, *send_buf;
   unsigned char handle_len = 0;
   int target_socket;

   size++;//whole message with flag back in
   handle_len = buf[0];
   handle = calloc(handle_len + 1, 1);
   strncpy(handle, buf + 1, handle_len);
   handle[handle_len] = '\0';
   //printf("%s: %s\n", handle, buf + 1 + handle_len);//needs bash prompt?

   if ((target_socket = get_socket(handle)) != -1) {
      buf--;//go back so pointing to flag

      send_buf = calloc(size + 2, 1);
      memcpy(send_buf + 2, buf, size);
      *(short *)send_buf = htons(size + 2);

      if(send(target_socket, send_buf, size + 2, 0) < 0)
      {
         free(send_buf);
         perror("send call");
         exit(-1);
      }
      
   }
   else {
      send_buf = calloc(4 + handle_len, 1);
      *(short *)send_buf = htons(4 + handle_len);
      *(char *)(send_buf + sizeof(short)) = 7;//1 byte client to server flag
      *(char *)(send_buf + 1 * sizeof(short) + 1) = handle_len;
      strncpy(send_buf + 1 * sizeof(short) + 2, handle, handle_len);
      
      if(send(socket, send_buf, 4 + handle_len, 0) < 0)
      {
         free(buf);
         perror("send call");
         exit(-1);
      }
   }
   free(send_buf);
   free(handle);
}

void forward_broadcast(char *buf, int size, int socket) {
   char *handle, *send_buf;
   unsigned char handle_len = 0;
   int source_socket, i;

   size++;//whole message with flag back in
   handle_len = buf[0];
   handle = calloc(handle_len + 1, 1);
   strncpy(handle, buf + 1, handle_len);
   handle[handle_len] = '\0';
   //printf("%s: %s\n", handle, buf + 1 + handle_len);//needs bash prompt?
   source_socket = get_socket(handle);

   buf--;//go back so pointing to flag

   send_buf = calloc(size + 2, 1);
   memcpy(send_buf + 2, buf, size);
   *(short *)send_buf = htons(size + 2);

   for (i = 0; i < max_handles; i++) {//maybe handle_list_size
      if (handle_list[i].socket != source_socket && handle_list[i].socket) {
         if(send(handle_list[i].socket, send_buf, size + 2, 0) < 0)
         {
            free(send_buf);
            perror("send call");
            exit(-1);
         }
      }
   }
   free(send_buf);
   free(handle);
}

int get_socket(char *handle) {
   int i, ret = -1;

   for (i = 0; i < max_handles; i++) {//maybe handle_list_size
      if (!strcmp(handle_list[i].name, handle)) {
            ret = handle_list[i].socket;
      }
   }
   return ret;
}

void send_handles(socket) {
   int size = 0, pos = 0, i;

   char *buf = calloc(7, 1);

   *(short *)buf = htons(7);//2 byte packet length IN NETWORK ORDER
   *(char *)(buf + sizeof(short)) = 11;//1 byte client to server flag
   *(int *)(buf + sizeof(short) + 1) = htonl(num_handles);
   if(send(socket, buf, 7, 0) < 0)
   {
      free(buf);
      perror("send call");
      exit(-1);
   }
   free(buf);
   size += 3; //normal header
   for (i = 0; i < max_handles; i++) {//maybe handle_list_size
      if (handle_list[i].socket)
         size += strlen(handle_list[i].name) + 1;
   }
   buf = calloc(size, 1);
   
   *(short *)buf = htons(0);//2 byte packet length IN NETWORK ORDER
   *(char *)(buf + sizeof(short)) = 12;//1 byte client to server flag
   pos = 3;//after normal header
   for (i = 0; i < max_handles; i++) {//maybe handle_list_size
      if (handle_list[i].socket) {
         buf[pos++] = strlen(handle_list[i].name);
         strncpy(&buf[pos], handle_list[i].name, strlen(handle_list[i].name));
         pos += strlen(handle_list[i].name);
      }
   }
   //printf("Sending packet 12 size %d, size without normal header %d\n", size, size - 3);
   if(send(socket, buf, size, 0) < 0)
   {
      free(buf);
      perror("send call");
      exit(-1);
   }
   free(buf);
   
}

void remove_client(int socket) {

   int i;
   //printf("Client on socket %d is terminated\n", socket);
   for (i = 0; i < max_handles; i++) {//maybe handle_list_size
      if (handle_list[i].socket == socket) {
         memset(&handle_list[i], '\0', sizeof(struct handle));
         num_handles--;
      }
   }
}

void exit_reply(int socket) {
   char *buf = calloc(3, 1);

   *(short *)buf = htons(3);//2 byte packet length IN NETWORK ORDER
   *(char *)(buf + sizeof(short)) = 9;//1 byte client to server flag
   if(send(socket, buf, 3, 0) < 0)
   {
      free(buf);
      perror("send call");
      exit(-1);
   }
   //printf("sent exit reply with flag 9\n");
   free(buf);
}


void handle_reply(int valid, int socket) {
   char *buf = calloc(3, 1);
   int flag = valid == 1 ? 2 : 3;

   *(short *)buf = htons(3);//2 byte packet length IN NETWORK ORDER
   *(char *)(buf + sizeof(short)) = flag;//1 byte client to server flag
   if(send(socket, buf, 3, 0) < 0)
   {
      free(buf);
      perror("send call");
      exit(-1);
   }
   //printf("sent handle reply with flag %d\n", flag);
   free(buf);
}

int verify_handle(char *buf, int size, int socket) {

   unsigned char handle_len = 0;
   char *handle;
   int ret = 1, i;
   //printf("num_handles %d handle_list_size %d\n", num_handles, handle_list_size);
   if (max_handles == handle_list_size) {
      handle_list = realloc(handle_list, (handle_list_size + 10) * sizeof(struct handle));
      if (handle_list == NULL) {
         perror("realloc");
         exit(-1);
      }
      else
         handle_list_size += 10;
   }
   //check here for isssssssssssssues

   //HERE
   if (!max_handles) {
      handle_list[max_handles].socket = socket;
      handle_len = buf[0];
      strncpy(handle_list[max_handles].name, buf + 1, handle_len);
      handle_list[max_handles].name[handle_len] = '\0';
      num_handles++;
      max_handles++;
   }
   else {
      handle_len = buf[0];
      handle = calloc(handle_len + 1, 1);
      strncpy(handle, buf + 1, handle_len);
      handle[handle_len] = '\0';

      for (i = 0; i < max_handles; i++) {//maybe handle_list_size
         //printf("Comparing |%s|:|%s|\n", handle_list[i].name, handle);
         if (!strcmp(handle_list[i].name, handle))
            ret = 0;
      }
      
      if (ret) {
         handle_list[max_handles].socket = socket;
         handle_len = buf[0];
         strncpy(handle_list[max_handles].name, buf + 1, handle_len);
         handle_list[max_handles].name[handle_len] = '\0';
         num_handles++;
         max_handles++;
      }
      free(handle);
   }
   //printf("Added handle %s with port %d\n", handle_list[0].name, handle_list[0].socket);
   /*for (i = 0; i < max_handles; i++) {//maybe handle_list_size
      printf("%d: %s %d\n", i, handle_list[i].name, handle_list[i].socket);
   }*/

   return ret;
}

int server_parse_flags(int flag, char *buf, int size, int socket) {//buf needs to be freed after use   

   char *freeThis = buf;
   //char *handle;
   //unsigned char handle_len = 0;
   //int message_len = 0;

   buf = buf + 1; //skip flag
   size--;//remove flag from size

   switch(flag) {
      case 1:
         handle_reply(verify_handle(buf, size, socket), socket);
         break;
      /*case 2://good handle
         break;
      case 3://bad handle
         break;*/
      case 4://broadcastrecv
         forward_broadcast(buf, size, socket);

         /*handle_len = buf[0];
         handle = calloc(handle_len + 1, 1);
         strncpy(handle, buf + 1, handle_len);
         handle[handle_len] = '\0';
         printf("%s: %s\n", handle, buf + 1 + handle_len);//needs bash prompt?
         free(handle);*/
         break;
      case 5://1-1 message
         forward_message(buf, size, socket);

         /*//printf("Received 1-1 message\n\n");
         handle_len = *(buf + 1 + buf[0]);//get src handle len
         //printf("handle len recv %d\n", handle_len);
         handle = calloc(handle_len + 1, 1);
         strncpy(handle, buf + 2 + buf[0], handle_len);
         handle[handle_len] = '\0';
         printf("%s: %s\n", handle, buf + 2 + buf[0] + handle_len);//needs bash prompt?
         free(handle);*/
         break;
      /*case 7://dest handle invalid
         break;*/
      case 8:
         exit_reply(socket);
         break;
      /*case 9://server->client exit confirm
         exit(-1);
         break;*/
      case 10:
         //printf("Received handle request\n");
         send_handles(socket);
         break;
      /*case 11://num handles
         break;
      case 12://follows 11 with list of handles
         break;*/
   }
   free(freeThis);
   return flag;
}

int receive_data(int socket) {
   char *normal_buf;
   char *buf;
   int size, rec_bytes, count = 0;

   normal_buf = calloc(2, 1);

   if ((rec_bytes = recv(socket, normal_buf, 2, 0)) < 0)
   {
      perror("recv call");
      exit(-1);
   }
   //printf("Server received %d bytes and size of %d\n", rec_bytes, ntohs(*(short *)normal_buf));
   if (!rec_bytes) {
      //printf("Client on socket %d Terminated\n", socket);
      //remove client info and handles
   }
   else if (rec_bytes < 2) {
      if ((rec_bytes = recv(socket, normal_buf + 1, 1, 0)) < 0)
      {
         perror("recv call");
         exit(-1);
      }
   }
   size = ntohs(*(short *)normal_buf) - 2; //without size short
   buf = calloc(size, 1);


   while (count < size) {//only read up to size bytes
      rec_bytes = recv(socket, buf + count, size - count, 0);
      //printf("size %d, count %d, rec_bytes %d\n", size, count, rec_bytes);
      if (!rec_bytes) {
         //printf("Client on socket %d Terminated\n", socket);
         break;
      }
      count += rec_bytes;
   }
   //printf("Size of packet: %d Flag: %d\n", size, buf[0]);

   //return something if client terminates?
   return rec_bytes == 0 ? -1 : server_parse_flags(buf[0], buf, size, socket);
   
}

void accept_connections(int socket) {
   int i, client;
   char *buf;              //buffer for receiving from client
   int buffer_size = 1024;  //packet size variable
   //int message_len= 0;     //length of the received message
   fd_set active_set, read_set;

   FD_ZERO(&active_set);//init/zero out fd set
   FD_SET(socket, &active_set);//add server socket to active list
   buf=  (char *) malloc(buffer_size);

   while(1) {
      read_set = active_set;
      if (select(FD_SETSIZE, &read_set, NULL, NULL, NULL) < 0) {
         perror("select");
         exit(-1);
      }
      //printf("Server socket: %d\n", socket);
      for (i = 0; i < FD_SETSIZE; i++) {//++i in C example?
         if (FD_ISSET(i, &read_set)) {//either connecting or already connected
            if (i == socket) {//if initial connection
               client = tcp_listen(socket, 5);
               
               //printf("A client has connected on socket: %d\n", client);
               FD_SET(client, &active_set);
            }
            else {
               if (receive_data(i) == -1) {//returns -1 if client terminated
                  remove_client(i);
                  close(i);
                  FD_CLR(i, &active_set);
               }
            }
         }
      }
   }
}

void init_connections() {
   
   int server_socket = 0;   //socket descriptor for the server socket
   server_socket = tcp_recv_setup();
   //this must also go in line with other data structure for handles
   accept_connections(server_socket);
}

void close_all() {
   
}

int main(int argc, char *argv[])
{
   handle_list = calloc(sizeof(struct handle), 10);
   handle_list_size = 10;
   init_connections();
   close_all();
   return 0;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup()
{
   int server_socket = 0;
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

   printf("Server is using port %d\n", ntohs(local.sin_port));
        
   return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

int tcp_listen(int server_socket, int back_log)
{
   int client_socket= 0;
   if (listen(server_socket, back_log) < 0)
   {
      perror("listen call");
      exit(-1);
   }

   if ((client_socket = accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
   {
      perror("accept call");
      exit(-1);
   }

   return(client_socket);
}

