#ifndef SYS_H
#define SYS_H
#include <sys/types.h>
#include <sys/socket.h>

int myselect(int nfds, fd_set *readfds, fd_set *writefds,
   fd_set *exceptfds, struct timeval *timeout);

void mysend(int sockfd, const void *buf, size_t len, int flags);

int myrecv(int, void*, size_t, int);

int myaccept(int sockfd, struct sockaddr * addr, socklen_t * addrlen);

void mylisten(int sockfd, int backlog);

#endif