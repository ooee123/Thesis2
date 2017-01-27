

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
Server c; /* Server server ; */
setup(a, b, &c); /* setup ( argc , argv , & server ) ; */
while (1) /* while (1) */ {
processNextRequest(&c); /* processNextRequest ( & server ) ; */
}
cleanup(&c); /* cleanup ( & server ) ; */
return 0; /* return 0 */
}
void setup(int a, char *b, Server *c) /* void setup ( int argc , char * argv [ ] , Server * s ) { */{
int d = 0; /* int port = 0 ; */
c->server_socket = 0; /* s -> server_socket = 0 ; */
c->client_socket = 0; /* s -> client_socket = 0 ; */
c->buf = ((void *)0 ); /* s -> buf = ( ( void * ) 0 ) ; */
c->handleList = ((void *)0 ); /* s -> handleList = ( ( void * ) 0 ) ; */
FD_ZERO(&c->fdSet); /* FD_ZERO ( & s -> fdSet ) ; */
if (a == 2) /* if (argc == 2) */ {
if ((d = atoi(b[1]) )) /* if ((port = atoi(argv[1]) )) */ {

} else {
puts("Invalid port number.  Exiting."); /* puts ( "Invalid port number.  Exiting." ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
if ((c->buf = (char *)malloc(1024) ) == ((void *)0 )) /* if ((s->buf = (char *)malloc(1024) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
c->server_socket = tcpServerSetup(d); /* s -> server_socket = tcpServerSetup ( port ) ; */
FD_SET(c->server_socket, &c->fdSet); /* FD_SET ( s -> server_socket , & s -> fdSet ) ; */
c->highestFd = c->server_socket; /* s -> highestFd = s -> server_socket ; */
}
void cleanup(Server *a) /* void cleanup ( Server * s ) { */{
close(a->server_socket); /* close ( s -> server_socket ) ; */
close(a->client_socket); /* close ( s -> client_socket ) ; */
}
void processNextRequest(Server *a) /* void processNextRequest ( Server * s ) { */{
int b, c; /* int readSize , lenPacket ; */
ChatHeader d; /* ChatHeader ch ; */
int e = getNextReadyFd(a); /* int nextFd = getNextReadyFd ( s ) ; */
if (e == a->server_socket) /* if (nextFd == s->server_socket) */ {
a->client_socket = tcpAccept(a->server_socket); /* s -> client_socket = tcpAccept ( s -> server_socket ) ; */
} else {
a->client_socket = e; /* s -> client_socket = nextFd ; */
}
if ((b = myRecv(a->client_socket, &d, sizeof(ChatHeader), 0) )<0) /* if ((readSize = myRecv(s->client_socket, &ch, sizeof(ChatHeader), 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(a->buf, &d, sizeof(ChatHeader)); /* memcpy ( s -> buf , & ch , sizeof ( ChatHeader ) ) ; */
c = htons(d.packetLen); /* lenPacket = htons ( ch . packetLen ) ; */
if (b == 0) /* if (readSize == 0) */ {
removeUserWithSocket(a->client_socket, a); /* removeUserWithSocket ( s -> client_socket , s ) ; */
} else {
if (c != sizeof(ChatHeader)) /* if (lenPacket != sizeof(ChatHeader)) */ {
if (myRecv(a->client_socket, a->buf + sizeof(ChatHeader), c - sizeof(ChatHeader), 0)<0) /* if (myRecv(s->client_socket, s->buf + sizeof(ChatHeader), lenPacket - sizeof(ChatHeader), 0)<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
handleRequest(a); /* handleRequest ( s ) ; */
}
}
int getNextReadyFd(Server *a) /* int getNextReadyFd ( Server * s ) { */{
HandleNode *b = a->handleList; /* HandleNode * current = s -> handleList ; */
resetFdSet(a); /* resetFdSet ( s ) ; */
if (mySelect(a->highestFd + 1, &a->fdSet, ((void *)0 ), ((void *)0 ), ((void *)0 )) == -1) /* if (mySelect(s->highestFd + 1, &s->fdSet, ((void *)0 ), ((void *)0 ), ((void *)0 )) == -1) */ {
perror("select call"); /* perror ( "select call" ) ; */
}
if (FD_ISSET(a->server_socket, &a->fdSet)) /* if (FD_ISSET(s->server_socket, &s->fdSet)) */ {
return a->server_socket; /* return s->server_socket */
}
while (b) /* while (current) */ {
if (FD_ISSET(b->socket, &a->fdSet)) /* if (FD_ISSET(current->socket, &s->fdSet)) */ {
return b->socket; /* return current->socket */
}
b = b->next; /* current = current -> next ; */
}
return -1; /* return -1 */
}
void handleRequest(Server *a) /* void handleRequest ( Server * s ) { */{
ChatHeader *b = (ChatHeader *)a->buf, c; /* ChatHeader * ch = ( ChatHeader * ) s -> buf , response ; */
switch (b->flag) /* switch (ch->flag) */ {
case 1: handleInitialPacket(a); /* handleInitialPacket ( s ) ; */
break;
case 5: handleMessagePacket(a); /* handleMessagePacket ( s ) ; */
break;
case 8: removeUserWithSocket(a->client_socket, a); /* removeUserWithSocket ( s -> client_socket , s ) ; */
mySend(a->client_socket, &c, sizeof(ChatHeader), 0); /* mySend ( s -> client_socket , & response , sizeof ( ChatHeader ) , 0 ) ; */
close(a->client_socket); /* close ( s -> client_socket ) ; */
c.flag = 9; /* response . flag = 9 ; */
c.packetLen = htons(sizeof(ChatHeader)); /* response . packetLen = htons ( sizeof ( ChatHeader ) ) ; */
break;
case 4: handleBroadcastPacket(a); /* handleBroadcastPacket ( s ) ; */
break;
case 10: sendHandles(a); /* sendHandles ( s ) ; */
break;
default: puts("Unknown packet flag"); /* puts ( "Unknown packet flag" ) ; */
exit(1); /* exit ( 1 ) ; */
break;
}
}
void sendHandles(Server *a) /* void sendHandles ( Server * s ) { */{
ChatHeader b; /* ChatHeader ch ; */
uint32_t c, d = 0; /* uint32_t numHandlesNetwork , numHandles = 0 ; */
HandleNode *e = a->handleList; /* HandleNode * current = s -> handleList ; */
b.flag = 11; /* ch . flag = 11 ; */
b.packetLen = htons(sizeof(ChatHeader) + sizeof(uint32_t)); /* ch . packetLen = htons ( sizeof ( ChatHeader ) + sizeof ( uint32_t ) ) ; */
while (e != ((void *)0 )) /* while (current != ((void *)0 )) */ {
d++; /* numHandles ++ ; */
e = e->next; /* current = current -> next ; */
}
c = htonl(d); /* numHandlesNetwork = htonl ( numHandles ) ; */
memcpy(a->buf, &b, sizeof(ChatHeader)); /* memcpy ( s -> buf , & ch , sizeof ( ChatHeader ) ) ; */
memcpy(a->buf + sizeof(ChatHeader), &c, sizeof(uint32_t)); /* memcpy ( s -> buf + sizeof ( ChatHeader ) , & numHandlesNetwork , sizeof ( uint32_t ) ) ; */
mySend(a->client_socket, a->buf, sizeof(ChatHeader) + sizeof(uint32_t), 0); /* mySend ( s -> client_socket , s -> buf , sizeof ( ChatHeader ) + sizeof ( uint32_t ) , 0 ) ; */
e = a->handleList; /* current = s -> handleList ; */
while (e != ((void *)0 )) /* while (current != ((void *)0 )) */ {
b.flag = 12; /* ch . flag = 12 ; */
b.packetLen = htons(sizeof(ChatHeader) + sizeof(u_char) + e->handleLen); /* ch . packetLen = htons ( sizeof ( ChatHeader ) + sizeof ( u_char ) + current -> handleLen ) ; */
memcpy(a->buf, &b, sizeof(ChatHeader)); /* memcpy ( s -> buf , & ch , sizeof ( ChatHeader ) ) ; */
memcpy(a->buf + sizeof(ChatHeader), &(e->handleLen ), sizeof(u_char)); /* memcpy ( s -> buf + sizeof ( ChatHeader ) , & ( current -> handleLen ) , sizeof ( u_char ) ) ; */
memcpy(a->buf + sizeof(ChatHeader) + sizeof(u_char), e->handle, e->handleLen); /* memcpy ( s -> buf + sizeof ( ChatHeader ) + sizeof ( u_char ) , current -> handle , current -> handleLen ) ; */
mySend(a->client_socket, a->buf, ntohs(b.packetLen), 0); /* mySend ( s -> client_socket , s -> buf , ntohs ( ch . packetLen ) , 0 ) ; */
e = e->next; /* current = current -> next ; */
}
}
void handleInitialPacket(Server *a) /* void handleInitialPacket ( Server * s ) { */{
char *b; /* char * handle ; */
u_char c; /* u_char handleLen ; */
ChatHeader d; /* ChatHeader response ; */
c = *(a->buf + sizeof(ChatHeader) ); /* handleLen = * ( s -> buf + sizeof ( ChatHeader ) ) ; */
b = a->buf + sizeof(ChatHeader) + 1; /* handle = s -> buf + sizeof ( ChatHeader ) + 1 ; */
d.packetLen = htons(sizeof(ChatHeader)); /* response . packetLen = htons ( sizeof ( ChatHeader ) ) ; */
if (addUser(b, c, a) == 0) /* if (addUser(handle, handleLen, s) == 0) */ {
d.flag = 2; /* response . flag = 2 ; */
} else {
d.flag = 3; /* response . flag = 3 ; */
}
mySend(a->client_socket, &d, sizeof(ChatHeader), 0); /* mySend ( s -> client_socket , & response , sizeof ( ChatHeader ) , 0 ) ; */
}
void handleMessagePacket(Server *a) /* void handleMessagePacket ( Server * s ) { */{
u_char *b, *c, *d; /* u_char * flag , * destHandleLen , * fromHandleLen ; */
char *e, *f, *g; /* char * destHandle , * fromHandle , * message ; */
int h; /* int destSocket ; */
u_short *i, j; /* u_short * packetLenPtr , packetLen ; */
i = (u_short *)a->buf; /* packetLenPtr = ( u_short * ) s -> buf ; */
j = *i; /* packetLen = * packetLenPtr ; */
b = (u_char *)a->buf + sizeof(short); /* flag = ( u_char * ) s -> buf + sizeof ( short ) ; */
c = b + 1; /* destHandleLen = flag + 1 ; */
e = (char *)c + 1; /* destHandle = ( char * ) destHandleLen + 1 ; */
d = (u_char *)e + *c; /* fromHandleLen = ( u_char * ) destHandle + * destHandleLen ; */
f = (char *)d + 1; /* fromHandle = ( char * ) fromHandleLen + 1 ; */
g = (char *)f + *d; /* message = ( char * ) fromHandle + * fromHandleLen ; */
h = lookupSocketForHandle(e, *c, a); /* destSocket = lookupSocketForHandle ( destHandle , * destHandleLen , s ) ; */
if (h == -1) /* if (destSocket == -1) */ {
*i = htons(sizeof(ChatHeader) + *c + 1); /* * packetLenPtr = htons ( sizeof ( ChatHeader ) + * destHandleLen + 1 ) ; */
*b = 7; /* * flag = 7 ; */
mySend(a->client_socket, a->buf, ntohs(*i), 0); /* mySend ( s -> client_socket , s -> buf , ntohs ( * packetLenPtr ) , 0 ) ; */
} else {
mySend(h, a->buf, ntohs(j), 0); /* mySend ( destSocket , s -> buf , ntohs ( packetLen ) , 0 ) ; */
}
}
void handleBroadcastPacket(Server *a) /* void handleBroadcastPacket ( Server * s ) { */{
char *b, *c; /* char * fromHandle , * message ; */
u_short *d, e; /* u_short * packetLenPtr , packetLen ; */
u_char *f, *g; /* u_char * flag , * fromHandleLen ; */
HandleNode *h = a->handleList; /* HandleNode * current = s -> handleList ; */
while (h) /* while (current) */ {
if (h->socket != a->client_socket) /* if (current->socket != s->client_socket) */ {
mySend(h->socket, a->buf, ntohs(e), 0); /* mySend ( current -> socket , s -> buf , ntohs ( packetLen ) , 0 ) ; */
}
h = h->next; /* current = current -> next ; */
}
}
int addUser(char *a, u_char b, Server *c) /* int addUser ( char * handle , u_char handleLen , Server * s ) { */{
HandleNode *d, *e, *f; /* HandleNode * current , * prev , * new ; */
e = ((void *)0 ); /* prev = ( ( void * ) 0 ) ; */
d = c->handleList; /* current = s -> handleList ; */
if (lookupSocketForHandle(a, b, c) != -1) /* if (lookupSocketForHandle(handle, handleLen, s) != -1) */ {
return -2; /* return -2 */
}
f = malloc(sizeof(HandleNode)); /* new = malloc ( sizeof ( HandleNode ) ) ; */
f->handle = malloc(b); /* new -> handle = malloc ( handleLen ) ; */
memcpy(f->handle, a, b); /* memcpy ( new -> handle , handle , handleLen ) ; */
f->handleLen = b; /* new -> handleLen = handleLen ; */
f->socket = c->client_socket; /* new -> socket = s -> client_socket ; */
f->next = ((void *)0 ); /* new -> next = ( ( void * ) 0 ) ; */
while (d) /* while (current) */ {
e = d; /* prev = current ; */
d = d->next; /* current = current -> next ; */
}
if (e) /* if (prev) */ {
e->next = f; /* prev -> next = new ; */
} else {
c->handleList = f; /* s -> handleList = new ; */
}
FD_SET(c->client_socket, &c->fdSet); /* FD_SET ( s -> client_socket , & s -> fdSet ) ; */
if (c->client_socket>c->highestFd) /* if (s->client_socket>s->highestFd) */ {
c->highestFd = c->client_socket; /* s -> highestFd = s -> client_socket ; */
}
return 0; /* return 0 */
}
int lookupSocketForHandle(char *a, u_char b, Server *c) /* int lookupSocketForHandle ( char * handle , u_char handleLen , Server * s ) { */{
HandleNode *d = c->handleList; /* HandleNode * current = s -> handleList ; */
while (d) /* while (current) */ {
if (d->handleLen == b) /* if (current->handleLen == handleLen) */ {
if (memcmp(d->handle, a, b) == 0) /* if (memcmp(current->handle, handle, handleLen) == 0) */ {
return d->socket; /* return current->socket */
}
}
d = d->next; /* current = current -> next ; */
}
return -1; /* return -1 */
}
char * lookupHandleForSocket(int a, Server *b) /* char * lookupHandleForSocket ( int socketNum , Server * s ) { */{
HandleNode *c = b->handleList; /* HandleNode * current = s -> handleList ; */
while (c) /* while (current) */ {
if (c->socket == a) /* if (current->socket == socketNum) */ {
return c->handle; /* return current->handle */
}
c = c->next; /* current = current -> next ; */
}
return ((void *)0 ); /* return ((void *)0 ) */
}
void removeUserWithSocket(int a, Server *b) /* void removeUserWithSocket ( int socketNum , Server * s ) { */{
HandleNode *c, *d = b->handleList; /* HandleNode * prev , * current = s -> handleList ; */
c = ((void *)0 ); /* prev = ( ( void * ) 0 ) ; */
while (d) /* while (current) */ {
if (d->socket == a) /* if (current->socket == socketNum) */ {
if (c == ((void *)0 )) /* if (prev == ((void *)0 )) */ {
free(d->handle); /* free ( current -> handle ) ; */
free(d); /* free ( current ) ; */
b->handleList = ((void *)0 ); /* s -> handleList = ( ( void * ) 0 ) ; */
} else {
free(d->handle); /* free ( current -> handle ) ; */
free(d); /* free ( current ) ; */
c->next = d->next; /* prev -> next = current -> next ; */
}
}
d = d->next; /* current = current -> next ; */
}
((void)(((&b->fdSet )->__fds_bits )[((a ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((a ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( & s -> fdSet ) -> __fds_bits ) [ ( ( socketNum ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( socketNum ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
FD_SET(b->server_socket, &b->fdSet); /* FD_SET ( s -> server_socket , & s -> fdSet ) ; */
if (b->highestFd == a) /* if (s->highestFd == socketNum) */ {
b->highestFd = b->server_socket; /* s -> highestFd = s -> server_socket ; */
d = b->handleList; /* current = s -> handleList ; */
while (d) /* while (current) */ {
if (d->socket>b->highestFd) /* if (current->socket>s->highestFd) */ {
b->highestFd = d->socket; /* s -> highestFd = current -> socket ; */
}
d = d->next; /* current = current -> next ; */
}
}
}
void printAllHandles(Server *a) /* void printAllHandles ( Server * s ) { */{
HandleNode *b = a->handleList; /* HandleNode * current = s -> handleList ; */
puts("Print all"); /* puts ( "Print all" ) ; */
while (b != ((void *)0 )) /* while (current != ((void *)0 )) */ {
b = b->next; /* current = current -> next ; */
}
puts("Exit Print all"); /* puts ( "Exit Print all" ) ; */
}
void resetFdSet(Server *a) /* void resetFdSet ( Server * s ) { */{
HandleNode *b = a->handleList; /* HandleNode * current = s -> handleList ; */
while (b) /* while (current) */ {
FD_SET(b->socket, &a->fdSet); /* FD_SET ( current -> socket , & s -> fdSet ) ; */
b = b->next; /* current = current -> next ; */
}
FD_SET(a->server_socket, &a->fdSet); /* FD_SET ( s -> server_socket , & s -> fdSet ) ; */
}
