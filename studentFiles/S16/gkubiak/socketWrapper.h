#ifndef __WRAPSOCKET_H__
#define __WRAPSOCKET_H__

ssize_t wrapSend(int sockfd, const void *buf, size_t len, int flags);
ssize_t wrapRecv(int sockfd, void *buf, size_t len, int flags);
int wrapBind(int sockfd, const struct sockaddr *addr,  socklen_t addrlen);
int wrapSelect(int nfds, fd_set *readfds, fd_set *writefds,
          fd_set *exceptfds, struct timeval *timeout);

#endif
