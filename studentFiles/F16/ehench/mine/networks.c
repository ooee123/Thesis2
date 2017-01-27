

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int32_t tcpServerSetup() /* int32_t tcpServerSetup ( ) { */{
int32_t a = 0; /* int32_t server_socket = 0 ; */
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
if (myBind(a, (struct sockaddr *)&b, sizeof (b ))<0) /* if (myBind(server_socket, (struct sockaddr *)&local, sizeof (local ))<0) */ {
perror("bind call"); /* perror ( "bind call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (getsockname(a, (struct sockaddr *)&b, &c)<0) /* if (getsockname(server_socket, (struct sockaddr *)&local, &len)<0) */ {
perror("getsockname call"); /* perror ( "getsockname call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (listen(a, 5)<0) /* if (listen(server_socket, 5)<0) */ {
perror("listen call"); /* perror ( "listen call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("Server is using port %d \n", ntohs(b.sin_port)); /* printf ( "Server is using port %d \n" , ntohs ( local . sin_port ) ) ; */
return a; /* return server_socket */
}
int32_t tcpAccept(int32_t a) /* int32_t tcpAccept ( int32_t server_socket ) { */{
int32_t b = 0; /* int32_t client_socket = 0 ; */
if ((b = accept(a, (struct sockaddr *)0, (socklen_t *)0) )<0) /* if ((client_socket = accept(server_socket, (struct sockaddr *)0, (socklen_t *)0) )<0) */ {
perror("accept call"); /* perror ( "accept call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return b; /* return client_socket */
}
int32_t tcp_select(fd_set *a, int32_t b, struct timeval c) /* int32_t tcp_select ( fd_set * socket_fds , int32_t max , struct timeval time ) { */{
int32_t d; /* int32_t ret_sockets ; */
if ((d = mySelect(b + 1, a, (fd_set *)0, (fd_set *)0, &c) )<0) /* if ((ret_sockets = mySelect(max + 1, socket_fds, (fd_set *)0, (fd_set *)0, &time) )<0) */ {
perror("accept call"); /* perror ( "accept call" ) ; */
exit(1); /* exit ( 1 ) ; */
}
return d; /* return ret_sockets */
}
int32_t tcpClientSetup(char *a, char *b) /* int32_t tcpClientSetup ( char * host_name , char * port ) { */{
struct sockaddr_in c; /* struct sockaddr_in remote ; */
int32_t d; /* int32_t socket_num ; */
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
memcpy((int8_t *)&c.sin_addr, (int8_t *)e->h_addr_list[0], e->h_length); /* memcpy ( ( int8_t * ) & remote . sin_addr , ( int8_t * ) hp -> h_addr_list [ 0 ] , hp -> h_length ) ; */
c.sin_port = htons(atoi(b)); /* remote . sin_port = htons ( atoi ( port ) ) ; */
if (connect(d, (struct sockaddr *)&c, sizeof(struct sockaddr_in))<0) /* if (connect(socket_num, (struct sockaddr *)&remote, sizeof(struct sockaddr_in))<0) */ {
perror("connect call"); /* perror ( "connect call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return d; /* return socket_num */
}
void chat_header_setup(int8_t **a, int8_t b, int16_t c) /* void chat_header_setup ( int8_t * * send_buf , int8_t flag , int16_t packet_len ) { */{
ChatHeader d; /* ChatHeader hdr ; */
d.flag = b; /* hdr . flag = flag ; */
d.packet_len = htons(c); /* hdr . packet_len = htons ( packet_len ) ; */
memcpy((void *)*a, (void *)&d, 3); /* memcpy ( ( void * ) * send_buf , ( void * ) & hdr , 3 ) ; */
}
void reset(int8_t *a, int32_t b) /* void reset ( int8_t * buf , int32_t message_len ) { */{
int c; /* int count ; */
for (c = 0; c<b; c++) {
a[c] = 0; /* buf [ count ] = 0 ; */
}
}
void create_buffer(int8_t **a, int32_t b) /* void create_buffer ( int8_t * * buf , int32_t size ) { */{
if ((*a = (int8_t *)calloc(1, b) ) == ((void *)0 )) /* if ((*buf = (int8_t *)calloc(1, size) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(1); /* exit ( 1 ) ; */
}
}
void send_data(int8_t **a, int32_t b, int16_t c) /* void send_data ( int8_t * * send_buf , int32_t socket , int16_t packet_len ) { */{
int32_t d; /* int32_t sent ; */
int8_t *e = *a; /* int8_t * temp = * send_buf ; */
d = mySend(b, e, c, 0); /* sent = mySend ( socket , temp , packet_len , 0 ) ; */
if (d<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(1); /* exit ( 1 ) ; */
}
}
int8_t check_flag(int32_t a, int8_t *b) /* int8_t check_flag ( int32_t message_len , int8_t * buf ) { */{
printf("Incoming Packet:\n"); /* printf ( "Incoming Packet:\n" ) ; */
printf("\tPacket Length: %d\n", ntohs(*(int16_t *)b)); /* printf ( "\tPacket Length: %d\n" , ntohs ( * ( int16_t * ) buf ) ) ; */
printf("\tFlag: %d\n", b[2]); /* printf ( "\tFlag: %d\n" , buf [ 2 ] ) ; */
return b[2]; /* return buf[2] */
}
int8_t check_socket(int8_t *a, Client *b, int8_t c) /* int8_t check_socket ( int8_t * buf , Client * socket , int8_t client_or_server ) { */{
int32_t d = 0; /* int32_t message_len = 0 ; */
int8_t e = 0; /* int8_t flag = 0 ; */
if ((d = myRecv(b->socket, a, 1620, 0) )<0) /* if ((message_len = myRecv(socket->socket, buf, 1620, 0) )<0) */ {
if (1 == c) /* if (1 == client_or_server) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(1); /* exit ( 1 ) ; */
} else {
fprintf(stderr, "Server Terminated\n"); /* fprintf ( stderr , "Server Terminated\n" ) ; */
exit(1); /* exit ( 1 ) ; */
}
} else if (0 == d) /* if (0 == message_len) */ {
b->handle_len = 0; /* socket -> handle_len = 0 ; */
if (b->handle != ((void *)0 )) /* if (socket->handle != ((void *)0 )) */ {
free(b->handle); /* free ( socket -> handle ) ; */
}
b->handle = ((void *)0 ); /* socket -> handle = ( ( void * ) 0 ) ; */
b->socket = 0; /* socket -> socket = 0 ; */
} else {
e = a[2]; /* flag = buf [ 2 ] ; */
}
return e; /* return flag */
}
