

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int tcpServerSetup(char *a) /* int tcpServerSetup ( char * port ) { */{
int b = 0; /* int server_socket = 0 ; */
struct sockaddr_in c; /* struct sockaddr_in local ; */
socklen_t d = sizeof (c ); /* socklen_t len = sizeof ( local ) ; */
b = socket(2, SOCK_STREAM, 0); /* server_socket = socket ( 2 , SOCK_STREAM , 0 ) ; */
c.sin_family = 2; /* local . sin_family = 2 ; */
c.sin_addr.s_addr = ((in_addr_t)0 ); /* local . sin_addr . s_addr = ( ( in_addr_t ) 0x00000000 ) ; */
if (b<0) /* if (server_socket<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(1); /* exit ( 1 ) ; */
}
if (a != ((void *)0 )) /* if (port != ((void *)0 )) */ {
c.sin_port = htons(atoi(a)); /* local . sin_port = htons ( atoi ( port ) ) ; */
} else {
c.sin_port = htons(0); /* local . sin_port = htons ( 0 ) ; */
}
if (bind(b, (struct sockaddr *)&c, sizeof (c ))<0) /* if (bind(server_socket, (struct sockaddr *)&local, sizeof (local ))<0) */ {
perror("bind call"); /* perror ( "bind call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (getsockname(b, (struct sockaddr *)&c, &d)<0) /* if (getsockname(server_socket, (struct sockaddr *)&local, &len)<0) */ {
perror("getsockname call"); /* perror ( "getsockname call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (listen(b, 5)<0) /* if (listen(server_socket, 5)<0) */ {
perror("listen call"); /* perror ( "listen call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("socket has port %d \n", ntohs(c.sin_port)); /* printf ( "socket has port %d \n" , ntohs ( local . sin_port ) ) ; */
return b; /* return server_socket */
}
int tcpAccept(int a) /* int tcpAccept ( int server_socket ) { */{
int b = 0; /* int client_socket = 0 ; */
if ((b = accept(a, (struct sockaddr *)0, (socklen_t *)0) )<0) /* if ((client_socket = accept(server_socket, (struct sockaddr *)0, (socklen_t *)0) )<0) */ {
perror("accept call"); /* perror ( "accept call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return (b ); /* return (client_socket ) */
}
int tcpClientSetup(char *a, char *b) /* int tcpClientSetup ( char * host_name , char * port ) { */{
struct hostent *c; /* struct hostent * hp ; */
struct sockaddr_in d; /* struct sockaddr_in remote ; */
int e; /* int socket_num ; */
d.sin_family = 2; /* remote . sin_family = 2 ; */
if ((e = socket(2, SOCK_STREAM, 0) )<0) /* if ((socket_num = socket(2, SOCK_STREAM, 0) )<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((c = gethostbyname(a) ) == ((void *)0 )) /* if ((hp = gethostbyname(host_name) ) == ((void *)0 )) */ {
printf("Error getting hostname: %s\n", a); /* printf ( "Error getting hostname: %s\n" , host_name ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy((char *)&d.sin_addr, (char *)c->h_addr_list[0], c->h_length); /* memcpy ( ( char * ) & remote . sin_addr , ( char * ) hp -> h_addr_list [ 0 ] , hp -> h_length ) ; */
d.sin_port = htons(atoi(b)); /* remote . sin_port = htons ( atoi ( port ) ) ; */
if (connect(e, (struct sockaddr *)&d, sizeof(struct sockaddr_in))<0) /* if (connect(socket_num, (struct sockaddr *)&remote, sizeof(struct sockaddr_in))<0) */ {
perror("connect call"); /* perror ( "connect call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return e; /* return socket_num */
}
