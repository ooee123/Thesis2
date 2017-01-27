

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int clientInit(char *a, char *b) /* int clientInit ( char * serverName , char * portNum ) { */{
struct sockaddr_in c; /* struct sockaddr_in svr ; */
int d; /* int sd ; */
struct hostent *e = ((void *)0 ); /* struct hostent * host = ( ( void * ) 0 ) ; */
c.sin_family = 2; /* svr . sin_family = 2 ; */
if ((d = socket(2, SOCK_STREAM, 0) )<0) /* if ((sd = socket(2, SOCK_STREAM, 0) )<0) */ {
perror("Error creating socket: "); /* perror ( "Error creating socket: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((e = gethostbyname(a) ) == ((void *)0 )) /* if ((host = gethostbyname(serverName) ) == ((void *)0 )) */ {
printf("Error getting server address: %s\n", a); /* printf ( "Error getting server address: %s\n" , serverName ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy((char *)&c.sin_addr, (char *)e->h_addr_list[0], e->h_length); /* memcpy ( ( char * ) & svr . sin_addr , ( char * ) host -> h_addr_list [ 0 ] , host -> h_length ) ; */
c.sin_port = htons(atoi(b)); /* svr . sin_port = htons ( atoi ( portNum ) ) ; */
if (connect(d, (struct sockaddr *)&c, sizeof(struct sockaddr_in))<0) /* if (connect(sd, (struct sockaddr *)&svr, sizeof(struct sockaddr_in))<0) */ {
perror("Error connecting client to server: "); /* perror ( "Error connecting client to server: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return d; /* return sd */
}
int parseClientArgs(int a, char *b, char **c) /* int parseClientArgs ( int argc , char * argv [ ] , char * * handle ) { */{
if (a != 4) /* if (argc != 4) */ {
printf("Error in parameters:\t\"./cclient %s [server-name] [port-number]\"\n", b[0]); /* printf ( "Error in parameters:\t\"./cclient %s [server-name] [port-number]\"\n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
}
*c = b[1]; /* * handle = argv [ 1 ] ; */
return clientInit(b[2], b[3]); /* return clientInit(argv[2], argv[3]) */
}
int serverInit(char *a) /* int serverInit ( char * portNum ) { */{
struct sockaddr_in b; /* struct sockaddr_in svr ; */
int c; /* int sd ; */
socklen_t d; /* socklen_t addrLen ; */
b.sin_family = 2; /* svr . sin_family = 2 ; */
b.sin_addr.s_addr = ((in_addr_t)0 ); /* svr . sin_addr . s_addr = ( ( in_addr_t ) 0x00000000 ) ; */
if ((c = socket(2, SOCK_STREAM, 0) )<0) /* if ((sd = socket(2, SOCK_STREAM, 0) )<0) */ {
perror("Error creating socket: "); /* perror ( "Error creating socket: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
b.sin_port = htons(0); /* svr . sin_port = htons ( 0 ) ; */
if (bind(c, (struct sockaddr *)&b, sizeof (b ))<0) /* if (bind(sd, (struct sockaddr *)&svr, sizeof (svr ))<0) */ {
perror("Error binding server: "); /* perror ( "Error binding server: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
d = sizeof (b ); /* addrLen = sizeof ( svr ) ; */
if (getsockname(c, (struct sockaddr *)&b, &d)<0) /* if (getsockname(sd, (struct sockaddr *)&svr, &addrLen)<0) */ {
perror("Error getting socket name: "); /* perror ( "Error getting socket name: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (listen(c, 5)<0) /* if (listen(sd, 5)<0) */ {
perror("Error in listen call: "); /* perror ( "Error in listen call: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("Server socket has port number: %d\n", ntohs(b.sin_port)); /* printf ( "Server socket has port number: %d\n" , ntohs ( svr . sin_port ) ) ; */
return c; /* return sd */
}
int serverAccept(int a) /* int serverAccept ( int server_socket ) { */{
int b; /* int client_socket ; */
if ((b = accept(a, (struct sockaddr *)0, (socklen_t *)0) )<0) /* if ((client_socket = accept(server_socket, (struct sockaddr *)0, (socklen_t *)0) )<0) */ {
perror("Error accepting client: "); /* perror ( "Error accepting client: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return b; /* return client_socket */
}
int parseServerArgs(int a, char *b) /* int parseServerArgs ( int argc , char * argv [ ] ) { */{
if (a == 2) /* if (argc == 2) */ {
return serverInit(b[1]); /* return serverInit(argv[1]) */
}
if (a<1&&a>2) /* if (argc<1&&argc>2) */ {
printf("Error in parameters:\t\"./server [optional-port-number]\"\n"); /* printf ( "Error in parameters:\t\"./server [optional-port-number]\"\n" ) ; */
exit(1); /* exit ( 1 ) ; */
}
return serverInit(((void *)0 )); /* return serverInit(((void *)0 )) */
}
