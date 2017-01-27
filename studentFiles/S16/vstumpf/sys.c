#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>

#include "sys.h"

int myselect(int nfds, fd_set *readfds, fd_set *writefds,
   fd_set *exceptfds, struct timeval *timeout) {
   int ret = select(nfds, readfds, writefds, exceptfds, timeout);
   if (ret < 0) {
      perror("select");
      exit(-1);
   }
   return ret;
}

void mysend(int sockfd, const void *buf, size_t len, int flags) {
   int fd = send(sockfd, buf, len, flags);
   if (fd < 0) {
      perror("send");
      exit(-1);
   }
}

int myrecv(int socket, void *buffer, size_t len, int flags) {
   int ret = recv(socket, buffer, len, flags);
   if (ret < 0) {
      perror("recv");
      exit(-1);
   }
   return ret;
}

int myaccept(int sockfd, struct sockaddr *addr, socklen_t * addrlen) {
   int ret = accept(sockfd, addr, addrlen);
   if (ret < 0) {
      perror("accept");
      exit(-1);
   }
   return ret;
}

void mylisten(int sockfd, int backlog) {
   int ret = listen(sockfd, backlog);
   if (ret < 0) {
      perror("listen");
      exit(-1);
   }
}