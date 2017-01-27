int global_e = 0; /* int socketNum = 0 ; */
char *global_c = ((void *)0 ); /* char * sendBuffer = ( ( void * ) 0 ) ; */
char *global_b = ((void *)0 ); /* char * inputBuffer = ( ( void * ) 0 ) ; */
char *global_f = ((void *)0 ); /* char * origInputBuffer = ( ( void * ) 0 ) ; */
struct chatHdr {
uint16_t packetLength;
uint8_t flag;
} global_a; /* struct chatHdr myHdr ; */
char *global_h; /* char * myHandle ; */
uint8_t global_d; /* uint8_t myHandleLength ; */
int global_g = 0; /* int numClients = 0 ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void sendInitPacket() /* void sendInitPacket ( ) { */{
int a = 0; /* int sent = 0 ; */
global_a.flag = 1; /* myHdr . flag = 1 ; */
global_a.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint8_t) + global_d); /* myHdr . packetLength = htons ( sizeof ( struct chatHdr ) + sizeof ( uint8_t ) + myHandleLength ) ; */
memcpy(global_c, &global_a, sizeof(struct chatHdr)); /* memcpy ( sendBuffer , & myHdr , sizeof ( struct chatHdr ) ) ; */
memcpy(global_c + sizeof(struct chatHdr), &global_d, sizeof(uint8_t)); /* memcpy ( sendBuffer + sizeof ( struct chatHdr ) , & myHandleLength , sizeof ( uint8_t ) ) ; */
memcpy(global_c + sizeof(struct chatHdr) + sizeof(uint8_t), global_h, global_d); /* memcpy ( sendBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , myHandle , myHandleLength ) ; */
a = mySend(global_e, global_c, ntohs(global_a.packetLength), 0); /* sent = mySend ( socketNum , sendBuffer , ntohs ( myHdr . packetLength ) , 0 ) ; */
if (a<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void initClientConnection(int a, char *b) /* void initClientConnection ( int argc , char * argv [ ] ) { */{
if (a != 4) /* if (argc != 4) */ {
printf("usage: %s host-name port-number \n", b[0]); /* printf ( "usage: %s host-name port-number \n" , argv [ 0 ] ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((global_c = (char *)malloc(1515) ) == ((void *)0 )) /* if ((sendBuffer = (char *)malloc(1515) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((global_b = (char *)malloc(65535) ) == ((void *)0 )) /* if ((inputBuffer = (char *)malloc(65535) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
global_f = global_b; /* origInputBuffer = inputBuffer ; */
global_h = b[1]; /* myHandle = argv [ 1 ] ; */
global_d = strlen(b[1]); /* myHandleLength = strlen ( argv [ 1 ] ) ; */
global_e = tcpClientSetup(b[2], b[3]); /* socketNum = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
sendInitPacket(); /* sendInitPacket ( ) ; */
}
void initSet(fd_set *a) /* void initSet ( fd_set * mySet ) { */{
FD_ZERO(a); /* FD_ZERO ( mySet ) ; */
FD_SET(0, a); /* FD_SET ( 0 , mySet ) ; */
FD_SET(global_e, a); /* FD_SET ( socketNum , mySet ) ; */
}
int iterSet(fd_set *a) /* int iterSet ( fd_set * mySet ) { */{
if (FD_ISSET(0, a)) /* if (FD_ISSET(0, mySet)) */ {
return 0; /* return 0 */
} else if (FD_ISSET(global_e, a)) /* if (FD_ISSET(socketNum, mySet)) */ {
return global_e; /* return socketNum */
} else {
perror("iter set failed"); /* perror ( "iter set failed" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void makeMessage(uint8_t a, char *b, int c) /* void makeMessage ( uint8_t destHandleLength , char * client , int messageLength ) { */{
int d = 0; /* int sent = 0 ; */
global_a.packetLength = htons(sizeof(struct chatHdr) + 2 * sizeof(uint8_t) + a + global_d + c); /* myHdr . packetLength = htons ( sizeof ( struct chatHdr ) + 2 * sizeof ( uint8_t ) + destHandleLength + myHandleLength + messageLength ) ; */
memcpy(global_c, &global_a, sizeof(struct chatHdr)); /* memcpy ( sendBuffer , & myHdr , sizeof ( struct chatHdr ) ) ; */
memcpy(global_c + sizeof(struct chatHdr), &a, sizeof(uint8_t)); /* memcpy ( sendBuffer + sizeof ( struct chatHdr ) , & destHandleLength , sizeof ( uint8_t ) ) ; */
memcpy(global_c + sizeof(struct chatHdr) + sizeof(uint8_t), b, a); /* memcpy ( sendBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , client , destHandleLength ) ; */
memcpy(global_c + sizeof(struct chatHdr) + sizeof(uint8_t) + a, &global_d, sizeof(uint8_t)); /* memcpy ( sendBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) + destHandleLength , & myHandleLength , sizeof ( uint8_t ) ) ; */
memcpy(global_c + sizeof(struct chatHdr) + 2 * sizeof(uint8_t) + a, global_h, global_d); /* memcpy ( sendBuffer + sizeof ( struct chatHdr ) + 2 * sizeof ( uint8_t ) + destHandleLength , myHandle , myHandleLength ) ; */
memcpy(global_c + sizeof(struct chatHdr) + 2 * sizeof(uint8_t) + a + global_d, global_b, c); /* memcpy ( sendBuffer + sizeof ( struct chatHdr ) + 2 * sizeof ( uint8_t ) + destHandleLength + myHandleLength , inputBuffer , messageLength ) ; */
d = mySend(global_e, global_c, ntohs(global_a.packetLength), 0); /* sent = mySend ( socketNum , sendBuffer , ntohs ( myHdr . packetLength ) , 0 ) ; */
if (d<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void makeBroadcast(char *a, char *b, int c) /* void makeBroadcast ( char * sendBuffer , char * message , int messageLength ) { */{
int d = 0; /* int sent = 0 ; */
memcpy(a, &global_a, sizeof(struct chatHdr)); /* memcpy ( sendBuffer , & myHdr , sizeof ( struct chatHdr ) ) ; */
memcpy(a + sizeof(struct chatHdr), &global_d, sizeof(uint8_t)); /* memcpy ( sendBuffer + sizeof ( struct chatHdr ) , & myHandleLength , sizeof ( uint8_t ) ) ; */
memcpy(a + sizeof(struct chatHdr) + sizeof(uint8_t), global_h, global_d); /* memcpy ( sendBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , myHandle , myHandleLength ) ; */
memcpy(a + sizeof(struct chatHdr) + sizeof(uint8_t) + global_d, b, c); /* memcpy ( sendBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) + myHandleLength , message , messageLength ) ; */
d = mySend(global_e, a, ntohs(global_a.packetLength), 0); /* sent = mySend ( socketNum , sendBuffer , ntohs ( myHdr . packetLength ) , 0 ) ; */
if (d<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void createMessagePacket(int a) /* void createMessagePacket ( int length ) { */{
int b = 0; /* int totalMessageLength = 0 ; */
int c = 0; /* int leftLength = 0 ; */
uint8_t d = 0; /* uint8_t handleLength = 0 ; */
char *e; /* char * temp ; */
char *f = (char *)malloc(255); /* char * client = ( char * ) malloc ( 255 ) ; */
global_b += 2; /* inputBuffer += 2 ; */
global_a.flag = 5; /* myHdr . flag = 5 ; */
char *g = (char *)malloc(strlen(global_b)); /* char * message = ( char * ) malloc ( strlen ( inputBuffer ) ) ; */
memcpy(g, global_b, strlen(global_b)); /* memcpy ( message , inputBuffer , strlen ( inputBuffer ) ) ; */
e = strtok(g, " "); /* temp = strtok ( message , " " ) ; */
d = strlen(e); /* handleLength = strlen ( temp ) ; */
memcpy(f, e, d); /* memcpy ( client , temp , handleLength ) ; */
global_b += d + 1; /* inputBuffer += handleLength + 1 ; */
b = a - d - 4; /* totalMessageLength = length - handleLength - 4 ; */
if (b>1000) /* if (totalMessageLength>1000) */ {
global_b += 1000; /* inputBuffer += 1000 ; */
c = b - 1000; /* leftLength = totalMessageLength - 1000 ; */
b = 1000; /* totalMessageLength = 1000 ; */
makeMessage(d, f, b); /* makeMessage ( handleLength , client , totalMessageLength ) ; */
makeMessage(d, f, c); /* makeMessage ( handleLength , client , leftLength ) ; */
} else {
makeMessage(d, f, b); /* makeMessage ( handleLength , client , totalMessageLength ) ; */
}
free(f); /* free ( client ) ; */
}
void createBroadcastPacket() /* void createBroadcastPacket ( ) { */{
global_b += 2; /* inputBuffer += 2 ; */
global_a.flag = 4; /* myHdr . flag = 4 ; */
int a = strlen(global_b); /* int totalMessageLength = strlen ( inputBuffer ) ; */
if (a>1000) /* if (totalMessageLength>1000) */ {
global_b += 1000; /* inputBuffer += 1000 ; */
global_a.packetLength = htons(1000); /* myHdr . packetLength = htons ( 1000 ) ; */
makeBroadcast(global_c, global_b, 1000); /* makeBroadcast ( sendBuffer , inputBuffer , 1000 ) ; */
global_a.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint8_t) + global_d + strlen(global_b)); /* myHdr . packetLength = htons ( sizeof ( struct chatHdr ) + sizeof ( uint8_t ) + myHandleLength + strlen ( inputBuffer ) ) ; */
makeBroadcast(global_c, global_b, a - 1000); /* makeBroadcast ( sendBuffer , inputBuffer , totalMessageLength - 1000 ) ; */
} else {
global_a.packetLength = htons(sizeof(struct chatHdr) + sizeof(uint8_t) + global_d + a); /* myHdr . packetLength = htons ( sizeof ( struct chatHdr ) + sizeof ( uint8_t ) + myHandleLength + totalMessageLength ) ; */
makeBroadcast(global_c, global_b, a); /* makeBroadcast ( sendBuffer , inputBuffer , totalMessageLength ) ; */
}
}
void killClient() /* void killClient ( ) { */{
close(global_e); /* close ( socketNum ) ; */
free(global_f); /* free ( origInputBuffer ) ; */
free(global_c); /* free ( sendBuffer ) ; */
exit(1); /* exit ( 1 ) ; */
}
void iterClients(int a, int b) /* void iterClients ( int grabbed , int grabLength ) { */{
char *c = (char *)malloc(255); /* char * handle = ( char * ) malloc ( 255 ) ; */
uint8_t d; /* uint8_t handleLength ; */
struct chatHdr e; /* struct chatHdr myHdr ; */
int f = 0; /* int secondPass = 0 ; */
int g = 0; /* int count = 0 ; */
int h = grabLength; /* int messageLength = grabLength ; */
if (!a) /* if (!grabbed) */ {
if ((h = myRecv(global_e, global_b, 1515, 0) )<0) /* if ((messageLength = myRecv(socketNum, inputBuffer, 1515, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
while (g != global_g) /* while (count != numClients) */ {
if (f) /* if (secondPass) */ {
if ((h = myRecv(global_e, global_b, 1515, 0) )<0) /* if ((messageLength = myRecv(socketNum, inputBuffer, 1515, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
memcpy(&e, global_b, sizeof(struct chatHdr)); /* memcpy ( & myHdr , inputBuffer , sizeof ( struct chatHdr ) ) ; */
f = 1; /* secondPass = 1 ; */
while (h>=ntohs(e.packetLength)) /* while (messageLength>=ntohs(myHdr.packetLength)) */ {
memcpy(&d, global_b + sizeof(struct chatHdr), sizeof(uint8_t)); /* memcpy ( & handleLength , inputBuffer + sizeof ( struct chatHdr ) , sizeof ( uint8_t ) ) ; */
g++; /* count ++ ; */
memcpy(c, global_b + sizeof(struct chatHdr) + sizeof(uint8_t), d); /* memcpy ( handle , inputBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , handleLength ) ; */
c[d] = '\0'; /* handle [ handleLength ] = '\0' ; */
printf("  %s\n", c); /* printf ( "  %s\n" , handle ) ; */
h -= ntohs(e.packetLength); /* messageLength -= ntohs ( myHdr . packetLength ) ; */
global_b += ntohs(e.packetLength); /* inputBuffer += ntohs ( myHdr . packetLength ) ; */
if (h>=3) /* if (messageLength>=3) */ {
memcpy(&e, global_b, sizeof(struct chatHdr)); /* memcpy ( & myHdr , inputBuffer , sizeof ( struct chatHdr ) ) ; */
}
}
if (h>=0) /* if (messageLength>=0) */ {
memcpy(global_f, global_b, h); /* memcpy ( origInputBuffer , inputBuffer , messageLength ) ; */
global_b = global_f; /* inputBuffer = origInputBuffer ; */
global_b += h; /* inputBuffer += messageLength ; */
}
}
free(c); /* free ( handle ) ; */
}
void parseServerInput() /* void parseServerInput ( ) { */{
char *a = (char *)malloc(255); /* char * handle = ( char * ) malloc ( 255 ) ; */
uint8_t b; /* uint8_t handleLength ; */
int c, d; /* int messageLength , netNumClients ; */
struct chatHdr e; /* struct chatHdr serverHdr ; */
char *f = (char *)malloc(1000); /* char * message = ( char * ) malloc ( 1000 ) ; */
if ((c = myRecv(global_e, global_b, 1515, 0) )<0) /* if ((messageLength = myRecv(socketNum, inputBuffer, 1515, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(&e, global_b, sizeof(struct chatHdr)); /* memcpy ( & serverHdr , inputBuffer , sizeof ( struct chatHdr ) ) ; */
switch (e.flag) /* switch (serverHdr.flag) */ {
case 0: memcpy(&b, global_b + sizeof(struct chatHdr), sizeof(uint8_t)); /* memcpy ( & handleLength , inputBuffer + sizeof ( struct chatHdr ) , sizeof ( uint8_t ) ) ; */
printf("\n%s: %s\n", a, f); /* printf ( "\n%s: %s\n" , handle , message ) ; */
memcpy(a, global_b + sizeof(struct chatHdr) + sizeof(uint8_t), b); /* memcpy ( handle , inputBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , handleLength ) ; */
memcpy(f, global_b + sizeof(struct chatHdr) + sizeof(uint8_t) + b, c); /* memcpy ( message , inputBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) + handleLength , messageLength ) ; */
c = ntohs(e.packetLength) - sizeof(struct chatHdr) - sizeof(uint8_t) - b; /* messageLength = ntohs ( serverHdr . packetLength ) - sizeof ( struct chatHdr ) - sizeof ( uint8_t ) - handleLength ; */
a[b] = '\0'; /* handle [ handleLength ] = '\0' ; */
f[c] = '\0'; /* message [ messageLength ] = '\0' ; */
case 2: break;
case 3: printf("\nHandle already in use: %s\n", global_h); /* printf ( "\nHandle already in use: %s\n" , myHandle ) ; */
exit(-1); /* exit ( - 1 ) ; */
break;
case 7: memcpy(&b, global_b + sizeof(struct chatHdr), sizeof(uint8_t)); /* memcpy ( & handleLength , inputBuffer + sizeof ( struct chatHdr ) , sizeof ( uint8_t ) ) ; */
printf("\nClient with handle %s does not exist\n", a); /* printf ( "\nClient with handle %s does not exist\n" , handle ) ; */
memcpy(a, global_b + sizeof(struct chatHdr) + sizeof(uint8_t), b); /* memcpy ( handle , inputBuffer + sizeof ( struct chatHdr ) + sizeof ( uint8_t ) , handleLength ) ; */
a[b] = '\0'; /* handle [ handleLength ] = '\0' ; */
break;
case 9: killClient(); /* killClient ( ) ; */
break;
case 11: memcpy(&d, global_b + sizeof(struct chatHdr), sizeof(uint32_t)); /* memcpy ( & netNumClients , inputBuffer + sizeof ( struct chatHdr ) , sizeof ( uint32_t ) ) ; */
printf("Number of clients: %d\n", global_g); /* printf ( "Number of clients: %d\n" , numClients ) ; */
global_g = ntohl(d); /* numClients = ntohl ( netNumClients ) ; */
if (c>ntohs(e.packetLength)) /* if (messageLength>ntohs(serverHdr.packetLength)) */ {
global_b += ntohs(e.packetLength); /* inputBuffer += ntohs ( serverHdr . packetLength ) ; */
iterClients(1, c - ntohs(global_a.packetLength)); /* iterClients ( 1 , messageLength - ntohs ( myHdr . packetLength ) ) ; */
} else {
iterClients(0, c); /* iterClients ( 0 , messageLength ) ; */
}
break;
default: break;
}
}
void makeListPacket() /* void makeListPacket ( ) { */{
int a = 0; /* int sent = 0 ; */
global_b += 2; /* inputBuffer += 2 ; */
global_a.flag = 10; /* myHdr . flag = 10 ; */
global_a.packetLength = htons(sizeof(struct chatHdr)); /* myHdr . packetLength = htons ( sizeof ( struct chatHdr ) ) ; */
memcpy(global_c, &global_a, sizeof(struct chatHdr)); /* memcpy ( sendBuffer , & myHdr , sizeof ( struct chatHdr ) ) ; */
a = mySend(global_e, global_c, ntohs(global_a.packetLength), 0); /* sent = mySend ( socketNum , sendBuffer , ntohs ( myHdr . packetLength ) , 0 ) ; */
if (a<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void createEndPacket() /* void createEndPacket ( ) { */{
int a = 0; /* int sent = 0 ; */
global_b += 2; /* inputBuffer += 2 ; */
global_a.flag = 8; /* myHdr . flag = 8 ; */
global_a.packetLength = htons(sizeof(struct chatHdr)); /* myHdr . packetLength = htons ( sizeof ( struct chatHdr ) ) ; */
memcpy(global_c, &global_a, sizeof(struct chatHdr)); /* memcpy ( sendBuffer , & myHdr , sizeof ( struct chatHdr ) ) ; */
a = mySend(global_e, global_c, ntohs(global_a.packetLength), 0); /* sent = mySend ( socketNum , sendBuffer , ntohs ( myHdr . packetLength ) , 0 ) ; */
if (a<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
parseServerInput(); /* parseServerInput ( ) ; */
}
void parseInput() /* void parseInput ( ) { */{
int a = 0; /* int length = 0 ; */
while ((global_b[a] = getchar() ) != '\n') /* while ((inputBuffer[length] = getchar() ) != '\n') */ {

}
global_b[a] = '\0'; /* inputBuffer [ length ] = '\0' ; */
global_b = global_f; /* inputBuffer = origInputBuffer ; */
if (*global_b != '%') /* if (*inputBuffer != '%') */ {
printf("Command with incorrect format\n"); /* printf ( "Command with incorrect format\n" ) ; */
return; /* return */
} else {
switch (*(++global_b )) /* switch (*(++inputBuffer )) */ {
case 'M': case 'm': createMessagePacket(a); /* createMessagePacket ( length ) ; */
break;
case 'B': case 'b': createBroadcastPacket(); /* createBroadcastPacket ( ) ; */
break;
case 'L': case 'l': makeListPacket(); /* makeListPacket ( ) ; */
break;
case 'E': case 'e': createEndPacket(); /* createEndPacket ( ) ; */
break;
default: break;
}
}
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
fd_set c; /* fd_set mySet ; */
int d = -1; /* int activeSocket = - 1 ; */
initClientConnection(a, b); /* initClientConnection ( argc , argv ) ; */
initSet(&c); /* initSet ( & mySet ) ; */
while (mySelect(global_e + 1, &c, ((void *)0 ), ((void *)0 ), ((void *)0 ))) /* while (mySelect(socketNum + 1, &mySet, ((void *)0 ), ((void *)0 ), ((void *)0 ))) */ {
d = iterSet(&c); /* activeSocket = iterSet ( & mySet ) ; */
if (d == 0) /* if (activeSocket == 0) */ {
parseInput(); /* parseInput ( ) ; */
} else {
parseServerInput(); /* parseServerInput ( ) ; */
}
initSet(&c); /* initSet ( & mySet ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
return 0; /* return 0 */
}
