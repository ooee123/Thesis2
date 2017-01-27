/******************************************************************************
 * tcp_server.c
 * Jacob Russ 
 * CPE 464 - Project 2
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

fd_set main_set;
char **client_list;
int *fd_list;
int client_count = 0;

/* 
 * The main calls the function that endless loops
 * until a control-c command is detected
 */
int main(int argc, char *argv[])
{
   setup_and_run(argc, argv);
   return 0;
}

/* This function initializes the variables and loops to listen */
void setup_and_run(int argc, char **argv) {
   int server_socket= 0;   //socket descriptor for the server socket
   fd_set curr_set; 
   int fdIndex = 0;

   client_list = (char **) malloc(client_count + 1);
   fd_list = (int *) malloc(client_count + 1);
   
   //create packet buffer
   server_socket = makeBuffer(argc, argv);

   //zero out the set and add the server
   initSet(server_socket);

   while (1) {
      //save the working set
      curr_set = main_set;
      get_listening(curr_set, fdIndex, server_socket);
   }
   close(server_socket);
}

/* This function makes the buffer for the server */
int makeBuffer(int argc, char **argv) {
   char *port_data;
   int server_socket;

   if (argc == 2) 
      server_socket= tcp_recv_setup(strtol(argv[1], &port_data, BASE_TEN));
   else
      server_socket= tcp_recv_setup(0);
   tcp_listen(server_socket, 5);
   return server_socket;
}

/* This function initializes the fd set */
void initSet(int server_socket) {
   FD_ZERO(&main_set);
   FD_SET(server_socket, &main_set);
}

/* This function checks for any incoming packets */
void get_listening(fd_set curr_set, int fdIndex, int server_socket) {
   if (select(FD_SETSIZE, &curr_set, NULL, NULL, NULL) < 0) {
      perror ("select");
      exit (-1);
   }

   for (fdIndex = 0; fdIndex < FD_SETSIZE; ++fdIndex) {
      if (FD_ISSET(fdIndex, &curr_set)) {
	 if (fdIndex == server_socket) {
	    setupNewClient(server_socket, fdIndex);
	 }
	 else 
	    beginReceive(fdIndex);
      }
   }
}

/* This function handles a new client request */
void setupNewClient(int server_socket, int fdIndex) {
   char *packet;
   int client_socket;
   client_socket = tcp_accept(server_socket);
   packet = recvPacket(client_socket);
   if (processACK(fdIndex, client_socket, packet) == GOOD_ACK)
      FD_SET(client_socket, &main_set);
}

/* This function receives a packet and hands it off accordingly */
void beginReceive(int fdIndex) {
   char *recv_pkt;

   recv_pkt = recvPacket(fdIndex);
   if(strcmp(recv_pkt, ""))
      handOffPacket(recv_pkt, fdIndex);
   else
      removeClient(fdIndex);
}

/* This function processes the initial ACK from the client */
uint8_t processACK(int fdIndex, int client_socket, char *packet) {
   char *new_name;	 
   int socket;
   uint8_t name_len, flag;

   memcpy(&flag, packet + 2, 1);
   if (flag == 1) {
      new_name = getName(packet);
      socket = findFD(new_name);
      if (socket == -1) {
	 name_len = strlen(new_name) + 1;
	 client_list = (char **) realloc(client_list, client_count + 2);
	 fd_list = (int *) realloc(fd_list, client_count + 2);
	 client_list[client_count] = (char *) malloc(name_len);
	 memcpy(client_list[client_count], new_name, name_len);
	 fd_list[client_count++] = client_socket;
	 flag = GOOD_ACK;
      }
      else
	 flag = BAD_ACK;
      sendACK(flag, client_socket);
      //memset(new_name, 0, name_len);
      //free(new_name);
   }
   return flag;
}

/* send intitialization ACK packet */
void sendACK(uint8_t flag, int socket_num) {
   uint16_t init_len = 3;
   char *init_pkt;

   init_pkt = (char *) malloc(3);
   memcpy(init_pkt, &init_len, 2);
   memcpy(init_pkt + 2, &flag, 1);
   sendPacket(init_pkt, socket_num);
}

/* This function removes a client after they enter %e, %E, or control-c */
void removeClient(int fdIndex) {
   int index;

   for (index = 0; index < client_count; index++) {
      if (fd_list[index] == fdIndex) {
	 client_count--;
	 memcpy(client_list + index, client_list + index + 1,
	  client_count - index);
	 client_list = (char **) realloc(client_list, client_count);
	 memcpy(fd_list + index, fd_list + index + 1, client_count - index);
	 fd_list = (int *) realloc(fd_list, client_count);
	 index += client_count;
      }
   }
}

