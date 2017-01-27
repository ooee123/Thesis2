

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int tcpServerAccept(uint32_t a) /* int tcpServerAccept ( uint32_t server_socket ) { */{
socklen_t b = 0; /* socklen_t len = 0 ; */
int c = 0; /* int client = 0 ; */
c = accept(a, ((void *)0 ), &b); /* client = accept ( server_socket , ( ( void * ) 0 ) , & len ) ; */
if (c<0) /* if (client<0) */ {
perror("accept call"); /* perror ( "accept call" ) ; */
}
return c; /* return client */
}
int tcpServerSetup(uint16_t a) /* int tcpServerSetup ( uint16_t port ) { */{
struct sockaddr_in b; /* struct sockaddr_in addr ; */
int c = 0; /* int status = 0 ; */
int d = 0; /* int sock = 0 ; */
socklen_t e = sizeof (b ); /* socklen_t len = sizeof ( addr ) ; */
d = socket(2, SOCK_STREAM, 0); /* sock = socket ( 2 , SOCK_STREAM , 0 ) ; */
b.sin_family = 2; /* addr . sin_family = 2 ; */
b.sin_addr.s_addr = ((in_addr_t)0 ); /* addr . sin_addr . s_addr = ( ( in_addr_t ) 0x00000000 ) ; */
if (d<3) /* if (sock<3) */ {
perror("server socket call"); /* perror ( "server socket call" ) ; */
exit(-2); /* exit ( - 2 ) ; */
}
b.sin_port = htons(a); /* addr . sin_port = htons ( port ) ; */
c = bind(d, (struct sockaddr *)&b, e); /* status = bind ( sock , ( struct sockaddr * ) & addr , len ) ; */
if (c<0) /* if (status<0) */ {
perror("bind call"); /* perror ( "bind call" ) ; */
exit(-3); /* exit ( - 3 ) ; */
}
c = listen(d, 50); /* status = listen ( sock , 50 ) ; */
if (c<0) /* if (status<0) */ {
perror("listen call"); /* perror ( "listen call" ) ; */
exit(-4); /* exit ( - 4 ) ; */
}
c = getsockname(d, (struct sockaddr *)&b, &e); /* status = getsockname ( sock , ( struct sockaddr * ) & addr , & len ) ; */
if (c<0) /* if (status<0) */ {
perror("getsockname call"); /* perror ( "getsockname call" ) ; */
exit(-8); /* exit ( - 8 ) ; */
}
printf("Server is using port %u\n", ntohs(b.sin_port)); /* printf ( "Server is using port %u\n" , ntohs ( addr . sin_port ) ) ; */
return d; /* return sock */
}
int tcpClientSetup(struct addrinfo *a) /* static int tcpClientSetup ( struct addrinfo * info ) { */{
int b = 0; /* int sock = 0 ; */
int c = 0; /* int status = 0 ; */
struct sockaddr_in *d = (struct sockaddr_in *)(a->ai_addr ); /* struct sockaddr_in * addr = ( struct sockaddr_in * ) ( info -> ai_addr ) ; */
b = socket(2, SOCK_STREAM, 0); /* sock = socket ( 2 , SOCK_STREAM , 0 ) ; */
if (!d) /* if (!addr) */ {
fprintf(stderr, "error\n"); /* fprintf ( stderr , "error\n" ) ; */
exit(-4); /* exit ( - 4 ) ; */
}
if (b<3) /* if (sock<3) */ {
perror("client socket call"); /* perror ( "client socket call" ) ; */
exit(-2); /* exit ( - 2 ) ; */
}
d->sin_family = 2; /* addr -> sin_family = 2 ; */
memset(d->sin_zero, 0, 8); /* memset ( addr -> sin_zero , 0 , 8 ) ; */
c = connect(b, (struct sockaddr *)d, sizeof (*d )); /* status = connect ( sock , ( struct sockaddr * ) addr , sizeof ( * addr ) ) ; */
if (c<0) /* if (status<0) */ {
perror("client connect call"); /* perror ( "client connect call" ) ; */
exit(-3); /* exit ( - 3 ) ; */
}
return b; /* return sock */
}
int dns_lookup(char *a, char *b) /* int dns_lookup ( const char * addr , const char * port ) { */{
int c; /* int sock ; */
struct addrinfo d; /* struct addrinfo base ; */
struct addrinfo *e = ((void *)0 ); /* struct addrinfo * info = ( ( void * ) 0 ) ; */
memset(&d, 0, sizeof (d )); /* memset ( & base , 0 , sizeof ( base ) ) ; */
d.ai_flags = 1; /* base . ai_flags = 0x0001 ; */
d.ai_family = 2; /* base . ai_family = 2 ; */
d.ai_protocol = IPPROTO_TCP; /* base . ai_protocol = IPPROTO_TCP ; */
d.ai_socktype = SOCK_STREAM; /* base . ai_socktype = SOCK_STREAM ; */
if (getaddrinfo(a, b, &d, &e)) /* if (getaddrinfo(addr, port, &base, &info)) */ {
fprintf(stderr, "Could not resolve \"%s\"\n", a); /* fprintf ( stderr , "Could not resolve \"%s\"\n" , addr ) ; */
exit(-5); /* exit ( - 5 ) ; */
}
c = tcpClientSetup(e); /* sock = tcpClientSetup ( info ) ; */
freeaddrinfo(e); /* freeaddrinfo ( info ) ; */
return c; /* return sock */
}
