/******************************************************************************
 * Jacob Russ
 * tcp_client.c
 * CPE 464 Project 2
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
#include "testing.h"

uint32_t client_count = 0;
fd_set server_set;
int active = 1;

/* 
 * The main simply calls the loop function and exits
 * only when the loop function is exited
 */
int main(int argc, char * argv[])
{
   initialize_and_loop(argc, argv);
   return 0;
}

/* This function initializes the variables and begins the listening loop */
void initialize_and_loop(int argc, char **argv) {
   int socket_num;         //socket descriptor
   char *name;
   int nameLen;

   /* check command line arguments  */
   checkArguments(argc, argv);

   /* grab the client's name */
   nameLen = strlen(argv[1]);
   name = getClientName(nameLen, argv);

   /* set up the socket for TCP transmission  */
   socket_num= tcp_send_setup(argv[2], argv[3]);

   /* initialize receiving */
   initRecv(socket_num);

   /* send the initial packet containing the client name */
   initClient(socket_num, nameLen, name); 

   printf("$: ");
   fd_set working_set;
   while (active) {
      fflush(stdout);
      working_set = server_set;
      listen_and_receive(socket_num, nameLen, name, working_set);
   }
   close(socket_num);
}

/* This function checks that the arguments are correct */
void checkArguments(int argc, char **argv) { 
   if(argc!= 4)
   {
      printf("usage: %s host-name port-number \n", argv[0]);
      exit(1);
   }
}

/* This function grabs the client's handle */
char *getClientName(int nameLen, char **argv) {
   char *name;
   
   if (nameLen > 255) {
      printf("Handle cannot exceed 255 characters\n");
      exit(-1);
   }
   name = (char *) malloc(nameLen);
   memcpy(name, argv[1], nameLen);
   return name;
}

/* This function checks for client input and/or incoming packets */
void listen_and_receive(int socket_num, uint8_t nameLen, char *name,
 fd_set working_set) {
   if (select(socket_num + 1, &working_set, NULL, NULL, NULL) < 0) {
      perror ("select");
      exit (-1);
   }

   if (FD_ISSET(USER_INPUT, &working_set)) {
      processCommand(name, nameLen, socket_num);
   }
   else {
      recvPacket(socket_num);
   }
}

/* This function processes a client's command input */
void processCommand(char *name, uint8_t nameLen, int socket_num) {
   char *extra_buf, *send_buf;
   uint16_t send_len= 0;        //amount of data to send

   /* initialize data buffer for the packet */
   send_buf= (char *) malloc(LARGE_BUFFER);
   extra_buf= (char *) malloc(LARGE_BUFFER);

   while ((send_buf[send_len] = getchar()) != '\n')
      send_len++;
   send_buf[send_len] = '\0';
   if (send_buf[0] == '%') {
      /* Process a %L command from the client */
      if ((send_buf[1] == 'l' || send_buf[1] == 'L') && send_buf[2] == '\0')
	 processList(socket_num);

      /* Process a %E command from the client */
      else if ((send_buf[1] == 'e' || send_buf[1] == 'E')
       && send_buf[2] == '\0')
	 sendExitPacket(socket_num);

      else if(send_buf[2] == ' ') { 
	 /* Specifically if a message command is sent */
	 if (send_buf[1] == 'm' || send_buf[1] == 'M') {
	    handleMessage(send_buf, send_len, extra_buf, name,
	     nameLen, socket_num); 
	 }
	 /* Specifically if a broadcast command is sent */
	 else if ((send_buf[1] == 'b' || send_buf[1] == 'B'))  {
	    handleBroadcast(send_buf, send_len, extra_buf, name,
	     nameLen, socket_num);
	 }
	 else
	    printf("Invalid command\n");
      }
      else
	 printf("Invalid command\n");
   }
   else
      printf("Invalid command\n");
   printf("$: ");
   memset(send_buf, 0, LARGE_BUFFER);
   memset(extra_buf, 0, LARGE_BUFFER);
   free(send_buf);
   free(extra_buf);
}

/* 
 * This function handles message commands by calling the parse function and 
 * sending the packets out
 */
