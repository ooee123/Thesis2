
/*    Code originally give to Prof. Smith by his TA in 1994.
   No idea who wrote it.  Copy and use at your own Risk
*/


#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#define BACKLOG 5

// for the server side
int tcpServerSetup(unsigned short port);
int tcpAccept(int server_socket);

// for the client side
int tcpClientSetup(char *host_name, char *port);

// send and recv for server/client
void send_pkt(int socket, void *buffer, int size);
int recv_pkt(int socket, void *buffer, int size);

#endif