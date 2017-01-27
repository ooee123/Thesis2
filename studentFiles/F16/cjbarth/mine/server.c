struct chatHeader {
uint16_t length;
uint8_t flag;
};
struct NameTable {
char *name;
};

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int serverSetup(int a, char **b) /* int serverSetup ( int argc , char * * argv ) { */{
unsigned short c; /* unsigned short portnum ; */
struct sockaddr_in d; /* struct sockaddr_in local ; */
int e = 0; /* int server_socket = 0 ; */
socklen_t f = sizeof (d ); /* socklen_t len = sizeof ( local ) ; */
e = socket(2, SOCK_STREAM, 0); /* server_socket = socket ( 2 , SOCK_STREAM , 0 ) ; */
d.sin_family = 2; /* local . sin_family = 2 ; */
d.sin_addr.s_addr = ((in_addr_t)0 ); /* local . sin_addr . s_addr = ( ( in_addr_t ) 0x00000000 ) ; */
if (e<0) /* if (server_socket<0) */ {
perror("socket call\n"); /* perror ( "socket call\n" ) ; */
exit(1); /* exit ( 1 ) ; */
}
if (a == 2) /* if (argc == 2) */ {
c = atoi(b[2 - 1]); /* portnum = atoi ( argv [ 2 - 1 ] ) ; */
d.sin_port = htons(c); /* local . sin_port = htons ( portnum ) ; */
} else {
d.sin_port = htons(0); /* local . sin_port = htons ( 0 ) ; */
}
if (bind(e, (struct sockaddr *)&d, sizeof (d ))<0) /* if (bind(server_socket, (struct sockaddr *)&local, sizeof (local ))<0) */ {
perror("bind call\n"); /* perror ( "bind call\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (getsockname(e, (struct sockaddr *)&d, &f)<0) /* if (getsockname(server_socket, (struct sockaddr *)&local, &len)<0) */ {
perror("getsockname call\n"); /* perror ( "getsockname call\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (listen(e, 5)<0) /* if (listen(server_socket, 5)<0) */ {
perror("listen call"); /* perror ( "listen call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("socket has port %d \n", ntohs(d.sin_port)); /* printf ( "socket has port %d \n" , ntohs ( local . sin_port ) ) ; */
return e; /* return server_socket */
}
int tcpAccept(int a) /* int tcpAccept ( int server_socket ) { */{
int b = 0; /* int client_socket = 0 ; */
if ((b = accept(a, (struct sockaddr *)0, (socklen_t *)0) )<0) /* if ((client_socket = accept(server_socket, (struct sockaddr *)0, (socklen_t *)0) )<0) */ {
perror("accept call"); /* perror ( "accept call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return (b ); /* return (client_socket ) */
}
struct NameTable * tableInit() /* struct NameTable * tableInit ( ) { */{
struct NameTable *a; /* struct NameTable * table ; */
int b = 0; /* int index = 0 ; */
if ((a = malloc(20) ) == ((void *)0 )) /* if ((table = malloc(20) ) == ((void *)0 )) */ {
perror("malloc() error\n"); /* perror ( "malloc() error\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
while (b<20) /* while (index<20) */ {
a[b++].name = ((void *)0 ); /* table [ index ++ ] . name = ( ( void * ) 0 ) ; */
}
return a; /* return table */
}
int getClientName(struct NameTable *a, int *b, int c) /* int getClientName ( struct NameTable * table , int * tableIndex , int socket ) { */{
uint8_t d = 2, e = 3; /* uint8_t pos = 2 , err = 3 ; */
char *f, *g, h[3]; /* char * handle , * src , rsp [ 3 ] ; */
int i = 0, j, k; /* int tempIndex = 0 , rec , srclen ; */
if ((f = (char *)calloc(1, 256 + 3) ) == ((void *)0 )) /* if ((handle = (char *)calloc(1, 256 + 3) ) == ((void *)0 )) */ {
perror("Realloc() error\n"); /* perror ( "Realloc() error\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((j = recv(c, f, 256 + 3, 0) )<0) /* if ((rec = recv(socket, handle, 256 + 3, 0) )<0) */ {
perror("Receive message error in getClientName\n"); /* perror ( "Receive message error in getClientName\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
k = f[3]; /* srclen = handle [ 3 ] ; */
if ((g = (char *)malloc(k + 1) ) == ((void *)0 )) /* if ((src = (char *)malloc(srclen + 1) ) == ((void *)0 )) */ {
perror("malloc get client name"); /* perror ( "malloc get client name" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(g, f + 3 + 1, k); /* memcpy ( src , handle + 3 + 1 , srclen ) ; */
g[k] = '\0'; /* src [ srclen ] = '\0' ; */
while (i<*b) /* while (tempIndex<*tableIndex) */ {
i++; /* tempIndex ++ ; */
if (a[i].name != ((void *)0 )&&strcmp(g, a[i].name) == 0) /* if (table[tempIndex].name != ((void *)0 )&&strcmp(src, table[tempIndex].name) == 0) */ {
memcpy(h + 2, &e, 1); /* memcpy ( rsp + 2 , & err , 1 ) ; */
if (send(c, h, 3, MSG_CONFIRM)<0) /* if (send(socket, rsp, 3, MSG_CONFIRM)<0) */ {
perror("Error on sending error message"); /* perror ( "Error on sending error message" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(f); /* free ( handle ) ; */
free(g); /* free ( src ) ; */
return -1; /* return -1 */
}
}
memcpy(h + 2, &d, 1); /* memcpy ( rsp + 2 , & pos , 1 ) ; */
if (send(c, h, 3, MSG_CONFIRM)<0) /* if (send(socket, rsp, 3, MSG_CONFIRM)<0) */ {
perror("Error on sending pos message"); /* perror ( "Error on sending pos message" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
i = 0; /* tempIndex = 0 ; */
while (a[i].name != ((void *)0 )) /* while (table[tempIndex].name != ((void *)0 )) */ {
i++; /* tempIndex ++ ; */
}
if (i<*b) /* if (tempIndex<*tableIndex) */ {
if ((a[i].name = (char *)malloc(k + 1) ) == ((void *)0 )) /* if ((table[tempIndex].name = (char *)malloc(srclen + 1) ) == ((void *)0 )) */ {
perror("Handle name malloc\n"); /* perror ( "Handle name malloc\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(a[i].name, g, k + 1); /* memcpy ( table [ tempIndex ] . name , src , srclen + 1 ) ; */
} else {
if ((a[*b].name = (char *)malloc(k + 1) ) == ((void *)0 )) /* if ((table[*tableIndex].name = (char *)malloc(srclen + 1) ) == ((void *)0 )) */ {
perror("Handle name malloc\n"); /* perror ( "Handle name malloc\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(a[*b].name, g, k + 1); /* memcpy ( table [ * tableIndex ] . name , src , srclen + 1 ) ; */
*b = *b + 1; /* * tableIndex = * tableIndex + 1 ; */
if (*b == sizeof (a )) /* if (*tableIndex == sizeof (table )) */ {
if (realloc(a, sizeof (a ) * 2) == ((void *)0 )) /* if (realloc(table, sizeof (table ) * 2) == ((void *)0 )) */ {
perror("Realloc() error\n"); /* perror ( "Realloc() error\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
}
free(f); /* free ( handle ) ; */
return 0; /* return 0 */
}
int tableLookUp(struct NameTable *a, int b, int c, char d) /* int tableLookUp ( struct NameTable * table , int tblIdx , int startFd , char message [ ] ) { */{
int e = 3 + 1; /* int index = 3 + 1 ; */
char f[256]; /* char destHandle [ 256 ] ; */
uint8_t g = 0; /* uint8_t destLen = 0 ; */
memcpy(&g, d + 3, 1); /* memcpy ( & destLen , message + 3 , 1 ) ; */
memcpy(f, d + e, g); /* memcpy ( destHandle , message + index , destLen ) ; */
f[g] = '\0'; /* destHandle [ destLen ] = '\0' ; */
e = 0; /* index = 0 ; */
while (e<b) /* while (index<tblIdx) */ {
e++; /* index ++ ; */
if (a[e].name != ((void *)0 )&&strcmp(f, a[e].name) == 0) /* if (table[index].name != ((void *)0 )&&strcmp(destHandle, table[index].name) == 0) */ {
return (c + e ); /* return (startFd + index ) */
}
}
return -1; /* return -1 */
}
void broadcastMsg(int a, int b, int c, char d, uint16_t e) /* void broadcastMsg ( int tblIdx , int client_soc , int startFd , char message [ ] , uint16_t msgLen ) { */{
int f = 0, g; /* int index = 0 , fd ; */
while (f<a) /* while (index<tblIdx) */ {
if (f != (b - c )) /* if (index != (client_soc - startFd )) */ {
g = f + c; /* fd = index + startFd ; */
if (send(g, d, e, MSG_CONFIRM)<0) /* if (send(fd, message, msgLen, MSG_CONFIRM)<0) */ {
perror("Error on send"); /* perror ( "Error on send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
f++; /* index ++ ; */
}
}
void forwardMsg(int a, char b, uint16_t c) /* void forwardMsg ( int handleFd , char message [ ] , uint16_t msgLen ) { */{
if (send(a, b, c, MSG_CONFIRM)<0) /* if (send(handleFd, message, msgLen, MSG_CONFIRM)<0) */ {
perror("Error on send"); /* perror ( "Error on send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void sendErrorMsg(char a, int b) /* void sendErrorMsg ( char message [ ] , int client_soc ) { */{
uint8_t c = 0, d; /* uint8_t destLen = 0 , flag ; */
char e[256], *f; /* char destHandle [ 256 ] , * errPacket ; */
int g = 3 + 1; /* int index = 3 + 1 ; */
uint16_t h, i; /* uint16_t pktLen , temp ; */
memcpy(&c, a + 3, 1); /* memcpy ( & destLen , message + 3 , 1 ) ; */
memcpy(e, a + g, c); /* memcpy ( destHandle , message + index , destLen ) ; */
d = 7; /* flag = 7 ; */
e[c] = '\0'; /* destHandle [ destLen ] = '\0' ; */
if ((f = malloc(c + 3 + 1) ) == ((void *)0 )) /* if ((errPacket = malloc(destLen + 3 + 1) ) == ((void *)0 )) */ {
perror("err Msg malloc"); /* perror ( "err Msg malloc" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
h = c + 3 + 1; /* pktLen = destLen + 3 + 1 ; */
i = htons(h); /* temp = htons ( pktLen ) ; */
h = i; /* pktLen = temp ; */
memcpy(f, a, c + 3 + 1); /* memcpy ( errPacket , message , destLen + 3 + 1 ) ; */
memcpy(f, &h, 2); /* memcpy ( errPacket , & pktLen , 2 ) ; */
memcpy(f + 2, &d, 1); /* memcpy ( errPacket + 2 , & flag , 1 ) ; */
if (send(b, f, ntohs(h), MSG_CONFIRM)<0) /* if (send(client_soc, errPacket, ntohs(pktLen), MSG_CONFIRM)<0) */ {
perror("Error on send"); /* perror ( "Error on send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void ackClientExit(int a) /* void ackClientExit ( int client_soc ) { */{
char b[3]; /* char packet [ 3 ] ; */
struct chatHeader c; /* struct chatHeader hdr ; */
c.flag = 9; /* hdr . flag = 9 ; */
c.length = htons(3); /* hdr . length = htons ( 3 ) ; */
memcpy(b, &c, 3); /* memcpy ( packet , & hdr , 3 ) ; */
if (send(a, b, 3, 0)<0) /* if (send(client_soc, packet, 3, 0)<0) */ {
perror("Error on Exit Send"); /* perror ( "Error on Exit Send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void list(struct NameTable *a, int b, int c) /* void list ( struct NameTable * table , int tblIdx , int client_soc ) { */{
struct chatHeader d; /* struct chatHeader hdr ; */
uint16_t e, f; /* uint16_t length , handleLen ; */
uint32_t g = 0, h; /* uint32_t index = 0 , handles ; */
char *i; /* char * data ; */
char j[3 + sizeof(uint32_t)]; /* char numHandles [ 3 + sizeof ( uint32_t ) ] ; */
d.flag = 11; /* hdr . flag = 11 ; */
d.length = htons(3 + sizeof(uint32_t)); /* hdr . length = htons ( 3 + sizeof ( uint32_t ) ) ; */
h = htonl((unsigned)b); /* handles = htonl ( ( unsigned int ) tblIdx ) ; */
memcpy(j, &d, 3); /* memcpy ( numHandles , & hdr , 3 ) ; */
memcpy(j + 3, &h, sizeof(uint32_t)); /* memcpy ( numHandles + 3 , & handles , sizeof ( uint32_t ) ) ; */
if (send(c, j, 3 + sizeof(uint32_t), 0)<0) /* if (send(client_soc, numHandles, 3 + sizeof(uint32_t), 0)<0) */ {
perror("Error on Exit Send"); /* perror ( "Error on Exit Send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
while (g<b) /* while (index<tblIdx) */ {
g++; /* index ++ ; */
if (a[g].name != ((void *)0 )) /* if (table[index].name != ((void *)0 )) */ {
f = strlen(a[g].name); /* handleLen = strlen ( table [ index ] . name ) ; */
e = 3 + 1 + f; /* length = 3 + 1 + handleLen ; */
if ((i = malloc(e) ) == ((void *)0 )) /* if ((data = malloc(length) ) == ((void *)0 )) */ {
perror("List malloc"); /* perror ( "List malloc" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
d.length = htons(e); /* hdr . length = htons ( length ) ; */
memcpy(i, &d, 3); /* memcpy ( data , & hdr , 3 ) ; */
memcpy(i + 3, &f, 1); /* memcpy ( data + 3 , & handleLen , 1 ) ; */
memcpy(i + 3 + 1, a[g].name, f); /* memcpy ( data + 3 + 1 , table [ index ] . name , handleLen ) ; */
if (send(c, i, e, 0)<0) /* if (send(client_soc, data, length, 0)<0) */ {
perror("Error on Exit Send"); /* perror ( "Error on Exit Send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(i); /* free ( data ) ; */
}
}
}
void messageOpt(struct NameTable *a, int b, int c, int d, char e) /* void messageOpt ( struct NameTable * table , int tblIdx , int client_soc , int startFd , char message [ ] ) { */{
int f; /* int handleFd ; */
uint8_t g; /* uint8_t flag ; */
uint16_t h, i; /* uint16_t length , temp ; */
memcpy(&h, e, 2); /* memcpy ( & length , message , 2 ) ; */
i = ntohs(h); /* temp = ntohs ( length ) ; */
h = i; /* length = temp ; */
memcpy(&g, e + 2, 1); /* memcpy ( & flag , message + 2 , 1 ) ; */
if (g == 4) /* if (flag == 4) */ {
broadcastMsg(b, c, d, e, h); /* broadcastMsg ( tblIdx , client_soc , startFd , message , length ) ; */
} else if (g == 5) /* if (flag == 5) */ {
f = tableLookUp(a, b, d, e); /* handleFd = tableLookUp ( table , tblIdx , startFd , message ) ; */
if (f<0) /* if (handleFd<0) */ {
sendErrorMsg(e, c); /* sendErrorMsg ( message , client_soc ) ; */
} else {
forwardMsg(f, e, h); /* forwardMsg ( handleFd , message , length ) ; */
}
} else if (g == 8) /* if (flag == 8) */ {
ackClientExit(c); /* ackClientExit ( client_soc ) ; */
} else if (g == 10) /* if (flag == 10) */ {
list(a, b, c); /* list ( table , tblIdx , client_soc ) ; */
}
}
void receiveMsg(struct NameTable *a, fd_set *b, int *c, int *d, int *e, int *f, int g) /* void receiveMsg ( struct NameTable * table , fd_set * act , int * tblIdx , int * idx , int * maxFd , int * preFd , int startFd ) { */{
char h[1000]; /* char message [ 1000 ] ; */
int i; /* int rec ; */
if ((i = recv(*d, h, 1000, 0) )<0) /* if ((rec = recv(*idx, message, 1000, 0) )<0) */ {
perror("Receive message error\n"); /* perror ( "Receive message error\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (i == 0) /* if (rec == 0) */ {
if (*d == *e - 1) /* if (*idx == *maxFd - 1) */ {
*e = *f; /* * maxFd = * preFd ; */
*f = *e - 1; /* * preFd = * maxFd - 1 ; */
}
free(a[*d - g].name); /* free ( table [ * idx - startFd ] . name ) ; */
a[*d - g].name = ((void *)0 ); /* table [ * idx - startFd ] . name = ( ( void * ) 0 ) ; */
((void)(((b )->__fds_bits )[((*d ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((*d ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( act ) -> __fds_bits ) [ ( ( * idx ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( * idx ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
close(*d); /* close ( * idx ) ; */
if (*e == g) /* if (*maxFd == startFd) */ {
*c = 0; /* * tblIdx = 0 ; */
} else if (*d - g == *c - 1) /* if (*idx - startFd == *tblIdx - 1) */ {
*c = *c - 1; /* * tblIdx = * tblIdx - 1 ; */
}
} else {
messageOpt(a, *c, *d, g, h); /* messageOpt ( table , * tblIdx , * idx , startFd , message ) ; */
memset(h, 0, 1000); /* memset ( message , 0 , 1000 ) ; */
*d = *e; /* * idx = * maxFd ; */
}
}
int systemControl(int a) /* int systemControl ( int server_socket ) { */{
struct NameTable *b; /* struct NameTable * table ; */
fd_set c, d; /* fd_set active , current ; */
int e = a + 1, f, g, h = 0; /* int maxFd = server_socket + 1 , index , temp_socket , tableIndex = 0 ; */
int i = maxFd, j = a + 1; /* int preFd = maxFd , startFd = server_socket + 1 ; */
FD_ZERO(&c); /* FD_ZERO ( & active ) ; */
FD_SET(a, &c); /* FD_SET ( server_socket , & active ) ; */
b = tableInit(); /* table = tableInit ( ) ; */
while (1) /* while (1) */ {
f = 0; /* index = 0 ; */
d = c; /* current = active ; */
if (select(e, &d, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (select(maxFd, &current, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select\n"); /* perror ( "select\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
while (f<e) /* while (index<maxFd) */ {
if (FD_ISSET(f, &d)) /* if (FD_ISSET(index, &current)) */ {
if (f == a) /* if (index == server_socket) */ {
g = tcpAccept(a); /* temp_socket = tcpAccept ( server_socket ) ; */
if (getClientName(b, &h, g)<0) /* if (getClientName(table, &tableIndex, temp_socket)<0) */ {
close(g); /* close ( temp_socket ) ; */
} else {
FD_SET(g, &c); /* FD_SET ( temp_socket , & active ) ; */
if (g == e) /* if (temp_socket == maxFd) */ {
e = g + 1; /* maxFd = temp_socket + 1 ; */
f = e; /* index = maxFd ; */
}
}
} else {
receiveMsg(b, &c, &h, &f, &e, &i, j); /* receiveMsg ( table , & active , & tableIndex , & index , & maxFd , & preFd , startFd ) ; */
}
}
f++; /* index ++ ; */
}
}
return 0; /* return 0 */
}
int main(int a, char **b) /* int main ( int argc , char * * argv ) { */{
int c = 0; /* int server_socket = 0 ; */
c = serverSetup(a, b); /* server_socket = serverSetup ( argc , argv ) ; */
systemControl(c); /* systemControl ( server_socket ) ; */
return 0; /* return 0 */
}
