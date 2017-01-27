//
//  networks.c
//  Chat_TCP_Final
//
//  Created by Rebecca McKinley on 4/25/16.
//  Copyright © 2016 Rebecca McKinley. All rights reserved.
//

#include "networks.h"

//will call select and process errors
void Safe_Select(fd_set *read_fds, int n) {
   int sel_ret = -1;
   
   sel_ret = select(n, read_fds, NULL, NULL, NULL);
   
   if (sel_ret == -1)
      printf("There was an error\n");
   if (sel_ret == 0) 
      printf("There was a timeout\n");
}

//will send and figure out error messafes
void Safe_Send(char *packet, int packet_len, int socket_num) {
   ssize_t sent = 0;
   
   sent = send(socket_num, packet, packet_len, 0);
   if(sent < 0) {
      perror("send call");
      //Clean_Up(socket_num, packet, -1);
   }
}

/* This function sets the server socket.  It lets the system
 determine the port number.  The function returns the server
 socket number and prints the port number to the screen.  */

int tcp_recv_setup() {
   int server_socket= 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */
   
   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_STREAM, 0);
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
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
      perror("getsockname call");
      exit(-1);
   }
   
   printf("Server is using port %d \n", ntohs(local.sin_port));
   
   return server_socket;
}

/* This function waits for a client to ask for services.  It returns
 the socket number to service the client on.    */

int tcp_listen(int server_socket, int back_log) {
   int sup = -234;
   int client_socket= 0;
   
   if ((sup = listen(server_socket, back_log)) < 0) {
      perror("listen call");
      exit(-1);
   }
   if ((client_socket = accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
      perror("accept call");
      exit(-1);
   }
   
   return(client_socket);
}

int tcp_send_setup(char *host_name, char *port) {
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
   
   if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in))
      < 0) {
      
      perror("connect call");
      exit(-1);
   }
   
   return socket_num;
}
