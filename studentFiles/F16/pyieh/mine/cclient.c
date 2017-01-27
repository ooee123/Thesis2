char global_b[255]; /* static char myHandle [ 255 ] ; */
int global_a = 0; /* static int numHandles = 0 ; */
packet readPacket(int socketNum); /* packet readPacket(int socketNum) */

unsigned int __bswap_32(unsigned int a) /* ( 0 , & rfds ) ; FD_SET ( socketNum , */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /* ( 1024 , & rfds , ( ( void */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
char * createPacket(char *a, uint8_t b, uint16_t c, int d) /* char * createPacket ( char * data , uint8_t flag , uint16_t size , int needFree ) { */{
char *e = calloc(c, sizeof(char)); /* char * packet = calloc ( size , sizeof ( char ) ) ; */
chat_header *f = (chat_header *)e; /* chat_header * pckt_hdr = ( chat_header * ) packet ; */
f->len = htons(c); /* pckt_hdr -> len = htons ( size ) ; */
f->flag = b; /* pckt_hdr -> flag = flag ; */
if (a) /* if (data) */ {
memcpy(e + 3, a, c + 1 - 3); /* memcpy ( packet + 3 , data , size - 3 + 1 ) ; */
if (d) /* if (needFree) */ {
free(a); /* free ( data ) ; */
}
}
return e; /* return packet */
}
int sendPacket(char *a, int b, int c) /* int sendPacket ( char * pkt , int socketNum , int needFree ) { */{
uint8_t d; /* uint8_t handleLen ; */
int e; /* int totalSent ; */
uint16_t f = ntohs(((chat_header *)a )->len); /* uint16_t packetLen = ntohs ( ( ( chat_header * ) pkt ) -> len ) ; */
uint8_t g = ((chat_header *)a )->flag; /* uint8_t flag = ( ( chat_header * ) pkt ) -> flag ; */
if (c) /* if (needFree) */ {
free(a); /* free ( pkt ) ; */
}
if (g == 12) /* if (flag == 12) */ {
memcpy(&d, a + 3, sizeof(uint8_t)); /* memcpy ( & handleLen , pkt + 3 , sizeof ( uint8_t ) ) ; */
f = 3 + sizeof(uint8_t) + d; /* packetLen = 3 + sizeof ( uint8_t ) + handleLen ; */
}
e = mySend(b, a, f, 0); /* totalSent = mySend ( socketNum , pkt , packetLen , 0 ) ; */
return (f - e ); /* return (packetLen - totalSent ) */
}
void newLine() /* void newLine ( ) { */{
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
void clearStdIn() /* void clearStdIn ( ) { */{
while (getchar() != '\n') /* while (getchar() != '\n') */ {

}
}
int checkSelect(int a) /* int checkSelect ( int socketNum ) { */{
int b; /* int selectSocket ; */
fd_set c; /* fd_set rfds ; */
FD_ZERO(&c); /* FD_ZERO ( & rfds ) ; */
FD_SET(0, &c); /* FD_SET ( 0 , & rfds ) ; */
FD_SET(a, &c); /* FD_SET ( socketNum , & rfds ) ; */
b = mySelect(1024, &c, ((void *)0 ), ((void *)0 ), ((void *)0 )); /* selectSocket = mySelect ( 1024 , & rfds , ( ( void * ) 0 ) , ( ( void * ) 0 ) , ( ( void * ) 0 ) ) ; */
if (b<0) /* if (selectSocket<0) */ {
perror("check_select:select"); /* perror ( "check_select:select" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (b) /* if (selectSocket) */ {
if (FD_ISSET(0, &c)) /* if (FD_ISSET(0, &rfds)) */ {
return 0; /* return 0 */
} else if (FD_ISSET(a, &c)) /* if (FD_ISSET(socketNum, &rfds)) */ {
return a; /* return socketNum */
}
}
return -1; /* return -1 */
}
void printClientMessage(packet a) /* void printClientMessage ( packet pkt ) { */{
uint8_t b, c; /* uint8_t hLen1 , hLen2 ; */
int d; /* int offset ; */
char e[255]; /* char srcHandle [ 255 ] ; */
memcpy(&b, a.data, sizeof (b )); /* memcpy ( & hLen1 , pkt . data , sizeof ( hLen1 ) ) ; */
memcpy(&c, a.data + sizeof (b ) + b, sizeof (c )); /* memcpy ( & hLen2 , pkt . data + sizeof ( hLen1 ) + hLen1 , sizeof ( hLen2 ) ) ; */
memcpy(e, a.data + sizeof (b ) + b + sizeof (c ), c); /* memcpy ( srcHandle , pkt . data + sizeof ( hLen1 ) + hLen1 + sizeof ( hLen2 ) , hLen2 ) ; */
e[c] = '\0'; /* srcHandle [ hLen2 ] = '\0' ; */
d = 1 + b + 1 + c; /* offset = 1 + hLen1 + 1 + hLen2 ; */
printf("\n%s: %s\n", e, a.data + d); /* printf ( "\n%s: %s\n" , srcHandle , pkt . data + offset ) ; */
}
void printClientBroadcast(packet a) /* void printClientBroadcast ( packet pkt ) { */{
char b[255]; /* char srcHandle [ 255 ] ; */
uint8_t c; /* uint8_t hLen ; */
int d; /* int offset ; */
memcpy(&c, a.data, sizeof (c )); /* memcpy ( & hLen , pkt . data , sizeof ( hLen ) ) ; */
memcpy(b, a.data + sizeof (c ), c); /* memcpy ( srcHandle , pkt . data + sizeof ( hLen ) , hLen ) ; */
b[c] = '\0'; /* srcHandle [ hLen ] = '\0' ; */
d = 1 + c; /* offset = 1 + hLen ; */
printf("\n%s: %s\n", b, a.data + d); /* printf ( "\n%s: %s\n" , srcHandle , pkt . data + offset ) ; */
}
uint32_t getNumHandles(packet a) /* uint32_t getNumHandles ( packet pkt ) { */{
uint32_t b; /* uint32_t numHandles ; */
memcpy(&b, a.data, sizeof (b )); /* memcpy ( & numHandles , pkt . data , sizeof ( numHandles ) ) ; */
return ntohl(b); /* return ntohl(numHandles) */
}
void makeHandleData(char *a, char *b, uint8_t c) /* void makeHandleData ( char * buf , char * handle , uint8_t len ) { */{
memcpy(a, &c, sizeof (c )); /* memcpy ( buf , & len , sizeof ( len ) ) ; */
memcpy(a + sizeof (c ), b, c); /* memcpy ( buf + sizeof ( len ) , handle , len ) ; */
}
void printClientHandle(packet a) /* void printClientHandle ( packet pkt ) { */{
uint8_t b; /* uint8_t handleLen ; */
char c[255]; /* char handle [ 255 ] ; */
memcpy(&b, a.data, sizeof (b )); /* memcpy ( & handleLen , pkt . data , sizeof ( handleLen ) ) ; */
memcpy(c, a.data + sizeof(uint8_t), b); /* memcpy ( handle , pkt . data + sizeof ( uint8_t ) , handleLen ) ; */
c[b] = '\0'; /* handle [ handleLen ] = '\0' ; */
printf("\t%s\n", c); /* printf ( "\t%s\n" , handle ) ; */
if (!(--global_a )) /* if (!(--numHandles )) */ {
newLine(); /* newLine ( ) ; */
}
}
void sendHandleListRequest(int a) /* void sendHandleListRequest ( int socketNum ) { */{
char *b = createPacket(((void *)0 ), 10, 3, 0); /* char * pkt = createPacket ( ( ( void * ) 0 ) , 10 , 3 , 0 ) ; */
if (sendPacket(b, a, 1)) /* if (sendPacket(pkt, socketNum, 1)) */ {
perror("runClient:L:send"); /* perror ( "runClient:L:send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void sendExitRequest(int a) /* void sendExitRequest ( int socketNum ) { */{
char *b = createPacket(((void *)0 ), 8, 3, 0); /* char * pkt = createPacket ( ( ( void * ) 0 ) , 8 , 3 , 0 ) ; */
if (sendPacket(b, a, 1)) /* if (sendPacket(pkt, socketNum, 1)) */ {
perror("runClient:L:send"); /* perror ( "runClient:L:send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void action(packet a) /* void action ( packet pkt ) { */{
char b[255]; /* char handle [ 255 ] ; */
uint8_t c; /* uint8_t handleLen ; */
packet *d = (packet *)(&a ); /* packet * pkt_ptr = ( packet * ) ( & pkt ) ; */
int e = (d->header ).flag; /* int flag = ( pkt_ptr -> header ) . flag ; */
if (e == 7) /* if (flag == 7) */ {
memcpy(&c, (a.data ), sizeof (c )); /* memcpy ( & handleLen , ( pkt . data ) , sizeof ( handleLen ) ) ; */
memcpy(b, a.data + sizeof (c ), c); /* memcpy ( handle , pkt . data + sizeof ( handleLen ) , handleLen ) ; */
*(b + c ) = '\0'; /* * ( handle + handleLen ) = '\0' ; */
fprintf(stderr, "\nClient with handle %s does not exist.\n", b); /* fprintf ( stderr , "\nClient with handle %s does not exist.\n" , handle ) ; */
newLine(); /* newLine ( ) ; */
} else if (e == 5) /* if (flag == 5) */ {
printClientMessage(a); /* printClientMessage ( pkt ) ; */
newLine(); /* newLine ( ) ; */
} else if (e == 4) /* if (flag == 4) */ {
printClientBroadcast(a); /* printClientBroadcast ( pkt ) ; */
newLine(); /* newLine ( ) ; */
} else if (e == 11) /* if (flag == 11) */ {
global_a = getNumHandles(a); /* numHandles = getNumHandles ( pkt ) ; */
printf("Number of clients: %d\n", global_a); /* printf ( "Number of clients: %d\n" , numHandles ) ; */
} else if (e == 12) /* if (flag == 12) */ {
printClientHandle(a); /* printClientHandle ( pkt ) ; */
} else if (e == 9) /* if (flag == 9) */ {
free(a.data); /* free ( pkt . data ) ; */
exit(0); /* exit ( 0 ) ; */
} else {
fprintf(stderr, "Received packet with unknown flag: %d\n", e); /* fprintf ( stderr , "Received packet with unknown flag: %d\n" , flag ) ; */
}
free(a.data); /* free ( pkt . data ) ; */
}
int runClient(int a, char **b) /* int runClient ( int socketNum , char * * argv ) { */{
char *c, d[1000 + 1], e[255 + 1], *f; /* char * pkt , text [ 1000 + 1 ] , dest_handle [ 255 + 1 ] , * data ; */
packet g; /* packet rec_packet ; */
char h = 'a'; /* char c = 'a' ; */
int i, j, k; /* int d , t , ret_select ; */
newLine(); /* newLine ( ) ; */
while (1) /* while (1) */ {
while ((k = checkSelect(a) )<0) /* while ((ret_select = checkSelect(socketNum) )<0) */ {

}
if (!k) /* if (!ret_select) */ {
if (getchar() == '%') /* if (getchar() == '%') */ {
h = getchar(); /* c = getchar ( ) ; */
getchar(); /* getchar ( ) ; */
if (h == 'M' || h == 'm') /* if (c == 'M' || c == 'm') */ {
for (i = 0; (h = getchar() ) != ' '; i++) {
e[i] = h; /* dest_handle [ d ] = c ; */
}
while (h != '\n') /* while (c != '\n') */ {
for (j = 0; ((h = getchar() ) != '\n' )&&j<=1000; j++) {
d[j] = h; /* text [ t ] = c ; */
}
d[j] = '\0'; /* text [ t ] = '\0' ; */
f = calloc(2 * sizeof(uint8_t) + i + (j + 1 ) + strlen(global_b), sizeof(char)); /* data = calloc ( 2 * sizeof ( uint8_t ) + d + ( t + 1 ) + strlen ( myHandle ) , sizeof ( char ) ) ; */
makeHandleData(f, e, i); /* makeHandleData ( data , dest_handle , d ) ; */
makeHandleData(f + sizeof(uint8_t) + i, global_b, strlen(global_b)); /* makeHandleData ( data + sizeof ( uint8_t ) + d , myHandle , strlen ( myHandle ) ) ; */
strcpy(f + sizeof(uint8_t) + i + sizeof(uint8_t) + strlen(global_b), d); /* strcpy ( data + sizeof ( uint8_t ) + d + sizeof ( uint8_t ) + strlen ( myHandle ) , text ) ; */
c = createPacket(f, 5, 3 + 2 * sizeof(uint8_t) + i + j + 1 + strlen(global_b), 1); /* pkt = createPacket ( data , 5 , 3 + 2 * sizeof ( uint8_t ) + d + t + 1 + strlen ( myHandle ) , 1 ) ; */
if (sendPacket(c, a, 1)) /* if (sendPacket(pkt, socketNum, 1)) */ {
perror("runClient:M:send"); /* perror ( "runClient:M:send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
newLine(); /* newLine ( ) ; */
} else if (h == 'B' || h == 'b') /* if (c == 'B' || c == 'b') */ {
while (h != '\n') /* while (c != '\n') */ {
for (j = 0; ((h = getchar() ) != '\n' )&&j<=1000; j++) {
d[j] = h; /* text [ t ] = c ; */
}
d[j] = '\0'; /* text [ t ] = '\0' ; */
f = calloc(sizeof(uint8_t) + strlen(global_b) + (j + 1 ), sizeof(char)); /* data = calloc ( sizeof ( uint8_t ) + strlen ( myHandle ) + ( t + 1 ) , sizeof ( char ) ) ; */
makeHandleData(f, global_b, strlen(global_b)); /* makeHandleData ( data , myHandle , strlen ( myHandle ) ) ; */
strcpy(f + sizeof(uint8_t) + strlen(global_b), d); /* strcpy ( data + sizeof ( uint8_t ) + strlen ( myHandle ) , text ) ; */
c = createPacket(f, 4, 3 + sizeof(uint8_t) + strlen(global_b) + (j + 1 ), 1); /* pkt = createPacket ( data , 4 , 3 + sizeof ( uint8_t ) + strlen ( myHandle ) + ( t + 1 ) , 1 ) ; */
if (sendPacket(c, a, 1)) /* if (sendPacket(pkt, socketNum, 1)) */ {
perror("runClient:B:send"); /* perror ( "runClient:B:send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
newLine(); /* newLine ( ) ; */
} else if (h == 'L' || h == 'l') /* if (c == 'L' || c == 'l') */ {
sendHandleListRequest(a); /* sendHandleListRequest ( socketNum ) ; */
} else if (h == 'E' || h == 'e') /* if (c == 'E' || c == 'e') */ {
sendExitRequest(a); /* sendExitRequest ( socketNum ) ; */
} else {
fprintf(stderr, "Invalid input\n"); /* fprintf ( stderr , "Invalid input\n" ) ; */
clearStdIn(); /* clearStdIn ( ) ; */
newLine(); /* newLine ( ) ; */
}
} else {
fprintf(stderr, "Invalid input\n"); /* fprintf ( stderr , "Invalid input\n" ) ; */
clearStdIn(); /* clearStdIn ( ) ; */
newLine(); /* newLine ( ) ; */
}
} else {
g = readPacket(a); /* rec_packet = readPacket ( socketNum ) ; */
action((g )); /* action ( ( rec_packet ) ) ; */
}
}
}
packet readPacket(int a) /* packet readPacket ( int socketNum ) { */{
chat_header b; /* chat_header hdr ; */
int c; /* int bytesRead ; */
uint8_t d; /* uint8_t handleLen ; */
packet e; /* packet pkt ; */
if ((c = myRecv(a, &(b.len ), 2, 0) )<0) /* if ((bytesRead = myRecv(socketNum, &(hdr.len ), 2, 0) )<0) */ {
fprintf(stderr, "readPacket1:recv"); /* fprintf ( stderr , "readPacket1:recv" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (c == 0) /* if (bytesRead == 0) */ {
fprintf(stderr, "\nServer is closed\n"); /* fprintf ( stderr , "\nServer is closed\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
b.len = ntohs(b.len); /* hdr . len = ntohs ( hdr . len ) ; */
if ((c = myRecv(a, &(b.flag ), 1, 0) )<0) /* if ((bytesRead = myRecv(socketNum, &(hdr.flag ), 1, 0) )<0) */ {
fprintf(stderr, "readPacket2:recv"); /* fprintf ( stderr , "readPacket2:recv" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (c == 0) /* if (bytesRead == 0) */ {
fprintf(stderr, "\nServer is closed\n"); /* fprintf ( stderr , "\nServer is closed\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(&e, &b, 3); /* memcpy ( & pkt , & hdr , 3 ) ; */
if (b.flag == 12) /* if (hdr.flag == 12) */ {
if ((c = myRecv(a, &d, sizeof(uint8_t), 0) )<0) /* if ((bytesRead = myRecv(socketNum, &handleLen, sizeof(uint8_t), 0) )<0) */ {
fprintf(stderr, "readPacket4:recv"); /* fprintf ( stderr , "readPacket4:recv" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (c == 0) /* if (bytesRead == 0) */ {
fprintf(stderr, "\nServer is closed\n"); /* fprintf ( stderr , "\nServer is closed\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
e.data = calloc(sizeof(uint8_t) + d, sizeof(char)); /* pkt . data = calloc ( sizeof ( uint8_t ) + handleLen , sizeof ( char ) ) ; */
memcpy(e.data, &d, sizeof(uint8_t)); /* memcpy ( pkt . data , & handleLen , sizeof ( uint8_t ) ) ; */
if ((c = myRecv(a, e.data + sizeof(uint8_t), d, 0) )<0) /* if ((bytesRead = myRecv(socketNum, pkt.data + sizeof(uint8_t), handleLen, 0) )<0) */ {
fprintf(stderr, "readPacket5:recv"); /* fprintf ( stderr , "readPacket5:recv" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (c == 0) /* if (bytesRead == 0) */ {
fprintf(stderr, "\nServer is closed\n"); /* fprintf ( stderr , "\nServer is closed\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
} else {
if ((b.len - 3 )>0) /* if ((hdr.len - 3 )>0) */ {
e.data = calloc(b.len - 3, sizeof(char)); /* pkt . data = calloc ( hdr . len - 3 , sizeof ( char ) ) ; */
if ((c = myRecv(a, e.data, b.len - 3, 0) )<0) /* if ((bytesRead = myRecv(socketNum, pkt.data, hdr.len - 3, 0) )<0) */ {
fprintf(stderr, "readPacket3:recv"); /* fprintf ( stderr , "readPacket3:recv" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (c == 0) /* if (bytesRead == 0) */ {
fprintf(stderr, "\nServer is closed\n"); /* fprintf ( stderr , "\nServer is closed\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
} else {
e.data = ((void *)0 ); /* pkt . data = ( ( void * ) 0 ) ; */
}
}
return e; /* return pkt */
}
int checkHandle(int a, char *b) /* int checkHandle ( int socketNum , char * handle ) { */{
fd_set c; /* fd_set readFDs ; */
int d; /* int selectSocket ; */
char *e; /* char * handlePacket ; */
packet f; /* packet pkt ; */
uint8_t g = strlen(b); /* uint8_t handleLen = strlen ( handle ) ; */
char *h = calloc(1 + g, sizeof(char)); /* char * handlePacketData = calloc ( 1 + handleLen , sizeof ( char ) ) ; */
memcpy(h, &g, sizeof (g )); /* memcpy ( handlePacketData , & handleLen , sizeof ( handleLen ) ) ; */
memcpy(h + sizeof (g ), b, g); /* memcpy ( handlePacketData + sizeof ( handleLen ) , handle , handleLen ) ; */
e = createPacket(h, 1, 3 + g + sizeof (g ), 1); /* handlePacket = createPacket ( handlePacketData , 1 , 3 + handleLen + sizeof ( handleLen ) , 1 ) ; */
FD_ZERO(&c); /* FD_ZERO ( & readFDs ) ; */
FD_SET(a, &c); /* FD_SET ( socketNum , & readFDs ) ; */
if (sendPacket(e, a, 1)) /* if (sendPacket(handlePacket, socketNum, 1)) */ {
perror("send()"); /* perror ( "send()" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
d = mySelect(a + 1, &c, ((void *)0 ), ((void *)0 ), ((void *)0 )); /* selectSocket = mySelect ( socketNum + 1 , & readFDs , ( ( void * ) 0 ) , ( ( void * ) 0 ) , ( ( void * ) 0 ) ) ; */
if (d<0) /* if (selectSocket<0) */ {
perror("select()"); /* perror ( "select()" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (d) /* if (selectSocket) */ {
if (f.header.flag == 3) /* if (pkt.header.flag == 3) */ {
fprintf(stderr, "Handle already in use: %s\n", b); /* fprintf ( stderr , "Handle already in use: %s\n" , handle ) ; */
free(f.data); /* free ( pkt . data ) ; */
exit(0); /* exit ( 0 ) ; */
} else if (f.header.flag != 2) /* if (pkt.header.flag != 2) */ {
fprintf(stderr, "Received packet with flag=%d while waiting for flag=2\n", f.header.flag); /* fprintf ( stderr , "Received packet with flag=%d while waiting for flag=2\n" , pkt . header . flag ) ; */
free(f.data); /* free ( pkt . data ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
while ((f = readPacket(a) ).data == ((void *)0 )&&(f.header.len>3 )) /* while ((pkt = readPacket(socketNum) ).data == ((void *)0 )&&(pkt.header.len>3 )) */ {

}
free(f.data); /* free ( pkt . data ) ; */
memcpy(global_b, b, strlen(b)); /* memcpy ( myHandle , handle , strlen ( handle ) ) ; */
}
return 0; /* return 0 */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
int c; /* int socketNum ; */
if (a != 4) /* if (argc != 4) */ {
fprintf(stderr, "Usage: %s handle server-name server-port\n", b[0]); /* fprintf ( stderr , "Usage: %s handle server-name server-port\n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
}
if (strlen(b[1])>255) /* if (strlen(argv[1])>255) */ {
fprintf(stderr, "Handle must be less than 256 characters\n"); /* fprintf ( stderr , "Handle must be less than 256 characters\n" ) ; */
exit(1); /* exit ( 1 ) ; */
}
c = tcpClientSetup(b[2], b[3]); /* socketNum = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
checkHandle(c, b[1]); /* checkHandle ( socketNum , argv [ 1 ] ) ; */
runClient(c, b); /* runClient ( socketNum , argv ) ; */
return 0; /* return 0 */
}
