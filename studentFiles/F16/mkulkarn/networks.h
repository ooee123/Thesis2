
/* 	Code originally give to Prof. Smith by his TA in 1994.
	No idea who wrote it.  Copy and use at your own Risk
*/


#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#define BACKLOG 5


typedef struct chat_header {
	uint16_t packet_length; 
	uint8_t flag; 
} chat_header; 

// for the server side
int tcpServerSetup();
int tcpServerSetup_SpecifiedPort(uint16_t port); 
int tcpAccept(int server_socket);

// for the client side
int tcpClientSetup(char *host_name, char *port);


#endif