void handleMessage(char *send_buf, int send_len, char *extra_buf, char *name,
 uint8_t nameLen, int socket_num) {
   char *packet, *destName, *send_ptr;
   uint8_t dest_len;
   uint16_t pkt_len, text_len;

   send_ptr = send_buf;
   packet = parseMessage(send_buf, send_len, MSG_FLAG, name, nameLen);
   if (strcmp(packet, "NULL")) { 
      memcpy(&pkt_len, packet, 2);
      memcpy(&dest_len, packet + 3, 1);
      destName = (char *) malloc(dest_len);
      memcpy(destName, packet + 4, dest_len);
      text_len = pkt_len - 5 - nameLen - dest_len;

      if (text_len > 999) {
	 while (text_len > 999) {
	    memcpy(extra_buf + 3, destName, dest_len);
	    memcpy(extra_buf + 3 + dest_len, " ", 1);
	    memcpy(extra_buf + 4 + dest_len, send_ptr + 4 + dest_len, 999);
	    extra_buf[1003 + dest_len] = '\0';
	    memcpy(extra_buf, "%m ", 3);
	    send_ptr += 999;
	    text_len -= 999;
	    send_len -= 999;
	    packet = parseMessage(extra_buf, 1004 + dest_len, MSG_FLAG,
	     name, nameLen);
	    sendPacket(packet, socket_num);
	 }
	 memcpy(extra_buf, "%m ", 3);
	 memcpy(extra_buf + 3, destName, dest_len);
	 memcpy(extra_buf + 3 + dest_len, " ", 1);
	 memcpy(extra_buf + 4 + dest_len, send_ptr + 4 + dest_len, text_len);
	 extra_buf[send_len] = '\0';
	 packet = parseMessage(extra_buf, send_len, MSG_FLAG, name, nameLen);
	 sendPacket(packet, socket_num);
	 memset(destName, 0, dest_len);
	 free(destName);
      }
      else {
	 sendPacket(packet, socket_num);
      }
   }
   else
      printf("Invalid command\n");
}

/* 
 * This function handles broadcast commands by calling the parse function and 
 * sending the packets out
 */
void handleBroadcast(char *send_buf, int send_len, char *extra_buf, char *name,
 uint8_t nameLen, int socket_num) {
   char *packet, *send_ptr;

   send_ptr = send_buf;
   if (send_len - 3 > 1000) {
      while (send_len - 3 > 1000) {
	 memcpy(extra_buf + 3, send_ptr + 3, 999);
	 extra_buf[1002] = '\0';
	 memcpy(extra_buf, "%b ", 3);
	 send_ptr += 999;
	 send_len -= 999;
	 packet = parseBroadcast(extra_buf, 1003, BRDCST_FLAG, name, nameLen);
	 sendPacket(packet, socket_num);
      }
      memcpy(extra_buf, send_ptr, send_len);
      memcpy(extra_buf, "%b ", 3);
      extra_buf[send_len] = '\0';
      packet = parseBroadcast(extra_buf, send_len, BRDCST_FLAG, name, nameLen);
      sendPacket(packet, socket_num);
   }
   else {
      packet = parseBroadcast(send_buf, send_len, BRDCST_FLAG, name, nameLen);
      if(strcmp(packet, "NULL"))      
	 sendPacket(packet, socket_num);
      else
	 printf("Invalid command\n");
   }
}

/* This function sends a packet telling the server the client wants to exit */
void sendExitPacket(int socket_num) {
   char *packet;
   uint8_t flag = EXIT_FLAG;
   uint16_t length = 3;

   packet = (char *) malloc(length);
   memcpy(packet, &length, 2);
   memcpy(packet + 2, &flag, 1);
   sendPacket(packet, socket_num);
}

/* This function processes a list command from the client */
void processList(int socket_num) {
   char *packet;

   packet = listRequest(NORM_HEADER, LIST_FLAG);
   sendPacket(packet, socket_num);
}

