

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
char *c; /* char * buf ; */
int d = 0; /* int client_socket = 0 ; */
int e = 0; /* int message_len = 0 ; */
int f = 0; /* int server_socket = 0 ; */
int g = 1024; /* int buffer_size = 1024 ; */
printf("sockaddr: %d sockaddr_in %d\n", sizeof(struct sockaddr), sizeof(struct sockaddr_in)); /* printf ( "sockaddr: %d sockaddr_in %d\n" , sizeof ( struct sockaddr ) , sizeof ( struct sockaddr_in ) ) ; */
c = (char *)malloc(g); /* buf = ( char * ) malloc ( buffer_size ) ; */
f = tcp_recv_setup(); /* server_socket = tcp_recv_setup ( ) ; */
d = tcp_listen(f, 5); /* client_socket = tcp_listen ( server_socket , 5 ) ; */
if ((e = recv(d, c, g, 0) )<0) /* if ((message_len = recv(client_socket, buf, buffer_size, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("Message received, length: %d\n", e); /* printf ( "Message received, length: %d\n" , message_len ) ; */
printf("Data: %s\n", c); /* printf ( "Data: %s\n" , buf ) ; */
close(f); /* close ( server_socket ) ; */
close(d); /* close ( client_socket ) ; */
}
int tcp_recv_setup() /* int tcp_recv_setup ( ) { */{
int a = 0; /* int server_socket = 0 ; */
struct sockaddr_in b; /* struct sockaddr_in local ; */
socklen_t c = sizeof (b ); /* socklen_t len = sizeof ( local ) ; */
a = socket(2, SOCK_STREAM, 0); /* server_socket = socket ( 2 , SOCK_STREAM , 0 ) ; */
b.sin_family = 2; /* local . sin_family = 2 ; */
b.sin_addr.s_addr = ((in_addr_t)0 ); /* local . sin_addr . s_addr = ( ( in_addr_t ) 0x00000000 ) ; */
if (a<0) /* if (server_socket<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(1); /* exit ( 1 ) ; */
}
b.sin_port = htons(0); /* local . sin_port = htons ( 0 ) ; */
if (bind(a, (struct sockaddr *)&b, sizeof (b ))<0) /* if (bind(server_socket, (struct sockaddr *)&local, sizeof (local ))<0) */ {
perror("bind call"); /* perror ( "bind call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (getsockname(a, (struct sockaddr *)&b, &c)<0) /* if (getsockname(server_socket, (struct sockaddr *)&local, &len)<0) */ {
perror("getsockname call"); /* perror ( "getsockname call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("socket has port %d \n", ntohs(b.sin_port)); /* printf ( "socket has port %d \n" , ntohs ( local . sin_port ) ) ; */
return a; /* return server_socket */
}
int tcp_listen(int a, int b) /* int tcp_listen ( int server_socket , int back_log ) { */{
int c = 0; /* int client_socket = 0 ; */
if (listen(a, b)<0) /* if (listen(server_socket, back_log)<0) */ {
perror("listen call"); /* perror ( "listen call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((c = accept(a, (struct sockaddr *)0, (socklen_t *)0) )<0) /* if ((client_socket = accept(server_socket, (struct sockaddr *)0, (socklen_t *)0) )<0) */ {
perror("accept call"); /* perror ( "accept call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return (c ); /* return (client_socket ) */
}
