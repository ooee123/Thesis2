int global_b = 0; /* static int numHandles = 0 ; */
int global_a = 5; /* static int maxNumHandles = 5 ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void add_handle(my_socket **a, char *b, uint32_t c, fd_set *d) /* void add_handle ( my_socket * * head_ptr , char * handle , uint32_t socketNum , fd_set * rFds ) { */{
my_socket *e = *a; /* my_socket * head = * head_ptr ; */
my_socket *f = (e + global_b ); /* my_socket * next = ( head + numHandles ) ; */
f->handle = calloc(strlen(b) + 1, sizeof(char)); /* next -> handle = calloc ( strlen ( handle ) + 1 , sizeof ( char ) ) ; */
strcpy(f->handle, b); /* strcpy ( next -> handle , handle ) ; */
memcpy(&(f->socketNum ), &c, sizeof (c )); /* memcpy ( & ( next -> socketNum ) , & socketNum , sizeof ( socketNum ) ) ; */
if (++global_b>=global_a) /* if (++numHandles>=maxNumHandles) */ {
global_a = global_a * 2; /* maxNumHandles = maxNumHandles * 2 ; */
*a = realloc(e, (global_a * 2 ) * sizeof(my_socket)); /* * head_ptr = realloc ( head , ( maxNumHandles * 2 ) * sizeof ( my_socket ) ) ; */
}
FD_SET(c, d); /* FD_SET ( socketNum , rFds ) ; */
}
int sendPacket(char *a, int b, int c) /* int sendPacket ( char * pkt , int socketNum , int needFree ) { */{
int d; /* int totalSent ; */
uint8_t e; /* uint8_t handleLen ; */
uint16_t f = ntohs(((chat_header *)a )->len); /* uint16_t packetLen = ntohs ( ( ( chat_header * ) pkt ) -> len ) ; */
uint8_t g = ((chat_header *)a )->flag; /* uint8_t flag = ( ( chat_header * ) pkt ) -> flag ; */
if (c) /* if (needFree) */ {
free(a); /* free ( pkt ) ; */
}
if (g == 12) /* if (flag == 12) */ {
memcpy(&e, a + 3, sizeof(uint8_t)); /* memcpy ( & handleLen , pkt + 3 , sizeof ( uint8_t ) ) ; */
f = 3 + sizeof(uint8_t) + e; /* packetLen = 3 + sizeof ( uint8_t ) + handleLen ; */
}
d = send(b, a, f, 0); /* totalSent = send ( socketNum , pkt , packetLen , 0 ) ; */
return (f - d ); /* return (packetLen - totalSent ) */
}
char * createPacket(char *a, uint8_t b, uint16_t c, int d) /* char * createPacket ( char * text , uint8_t flag , uint16_t size , int needFree ) { */{
char *e = calloc(c, sizeof(char)); /* char * packet = calloc ( size , sizeof ( char ) ) ; */
chat_header *f = (chat_header *)e; /* chat_header * pckt_hdr = ( chat_header * ) packet ; */
f->len = htons(c); /* pckt_hdr -> len = htons ( size ) ; */
f->flag = b; /* pckt_hdr -> flag = flag ; */
if (a) /* if (text) */ {
memcpy(e + 3, a, c + 1 - 3); /* memcpy ( packet + 3 , text , size - 3 + 1 ) ; */
if (d) /* if (needFree) */ {
free(a); /* free ( text ) ; */
}
}
return e; /* return packet */
}
int exists_handle(my_socket *a, char *b) /* int exists_handle ( my_socket * head , char * handle ) { */{
int c; /* int i ; */
for (c = 0; c<global_b; c++) {
if (strcmp((a + c )->handle, b) == 0) /* if (strcmp((head + i )->handle, handle) == 0) */ {
return c; /* return i */
}
}
return -1; /* return -1 */
}
void remove_handle(my_socket *a, char *b, fd_set *c) /* void remove_handle ( my_socket * head , char * handle , fd_set * rFds ) { */{
int d; /* int i ; */
for (d = 0; d<global_b; d++) {
if (strcmp((a + d )->handle, b) == 0) /* if (strcmp((head + i )->handle, handle) == 0) */ {
((void)(((c )->__fds_bits )[(((a + d )->socketNum ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << (((a + d )->socketNum ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( rFds ) -> __fds_bits ) [ ( ( ( head + i ) -> socketNum ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( ( head + i ) -> socketNum ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
memmove(a + d, a + d + 1, sizeof(my_socket) * (global_b - d )); /* memmove ( head + i , head + i + 1 , sizeof ( my_socket ) * ( numHandles - i ) ) ; */
global_b--; /* numHandles -- ; */
return; /* return */
}
}
}
void remove_handle_by_socket(my_socket *a, int b, fd_set *c) /* void remove_handle_by_socket ( my_socket * head , int socketNum , fd_set * rFds ) { */{
int d; /* int i ; */
for (d = 0; d<global_b; d++) {
if ((a + d )->socketNum == b) /* if ((head + i )->socketNum == socketNum) */ {
((void)(((c )->__fds_bits )[(((a + d )->socketNum ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << (((a + d )->socketNum ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( rFds ) -> __fds_bits ) [ ( ( ( head + i ) -> socketNum ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( ( head + i ) -> socketNum ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
memmove(a + d, a + d + 1, sizeof(my_socket) * (global_b - d )); /* memmove ( head + i , head + i + 1 , sizeof ( my_socket ) * ( numHandles - i ) ) ; */
global_b--; /* numHandles -- ; */
return; /* return */
}
}
}
void makeHandleData(char *a, char *b, uint8_t c) /* void makeHandleData ( char * buf , char * handle , uint8_t len ) { */{
memcpy(a, &c, sizeof (c )); /* memcpy ( buf , & len , sizeof ( len ) ) ; */
memcpy(a + sizeof (c ), b, c); /* memcpy ( buf + sizeof ( len ) , handle , len ) ; */
}
int listenSockets(fd_set *a, int b) /* int listenSockets ( fd_set * readFds , int server_socket ) { */{
int c; /* int i ; */
if (select(1024, a, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (select(1024, readFds, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select"); /* perror ( "select" ) ; */
}
if (FD_ISSET(b, a)) /* if (FD_ISSET(server_socket, readFds)) */ {
return tcpAccept(b); /* return tcpAccept(server_socket) */
} else {
for (c = 0; c<1024; c++) {
if (FD_ISSET(c, a)&&c != b) /* if (FD_ISSET(i, readFds)&&i != server_socket) */ {
return c; /* return i */
}
}
}
return -1; /* return -1 */
}
void sockListen(int a) /* void sockListen ( int socketNum ) { */{
if (listen(a, 5)<0) /* if (listen(socketNum, 5)<0) */ {
perror("listen call"); /* perror ( "listen call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void broadcastMessage(char *a, my_socket *b, int c) /* void broadcastMessage ( char * buf , my_socket * all_handles , int client_socket ) { */{
char d[255]; /* char srcHandle [ 255 ] ; */
int e, f; /* int msgLen , i ; */
uint8_t g; /* uint8_t handleLen ; */
memcpy(&g, (a + 3 ), sizeof (g )); /* memcpy ( & handleLen , ( buf + 3 ) , sizeof ( handleLen ) ) ; */
memcpy(&d, a + 3 + sizeof (g ), g); /* memcpy ( & srcHandle , buf + 3 + sizeof ( handleLen ) , handleLen ) ; */
d[g] = '\0'; /* srcHandle [ handleLen ] = '\0' ; */
e = strlen(a + 3 + sizeof (g ) + g); /* msgLen = strlen ( buf + 3 + sizeof ( handleLen ) + handleLen ) ; */
for (f = 0; f<global_b; f++) {
if (strcmp((b + f )->handle, d) != 0) /* if (strcmp((all_handles + i )->handle, srcHandle) != 0) */ {
if (sendPacket(a, (b + f )->socketNum, 0)) /* if (sendPacket(buf, (all_handles + i )->socketNum, 0)) */ {
perror("send()"); /* perror ( "send()" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
}
}
void sendReadyToListHandlesAck(int a) /* void sendReadyToListHandlesAck ( int client_socket ) { */{
uint32_t *b = malloc(sizeof(uint32_t)); /* uint32_t * totalHandles = malloc ( sizeof ( uint32_t ) ) ; */
char *c; /* char * pkt ; */
*b = htonl(global_b); /* * totalHandles = htonl ( numHandles ) ; */
c = createPacket((char *)b, 11, 3 + sizeof(uint32_t), 0); /* pkt = createPacket ( ( char * ) totalHandles , 11 , 3 + sizeof ( uint32_t ) , 0 ) ; */
if (sendPacket(c, a, 1)) /* if (sendPacket(pkt, client_socket, 1)) */ {
perror("sendReadyToListHandlesAck:send"); /* perror ( "sendReadyToListHandlesAck:send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void sendListHandles(int a, my_socket *b) /* void sendListHandles ( int client_socket , my_socket * all_handles ) { */{
char *c, d[255 + 1]; /* char * pkt , buf [ 255 + 1 ] ; */
int e; /* int i ; */
uint8_t f; /* uint8_t handleLen ; */
for (e = 0; e<global_b; e++) {
f = strlen((b + e )->handle); /* handleLen = strlen ( ( all_handles + i ) -> handle ) ; */
makeHandleData(d, (b + e )->handle, f); /* makeHandleData ( buf , ( all_handles + i ) -> handle , handleLen ) ; */
c = createPacket(d, 12, 3 + sizeof(uint8_t) + f, 0); /* pkt = createPacket ( buf , 12 , 3 + sizeof ( uint8_t ) + handleLen , 0 ) ; */
((chat_header *)c )->len = 0; /* ( ( chat_header * ) pkt ) -> len = 0 ; */
sendPacket(c, a, 1); /* sendPacket ( pkt , client_socket , 1 ) ; */
}
}
void sendExitAck(int a) /* void sendExitAck ( int client_socket ) { */{
char *b = createPacket(((void *)0 ), 9, 3, 0); /* char * pkt = createPacket ( ( ( void * ) 0 ) , 9 , 3 , 0 ) ; */
if (sendPacket(b, a, 1)) /* if (sendPacket(pkt, client_socket, 1)) */ {
perror("runClient:L:send"); /* perror ( "runClient:L:send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void action(char *a, my_socket **b, int c, fd_set *d) /* void action ( char * buf , my_socket * * all_handles_ptr , int client_socket , fd_set * readFds ) { */{
int e; /* int handleOffset ; */
char f[255]; /* char handle [ 255 ] ; */
char *g; /* char * sending_pkt ; */
packet *h = (packet *)a; /* packet * pkt = ( packet * ) buf ; */
my_socket *i = *b; /* my_socket * all_handles = * all_handles_ptr ; */
uint8_t j = (h->header ).flag, k; /* uint8_t flag = ( pkt -> header ) . flag , handleLen ; */
if (j == 1) /* if (flag == 1) */ {
memcpy(&k, (a + 3 ), sizeof (k )); /* memcpy ( & handleLen , ( buf + 3 ) , sizeof ( handleLen ) ) ; */
memcpy(f, a + 3 + sizeof (k ), k); /* memcpy ( handle , buf + 3 + sizeof ( handleLen ) , handleLen ) ; */
f[k] = '\0'; /* handle [ handleLen ] = '\0' ; */
if (exists_handle(i, f)>-1) /* if (exists_handle(all_handles, handle)>-1) */ {
((chat_header *)a )->len = htons(((chat_header *)a )->len); /* ( ( chat_header * ) buf ) -> len = htons ( ( ( chat_header * ) buf ) -> len ) ; */
g = createPacket(((void *)0 ), 3, 3, 0); /* sending_pkt = createPacket ( ( ( void * ) 0 ) , 3 , 3 , 0 ) ; */
} else {
g = createPacket(((void *)0 ), 2, 3, 0); /* sending_pkt = createPacket ( ( ( void * ) 0 ) , 2 , 3 , 0 ) ; */
add_handle(b, f, c, d); /* add_handle ( all_handles_ptr , handle , client_socket , readFds ) ; */
}
if (sendPacket(g, c, 1)) /* if (sendPacket(sending_pkt, client_socket, 1)) */ {
perror("send()"); /* perror ( "send()" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
} else if (j == 4) /* if (flag == 4) */ {
broadcastMessage(a, i, c); /* broadcastMessage ( buf , all_handles , client_socket ) ; */
} else if (j == 5) /* if (flag == 5) */ {
memcpy(&k, (a + 3 ), sizeof (k )); /* memcpy ( & handleLen , ( buf + 3 ) , sizeof ( handleLen ) ) ; */
memcpy(f, a + 3 + sizeof (k ), k); /* memcpy ( handle , buf + 3 + sizeof ( handleLen ) , handleLen ) ; */
*(f + k ) = '\0'; /* * ( handle + handleLen ) = '\0' ; */
if ((e = exists_handle(i, f) )>-1) /* if ((handleOffset = exists_handle(all_handles, handle) )>-1) */ {
sendPacket(a, (i + e )->socketNum, 0); /* sendPacket ( buf , ( all_handles + handleOffset ) -> socketNum , 0 ) ; */
} else {
makeHandleData(a, f, k); /* makeHandleData ( buf , handle , handleLen ) ; */
g = createPacket(a, 7, 3 + sizeof(uint8_t) + k, 0); /* sending_pkt = createPacket ( buf , 7 , 3 + sizeof ( uint8_t ) + handleLen , 0 ) ; */
if (sendPacket(g, c, 1)) /* if (sendPacket(sending_pkt, client_socket, 1)) */ {
perror("send()"); /* perror ( "send()" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
} else if (j == 8) /* if (flag == 8) */ {
sendExitAck(c); /* sendExitAck ( client_socket ) ; */
} else if (j == 10) /* if (flag == 10) */ {
sendListHandles(c, i); /* sendListHandles ( client_socket , all_handles ) ; */
sendReadyToListHandlesAck(c); /* sendReadyToListHandlesAck ( client_socket ) ; */
}
}
int runServer(int a, char **b) /* int runServer ( int argc , char * * argv ) { */{
char *c = ((void *)0 ); /* char * buf = ( ( void * ) 0 ) ; */
int d; /* int server_socket ; */
int e; /* int client_socket ; */
fd_set f; /* fd_set readFds ; */
my_socket *g = calloc(5, sizeof(my_socket)); /* my_socket * all_handles = calloc ( 5 , sizeof ( my_socket ) ) ; */
int h; /* int i ; */
int i = 0; /* int message_len = 0 ; */
if ((c = (char *)malloc(2048) ) == ((void *)0 )) /* if ((buf = (char *)malloc(2048) ) == ((void *)0 )) */ {
perror("malloc"); /* perror ( "malloc" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
d = tcpServerSetup(); /* server_socket = tcpServerSetup ( ) ; */
sockListen(d); /* sockListen ( server_socket ) ; */
while (1) /* while (1) */ {
FD_ZERO(&f); /* FD_ZERO ( & readFds ) ; */
FD_SET(d, &f); /* FD_SET ( server_socket , & readFds ) ; */
for (h = 0; h<global_b; h++) {
FD_SET((g + h )->socketNum, &f); /* FD_SET ( ( all_handles + i ) -> socketNum , & readFds ) ; */
}
e = listenSockets(&f, d); /* client_socket = listenSockets ( & readFds , server_socket ) ; */
if ((i = recv(e, c, 2048, 0) )<0) /* if ((message_len = recv(client_socket, buf, 2048, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (i == 0) /* if (message_len == 0) */ {
remove_handle_by_socket(g, e, &f); /* remove_handle_by_socket ( all_handles , client_socket , & readFds ) ; */
} else {
action(c, &g, e, &f); /* action ( buf , & all_handles , client_socket , & readFds ) ; */
}
}
free(g); /* free ( all_handles ) ; */
free(c); /* free ( buf ) ; */
close(d); /* close ( server_socket ) ; */
close(e); /* close ( client_socket ) ; */
return 0; /* return 0 */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
runServer(a, b); /* runServer ( argc , argv ) ; */
return 0; /* return 0 */
}