/* This function sends a packet to the server */
void sendPacket(char *packet, int socket_num) {
   int sent;
   uint16_t pkt_len, net_order;

   memcpy(&pkt_len, packet, 2);
   /*convert the length to network order before sending */
   net_order = htons(pkt_len);
   memcpy(packet, &net_order, 2);

   sent =  send(socket_num, packet, pkt_len, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
   memset(packet, 0, pkt_len);
   free(packet);
}

/* This function receives a function from the server */
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

   packet = (char *) malloc(pkt_len + 3);

  /* if Server has been terminated, quit the client */ 
  if (message_len == 0) {
     printf("\nServer Terminated\n");
     active = 0;
   }

   /* Special case for Flag 12 */
   if (pkt_len == 0)
      recvFullList(fdIndex); 	 

   else if ((message_len = recv(fdIndex, (packet + 2), pkt_len - 2, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   memcpy(packet, &pkt_len, 2);
   processPacket(packet);
   memset(packet, 0, pkt_len);
   free(packet);
   return packet;
}

/* This function determines what needs to be done with a received packet */
void processPacket(char *packet) {
   uint8_t flag;

   memcpy(&flag, packet + 2, 1);
   if (flag == MSG_FLAG)
      recvMessage(packet);
   else if (flag == COUNT_FLAG)
      client_count = recvListNum(packet);
   else if (flag == EXIT_ACK)
      active = 0;
   else if (flag == NF_FLAG)
      printNotFoundError(packet);
   else if (flag == BRDCST_FLAG)
      recvBroadcast(packet);
}

/* This function prints the error when a destination client is not found */
void printNotFoundError(char *packet) {
   char *destName;
   uint8_t dest_len;

   memcpy(&dest_len, packet + 3, 1);
   destName = (char *) malloc(dest_len);
   memcpy(destName, packet + 4, dest_len);
   printf("\nClient with handle %s does not exist\n", destName);
   printf("$: ");
   memset(destName, 0, dest_len);
   free(destName);
}

/* This function handles a received broadcast packet */
void recvBroadcast(char *packet) {
   uint8_t sender_len;
   char *sender, *text;
   uint16_t total_len, text_len;

   memcpy(&total_len, packet, 2);
   memcpy(&sender_len, packet + 3, 1);
   sender = (char *) malloc(sender_len + 1);
   sender[sender_len] = '\0';
   memcpy(sender, packet + 4, sender_len);
   text_len = total_len - sender_len - 4;
   text = (char *) malloc(text_len);
   memcpy(text, packet + 4 + sender_len, text_len);
   printf("\n%s: %s\n", sender, text);
   printf("$: ");
   memset(text, 0, text_len);
   free(text);
   memset(sender, 0, sender_len);
   free(sender);
}

/* This function handles a received message packet */
void recvMessage(char *packet) {
   uint8_t dest_len, sender_len;
   char *sender, *text;
   uint16_t total_len, text_len;

   memcpy(&total_len, packet, 2);
   memcpy(&dest_len, packet + 3, 1);
   memcpy(&sender_len, packet + 4  + dest_len, 1);
   sender = (char *) malloc(sender_len + 1);
   sender[sender_len] = '\0';
   memcpy(sender, packet + 5 + dest_len, sender_len);
   text_len = total_len - dest_len - sender_len - 5;
   text = (char *) malloc(text_len);
   memcpy(text, packet + 5 + dest_len + sender_len, text_len);
   printf("\n%s: %s\n", sender, text);
   printf("$: ");
   memset(text, 0, text_len);
   free(text);
   memset(sender, 0, sender_len);
   free(sender);
}

/* This function handles receiving the list count packet */
uint32_t recvListNum(char *packet) {
   uint16_t message_len;
   uint8_t flag;
   uint32_t client_count;

  memcpy(&message_len, packet, 2);
  memcpy(&flag, packet + 2, 1);
  memcpy(&client_count, packet + 3, 4);
  client_count = ntohl(client_count);
  return client_count;
}

/* This function handles receiving the list of names packet */
void recvFullList(int socket_num) {
   int count = 0;
   char **names;
   uint8_t name_len, flag;
   names = (char **) malloc(client_count);
   
   if ((recv(socket_num, &flag, 1, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   
   printf("\nNumber of clients: %d\n", client_count);
   while(count < client_count) {
      if ((recv(socket_num, &name_len, 1, 0)) < 0) {
	 perror("recv call");
	 exit(-1);
      }
      names[count] = malloc(name_len + 1);
      if ((recv(socket_num, names[count], name_len, 0)) < 0) {
	 perror("recv call");
	 exit(-1);
      }
      names[count][name_len] = '\0';
      printf("   %s\n", names[count]);
      memset(names[count], 0, name_len);
      free(names[count++]);
   }
   printf("$: ");
   memset(names, 0, client_count);
   free(names);
}

/* This function process a list request command from the client */
char *listRequest(uint16_t length, uint8_t flag) {
   char *list_packet;

   list_packet = (char *) malloc(length);
   memcpy(list_packet, &length, 2);
   memcpy(list_packet + 2, &flag, 1);
   return list_packet;
}

/* This function initializes the client by handling the initial ACKs */
void initClient(int socket_num, uint8_t dataLen, char *name) {
   uint16_t pktLen = dataLen + 4;
   uint16_t pkt_len;
   char *packet;
   uint8_t flag = 1;
   uint16_t message_len = 0;

   /* Build the initial packet */
   packet = (char *) malloc(pktLen + 2);
   memcpy(packet, &pktLen, 2);
   memcpy(packet + 2, &flag, 1);
   memcpy(packet + 3, &dataLen, 1);
   memcpy(packet + 4, name, dataLen);  

   /* Send the initial packet */
   sendPacket(packet, socket_num);

   /* Receive the server response before continuing */
   if ((message_len = recv(socket_num, &pkt_len, 2, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   if ((recv(socket_num, &flag, 1, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }

   if (flag == BAD_ACK) {
      printf("Handle already in use: %s\n", name);
      exit (-1);
   }
}

/* This function parses a broadcast command into a sendable packet */
char *parseBroadcast(char *buf, uint16_t msg_len, uint8_t flag, char *sender,
 uint8_t senderLen) {
   char *cmd, *msg, *packet;
   const char space_delim[2] = " ";
   const char null_delim[2] = "\0";
   uint16_t pkt_len, text_len;

   /* parse the broadcast out */
   cmd = strtok(buf, space_delim);
   msg = strtok(NULL, null_delim);
   if (!msg)
      return "NULL";
   text_len = strlen(msg) + 1;


   /* restructure parsed broadcast */
   pkt_len = senderLen + text_len + 4;
   packet = (char *) malloc(pkt_len);
   memcpy(packet, &pkt_len, 2);
   memcpy(packet + 2, &flag, 1);
   memcpy(packet + 3, &senderLen, 1);
   memcpy(packet + 4, sender, senderLen);
   memcpy(packet + 4 + senderLen, msg, text_len);
   return packet;
}

/* This function parses a message command into a sendable packet */
char *parseMessage(char *buf, uint16_t msg_len, uint8_t flag, char *sender,
 uint8_t senderLen) {
   char *cmd, *dest, *msg, *packet; 
   const char space_delim[2] = " ";
   const char null_delim[2] = "\0";
   uint8_t destLen;
   uint16_t pkt_len, text_len;
  
  /* parse the message out */
   cmd = strtok(buf, space_delim);
   dest = strtok(NULL, space_delim);
   if (!dest) 
      return "NULL";
   destLen = strlen(dest);
   msg = strtok(NULL, null_delim);
 if (!msg) 
      return "NULL";
   text_len = strlen(msg) + 1;

   /* restructure parsed message */
   pkt_len = senderLen + destLen + text_len + 5;
   packet = (char *) malloc(pkt_len);
   memcpy(packet, &pkt_len, 2);
   memcpy(packet + 2, &flag, 1);
   memcpy(packet + 3, &destLen, 1);
   memcpy(packet + 4, dest, destLen);
   memcpy(packet + 4 + destLen, &senderLen, 1);
   memcpy(packet + 5 + destLen, sender, senderLen);
   memcpy(packet + 5 + destLen + senderLen, msg, text_len);
   return packet;
} 

/* This function sets up the fds for the client */
void initRecv(int socket_num) {
   FD_ZERO(&server_set);
   FD_SET(socket_num, &server_set);
   FD_SET(USER_INPUT, &server_set);
}

/* This function sets up the clients ability to send */
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
   remote.sin_family= AF_INET;

   // get the address of the remote host and store
   if ((hp = gethostbyname(host_name)) == NULL)
   {
      printf("Error getting hostname: %s\n", host_name);
      exit(-1);
   }

   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   // get the port used on the remote side and store
   remote.sin_port= htons(atoi(port));

   if(connect(socket_num, (struct sockaddr*)&remote,
    sizeof(struct sockaddr_in)) < 0)
   {
      perror("connect call");
      exit(-1);
   }

   return socket_num;
}
