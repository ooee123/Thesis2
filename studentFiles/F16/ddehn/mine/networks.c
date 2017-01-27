

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int tcpServerSetup() /* int tcpServerSetup ( ) { */{
struct sockaddr_in a; /* struct sockaddr_in local ; */
int b = 0; /* int server_socket = 0 ; */
socklen_t c = sizeof (a ); /* socklen_t len = sizeof ( local ) ; */
b = socket(2, SOCK_STREAM, 0); /* server_socket = socket ( 2 , SOCK_STREAM , 0 ) ; */
a.sin_family = 2; /* local . sin_family = 2 ; */
a.sin_addr.s_addr = ((in_addr_t)0 ); /* local . sin_addr . s_addr = ( ( in_addr_t ) 0x00000000 ) ; */
if (b<0) /* if (server_socket<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(1); /* exit ( 1 ) ; */
}
a.sin_port = htons(0); /* local . sin_port = htons ( 0 ) ; */
if (bind(b, (struct sockaddr *)&a, sizeof (a ))<0) /* if (bind(server_socket, (struct sockaddr *)&local, sizeof (local ))<0) */ {
perror("bind call"); /* perror ( "bind call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (getsockname(b, (struct sockaddr *)&a, &c)<0) /* if (getsockname(server_socket, (struct sockaddr *)&local, &len)<0) */ {
perror("getsockname call"); /* perror ( "getsockname call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (listen(b, 5)<0) /* if (listen(server_socket, 5)<0) */ {
perror("listen call"); /* perror ( "listen call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("socket has port %d \n", ntohs(a.sin_port)); /* printf ( "socket has port %d \n" , ntohs ( local . sin_port ) ) ; */
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
struct sockaddr_in c; /* struct sockaddr_in remote ; */
int d; /* int socket_num ; */
struct hostent *e; /* struct hostent * hp ; */
c.sin_family = 2; /* remote . sin_family = 2 ; */
if ((d = socket(2, SOCK_STREAM, 0) )<0) /* if ((socket_num = socket(2, SOCK_STREAM, 0) )<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((e = gethostbyname(a) ) == ((void *)0 )) /* if ((hp = gethostbyname(host_name) ) == ((void *)0 )) */ {
printf("Error getting hostname: %s\n", a); /* printf ( "Error getting hostname: %s\n" , host_name ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy((char *)&c.sin_addr, (char *)e->h_addr_list[0], e->h_length); /* memcpy ( ( char * ) & remote . sin_addr , ( char * ) hp -> h_addr_list [ 0 ] , hp -> h_length ) ; */
c.sin_port = htons(atoi(b)); /* remote . sin_port = htons ( atoi ( port ) ) ; */
if (connect(d, (struct sockaddr *)&c, sizeof(struct sockaddr_in))<0) /* if (connect(socket_num, (struct sockaddr *)&remote, sizeof(struct sockaddr_in))<0) */ {
perror("connect call"); /* perror ( "connect call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return d; /* return socket_num */
}
