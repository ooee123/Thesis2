char global_a[1024]; /* char BUFFER [ 1024 ] ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void sendInitPacket(cclient *a, int b) /* void sendInitPacket ( cclient * clientInfo , int socket_num ) { */{
initPacket c; /* initPacket sendInfo ; */
unsigned char *d = malloc(4 + a->len); /* unsigned char * data = malloc ( 4 + clientInfo -> len ) ; */
c.handle = a; /* sendInfo . handle = clientInfo ; */
c.head.flag = 1; /* sendInfo . head . flag = 1 ; */
c.head.packetLen = htons(4 + a->len); /* sendInfo . head . packetLen = htons ( 4 + clientInfo -> len ) ; */
memcpy(d, &c.head, 3); /* memcpy ( data , & sendInfo . head , 3 ) ; */
memcpy(&d[3], &a->len, 1); /* memcpy ( & data [ 3 ] , & clientInfo -> len , 1 ) ; */
memcpy(&d[4], a->name, a->len); /* memcpy ( & data [ 4 ] , clientInfo -> name , clientInfo -> len ) ; */
if (send(b, d, 4 + a->len, 0)<0) /* if (send(socket_num, data, 4 + clientInfo->len, 0)<0) */ {
perror("Send on Client"); /* perror ( "Send on Client" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(d); /* free ( data ) ; */
}
void sendMessage(cclient *a, cclient *b, msgData *c, int d) /* void sendMessage ( cclient * src , cclient * dest , msgData * msg , int socket ) { */{
messagePacket *e = malloc(sizeof(messagePacket)); /* messagePacket * msgPck = malloc ( sizeof ( messagePacket ) ) ; */
int f = 3; /* int index = 3 ; */
int g = 5 + a->len + b->len + c->size; /* int size = 5 + src -> len + dest -> len + msg -> size ; */
unsigned char *h = malloc(g); /* unsigned char * data = malloc ( size ) ; */
e->head.packetLen = htons(g); /* msgPck -> head . packetLen = htons ( size ) ; */
e->head.flag = 5; /* msgPck -> head . flag = 5 ; */
e->dest = b; /* msgPck -> dest = dest ; */
e->src = a; /* msgPck -> src = src ; */
e->text = c; /* msgPck -> text = msg ; */
memcpy(h, &e->head, 3); /* memcpy ( data , & msgPck -> head , 3 ) ; */
memcpy(&h[3], &e->dest->len, 1); /* memcpy ( & data [ 3 ] , & msgPck -> dest -> len , 1 ) ; */
f += 1; /* index += 1 ; */
memcpy(&h[f], e->dest->name, e->dest->len); /* memcpy ( & data [ index ] , msgPck -> dest -> name , msgPck -> dest -> len ) ; */
f += e->dest->len; /* index += msgPck -> dest -> len ; */
memcpy(&h[f], &e->src->len, 1); /* memcpy ( & data [ index ] , & msgPck -> src -> len , 1 ) ; */
f += 1; /* index += 1 ; */
memcpy(&h[f], e->src->name, e->src->len); /* memcpy ( & data [ index ] , msgPck -> src -> name , msgPck -> src -> len ) ; */
f += e->src->len; /* index += msgPck -> src -> len ; */
memcpy(&h[f], e->text->msg, e->text->size); /* memcpy ( & data [ index ] , msgPck -> text -> msg , msgPck -> text -> size ) ; */
if (send(d, (void *)h, g, 0)<0) /* if (send(socket, (void *)data, size, 0)<0) */ {
perror("Send Message"); /* perror ( "Send Message" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(e); /* free ( msgPck ) ; */
free(h); /* free ( data ) ; */
}
void sendMsgToAll(cclient *a, msgData *b, int c) /* void sendMsgToAll ( cclient * myInfo , msgData * textMsg , int socketNum ) { */{
chatHead d; /* chatHead head ; */
int e = 4 + a->len + b->size; /* int length = 4 + myInfo -> len + textMsg -> size ; */
char *f = malloc(e); /* char * data = malloc ( length ) ; */
d.flag = 4; /* head . flag = 4 ; */
d.packetLen = htons(e); /* head . packetLen = htons ( length ) ; */
memcpy(f, &d, 3); /* memcpy ( data , & head , 3 ) ; */
memcpy(&f[3], &a->len, 1); /* memcpy ( & data [ 3 ] , & myInfo -> len , 1 ) ; */
memcpy(&f[4], a->name, a->len); /* memcpy ( & data [ 4 ] , myInfo -> name , myInfo -> len ) ; */
memcpy(&f[4 + a->len], b->msg, strlen(b->msg)); /* memcpy ( & data [ 4 + myInfo -> len ] , textMsg -> msg , strlen ( textMsg -> msg ) ) ; */
if (send(c, (void *)f, e, 0)<0) /* if (send(socketNum, (void *)data, length, 0)<0) */ {
perror("Send broadcast"); /* perror ( "Send broadcast" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(f); /* free ( data ) ; */
}
void sendListRequest(cclient *a, int b) /* void sendListRequest ( cclient * myInfo , int socket ) { */{
listReq *c = malloc(sizeof(listReq)); /* listReq * data = malloc ( sizeof ( listReq ) ) ; */
c->packetLen = htons(3); /* data -> packetLen = htons ( 3 ) ; */
c->flag = 10; /* data -> flag = 10 ; */
if (send(b, (void *)c, 3, 0)<0) /* if (send(socket, (void *)data, 3, 0)<0) */ {
perror("Send List Req"); /* perror ( "Send List Req" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(c); /* free ( data ) ; */
}
void sendExitRequest(cclient *a, int b) /* void sendExitRequest ( cclient * myInfo , int socket ) { */{
exitReq *c = malloc(sizeof(exitReq)); /* exitReq * data = malloc ( sizeof ( exitReq ) ) ; */
c->packetLen = htons(3); /* data -> packetLen = htons ( 3 ) ; */
c->flag = 8; /* data -> flag = 8 ; */
if (send(b, (void *)c, 3, 0)<0) /* if (send(socket, (void *)data, 3, 0)<0) */ {
perror("Send Exit Req"); /* perror ( "Send Exit Req" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(c); /* free ( data ) ; */
}
int getHandle(cclient *a, char *b) /* int getHandle ( cclient * handleInfo , char * buff ) { */{
a->len = strcspn(b, " \n"); /* handleInfo -> len = strcspn ( buff , " \n" ) ; */
if (a->len == 0) /* if (handleInfo->len == 0) */ {
return -1; /* return -1 */
}
a->name = malloc(a->len); /* handleInfo -> name = malloc ( handleInfo -> len ) ; */
memcpy(a->name, b, a->len); /* memcpy ( handleInfo -> name , buff , handleInfo -> len ) ; */
return 0; /* return 0 */
}
void getText(msgData *a, char *b) /* void getText ( msgData * textMsg , char * buff ) { */{
a->size = strcspn(b, "\n"); /* textMsg -> size = strcspn ( buff , "\n" ) ; */
if (a->size>1000) /* if (textMsg->size>1000) */ {
a->msg = malloc(1000); /* textMsg -> msg = malloc ( 1000 ) ; */
memcpy(a->msg, b, 1000 - 1); /* memcpy ( textMsg -> msg , buff , 1000 - 1 ) ; */
a->msg[1000] = '\0'; /* textMsg -> msg [ 1000 ] = '\0' ; */
a->remaining = malloc(a->size - (1000 - 1 )); /* textMsg -> remaining = malloc ( textMsg -> size - ( 1000 - 1 ) ) ; */
memcpy(a->remaining, &b[1000], a->size - (1000 - 1 )); /* memcpy ( textMsg -> remaining , & buff [ 1000 ] , textMsg -> size - ( 1000 - 1 ) ) ; */
} else {
a->msg = malloc(a->size + 1); /* textMsg -> msg = malloc ( textMsg -> size + 1 ) ; */
memcpy(a->msg, b, a->size); /* memcpy ( textMsg -> msg , buff , textMsg -> size ) ; */
a->msg[a->size] = '\0'; /* textMsg -> msg [ textMsg -> size ] = '\0' ; */
}
}
void processCommands(cclient *a, int b) /* void processCommands ( cclient * myInfo , int socketNum ) { */{
char *c; /* char * data ; */
cclient d; /* cclient destInfo ; */
uint32_t e; /* uint32_t cmdSize ; */
char f, g[1024]; /* char option , buff [ 1024 ] ; */
msgData h; /* msgData textMsg ; */
fgets(g, 1024, stdin); /* fgets ( buff , 1024 , stdin ) ; */
e = strcspn(g, " \n") + 1; /* cmdSize = strcspn ( buff , " \n" ) + 1 ; */
c = &g[e]; /* data = & buff [ cmdSize ] ; */
if (e == 3&&g[0] == '%') /* if (cmdSize == 3&&buff[0] == '%') */ {
f = toupper(g[1]); /* option = toupper ( buff [ 1 ] ) ; */
} else {
f = 'I'; /* option = 'I' ; */
}
switch (f) /* switch (option) */ {
case 'M': if (getHandle(&d, c)<0) /* if (getHandle(&destInfo, data)<0) */ {
fprintf(stderr, "Invalid command.\n"); /* fprintf ( stderr , "Invalid command.\n" ) ; */
break;
}
sendMessage(a, &d, &h, b); /* sendMessage ( myInfo , & destInfo , & textMsg , socketNum ) ; */
free(h.msg); /* free ( textMsg . msg ) ; */
free(d.name); /* free ( destInfo . name ) ; */
getText(&h, c); /* getText ( & textMsg , data ) ; */
c += d.len + 1; /* data += destInfo . len + 1 ; */
break;
case 'B': getText(&h, c); /* getText ( & textMsg , data ) ; */
sendMsgToAll(a, &h, b); /* sendMsgToAll ( myInfo , & textMsg , socketNum ) ; */
free(h.msg); /* free ( textMsg . msg ) ; */
break;
case 'L': sendListRequest(a, b); /* sendListRequest ( myInfo , socketNum ) ; */
break;
case 'E': sendExitRequest(a, b); /* sendExitRequest ( myInfo , socketNum ) ; */
break;
default: fprintf(stderr, "Invalid command.\n"); /* fprintf ( stderr , "Invalid command.\n" ) ; */
break;
}
fprintf(stderr, "$:"); /* fprintf ( stderr , "$:" ) ; */
}
void getPacketHandle(cclient *a, int b) /* void getPacketHandle ( cclient * dest , int offset ) { */{
memcpy(&a->len, &global_a[b], 1); /* memcpy ( & dest -> len , & BUFFER [ offset ] , 1 ) ; */
a->name = malloc(a->len + 1); /* dest -> name = malloc ( dest -> len + 1 ) ; */
memcpy(a->name, &global_a[b + 1], a->len); /* memcpy ( dest -> name , & BUFFER [ offset + 1 ] , dest -> len ) ; */
a->name[a->len] = '\0'; /* dest -> name [ dest -> len ] = '\0' ; */
}
void getPacketText(msgData *a, int b, int c) /* void getPacketText ( msgData * msg , int index , int packetLen ) { */{
int d = c - b; /* int length = packetLen - index ; */
a->msg = malloc(d); /* msg -> msg = malloc ( length ) ; */
strcpy(a->msg, &global_a[b]); /* strcpy ( msg -> msg , & BUFFER [ index ] ) ; */
}
void parseDetails(int a, int b, int c, cclient *d) /* void parseDetails ( int numBytes , int socket , int destSocket , cclient * myInfo ) { */{
uint8_t e; /* uint8_t flag ; */
cclient f; /* cclient src ; */
int32_t g; /* int32_t numHandles ; */
uint16_t h; /* uint16_t length ; */
cclient i; /* cclient dest ; */
msgData j; /* msgData msg ; */
memcpy(&h, global_a, 2); /* memcpy ( & length , BUFFER , 2 ) ; */
h = ntohs(h); /* length = ntohs ( length ) ; */
memcpy(&e, &global_a[2], 1); /* memcpy ( & flag , & BUFFER [ 2 ] , 1 ) ; */
switch (e) /* switch (flag) */ {
case 2: break;
case 3: fprintf(stderr, "\nHandle already in use: %s\n", d->name); /* fprintf ( stderr , "\nHandle already in use: %s\n" , myInfo -> name ) ; */
exit(0); /* exit ( 0 ) ; */
close(b); /* close ( socket ) ; */
break;
case 5: getPacketHandle(&i, 3); /* getPacketHandle ( & dest , 3 ) ; */
fprintf(stderr, "$:"); /* fprintf ( stderr , "$:" ) ; */
fprintf(stderr, "\n%s: %s\n", f.name, j.msg); /* fprintf ( stderr , "\n%s: %s\n" , src . name , msg . msg ) ; */
free(f.name); /* free ( src . name ) ; */
free(j.msg); /* free ( msg . msg ) ; */
free(i.name); /* free ( dest . name ) ; */
getPacketHandle(&f, 4 + i.len); /* getPacketHandle ( & src , 4 + dest . len ) ; */
getPacketText(&j, 5 + i.len + f.len, h); /* getPacketText ( & msg , 5 + dest . len + src . len , length ) ; */
break;
case 7: getPacketHandle(&i, 3); /* getPacketHandle ( & dest , 3 ) ; */
fprintf(stderr, "$:"); /* fprintf ( stderr , "$:" ) ; */
fprintf(stderr, "\nClient with handle %s does not exist.\n", i.name); /* fprintf ( stderr , "\nClient with handle %s does not exist.\n" , dest . name ) ; */
free(i.name); /* free ( dest . name ) ; */
break;
case 9: close(b); /* close ( socket ) ; */
exit(0); /* exit ( 0 ) ; */
break;
case 11: memcpy(&g, &global_a[3], 4); /* memcpy ( & numHandles , & BUFFER [ 3 ] , 4 ) ; */
int k = 0; /* int received = 0 ; */
printf("Number of clients: %d\n", g); /* printf ( "Number of clients: %d\n" , numHandles ) ; */
g = ntohl(g); /* numHandles = ntohl ( numHandles ) ; */
while (k != g) /* while (received != numHandles) */ {
int l = recv(b, global_a, 1024, 0); /* int nbytes = recv ( socket , BUFFER , 1024 , 0 ) ; */
if (l<0) /* if (nbytes<0) */ {
perror("read"); /* perror ( "read" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (l == 0) /* if (nbytes == 0) */ {
fprintf(stderr, "\nServer Terminated.\n"); /* fprintf ( stderr , "\nServer Terminated.\n" ) ; */
exit(0); /* exit ( 0 ) ; */
} else {
k++; /* received ++ ; */
getPacketHandle(&i, 3); /* getPacketHandle ( & dest , 3 ) ; */
printf("\t%s\n", i.name); /* printf ( "\t%s\n" , dest . name ) ; */
free(i.name); /* free ( dest . name ) ; */
}
}
break;
case 12: break;
default: fprintf(stderr, "\nBad flag!\n"); /* fprintf ( stderr , "\nBad flag!\n" ) ; */
fprintf(stderr, "$:"); /* fprintf ( stderr , "$:" ) ; */
break;
}
}
void startChat(cclient *a, int b) /* void startChat ( cclient * myInfo , int socketNum ) { */{
fd_set c, d; /* fd_set useFds , saveFds ; */
int e; /* int ndx ; */
FD_ZERO(&d); /* FD_ZERO ( & saveFds ) ; */
FD_SET(0, &d); /* FD_SET ( 0 , & saveFds ) ; */
FD_SET(b, &d); /* FD_SET ( socketNum , & saveFds ) ; */
while (1) /* while (1) */ {
c = d; /* useFds = saveFds ; */
if (select(1024, &c, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (select(1024, &useFds, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select"); /* perror ( "select" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
for (e = 0; e<1024; e++) {
if (FD_ISSET(e, &c)) /* if (FD_ISSET(ndx, &useFds)) */ {
if (e == b) /* if (ndx == socketNum) */ {
int f = read(e, global_a, 1024); /* int nbytes = read ( ndx , BUFFER , 1024 ) ; */
if (f<0) /* if (nbytes<0) */ {
perror("read"); /* perror ( "read" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (f == 0) /* if (nbytes == 0) */ {
((void)(((&d )->__fds_bits )[((e ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((e ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( & saveFds ) -> __fds_bits ) [ ( ( ndx ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( ndx ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
fprintf(stderr, "\nServer Terminated.\n"); /* fprintf ( stderr , "\nServer Terminated.\n" ) ; */
exit(0); /* exit ( 0 ) ; */
} else {
parseDetails(f, b, e, a); /* parseDetails ( nbytes , socketNum , ndx , myInfo ) ; */
}
} else if (e == 0) /* if (ndx == 0) */ {
processCommands(a, b); /* processCommands ( myInfo , socketNum ) ; */
}
}
}
}
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
int c = 0; /* int socketNum = 0 ; */
cclient *d = malloc(sizeof(cclient)); /* cclient * myInfo = malloc ( sizeof ( cclient ) ) ; */
if (a != 4) /* if (argc != 4) */ {
printf("usage: %s handle server-name server-port\n", b[0]); /* printf ( "usage: %s handle server-name server-port\n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
}
d->len = strlen(b[1]); /* myInfo -> len = strlen ( argv [ 1 ] ) ; */
d->name = malloc(d->len); /* myInfo -> name = malloc ( myInfo -> len ) ; */
memcpy(d->name, b[1], d->len); /* memcpy ( myInfo -> name , argv [ 1 ] , myInfo -> len ) ; */
c = tcpClientSetup(b[2], b[3]); /* socketNum = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
sendInitPacket(d, c); /* sendInitPacket ( myInfo , socketNum ) ; */
fprintf(stderr, "$:"); /* fprintf ( stderr , "$:" ) ; */
startChat(d, c); /* startChat ( myInfo , socketNum ) ; */
close(c); /* close ( socketNum ) ; */
free(d); /* free ( myInfo ) ; */
return 0; /* return 0 */
}
