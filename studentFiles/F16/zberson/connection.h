#ifndef __CONNECTION_H
#define __CONNECTION_H

#define BACKLOG 50

#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//Server side
int tcpServerAccept(uint32_t server_socket);
int tcpServerSetup(uint16_t port);


// Client side
int dns_lookup(const char* addr, const char* port);



#endif
