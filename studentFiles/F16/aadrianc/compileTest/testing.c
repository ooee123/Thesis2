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
#include <sys/select.h>

ssize_t mySend(int a, void *b, size_t c, int d) /* ssize_t mySend ( int sockfd , const void * buf , size_t len , int flags ) { */{
return (send(a, b, c, d) ); /* return (send(sockfd, buf, len, flags) ) */
}
ssize_t myRecv(int a, void *b, size_t c, int d) /* ssize_t myRecv ( int sockfd , void * buf , size_t len , int flags ) { */{
return (recv(a, b, c, d) ); /* return (recv(sockfd, buf, len, flags) ) */
}
int myBind(int a, struct sockaddr *b, socklen_t c) /* int myBind ( int sockfd , const struct sockaddr * addr , socklen_t addrlen ) { */{
return (bind(a, b, c) ); /* return (bind(sockfd, addr, addrlen) ) */
}
int mySelect(int a, fd_set *b, fd_set *c, fd_set *d, struct timeval *e) /* int mySelect ( int nfds , fd_set * readfds , fd_set * writefds , fd_set * exceptfds , struct timeval * timeout ) { */{
return (select(a, b, c, d, e) ); /* return (select(nfds, readfds, writefds, exceptfds, timeout) ) */
}
