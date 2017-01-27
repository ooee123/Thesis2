/* networks.c

   Code originally give to Prof. Smith by his TA in 1994.
   No idea who wrote it.  Copy and use at your own Risk
*/

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

/* This function sets the server socket.  It lets the system
determine the port number.  The function returns the server
socket number and prints the port number to the screen.  */

int32_t tcpServerSetup() {
   int32_t server_socket = 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */

   /* create the socket  */
   server_socket = socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0) {
      perror("socket call");
      exit(1);
   }

   local.sin_family= AF_INET;         //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= htons(0);                 //let system choose the port

   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
      exit(-1);
   }
   
   //get the port name and print it out
   if (getsockname(server_socket, (struct sockaddr *)&local, &len) < 0) {
      perror("getsockname call");
      exit(-1);
   }

   if (listen(server_socket, BACKLOG) < 0) {
      perror("listen call");
      exit(-1);
   }
   
   printf("Server is using port %d \n", ntohs(local.sin_port));
   
   return server_socket;
}

/* This function waits for a client to ask for services.  It returns
the socket number to service the client on.    */

int32_t tcpAccept(int32_t server_socket) {
   int32_t client_socket= 0;

   if ((client_socket = accept(server_socket, (struct sockaddr *)0,
    (socklen_t *)0)) < 0) {
      perror("accept call");
      exit(-1);
   }

   return client_socket;
}

int32_t tcp_select(fd_set *socket_fds, int32_t max, struct timeval time) {
   int32_t ret_sockets;

   if ((ret_sockets = select(max + 1, socket_fds, (fd_set *) FALSE,
    (fd_set *) FALSE, &time)) < 0) {
      perror("accept call");
      exit(EXIT_FAILURE);
   }

   return ret_sockets;
}

int32_t tcpClientSetup(char *host_name, char *port) {
   int32_t socket_num;
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
   
   memcpy((int8_t *)&remote.sin_addr, (int8_t *)hp->h_addr, hp->h_length);

   // get the port used on the remote side and store
   remote.sin_port= htons(atoi(port));

   if(connect(socket_num, (struct sockaddr *)&remote,
    sizeof(struct sockaddr_in)) < 0) {
      perror("connect call");
      exit(-1);
   }

   return socket_num;
}

void chat_header_setup(int8_t **send_buf, int8_t flag, int16_t packet_len) {
   ChatHeader hdr;

   hdr.packet_len = htons(packet_len);
   hdr.flag = flag;

   memcpy((void *)*send_buf, (void *)&hdr, CHAT_HEADER_SIZE);
}

void reset(int8_t *buf, int32_t message_len) {
   int count;

   for (count = 0; count < message_len; count++)
      buf[count] = 0;
}

void create_buffer(int8_t **buf, int32_t size) {
   if ((*buf = (int8_t *) calloc(1, size)) == NULL) {
      perror("malloc() error");
      exit(EXIT_FAILURE);
   }
}

void send_data(int8_t **send_buf, int32_t socket, int16_t packet_len) {
   int8_t *temp = *send_buf;
   int32_t sent;

   /* now send the data */
   sent = send(socket, temp, packet_len, 0);
   if(sent < 0) {
      perror("send call");
      exit(EXIT_FAILURE);
   }
}

int8_t check_flag(int32_t message_len, int8_t *buf) {
/*   if (message_len < CHAT_HEADER_SIZE)
      printf("Bad header\n");
   else if (ntohs(*(int16_t *)buf) != message_len)
      printf("TODO: Do anything here?\n");
   else {*/
      printf("Incoming Packet:\n");
      printf("\tPacket Length: %d\n", ntohs(*(int16_t *)buf));
      printf("\tFlag: %d\n", buf[FLAG_BYTE]);
   //}

   return buf[FLAG_BYTE];
}

int8_t check_socket(int8_t *buf, Client *socket, int8_t client_or_server) {
   int32_t message_len = 0;     //length of the received message
   int8_t flag = 0;

   if ((message_len = recv(socket->socket, buf, MAX_BUF, 0)) < 0) {
      if (SERVER == client_or_server) {
         perror("recv call");
         exit(EXIT_FAILURE);
      }
      else {
         fprintf(stderr, "Server Terminated\n");
         exit(EXIT_FAILURE);
      }
   }
   else if (0 == message_len) {
      // This means that this client has closed. Remove this client from fd_set
      socket->handle_len = 0;
      if (socket->handle != NULL)
         free(socket->handle);
      socket->handle = NULL; // Need to free the handle as well here
      socket->socket = 0;
   }
   else {
      //flag = check_flag(message_len, buf);
      flag = buf[FLAG_BYTE];
   }

   return flag;
}
