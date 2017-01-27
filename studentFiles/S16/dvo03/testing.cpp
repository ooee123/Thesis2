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
#include <vector>

#define send send
#define bind bind
#define recv recv
#define select select


 ssize_t mySend(int sockfd, const void *buf, size_t len, int flags) {
	 ssize_t amount = send(sockfd, buf, len, flags);
	 if(amount < 0 ) {
		 perror("send call");
		 exit(-1);
	 }


	 else
		 return amount;
 }


ssize_t myRecv(int sockfd, void *buf, size_t len, int flags) {
	ssize_t amount = recv(sockfd, buf, len, flags);
	if(amount < 0) {
		perror("recv call");
		exit(-1);
	}


	else
		return amount;
}

 int myBind(int sockfd, const struct sockaddr *addr,  socklen_t addrlen) {
	 int amount = bind(sockfd, addr, addrlen);
	 if(amount < 0) {
		 perror("bind call");
		 exit(-1);
	 }
	 else
		 return amount;

}


 int mySelect(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout) {
	 int amount = select(nfds, readfds, writefds, exceptfds, timeout);

	 if(amount < 0) {
		 perror("select call");
		 exit(-1);
	 }
	 else
		 return amount;

 }

 int client_recv(int sockfd, void *buf, size_t len, int flags) {
	 ssize_t amount = recv(sockfd, buf, len, flags);
	 if(amount < 0) {
		 perror("recv call");
		 exit(-1);
	 }
	 else if(amount == 0) {
		 //kill server
		 printf("Server Terminated\n");
		 close(sockfd);
		 exit(-1);
	 }

	 else
		 return amount;
 }

