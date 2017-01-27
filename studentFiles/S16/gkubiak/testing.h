
#ifndef TESTING_H
#define TESTING_H

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
#include <stddef.h>

ssize_t mySend(int sockfd, const void *buf, size_t len, int flags);
ssize_t myRecv(int sockfd, void *buf, size_t len, int flags);
int myBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int mySelect(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout);

#define send mySend
#define recv myRecv
#define bind myBind
#define select mySelect




#endif
