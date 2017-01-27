

unsigned int __bswap_32(unsigned int __bsx) /*  */{
return __builtin_bswap32(__bsx); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t __bsx) /*  */{
return __builtin_bswap64(__bsx); /* return __builtin_bswap64(__bsx) */
}
ssize_t mySend(int sockfd, void *buf, size_t len, int flags) /* ssize_t mySend ( int sockfd , const void * buf , size_t len , int flags ) { */{
return (send(sockfd, buf, len, flags) ); /* return (send(sockfd, buf, len, flags) ) */
}
ssize_t myRecv(int sockfd, void *buf, size_t len, int flags) /* ssize_t myRecv ( int sockfd , void * buf , size_t len , int flags ) { */{
return (recv(sockfd, buf, len, flags) ); /* return (recv(sockfd, buf, len, flags) ) */
}
int myBind(int sockfd, struct sockaddr *addr, socklen_t addrlen) /* int myBind ( int sockfd , const struct sockaddr * addr , socklen_t addrlen ) { */{
return (bind(sockfd, addr, addrlen) ); /* return (bind(sockfd, addr, addrlen) ) */
}
int mySelect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) /* int mySelect ( int nfds , fd_set * readfds , fd_set * writefds , fd_set * exceptfds , struct timeval * timeout ) { */{
return (select(nfds, readfds, writefds, exceptfds, timeout) ); /* return (select(nfds, readfds, writefds, exceptfds, timeout) ) */
}
