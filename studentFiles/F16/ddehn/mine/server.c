int global_b; /* int serverSocket ; */
char *global_c; /* char * dataBuffer ; */
char **global_a; /* char * * clients ; */
int global_d; /* int numClients ; */
int global_f; /* int highestSocket ; */
fd_set global_e; /* fd_set mySet ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void initServerSetup() /* void initServerSetup ( ) { */{
if ((global_c = (char *)malloc(1515) ) == ((void *)0 )) /* if ((dataBuffer = (char *)malloc(1515) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((global_a = (char **)calloc(65535, sizeof(char *)) ) == ((void *)0 )) /* if ((clients = (char **)calloc(65535, sizeof(char *)) ) == ((void *)0 )) */ {
perror("client calloc() error"); /* perror ( "client calloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
global_b = tcpServerSetup(); /* serverSocket = tcpServerSetup ( ) ; */
global_f = global_b; /* highestSocket = serverSocket ; */
}
void initSet(fd_set *a) /* void initSet ( fd_set * mySet ) { */{
int b; /* int i ; */
FD_ZERO(a); /* FD_ZERO ( mySet ) ; */
FD_SET(global_b, a); /* FD_SET ( serverSocket , mySet ) ; */
for (b = 3; b<=global_f; b++) {
if (global_a[b] != 0) /* if (clients[i] != 0) */ {
FD_SET(b, a); /* FD_SET ( i , mySet ) ; */
}
}
}
int iterSet(fd_set *a) /* int iterSet ( fd_set * mySet ) { */{
int b; /* int i ; */
for (b = 3; b<=global_f; b++) {
if (FD_ISSET(b, a)) /* if (FD_ISSET(i, mySet)) */ {
return b; /* return i */
}
}
return -1; /* return -1 */
}
void makeResponsePacket(int a, uint8_t b) /* void makeResponsePacket ( int clientSocket , uint8_t flag ) { */{
int c = 0; /* int sent = 0 ; */
struct chatHdr {
uint16_t packetLength;
uint8_t flag;
} d; /* struct chatHdr myHdr ; */
d.flag = b; /* myHdr . flag = flag ; */
d.packetLength = htons(sizeof(struct chatHdr)); /* myHdr . packetLength = htons ( sizeof ( struct chatHdr ) ) ; */
memcpy(global_c, &d, sizeof(struct chatHdr)); /* memcpy ( dataBuffer , & myHdr , sizeof ( struct chatHdr ) ) ; */
c = mySend(a, global_c, sizeof(struct chatHdr), 0); /* sent = mySend ( clientSocket , dataBuffer , sizeof ( struct chatHdr ) , 0 ) ; */
if (c<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void addClient(int a, char *b) /* void addClient ( int clientSocket , char * handle ) { */{
int c; /* int i ; */
global_d++; /* numClients ++ ; */
for (c = 3; c<=global_f; c++) {
if (global_a[c] != 0) /* if (clients[i] != 0) */ {
if (strcmp(global_a[c], b) == 0) /* if (strcmp(clients[i], handle) == 0) */ {
makeResponsePacket(a, 3); /* makeResponsePacket ( clientSocket , 3 ) ; */
return; /* return */
}
}
}
makeResponsePacket(a, 2); /* makeResponsePacket ( clientSocket , 2 ) ; */
global_a[a] = b; /* clients [ clientSocket ] = handle ; */
if (a>global_f) /* if (clientSocket>highestSocket) */ {
global_f = a; /* highestSocket = clientSocket ; */
}
}
void acceptClient(int a) /* void acceptClient ( int activeSocket ) { */{
int b = -1; /* int messageLength = - 1 ; */
uint8_t c = 0; /* uint8_t handleLength = 0 ; */
char *d; /* char * handle ; */
struct chatHdr e; /* struct chatHdr myHdr ; */
int f = -1; /* int clientSocket = - 1 ; */
f = tcpAccept(a); /* clientSocket = tcpAccept ( activeSocket ) ; */
if ((b = myRecv(f, global_c, 1515, 0) )<0) /* if ((messageLength = myRecv(clientSocket, dataBuffer, 1515, 0) )<0) */ {
perror("this one, recv call"); /* perror ( "this one, recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(&e, global_c, sizeof(struct chatHdr)); /* memcpy ( & myHdr , dataBuffer , sizeof ( struct chatHdr ) ) ; */
memcpy(&c, global_c + sizeof(struct chatHdr), sizeof(uint8_t)); /* memcpy ( & handleLength , dataBuffer + sizeof ( struct chatHdr ) , sizeof ( uint8_t ) ) ; */
d = (char *)malloc(sizeof(char) * (c + 1 )); /* handle = ( char * ) malloc ( sizeof ( char ) * ( handleLength + 1 ) ) ; */
memcpy(d, global_c + sizeof(struct chatHdr) + sizeof(uint8_t), sizeof(char) * c); /* memcpy ( handle , dataBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , sizeof ( char ) * handleLength ) ; */
d[c] = '\0'; /* handle [ handleLength ] = '\0' ; */
addClient(f, d); /* addClient ( clientSocket , handle ) ; */
}
void makeHandlePacket(int a, char *b, int c, int d) /* void makeHandlePacket ( int clientSocket , char * handle , int handleLength , int flag ) { */{
struct chatHdr e; /* struct chatHdr myHdr ; */
int f = 0; /* int sent = 0 ; */
char *g = (char *)malloc(1515); /* char * temp = ( char * ) malloc ( 1515 ) ; */
e.flag = d; /* myHdr . flag = flag ; */
e.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint8_t) + c); /* myHdr . packetLength = htons ( sizeof ( struct chatHdr ) + sizeof ( uint8_t ) + handleLength ) ; */
memcpy(g, &e, sizeof(struct chatHdr)); /* memcpy ( temp , & myHdr , sizeof ( struct chatHdr ) ) ; */
memcpy(g + sizeof(struct chatHdr), &c, sizeof(uint8_t)); /* memcpy ( temp + sizeof ( struct chatHdr ) , & handleLength , sizeof ( uint8_t ) ) ; */
memcpy(g + sizeof(struct chatHdr) + sizeof(uint8_t), b, c); /* memcpy ( temp + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , handle , handleLength ) ; */
f = mySend(a, g, ntohs(e.packetLength), 0); /* sent = mySend ( clientSocket , temp , ntohs ( myHdr . packetLength ) , 0 ) ; */
if (f<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(g); /* free ( temp ) ; */
}
void makeClientNumPacket(int a) /* void makeClientNumPacket ( int clientSocket ) { */{
int b, c = 0; /* int i , sent = 0 ; */
struct chatHdr d; /* struct chatHdr myHdr ; */
uint32_t e = htonl(global_d); /* uint32_t copyNumClients = htonl ( numClients ) ; */
d.flag = 11; /* myHdr . flag = 11 ; */
d.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint32_t)); /* myHdr . packetLength = htons ( sizeof ( struct chatHdr ) + sizeof ( uint32_t ) ) ; */
memcpy(global_c, &d, sizeof(struct chatHdr)); /* memcpy ( dataBuffer , & myHdr , sizeof ( struct chatHdr ) ) ; */
memcpy(global_c + sizeof(struct chatHdr), &e, sizeof(uint32_t)); /* memcpy ( dataBuffer + sizeof ( struct chatHdr ) , & copyNumClients , sizeof ( uint32_t ) ) ; */
c = mySend(a, global_c, ntohs(d.packetLength), 0); /* sent = mySend ( clientSocket , dataBuffer , ntohs ( myHdr . packetLength ) , 0 ) ; */
if (c<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
for (b = 3; b<=global_f; b++) {
if (global_a[b] != 0) /* if (clients[i] != 0) */ {
makeHandlePacket(a, global_a[b], strlen(global_a[b]), 12); /* makeHandlePacket ( clientSocket , clients [ i ] , strlen ( clients [ i ] ) , 12 ) ; */
}
}
}
void removeClient(int a) /* void removeClient ( int clientSocket ) { */{
global_d--; /* numClients -- ; */
global_a[a] = 0; /* clients [ clientSocket ] = 0 ; */
((void)(((&global_e )->__fds_bits )[((a ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((a ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( & mySet ) -> __fds_bits ) [ ( ( clientSocket ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( clientSocket ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
makeResponsePacket(a, 9); /* makeResponsePacket ( clientSocket , 9 ) ; */
close(a); /* close ( clientSocket ) ; */
initSet(&global_e); /* initSet ( & mySet ) ; */
}
void broadcast() /* void broadcast ( ) { */{
uint8_t a; /* uint8_t handleLength ; */
struct chatHdr b; /* struct chatHdr myHdr ; */
char *c; /* char * srcHandle ; */
int d, e; /* int i , sent ; */
memcpy(&b, global_c, sizeof(struct chatHdr)); /* memcpy ( & myHdr , dataBuffer , sizeof ( struct chatHdr ) ) ; */
memcpy(&a, global_c + sizeof(struct chatHdr), sizeof(uint8_t)); /* memcpy ( & handleLength , dataBuffer + sizeof ( struct chatHdr ) , sizeof ( uint8_t ) ) ; */
c = (char *)malloc(a); /* srcHandle = ( char * ) malloc ( handleLength ) ; */
memcpy(c, global_c + sizeof(struct chatHdr) + sizeof(uint8_t), a); /* memcpy ( srcHandle , dataBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , handleLength ) ; */
for (d = 3; d<=global_f; d++) {
if (global_a[d] != 0&&(strncmp(c, global_a[d], a) != 0 )) /* if (clients[i] != 0&&(strncmp(srcHandle, clients[i], handleLength) != 0 )) */ {
memset(global_c + sizeof(uint16_t), 0, sizeof(uint8_t)); /* memset ( dataBuffer + sizeof ( uint16_t ) , 0 , sizeof ( uint8_t ) ) ; */
e = mySend(d, global_c, ntohs(b.packetLength), 0); /* sent = mySend ( i , dataBuffer , ntohs ( myHdr . packetLength ) , 0 ) ; */
if (e<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
}
}
void sendMessage(int a, char *b, int c, char *d, int e) /* void sendMessage ( int client , char * srcHandle , int srcHandleLength , char * message , int messageLength ) { */{
struct chatHdr f; /* struct chatHdr myHdr ; */
int g = 0; /* int sent = 0 ; */
char *h = (char *)malloc(1515); /* char * temp = ( char * ) malloc ( 1515 ) ; */
f.flag = 0; /* myHdr . flag = 0 ; */
f.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint8_t) + c + e); /* myHdr . packetLength = htons ( sizeof ( struct chatHdr ) + sizeof ( uint8_t ) + srcHandleLength + messageLength ) ; */
memcpy(h, &f, sizeof(struct chatHdr)); /* memcpy ( temp , & myHdr , sizeof ( struct chatHdr ) ) ; */
memcpy(h + sizeof(struct chatHdr), &c, sizeof(uint8_t)); /* memcpy ( temp + sizeof ( struct chatHdr ) , & srcHandleLength , sizeof ( uint8_t ) ) ; */
memcpy(h + sizeof(struct chatHdr) + sizeof(uint8_t), b, c); /* memcpy ( temp + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , srcHandle , srcHandleLength ) ; */
memcpy(h + sizeof(struct chatHdr) + sizeof(uint8_t) + c, d, e); /* memcpy ( temp + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) + srcHandleLength , message , messageLength ) ; */
g = mySend(a, h, ntohs(f.packetLength), 0); /* sent = mySend ( client , temp , ntohs ( myHdr . packetLength ) , 0 ) ; */
if (g<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
free(h); /* free ( temp ) ; */
}
void parsePacket(int a) /* void parsePacket ( int client ) { */{
char *b; /* char * srcHandle ; */
int c = 0; /* int found = 0 ; */
int d = -1; /* int messageLength = - 1 ; */
struct chatHdr e; /* struct chatHdr myHdr ; */
uint8_t f = 0; /* uint8_t destHandleLength = 0 ; */
char *g; /* char * message ; */
char *h; /* char * destHandle ; */
uint8_t i; /* uint8_t srcHandleLength ; */
int j, k = 0; /* int i , destSocket = 0 ; */
if ((d = myRecv(a, global_c, 1515, 0) )<0) /* if ((messageLength = myRecv(client, dataBuffer, 1515, 0) )<0) */ {
perror("parsePacket recv call"); /* perror ( "parsePacket recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(&e, global_c, sizeof(struct chatHdr)); /* memcpy ( & myHdr , dataBuffer , sizeof ( struct chatHdr ) ) ; */
switch (e.flag) /* switch (myHdr.flag) */ {
case 4: broadcast(); /* broadcast ( ) ; */
break;
case 5: memcpy(&f, global_c + sizeof(struct chatHdr), sizeof(uint8_t)); /* memcpy ( & destHandleLength , dataBuffer + sizeof ( struct chatHdr ) , sizeof ( uint8_t ) ) ; */
memcpy(h, global_c + sizeof(struct chatHdr) + sizeof(uint8_t), f); /* memcpy ( destHandle , dataBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , destHandleLength ) ; */
memcpy(b, global_c + sizeof(struct chatHdr) + 2 * sizeof(uint8_t) + f, i); /* memcpy ( srcHandle , dataBuffer + sizeof ( struct chatHdr ) + 2 * sizeof ( uint8_t ) + destHandleLength , srcHandleLength ) ; */
memcpy(&i, global_c + sizeof(struct chatHdr) + sizeof(uint8_t) + f, sizeof(uint8_t)); /* memcpy ( & srcHandleLength , dataBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) + destHandleLength , sizeof ( uint8_t ) ) ; */
h = (char *)malloc(f); /* destHandle = ( char * ) malloc ( destHandleLength ) ; */
b = (char *)malloc(i); /* srcHandle = ( char * ) malloc ( srcHandleLength ) ; */
g = global_c + 2 * sizeof(uint8_t) + sizeof(struct chatHdr) + i + f; /* message = dataBuffer + 2 * sizeof ( uint8_t ) + sizeof ( struct chatHdr ) + srcHandleLength + destHandleLength ; */
d = ntohs(e.packetLength) - 2 * sizeof(uint8_t) - i - f - sizeof(struct chatHdr); /* messageLength = ntohs ( myHdr . packetLength ) - 2 * sizeof ( uint8_t ) - srcHandleLength - destHandleLength - sizeof ( struct chatHdr ) ; */
if (!c) /* if (!found) */ {
makeHandlePacket(a, h, f, 7); /* makeHandlePacket ( client , destHandle , destHandleLength , 7 ) ; */
} else {
sendMessage(k, b, i, g, d); /* sendMessage ( destSocket , srcHandle , srcHandleLength , message , messageLength ) ; */
}
for (j = 3; j<=global_f; j++) {
if (global_a[j] != 0) /* if (clients[i] != 0) */ {
if (strncmp(global_a[j], h, f) == 0) /* if (strncmp(clients[i], destHandle, destHandleLength) == 0) */ {
c = 1; /* found = 1 ; */
k = j; /* destSocket = i ; */
}
}
}
break;
case 10: makeClientNumPacket(a); /* makeClientNumPacket ( client ) ; */
break;
case 8: removeClient(a); /* removeClient ( client ) ; */
break;
default: break;
}
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
int c; /* int activeSocket ; */
global_d = 0; /* numClients = 0 ; */
global_f = -1; /* highestSocket = - 1 ; */
initServerSetup(); /* initServerSetup ( ) ; */
initSet(&global_e); /* initSet ( & mySet ) ; */
while (mySelect(global_f + 1, &global_e, ((void *)0 ), ((void *)0 ), ((void *)0 ))) /* while (mySelect(highestSocket + 1, &mySet, ((void *)0 ), ((void *)0 ), ((void *)0 ))) */ {
c = iterSet(&global_e); /* activeSocket = iterSet ( & mySet ) ; */
if (c<0) /* if (activeSocket<0) */ {
perror("iterSet failed"); /* perror ( "iterSet failed" ) ; */
} else if (c == global_b) /* if (activeSocket == serverSocket) */ {
acceptClient(c); /* acceptClient ( activeSocket ) ; */
} else {
parsePacket(c); /* parsePacket ( activeSocket ) ; */
}
initSet(&global_e); /* initSet ( & mySet ) ; */
}
return 0; /* return 0 */
}
