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
#include <errno.h>

ssize_t wrapSend(int sockfd, const void *buf, size_t len, int flags)
{
    int ret = send(sockfd, buf, len, flags);
    if (ret == -1)
    {
        perror("send");
        exit(-1);
    }

    return ret;
}


ssize_t wrapRecv(int sockfd, void *buf, size_t len, int flags)
{
    int ret = recv(sockfd, buf, len, flags);

    if (ret == -1)
    {
        perror("recv");
        fprintf(stderr, "Socket num: %d\n", sockfd);
        exit(-1);
    }

    return ret;
}

int wrapBind(int sockfd, const struct sockaddr *addr,  socklen_t addrlen)
{
    int ret = bind(sockfd, addr, addrlen);

    if (ret == -1)
    {
        perror("bind");
        exit(-1);
    }

    return ret;
}


int wrapSelect(int nfds, fd_set *readfds, fd_set *writefds,
          fd_set *exceptfds, struct timeval *timeout)
{
    int ret = select(nfds, readfds, writefds, exceptfds, timeout);

    if (ret == -1)
    {
        perror("select");
        fprintf(stderr, "Errno: %d", errno);
        exit(-1);
    }

    return ret;
}

