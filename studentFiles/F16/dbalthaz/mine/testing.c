

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
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
