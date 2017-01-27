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

main(int argc, char *argv[])
{
    int server_socket= 0;   //socket descriptor for the server socket
    int client_socket= 0;   //socket descriptor for the client socket
    char *buf;              //buffer for receiving from client
    int buffer_size= 1024;  //packet size variable
    int message_len= 0;     //length of the received message

    printf("sockaddr: %d sockaddr_in %d\n", sizeof(struct sockaddr), sizeof(struct sockaddr_in));
    
    //create packet buffer
    buf=  (char *) malloc(buffer_size);

    //create the server socket
    server_socket= tcp_recv_setup();

    //look for a client to serve
    client_socket= tcp_listen(server_socket, 5);

    //now get the data on the client_socket
    if ((message_len = recv(client_socket, buf, buffer_size, 0)) < 0)
      {
	perror("recv call");
	exit(-1);
      }

    printf("Message received, length: %d\n", message_len);
    printf("Data: %s\n", buf);
    
    /* close the sockets */
    close(server_socket);
    close(client_socket);
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

    printf("socket has port %d \n", ntohs(local.sin_port));
	        
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
  
  if ((client_socket= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
    {
      perror("accept call");
      exit(-1);
    }
  
  return(client_socket);
}

