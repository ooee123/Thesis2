

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void sendBadHandle(int a) /* void sendBadHandle ( int socket ) { */{
handleErr b; /* handleErr data ; */
b.flag = 3; /* data . flag = 3 ; */
b.packetLen = htons(3); /* data . packetLen = htons ( 3 ) ; */
if (send(a, (void *)&b, 3, 0)<0) /* if (send(socket, (void *)&data, 3, 0)<0) */ {
perror("Send on Server"); /* perror ( "Send on Server" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void sendHandleACK(int a) /* void sendHandleACK ( int socket ) { */{
handleACK b; /* handleACK data ; */
b.flag = 2; /* data . flag = 2 ; */
b.packetLen = htons(3); /* data . packetLen = htons ( 3 ) ; */
if (send(a, (void *)&b, 3, 0)<0) /* if (send(socket, (void *)&data, 3, 0)<0) */ {
perror("Send on Server"); /* perror ( "Send on Server" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void sendNumClientsPacket(int a, int32_t b) /* void sendNumClientsPacket ( int socket , int32_t numHandles ) { */{
char *c = malloc(7); /* char * data = malloc ( 7 ) ; */
int d = htons(7); /* int length = htons ( 7 ) ; */
int e = 11; /* int flag = 11 ; */
int32_t f = htonl(b); /* int32_t num = htonl ( numHandles ) ; */
memcpy(c, &d, 2); /* memcpy ( data , & length , 2 ) ; */
memcpy(&c[2], &e, 1); /* memcpy ( & data [ 2 ] , & flag , 1 ) ; */
memcpy(&c[3], &f, 4); /* memcpy ( & data [ 3 ] , & num , 4 ) ; */
if (send(a, (void *)c, 7, 0)<0) /* if (send(socket, (void *)data, 7, 0)<0) */ {
perror("Sending number of clients"); /* perror ( "Sending number of clients" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void sendHandle(int a, Handle *b) /* void sendHandle ( int socket , Handle * handle ) { */{
int c = 12; /* int flag = 12 ; */
char *d = malloc(4 + b->length); /* char * data = malloc ( 4 + handle -> length ) ; */
int e = htons(4 + b->length); /* int length = htons ( 4 + handle -> length ) ; */
memcpy(d, &e, 2); /* memcpy ( data , & length , 2 ) ; */
memcpy(&d[2], &c, 1); /* memcpy ( & data [ 2 ] , & flag , 1 ) ; */
memcpy(&d[3], &e, 1); /* memcpy ( & data [ 3 ] , & length , 1 ) ; */
memcpy(&d[4], b->name, b->length); /* memcpy ( & data [ 4 ] , handle -> name , handle -> length ) ; */
if (send(a, (void *)d, 4 + b->length, 0)<0) /* if (send(socket, (void *)data, 4 + handle->length, 0)<0) */ {
perror("Sending handle data"); /* perror ( "Sending handle data" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(d); /* free ( data ) ; */
}
void sendHandlePackets(int a, int32_t b) /* void sendHandlePackets ( int socket , int32_t numHandles ) { */{
int c = 0; /* int ndx = 0 ; */
for (c = 0; c<b; c++) {
Handle *d = getClientData(c); /* Handle * handle = getClientData ( ndx ) ; */
sendHandle(a, d); /* sendHandle ( socket , handle ) ; */
}
}
void sendExitPacket(int a) /* void sendExitPacket ( int socket ) { */{
handleACK b; /* handleACK data ; */
b.flag = 9; /* data . flag = 9 ; */
b.packetLen = htons(3); /* data . packetLen = htons ( 3 ) ; */
if (send(a, (void *)&b, 3, 0)<0) /* if (send(socket, (void *)&data, 3, 0)<0) */ {
perror("Send Exit ACK"); /* perror ( "Send Exit ACK" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void sendBadDest(int a, cclient *b) /* void sendBadDest ( int socketNum , cclient * handle ) { */{
badDest *c = malloc(sizeof(badDest)); /* badDest * packet = malloc ( sizeof ( badDest ) ) ; */
int d = 4 + b->len; /* int size = 4 + handle -> len ; */
unsigned char *e = malloc(d); /* unsigned char * data = malloc ( size ) ; */
c->head.packetLen = htons(d); /* packet -> head . packetLen = htons ( size ) ; */
c->head.flag = 7; /* packet -> head . flag = 7 ; */
c->handle = b; /* packet -> handle = handle ; */
memcpy(e, &c->head, 3); /* memcpy ( data , & packet -> head , 3 ) ; */
memcpy(&e[3], &c->handle->len, 1); /* memcpy ( & data [ 3 ] , & packet -> handle -> len , 1 ) ; */
memcpy(&e[4], c->handle->name, c->handle->len); /* memcpy ( & data [ 4 ] , packet -> handle -> name , packet -> handle -> len ) ; */
if (send(a, e, d, 0)<0) /* if (send(socketNum, data, size, 0)<0) */ {
perror("Send Bad Dest"); /* perror ( "Send Bad Dest" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(e); /* free ( data ) ; */
free(c); /* free ( packet ) ; */
}
void sendMessage(int a, char *b, int c) /* void sendMessage ( int socket , char * buffer , int length ) { */{
unsigned char *d = malloc(c); /* unsigned char * data = malloc ( length ) ; */
memcpy(d, b, c); /* memcpy ( data , buffer , length ) ; */
if (send(a, d, c, 0)<0) /* if (send(socket, data, length, 0)<0) */ {
perror("Send Message"); /* perror ( "Send Message" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(d); /* free ( data ) ; */
}
void constructMessage(Handle *a, cclient *b, char *c, int d) /* void constructMessage ( Handle * destInfo , cclient * srcInfo , char * buffer , int length ) { */{
int e = d - 4; /* int remainingLen = length - 4 ; */
char *f = malloc(d + 1 + a->length); /* char * data = malloc ( length + 1 + destInfo -> length ) ; */
c[2] = 5; /* buffer [ 2 ] = 5 ; */
memcpy(f, c, 3); /* memcpy ( data , buffer , 3 ) ; */
memcpy(&f[3], &a->length, 1); /* memcpy ( & data [ 3 ] , & destInfo -> length , 1 ) ; */
memcpy(&f[4], a->name, a->length); /* memcpy ( & data [ 4 ] , destInfo -> name , destInfo -> length ) ; */
memcpy(&f[4 + a->length], &c[3], e); /* memcpy ( & data [ 4 + destInfo -> length ] , & buffer [ 3 ] , remainingLen ) ; */
if (send(a->socket, f, d + 1 + a->length, 0)<0) /* if (send(destInfo->socket, data, length + 1 + destInfo->length, 0)<0) */ {
perror("Send Message (To all)"); /* perror ( "Send Message (To all)" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(f); /* free ( data ) ; */
}
void sendMessageToAll(char *a, int b, cclient *c) /* void sendMessageToAll ( char * buffer , int length , cclient * srcInfo ) { */{
int d = 0, e = getNumHandles(); /* int ndx = 0 , numHandles = getNumHandles ( ) ; */
for (d = 0; d<e; d++) {
Handle *f = getClientData(d); /* Handle * destInfo = getClientData ( ndx ) ; */
if (strcmp(f->name, c->name)) /* if (strcmp(destInfo->name, srcInfo->name)) */ {
constructMessage(f, c, a, b); /* constructMessage ( destInfo , srcInfo , buffer , length ) ; */
}
}
}
void parseDetails(int a, char *b, int c, int d) /* void parseDetails ( int numBytes , char * buffer , int socket , int clientSocket ) { */{
cclient e; /* cclient srcInfo ; */
uint16_t f; /* uint16_t length ; */
uint8_t g; /* uint8_t flag ; */
int32_t h = 0; /* int32_t numHandles = 0 ; */
cclient i; /* cclient destInfo ; */
int j = 0; /* int destSocket = 0 ; */
memcpy(&f, b, 2); /* memcpy ( & length , buffer , 2 ) ; */
f = ntohs(f); /* length = ntohs ( length ) ; */
memcpy(&g, &b[2], 1); /* memcpy ( & flag , & buffer [ 2 ] , 1 ) ; */
switch (g) /* switch (flag) */ {
case 1: memcpy(&e.len, &b[3], 1); /* memcpy ( & srcInfo . len , & buffer [ 3 ] , 1 ) ; */
memcpy(e.name, &b[4], e.len); /* memcpy ( srcInfo . name , & buffer [ 4 ] , srcInfo . len ) ; */
free(e.name); /* free ( srcInfo . name ) ; */
e.name = malloc(e.len); /* srcInfo . name = malloc ( srcInfo . len ) ; */
if (findHandle(e.name)>0) /* if (findHandle(srcInfo.name)>0) */ {
sendBadHandle(d); /* sendBadHandle ( clientSocket ) ; */
} else {
addHandle(e.name, e.len, d); /* addHandle ( srcInfo . name , srcInfo . len , clientSocket ) ; */
sendHandleACK(d); /* sendHandleACK ( clientSocket ) ; */
}
break;
case 4: memcpy(&e.len, &b[3], 1); /* memcpy ( & srcInfo . len , & buffer [ 3 ] , 1 ) ; */
sendMessageToAll(b, f, &e); /* sendMessageToAll ( buffer , length , & srcInfo ) ; */
memcpy(e.name, &b[4], e.len); /* memcpy ( srcInfo . name , & buffer [ 4 ] , srcInfo . len ) ; */
e.name = malloc(e.len + 1); /* srcInfo . name = malloc ( srcInfo . len + 1 ) ; */
e.name[e.len] = '\0'; /* srcInfo . name [ srcInfo . len ] = '\0' ; */
break;
case 5: memcpy(&i.len, &b[3], 1); /* memcpy ( & destInfo . len , & buffer [ 3 ] , 1 ) ; */
memcpy(i.name, &b[4], i.len); /* memcpy ( destInfo . name , & buffer [ 4 ] , destInfo . len ) ; */
free(i.name); /* free ( destInfo . name ) ; */
i.name = malloc(i.len + 1); /* destInfo . name = malloc ( destInfo . len + 1 ) ; */
i.name[i.len] = '\0'; /* destInfo . name [ destInfo . len ] = '\0' ; */
if (findHandle(i.name) == 0) /* if (findHandle(destInfo.name) == 0) */ {
sendBadDest(d, &i); /* sendBadDest ( clientSocket , & destInfo ) ; */
} else {
j = getSocket(i.name); /* destSocket = getSocket ( destInfo . name ) ; */
if (j == -1) /* if (destSocket == -1) */ {
printf("Something went wrong in handleList!\n"); /* printf ( "Something went wrong in handleList!\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
sendMessage(j, b, f); /* sendMessage ( destSocket , buffer , length ) ; */
}
break;
case 8: sendExitPacket(d); /* sendExitPacket ( clientSocket ) ; */
break;
case 10: h = getNumHandles(); /* numHandles = getNumHandles ( ) ; */
sendHandlePackets(d, h); /* sendHandlePackets ( clientSocket , numHandles ) ; */
sendNumClientsPacket(d, h); /* sendNumClientsPacket ( clientSocket , numHandles ) ; */
break;
default: printf("Bad flag!\n"); /* printf ( "Bad flag!\n" ) ; */
printf("Flag is: %d\n", g); /* printf ( "Flag is: %d\n" , flag ) ; */
break;
}
}
void startServerChat(int a) /* void startServerChat ( int socketNum ) { */{
int b = 0; /* int client_socket = 0 ; */
int c, d = 0; /* int ndx , numFds = 0 ; */
fd_set e, f; /* fd_set useFds , saveFds ; */
FD_ZERO(&f); /* FD_ZERO ( & saveFds ) ; */
FD_SET(a, &f); /* FD_SET ( socketNum , & saveFds ) ; */
while (1) /* while (1) */ {
e = f; /* useFds = saveFds ; */
if (select(1024, &e, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (select(1024, &useFds, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select"); /* perror ( "select" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
for (c = 0; c<1024; c++) {
if (FD_ISSET(c, &e)) /* if (FD_ISSET(ndx, &useFds)) */ {
if (c == a) /* if (ndx == socketNum) */ {
b = tcpAccept(a); /* client_socket = tcpAccept ( socketNum ) ; */
FD_SET(b, &f); /* FD_SET ( client_socket , & saveFds ) ; */
} else {
char *g = malloc(1024); /* char * buffer = malloc ( 1024 ) ; */
int h = read(c, g, 1024); /* int nbytes = read ( ndx , buffer , 1024 ) ; */
if (h<0) /* if (nbytes<0) */ {
perror("read"); /* perror ( "read" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (h == 0) /* if (nbytes == 0) */ {
((void)(((&f )->__fds_bits )[((c ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((c ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( & saveFds ) -> __fds_bits ) [ ( ( ndx ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( ndx ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
removeSocket(c); /* removeSocket ( ndx ) ; */
} else {
parseDetails(h, g, a, c); /* parseDetails ( nbytes , buffer , socketNum , ndx ) ; */
}
free(g); /* free ( buffer ) ; */
}
}
}
}
close(b); /* close ( client_socket ) ; */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
int c = 0; /* int server_socket = 0 ; */
int d = 0; /* int port_number = 0 ; */
if (a == 2) /* if (argc == 2) */ {
d = atoi(b[1]); /* port_number = atoi ( argv [ 1 ] ) ; */
}
c = tcpServerSetup(d); /* server_socket = tcpServerSetup ( port_number ) ; */
startServerChat(c); /* startServerChat ( server_socket ) ; */
close(c); /* close ( server_socket ) ; */
return 0; /* return 0 */
}
