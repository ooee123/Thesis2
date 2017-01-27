#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
//#include <sys/stat.h>
//#include <sys/uio.h>
#include <sys/time.h>
//#include <unistd.h>
//#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "lib.h"
#include "connection.h"

int tcpServerAccept(uint32_t server_socket) {
   socklen_t len = 0;
   int client = 0;

   client = accept(server_socket, NULL, &len);
   if(client < 0) {
      perror("accept call");
   }

   return client;
}

int tcpServerSetup(uint16_t port) {
   int sock = 0;
   int status = 0;
   struct sockaddr_in addr;
   socklen_t len = sizeof(addr);

   sock = socket(AF_INET, SOCK_STREAM, 0);
   if(sock < 3) {
      perror("server socket call");
      exit(-2);
   }
   
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   addr.sin_addr.s_addr = INADDR_ANY;

   status = bind(sock, (struct sockaddr *) &addr, len);
   if(status < 0) {
      perror("bind call");
      exit(-3);
   }

   status = listen(sock, BACKLOG);
   if(status < 0) {
      perror("listen call");
      exit(-4);
   }

   status = getsockname(sock, (struct sockaddr *) &addr, &len);
   if(status < 0) {
      perror("getsockname call");
      exit(-8);
   }

   printf("Server is using port %u\n", ntohs(addr.sin_port));

   return sock;
}

static int tcpClientSetup(struct addrinfo* info) {
   int sock = 0;
   int status = 0;
   struct sockaddr_in* addr = (struct sockaddr_in*)(info->ai_addr);

   sock = socket(AF_INET, SOCK_STREAM, 0);
   if(sock < 3) {
      perror("client socket call");
      exit(-2);
   }

   if(!addr) {
      fprintf(stderr, "error\n");
      exit(-4);
   }

   addr->sin_family = AF_INET;
   memset(addr->sin_zero, 0, 8);

   status = connect(sock, (struct sockaddr*)addr, sizeof(*addr));
   if(status < 0) {
      perror("client connect call");
      exit(-3);
   }

   return sock;
}

int dns_lookup(const char* addr, const char* port) {
   struct addrinfo* info = NULL;
   struct addrinfo base;
   int sock;

   memset(&base, 0, sizeof(base));

   base.ai_family = AF_INET;
   base.ai_socktype = SOCK_STREAM;
   base.ai_protocol = IPPROTO_TCP;
   base.ai_flags = AI_PASSIVE;

   if(getaddrinfo(addr, port, &base, &info)) {
      fprintf(stderr, "Could not resolve \"%s\"\n", addr);
      exit(-5);
   }


   sock = tcpClientSetup(info);
   freeaddrinfo(info);

   return sock;
}
