struct chatHeader {
uint16_t length;
uint8_t flag;
};
struct messagePacket {
struct chatHeader chdr;
uint8_t destLen;
char *destHandle;
uint8_t srcLen;
char *srcHandle;
char *message;
};
uint32_t global_a; /* static uint32_t num_handles ; */

unsigned int __bswap_32(unsigned int a) /* int maxFd = socket_num + 1 , fdIdx , srcLen = */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /* * send_buf = ( ( void * ) 0 */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void sourceInit(char **a, char **b, int *c) /* void sourceInit ( char * * argv , char * * srcName , int * srcLen ) { */{
*c = strlen(a[1]); /* * srcLen = strlen ( argv [ 1 ] ) ; */
if ((*b = (char *)malloc(*c + 1) ) == ((void *)0 )) /* if ((*srcName = (char *)malloc(*srcLen + 1) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(*b, a[1], *c); /* memcpy ( * srcName , argv [ 1 ] , * srcLen ) ; */
(*b )[*c] = '\0'; /* ( * srcName ) [ * srcLen ] = '\0' ; */
}
void buffInit(char **a) /* void buffInit ( char * * buf ) { */{
if ((*a = (char *)malloc(2048) ) == ((void *)0 )) /* if ((*buf = (char *)malloc(2048) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
int parseMessage(struct messagePacket *a, int b, int c, char *d) /* int parseMessage ( struct messagePacket * msg , int tag , int mesLen , char * send_buf ) { */{
char *e, *f; /* char * destName , * message ; */
int g = 3, h = index; /* int index = 3 , start = index ; */
if (b) /* if (tag) */ {
while (d[g] != ' ') /* while (send_buf[index] != ' ') */ {
g++; /* index ++ ; */
}
if (g - h<256&&g<c) /* if (index - start<256&&index<mesLen) */ {
if ((e = (char *)malloc(g - h) ) == ((void *)0 )) /* if ((destName = (char *)malloc(index - start) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
} else {
printf("Invalid Command\n"); /* printf ( "Invalid Command\n" ) ; */
return -1; /* return -1 */
}
memcpy(e, d + h, g - h); /* memcpy ( destName , send_buf + start , index - start ) ; */
e[g - h] = '\0'; /* destName [ index - start ] = '\0' ; */
a->destHandle = e; /* msg -> destHandle = destName ; */
a->destLen = strlen(e); /* msg -> destLen = strlen ( destName ) ; */
}
if ((c - g )<(1000 - a->chdr.length )) /* if ((mesLen - index )<(1000 - msg->chdr.length )) */ {
if ((f = (char *)malloc(c + 1 - g) ) == ((void *)0 )) /* if ((message = (char *)malloc(mesLen + 1 - index) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(f, d + g, c - g); /* memcpy ( message , send_buf + index , mesLen - index ) ; */
a->message = f; /* msg -> message = message ; */
a->message[c - g] = '\0'; /* msg -> message [ mesLen - index ] = '\0' ; */
g = c; /* index = mesLen ; */
} else {
if ((f = (char *)malloc(1000 - a->chdr.length) ) == ((void *)0 )) /* if ((message = (char *)malloc(1000 - msg->chdr.length) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(f, d + g, (1000 - a->chdr.length - 1 )); /* memcpy ( message , send_buf + index , ( 1000 - msg -> chdr . length - 1 ) ) ; */
a->message = f; /* msg -> message = message ; */
a->message[1000 - a->chdr.length] = '\0'; /* msg -> message [ 1000 - msg -> chdr . length ] = '\0' ; */
g += (1000 - a->chdr.length ); /* index += ( 1000 - msg -> chdr . length ) ; */
}
return g; /* return index */
}
char * packer(struct messagePacket *a) /* char * packer ( struct messagePacket * msg ) { */{
int b = 2; /* int index = 2 ; */
char *c; /* char * packet ; */
int16_t d; /* int16_t length ; */
if ((c = (char *)malloc(a->chdr.length) ) == ((void *)0 )) /* if ((packet = (char *)malloc(msg->chdr.length) ) == ((void *)0 )) */ {
perror("Packer malloc() error"); /* perror ( "Packer malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
d = htons(a->chdr.length); /* length = htons ( msg -> chdr . length ) ; */
if (a->chdr.flag == 5) /* if (msg->chdr.flag == 5) */ {
memcpy(c, &(d ), 2); /* memcpy ( packet , & ( length ) , 2 ) ; */
memcpy(c + b, &(a->chdr.flag ), 1); /* memcpy ( packet + index , & ( msg -> chdr . flag ) , 1 ) ; */
b++; /* index ++ ; */
memcpy(c + b, &(a->destLen ), 1); /* memcpy ( packet + index , & ( msg -> destLen ) , 1 ) ; */
b++; /* index ++ ; */
memcpy(c + b, a->destHandle, a->destLen); /* memcpy ( packet + index , msg -> destHandle , msg -> destLen ) ; */
b = b + a->destLen; /* index = index + msg -> destLen ; */
memcpy(c + b, &(a->srcLen ), 1); /* memcpy ( packet + index , & ( msg -> srcLen ) , 1 ) ; */
b++; /* index ++ ; */
memcpy(c + b, a->srcHandle, a->srcLen); /* memcpy ( packet + index , msg -> srcHandle , msg -> srcLen ) ; */
b = b + a->srcLen; /* index = index + msg -> srcLen ; */
memcpy(c + b, a->message, (a->chdr.length ) - b); /* memcpy ( packet + index , msg -> message , ( msg -> chdr . length ) - index ) ; */
} else if (a->chdr.flag == 4) /* if (msg->chdr.flag == 4) */ {
b++; /* index ++ ; */
memcpy(c, &d, 2); /* memcpy ( packet , & length , 2 ) ; */
memcpy(c + b, &(a->chdr.flag ), 1); /* memcpy ( packet + index , & ( msg -> chdr . flag ) , 1 ) ; */
memcpy(c + b, &(a->srcLen ), 1); /* memcpy ( packet + index , & ( msg -> srcLen ) , 1 ) ; */
b++; /* index ++ ; */
memcpy(c + b, a->srcHandle, a->srcLen); /* memcpy ( packet + index , msg -> srcHandle , msg -> srcLen ) ; */
b = b + a->srcLen; /* index = index + msg -> srcLen ; */
memcpy(c + b, a->message, (a->chdr.length ) - b); /* memcpy ( packet + index , msg -> message , ( msg -> chdr . length ) - index ) ; */
}
return c; /* return packet */
}
void messageSender(char *a, int b, struct messagePacket c) /* void messageSender ( char * data , int socket_num , struct messagePacket msg ) { */{
if (send(b, a, c.chdr.length, 0)<0) /* if (send(socket_num, data, msg.chdr.length, 0)<0) */ {
perror("Error on send"); /* perror ( "Error on send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(a); /* free ( data ) ; */
}
int ackServer(int a) /* int ackServer ( int socket_num ) { */{
char b[3]; /* char handle [ 3 ] ; */
struct chatHeader c; /* struct chatHeader hdr ; */
int d; /* int rec ; */
if ((d = recv(a, b, 3, 0) )<0) /* if ((rec = recv(socket_num, handle, 3, 0) )<0) */ {
perror("Receive message error in getClientName\n"); /* perror ( "Receive message error in getClientName\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(&c, b, 3); /* memcpy ( & hdr , handle , 3 ) ; */
if (c.flag == 2) /* if (hdr.flag == 2) */ {
return 0; /* return 0 */
} else if (c.flag == 3) /* if (hdr.flag == 3) */ {
return -1; /* return -1 */
}
}
void printErrMsg(char a) /* void printErrMsg ( char buff [ ] ) { */{
struct chatHeader b; /* struct chatHeader hdr ; */
int c, d = 3; /* int destLen , index = 3 ; */
char e[256]; /* char dest [ 256 ] ; */
uint8_t f; /* uint8_t length ; */
memcpy(&b, a, 3); /* memcpy ( & hdr , buff , 3 ) ; */
memcpy(&f, a + 3, 1); /* memcpy ( & length , buff + 3 , 1 ) ; */
d++; /* index ++ ; */
c = f; /* destLen = length ; */
memcpy(e, a + d, c); /* memcpy ( dest , buff + index , destLen ) ; */
e[c] = '\0'; /* dest [ destLen ] = '\0' ; */
printf("\nClient with handle %s does not exist\n", e); /* printf ( "\nClient with handle %s does not exist\n" , dest ) ; */
}
void printMessage(char a) /* void printMessage ( char buff [ ] ) { */{
uint8_t b; /* uint8_t length ; */
int c, d = 3; /* int srcLen , index = 3 ; */
char e[256], f[1000]; /* char src [ 256 ] , msg [ 1000 ] ; */
uint16_t g; /* uint16_t tLen ; */
struct chatHeader h; /* struct chatHeader hdr ; */
memcpy(&h, a, 3); /* memcpy ( & hdr , buff , 3 ) ; */
c = b; /* srcLen = length ; */
g = ntohs(h.length); /* tLen = ntohs ( hdr . length ) ; */
h.length = g; /* hdr . length = tLen ; */
memcpy(&b, a + 3, 1); /* memcpy ( & length , buff + 3 , 1 ) ; */
d += b + 1; /* index += length + 1 ; */
memcpy(&b, a + d, 1); /* memcpy ( & length , buff + index , 1 ) ; */
d++; /* index ++ ; */
memcpy(e, a + d, c); /* memcpy ( src , buff + index , srcLen ) ; */
d += c; /* index += srcLen ; */
memcpy(f, a + d, h.length - d); /* memcpy ( msg , buff + index , hdr . length - index ) ; */
printf("\n%s: %s\n", e, f); /* printf ( "\n%s: %s\n" , src , msg ) ; */
}
void printBroadcast(char a) /* void printBroadcast ( char buff [ ] ) { */{
char b[256], c[1000]; /* char dest [ 256 ] , msg [ 1000 ] ; */
struct chatHeader d; /* struct chatHeader hdr ; */
int e, f = 3; /* int destLen , index = 3 ; */
uint8_t g; /* uint8_t length ; */
uint16_t h; /* uint16_t tLen ; */
memcpy(&d, a, 3); /* memcpy ( & hdr , buff , 3 ) ; */
e = g; /* destLen = length ; */
h = ntohs(d.length); /* tLen = ntohs ( hdr . length ) ; */
d.length = h; /* hdr . length = tLen ; */
memcpy(&g, a + 3, 1); /* memcpy ( & length , buff + 3 , 1 ) ; */
f++; /* index ++ ; */
memcpy(b, a + f, e); /* memcpy ( dest , buff + index , destLen ) ; */
b[e] = '\0'; /* dest [ destLen ] = '\0' ; */
f += e; /* index += destLen ; */
memcpy(c, a + f, d.length - f); /* memcpy ( msg , buff + index , hdr . length - index ) ; */
printf("\n%s: %s\n", b, c); /* printf ( "\n%s: %s\n" , dest , msg ) ; */
}
void listNumHandles(char a) /* void listNumHandles ( char buff [ ] ) { */{
uint32_t b; /* uint32_t temp ; */
memcpy(&b, a + 3, sizeof(uint32_t)); /* memcpy ( & temp , buff + 3 , sizeof ( uint32_t ) ) ; */
global_a = ntohl(b); /* num_handles = ntohl ( temp ) ; */
printf("\nNumber of clients %d\n", global_a); /* printf ( "\nNumber of clients %d\n" , num_handles ) ; */
}
void receiveList(char a, int b) /* void receiveList ( char buff [ ] , int socket_num ) { */{
uint8_t c, d = 3 + 1; /* uint8_t length , offset = 3 + 1 ; */
uint32_t e = 1; /* uint32_t index = 1 ; */
char f[1000]; /* char handle [ 1000 ] ; */
memcpy(&c, a + 3, 1); /* memcpy ( & length , buff + 3 , 1 ) ; */
memcpy(f, a + d, c); /* memcpy ( handle , buff + offset , length ) ; */
f[c] = '\0'; /* handle [ length ] = '\0' ; */
printf("\n   %s\n", f); /* printf ( "\n   %s\n" , handle ) ; */
while (e<global_a) /* while (index<num_handles) */ {
e++; /* index ++ ; */
if (recv(b, a, 1000, 0)<0) /* if (recv(socket_num, buff, 1000, 0)<0) */ {
perror("Error on Network receive"); /* perror ( "Error on Network receive" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(&c, a + 3, 1); /* memcpy ( & length , buff + 3 , 1 ) ; */
memcpy(f, a + d, c); /* memcpy ( handle , buff + offset , length ) ; */
f[c] = '\0'; /* handle [ length ] = '\0' ; */
printf("   %s\n", f); /* printf ( "   %s\n" , handle ) ; */
memset(f, 0, 1000); /* memset ( handle , 0 , 1000 ) ; */
}
}
void receiveMsg(int a) /* void receiveMsg ( int socket_num ) { */{
struct chatHeader b; /* struct chatHeader hdr ; */
int c = 0; /* int rec = 0 ; */
char d[1000]; /* char buff [ 1000 ] ; */
if ((c = recv(a, d, 1000, 0) )<0) /* if ((rec = recv(socket_num, buff, 1000, 0) )<0) */ {
perror("Error on Network receive"); /* perror ( "Error on Network receive" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (c == 0) /* if (rec == 0) */ {
close(a); /* close ( socket_num ) ; */
printf("\n"); /* printf ( "\n" ) ; */
exit(0); /* exit ( 0 ) ; */
}
memcpy(&b, d, 3); /* memcpy ( & hdr , buff , 3 ) ; */
if (b.flag == 4) /* if (hdr.flag == 4) */ {
printBroadcast(d); /* printBroadcast ( buff ) ; */
} else if (b.flag == 5) /* if (hdr.flag == 5) */ {
printMessage(d); /* printMessage ( buff ) ; */
} else if (b.flag == 7) /* if (hdr.flag == 7) */ {
printErrMsg(d); /* printErrMsg ( buff ) ; */
} else if (b.flag == 9) /* if (hdr.flag == 9) */ {
close(a); /* close ( socket_num ) ; */
printf("\n"); /* printf ( "\n" ) ; */
exit(0); /* exit ( 0 ) ; */
} else if (b.flag == 11) /* if (hdr.flag == 11) */ {
listNumHandles(d); /* listNumHandles ( buff ) ; */
} else if (b.flag == 12) /* if (hdr.flag == 12) */ {
receiveList(d, a); /* receiveList ( buff , socket_num ) ; */
}
}
void messagePacketInit(struct messagePacket *a) /* void messagePacketInit ( struct messagePacket * msg ) { */{
a->chdr.length = 3; /* msg -> chdr . length = 3 ; */
a->destHandle = ((void *)0 ); /* msg -> destHandle = ( ( void * ) 0 ) ; */
a->destLen = 0; /* msg -> destLen = 0 ; */
a->srcHandle = ((void *)0 ); /* msg -> srcHandle = ( ( void * ) 0 ) ; */
a->srcLen = 0; /* msg -> srcLen = 0 ; */
a->message = ((void *)0 ); /* msg -> message = ( ( void * ) 0 ) ; */
}
void exiting(int a) /* void exiting ( int socket_num ) { */{
char b[3]; /* char packet [ 3 ] ; */
struct chatHeader c; /* struct chatHeader hdr ; */
c.flag = 8; /* hdr . flag = 8 ; */
c.length = htons(3); /* hdr . length = htons ( 3 ) ; */
memcpy(b, &c, 3); /* memcpy ( packet , & hdr , 3 ) ; */
if (send(a, b, 3, 0)<0) /* if (send(socket_num, packet, 3, 0)<0) */ {
perror("Error on Exit Send"); /* perror ( "Error on Exit Send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void listHandles(int a) /* void listHandles ( int socket_num ) { */{
struct chatHeader b; /* struct chatHeader hdr ; */
char c[3]; /* char packet [ 3 ] ; */
b.flag = 10; /* hdr . flag = 10 ; */
b.length = htons(3); /* hdr . length = htons ( 3 ) ; */
memcpy(c, &b, 3); /* memcpy ( packet , & hdr , 3 ) ; */
if (send(a, c, 3, 0)<0) /* if (send(socket_num, packet, 3, 0)<0) */ {
perror("Error on List Send"); /* perror ( "Error on List Send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void messageChoice(char *a, char b, char *c, int d, int e, int f) /* void messageChoice ( char * send_buf , char pktType [ ] , char * srcName , int srcLen , int socket_num , int mesLen ) { */{
uint8_t g; /* uint8_t flag ; */
int h = 0, i = 0; /* int destFlag = 0 , index = 0 ; */
char *j = ((void *)0 ); /* char * data = ( ( void * ) 0 ) ; */
struct messagePacket k; /* struct messagePacket msg ; */
messagePacketInit(&k); /* messagePacketInit ( & msg ) ; */
k.srcLen = d; /* msg . srcLen = srcLen ; */
k.srcHandle = c; /* msg . srcHandle = srcName ; */
if ((strcmp(b, "%m") == 0 ) || (strcmp(b, "%M") == 0 )) /* if ((strcmp(pktType, "%m") == 0 ) || (strcmp(pktType, "%M") == 0 )) */ {
h = 1; /* destFlag = 1 ; */
k.chdr.flag = 5; /* msg . chdr . flag = 5 ; */
k.chdr.length = k.chdr.length + k.srcLen + 1; /* msg . chdr . length = msg . chdr . length + msg . srcLen + 1 ; */
while (i != f) /* while (index != mesLen) */ {
i = parseMessage(&k, h, f, a); /* index = parseMessage ( & msg , destFlag , mesLen , send_buf ) ; */
if (i<0) /* if (index<0) */ {
i = f; /* index = mesLen ; */
} else {
j = packer(&k); /* data = packer ( & msg ) ; */
memcpy(&g, j + 2, 1); /* memcpy ( & flag , data + 2 , 1 ) ; */
messageSender(j, e, k); /* messageSender ( data , socket_num , msg ) ; */
free(k.message); /* free ( msg . message ) ; */
}
}
free(k.destHandle); /* free ( msg . destHandle ) ; */
} else if ((strcmp(b, "%b") == 0 ) || (strcmp(b, "%B") == 0 )) /* if ((strcmp(pktType, "%b") == 0 ) || (strcmp(pktType, "%B") == 0 )) */ {
k.chdr.flag = 4; /* msg . chdr . flag = 4 ; */
k.chdr.length = k.chdr.length + k.srcLen + 1; /* msg . chdr . length = msg . chdr . length + msg . srcLen + 1 ; */
while (i != f) /* while (index != mesLen) */ {
i = parseMessage(&k, h, f, a); /* index = parseMessage ( & msg , destFlag , mesLen , send_buf ) ; */
if (i<0) /* if (index<0) */ {
i = f; /* index = mesLen ; */
} else {
j = packer(&k); /* data = packer ( & msg ) ; */
messageSender(j, e, k); /* messageSender ( data , socket_num , msg ) ; */
free(k.message); /* free ( msg . message ) ; */
k.message = ((void *)0 ); /* msg . message = ( ( void * ) 0 ) ; */
}
}
} else if ((strcmp(b, "%l") == 0 ) || (strcmp(b, "%L") == 0 )) /* if ((strcmp(pktType, "%l") == 0 ) || (strcmp(pktType, "%L") == 0 )) */ {
printf("List flag\n"); /* printf ( "List flag\n" ) ; */
listHandles(e); /* listHandles ( socket_num ) ; */
} else if ((strcmp(b, "%e") == 0 ) || (strcmp(b, "%E") == 0 )) /* if ((strcmp(pktType, "%e") == 0 ) || (strcmp(pktType, "%E") == 0 )) */ {
exiting(e); /* exiting ( socket_num ) ; */
}
}
void getRead(char **a, char b, int *c) /* void getRead ( char * * send_buf , char pktType [ ] [ 3 ] , int * mesLen ) { */{
while (((*a )[*c] = getchar() ) != '\n'&&*c<2048 - 1) /* while (((*send_buf )[*mesLen] = getchar() ) != '\n'&&*mesLen<2048 - 1) */ {
*c = *c + 1; /* * mesLen = * mesLen + 1 ; */
}
(*a )[*c] = '\0'; /* ( * send_buf ) [ * mesLen ] = '\0' ; */
*c = *c + 1; /* * mesLen = * mesLen + 1 ; */
memcpy(*b, *a, 2); /* memcpy ( * pktType , * send_buf , 2 ) ; */
(*b )[2] = '\0'; /* ( * pktType ) [ 2 ] = '\0' ; */
}
int msgSystem(char **a, int b) /* int msgSystem ( char * * argv , int socket_num ) { */{
fd_set c, d; /* fd_set active , current ; */
char *e = ((void *)0 ), *f = ((void *)0 ), g[3]; /* char * send_buf = ( ( void * ) 0 ) , * srcName = ( ( void * ) 0 ) , pktType [ 3 ] ; */
int h = b + 1, i, j = 0, k = 0; /* int maxFd = socket_num + 1 , fdIdx , srcLen = 0 , mesLen = 0 ; */
FD_ZERO(&c); /* FD_ZERO ( & active ) ; */
FD_SET(0, &c); /* FD_SET ( 0 , & active ) ; */
FD_SET(b, &c); /* FD_SET ( socket_num , & active ) ; */
sourceInit(a, &f, &j); /* sourceInit ( argv , & srcName , & srcLen ) ; */
buffInit(&e); /* buffInit ( & send_buf ) ; */
while (1) /* while (1) */ {
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
i = 0; /* fdIdx = 0 ; */
d = c; /* current = active ; */
if (select(h, &d, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (select(maxFd, &current, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select"); /* perror ( "select" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
while (i<h) /* while (fdIdx<maxFd) */ {
i++; /* fdIdx ++ ; */
if (FD_ISSET(i, &d)) /* if (FD_ISSET(fdIdx, &current)) */ {
if (i == 0) /* if (fdIdx == 0) */ {
getRead(&e, &g, &k); /* getRead ( & send_buf , & pktType , & mesLen ) ; */
messageChoice(e, g, f, j, b, k); /* messageChoice ( send_buf , pktType , srcName , srcLen , socket_num , mesLen ) ; */
} else if (i == b) /* if (fdIdx == socket_num) */ {
receiveMsg(i); /* receiveMsg ( fdIdx ) ; */
}
}
}
memset(e, 0, 2048); /* memset ( send_buf , 0 , 2048 ) ; */
memset(g, 0, 2); /* memset ( pktType , 0 , 2 ) ; */
}
}
int ccSetup(int a, char **b) /* int ccSetup ( int argc , char * * argv ) { */{
int c, d = 0; /* int socket_num , index = 0 ; */
uint16_t e = 0; /* uint16_t packetlen = 0 ; */
struct hostent *f; /* struct hostent * hp ; */
char *g; /* char * initPacket ; */
uint8_t h = 0, i = 1; /* uint8_t handlen = 0 , flag = 1 ; */
struct sockaddr_in j; /* struct sockaddr_in remote ; */
j.sin_family = 2; /* remote . sin_family = 2 ; */
if (a != 4) /* if (argc != 4) */ {
printf("cclient handle server-name server-port\n"); /* printf ( "cclient handle server-name server-port\n" ) ; */
exit(1); /* exit ( 1 ) ; */
}
if ((c = socket(2, SOCK_STREAM, 0) )<0) /* if ((socket_num = socket(2, SOCK_STREAM, 0) )<0) */ {
perror("socket call\n"); /* perror ( "socket call\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((f = gethostbyname(b[2]) ) == ((void *)0 )) /* if ((hp = gethostbyname(argv[2]) ) == ((void *)0 )) */ {
printf("Error getting hostname: %s\n", b[2]); /* printf ( "Error getting hostname: %s\n" , argv [ 2 ] ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy((char *)&j.sin_addr, (char *)f->h_addr_list[0], f->h_length); /* memcpy ( ( char * ) & remote . sin_addr , ( char * ) hp -> h_addr_list [ 0 ] , hp -> h_length ) ; */
j.sin_port = htons(atoi(b[a - 1])); /* remote . sin_port = htons ( atoi ( argv [ argc - 1 ] ) ) ; */
if (connect(c, (struct sockaddr *)&j, sizeof(struct sockaddr_in))<0) /* if (connect(socket_num, (struct sockaddr *)&remote, sizeof(struct sockaddr_in))<0) */ {
perror("connect call\n"); /* perror ( "connect call\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((g = (char *)malloc(3 + 1 + 1 + strlen(b[1])) ) == ((void *)0 )) /* if ((initPacket = (char *)malloc(3 + 1 + 1 + strlen(argv[1])) ) == ((void *)0 )) */ {
perror("Initial Packet malloc() error"); /* perror ( "Initial Packet malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
e = htons(3 + 1 + 1 + strlen(b[1])); /* packetlen = htons ( 3 + 1 + 1 + strlen ( argv [ 1 ] ) ) ; */
h = strlen(b[1]); /* handlen = strlen ( argv [ 1 ] ) ; */
memcpy(g, &e, 2); /* memcpy ( initPacket , & packetlen , 2 ) ; */
d += 2; /* index += 2 ; */
memcpy(g + d, &i, 1); /* memcpy ( initPacket + index , & flag , 1 ) ; */
d++; /* index ++ ; */
memcpy(g + d, &h, 1); /* memcpy ( initPacket + index , & handlen , 1 ) ; */
d++; /* index ++ ; */
memcpy(g + d, b[1], h); /* memcpy ( initPacket + index , argv [ 1 ] , handlen ) ; */
d += h; /* index += handlen ; */
g[d] = '\0'; /* initPacket [ index ] = '\0' ; */
if (send(c, g, ntohs(e), MSG_CONFIRM)<0) /* if (send(socket_num, initPacket, ntohs(packetlen), MSG_CONFIRM)<0) */ {
perror("Error on Send"); /* perror ( "Error on Send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (ackServer(c)<0) /* if (ackServer(socket_num)<0) */ {
printf("Handle already in use: %s\n", b[1]); /* printf ( "Handle already in use: %s\n" , argv [ 1 ] ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(g); /* free ( initPacket ) ; */
return c; /* return socket_num */
}
int main(int a, char **b) /* int main ( int argc , char * * argv ) { */{
int c = 0, d = 0; /* int socket_num = 0 , flag = 0 ; */
c = ccSetup(a, b); /* socket_num = ccSetup ( argc , argv ) ; */
d = msgSystem(b, c); /* flag = msgSystem ( argv , socket_num ) ; */
if (!d) /* if (!flag) */ {
close(c); /* close ( socket_num ) ; */
return 0; /* return 0 */
}
}
