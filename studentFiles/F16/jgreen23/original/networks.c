
/* 	Code originally give to Prof. Smith by his TA in 1994.
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

int tcpServerSetup()
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

	if (listen(server_socket, BACKLOG) < 0)
	{
		perror("listen call");
		exit(-1);
	}
	
	printf("socket has port %d \n", ntohs(local.sin_port));
	
	return server_socket;
}

/* This function waits for a client to ask for services.  It returns
the socket number to service the client on.    */

int tcpAccept(int server_socket)
{
	int client_socket= 0;

	if ((client_socket = accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
	{
		perror("accept call");
		exit(-1);
	}

	return(client_socket);
}


int tcpClientSetup(char *host_name, char *port)
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

	if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
	{
		perror("connect call");
		exit(-1);
	}

	return socket_num;
}