/* This function recieves an incoming packet from a client */
char *recvPacket(int fdIndex) {
   int message_len = 0;
   uint16_t pkt_len;
   char *packet;

   if ((message_len = recv(fdIndex, &pkt_len, 2, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   /*convert the length from network order*/
   pkt_len = ntohs(pkt_len);

   if (message_len == 0) {
      FD_CLR(fdIndex, &main_set);
      close(fdIndex);
      return "";
   }

   packet = (char *) malloc(pkt_len + 3);
   if ((message_len = recv(fdIndex, (packet + 2), pkt_len - 2, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   memcpy(packet, &pkt_len, 2);
   return packet;
}

/* 
 * This function determines what type of packet was received and sends it to
 * the appropriate function to process it
 */
void handOffPacket(char *packet, int fdIndex) {
   int socket;
   char *destName, *senderName;
   uint16_t pkt_len;
   uint8_t flag;

   memcpy(&flag, packet + 2, 1);
   memcpy(&pkt_len, packet, 2);
   if (flag == MSG_FLAG) {
      destName = getName(packet);
      socket = findFD(destName);
      if (socket != -1)
	 sendPacket(packet, socket);
      else
	 sendNotFoundPacket(fdIndex, destName);
   }

   if (flag == BRDCST_FLAG) {
      senderName = getName(packet);
      socket = findFD(senderName);
      sendBroadcast(packet, socket);
   }

   else if (flag == LIST_FLAG) {
      sendListCount(fdIndex, COUNT_FLAG, sizeof(int));
      sendListNames(fdIndex, NAME_FLAG);
   }

   else if (flag == EXIT_FLAG)
      sendExitAck(fdIndex);
}

/*
 * This function sends the packet if a destination name was not found 
 * for a %M command from the client
 */
void sendNotFoundPacket(int fdIndex, char *destName) {
   char *packet;
   uint8_t flag = NF_FLAG;
   uint8_t dest_len = strlen(destName);
   uint16_t length = 3 + dest_len + 1;

   packet = (char *) malloc(length);
   memcpy(packet, &length, 2);
   memcpy(packet + 2, &flag, 1);
   memcpy(packet + 3, &dest_len, 1);
   memcpy(packet + 4, destName, dest_len);
   sendPacket(packet, fdIndex);
}

/* 
 * This function sends off a broadcast packet to all clients
 * except the sender
 */
void sendBroadcast(char *packet, int socket) {
   int count;
   if (client_count > 1) {
      for(count = 0; count < client_count; count++) {
	 if(socket != fd_list[count])
	    sendPacket(packet, fd_list[count]);
      }
   }
}

/* This function sends the ACK so a client knows they can quit */
void sendExitAck(int fdIndex) {
   char *packet;
   uint8_t flag = EXIT_ACK;
   uint16_t length = 3;

   packet = (char *) malloc(length);
   memcpy(packet, &length, 2);
   memcpy(packet + 2, &flag, 1);
   sendPacket(packet, fdIndex);
}

/* This function grabs the first name from a packet */
char *getName(char* packet) {
   uint8_t dest_len;
   char *dest_name;

   memcpy(&dest_len, packet + 3, 1);
   dest_name = (char *) malloc(dest_len + 1);
   dest_name[dest_len] = '\0';
   memcpy(dest_name, packet + 4, dest_len);
   return dest_name;
}

/* This function finds the corresponding fd for a given client */
int findFD(char *name) {
   int count;

   for(count = 0; count < client_count; count++) {
      if(!strcmp(name, client_list[count]))
	 return fd_list[count];
   }
   return -1;
}

/* This function sends the packet containing the number of clients */
void sendListCount(int fdIndex, uint8_t flag, uint16_t length) {
   char *packet;
   uint16_t pkt_len = length + 3;
   packet = (char *) malloc(pkt_len);
   uint32_t net_order_count;

   memcpy(packet, &pkt_len, 2);
   memcpy(packet + 2, &flag, 1);
   /* convert the count to network order before sending */
   net_order_count = htonl(client_count);
   memcpy(packet + 3, &net_order_count, length);
   sendPacket(packet, fdIndex);
}

/* This function sends the list of client handles */
void sendListNames(int fdIndex, uint8_t flag) {
   char *packet;
   uint8_t length;
   int count = 0;
   int offset = 3;
   uint16_t total_len = 3;

   packet = (char *) malloc(total_len);
   memcpy(packet + 2, &flag, 1);
   
   while (count < client_count) {
      length = strlen(client_list[count]);
      total_len += (length + 1);
      packet = realloc(packet, total_len);
      memcpy(packet + offset++, &length, 1);
      memcpy(packet + offset, client_list[count++], length);
      offset += length;
   }
   memcpy(packet, &total_len, 2);
   sendPacket(packet, fdIndex);
}

/* This function sends a packet to the designated client */
void sendPacket(char *packet, int socket_num) {
   int sent;
   uint16_t pkt_len, net_order;
   uint8_t flag;
   uint16_t zero_len = 0;

   memcpy(&pkt_len, packet, 2);
   net_order = htons(pkt_len);
   memcpy(packet, &net_order, 2);
   memcpy(&flag, packet + 2, 1);
   if (flag == NAME_FLAG)
      memcpy(packet, &zero_len, 2);

   sent =  send(socket_num, packet, pkt_len, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */
int tcp_recv_setup(int input_port)
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
   local.sin_port= htons(input_port); //let system choose the port

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
   return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */
void tcp_listen(int server_socket, int back_log)
{
   if (listen(server_socket, back_log) < 0)
   {
      perror("listen call");
      exit(-1);
   }
}

int tcp_accept(int server_socket) {
   int client_socket = 0;

   if ((client_socket= accept(server_socket, (struct sockaddr*)0,
	       (socklen_t *)0)) < 0)
   {
      perror("accept call");
      exit(-1);
   }

   return(client_socket);
}

