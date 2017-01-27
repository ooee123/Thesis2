int global_c = 0; /* int socketFD = 0 ; */
int global_a = 0; /* int isConnected = 0 ; */
char *global_b = ((void *)0 ); /* char * myHandle = ( ( void * ) 0 ) ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
if (a != 4) /* if (argc != 4) */ {
printf("usage: %s handle host-name port-number \n", b[0]); /* printf ( "usage: %s handle host-name port-number \n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
}
global_c = tcpClientSetup(b[2], b[3]); /* socketFD = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
global_b = b[1]; /* myHandle = argv [ 1 ] ; */
sendPacketConnection(global_c); /* sendPacketConnection ( socketFD ) ; */
process(); /* process ( ) ; */
return 0; /* return 0 */
}
void sendPacketConnection(int a) /* void sendPacketConnection ( int socket ) { */{
int b = strlen(global_b); /* int hLen = strlen ( myHandle ) ; */
void *c = makePacketHandle(1, b, global_b); /* void * packet = makePacketHandle ( 1 , hLen , myHandle ) ; */
if (sendPacket(c, a)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Connection"); /* perror ( "Packet Connection" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(c); /* free ( packet ) ; */
}
void sendPacketBroadCast(char *a, int b) /* void sendPacketBroadCast ( char * mssg , int socket ) { */{
char *c; /* char * toSend ; */
void *d; /* void * packet ; */
int e = strlen(global_b); /* int hLen = strlen ( myHandle ) ; */
int f = 1000 - e - sizeof(header) - 1; /* int maxMssgLen = 1000 - hLen - sizeof ( header ) - 1 ; */
int g = (strlen(a) + 1 ) / f; /* int mssgNum = ( strlen ( mssg ) + 1 ) / maxMssgLen ; */
int h = 0; /* int consumed = 0 ; */
int i = ((strlen(a) + 1 ) % f ) + g; /* int theRest = ( ( strlen ( mssg ) + 1 ) % maxMssgLen ) + mssgNum ; */
c = malloc(f); /* toSend = malloc ( maxMssgLen ) ; */
while (g>0) /* while (mssgNum>0) */ {
memcpy(c, a + h, f - 1); /* memcpy ( toSend , mssg + consumed , maxMssgLen - 1 ) ; */
c[f - 1] = 0; /* toSend [ maxMssgLen - 1 ] = 0 ; */
h += f - 1; /* consumed += maxMssgLen - 1 ; */
d = makePacketBroadcast(4, e, global_b, c); /* packet = makePacketBroadcast ( 4 , hLen , myHandle , toSend ) ; */
if (sendPacket(d, b)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Broadcast"); /* perror ( "Packet Broadcast" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(d); /* free ( packet ) ; */
g--; /* mssgNum -- ; */
}
free(c); /* free ( toSend ) ; */
c = malloc(i); /* toSend = malloc ( theRest ) ; */
memcpy(c, a + h, i); /* memcpy ( toSend , mssg + consumed , theRest ) ; */
d = makePacketBroadcast(4, e, global_b, c); /* packet = makePacketBroadcast ( 4 , hLen , myHandle , toSend ) ; */
if (sendPacket(d, b)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Broadcast"); /* perror ( "Packet Broadcast" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(d); /* free ( packet ) ; */
free(c); /* free ( toSend ) ; */
}
void sendPacketMssg(char *a, char *b, int c) /* void sendPacketMssg ( char * mssg , char * dest , int socket ) { */{
char *d; /* char * toSend ; */
void *e; /* void * packet ; */
int f = strlen(global_b); /* int srcLen = strlen ( myHandle ) ; */
int g = strlen(b); /* int destLen = strlen ( dest ) ; */
int h = 1000 - (f + 1 ) - (g + 1 ) - sizeof(header); /* int maxMssgLen = 1000 - ( srcLen + 1 ) - ( destLen + 1 ) - sizeof ( header ) ; */
int i = (strlen(a) + 1 ) / h; /* int mssgNum = ( strlen ( mssg ) + 1 ) / maxMssgLen ; */
int j = 0; /* int consumed = 0 ; */
int k = ((strlen(a) + 1 ) % h ) + i; /* int theRest = ( ( strlen ( mssg ) + 1 ) % maxMssgLen ) + mssgNum ; */
d = malloc(h); /* toSend = malloc ( maxMssgLen ) ; */
while (i>0) /* while (mssgNum>0) */ {
memcpy(d, a + j, h - 1); /* memcpy ( toSend , mssg + consumed , maxMssgLen - 1 ) ; */
d[h - 1] = 0; /* toSend [ maxMssgLen - 1 ] = 0 ; */
j += h - 1; /* consumed += maxMssgLen - 1 ; */
e = makePacketMssg(5, g, b, f, global_b, d); /* packet = makePacketMssg ( 5 , destLen , dest , srcLen , myHandle , toSend ) ; */
if (sendPacket(e, c)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Message"); /* perror ( "Packet Message" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(e); /* free ( packet ) ; */
i--; /* mssgNum -- ; */
}
free(d); /* free ( toSend ) ; */
d = malloc(k); /* toSend = malloc ( theRest ) ; */
memcpy(d, a + j, k); /* memcpy ( toSend , mssg + consumed , theRest ) ; */
e = makePacketMssg(5, g, b, f, global_b, d); /* packet = makePacketMssg ( 5 , destLen , dest , srcLen , myHandle , toSend ) ; */
if (sendPacket(e, c)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Message"); /* perror ( "Packet Message" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(e); /* free ( packet ) ; */
free(d); /* free ( toSend ) ; */
}
int getFDForInput() /* int getFDForInput ( ) { */{
fd_set a; /* fd_set fdvar ; */
int b = global_c + 1; /* int maxFD = socketFD + 1 ; */
FD_ZERO(&a); /* FD_ZERO ( & fdvar ) ; */
FD_SET(global_c, &a); /* FD_SET ( socketFD , & fdvar ) ; */
if (global_a) /* if (isConnected) */ {
FD_SET(fileno(stdin), &a); /* FD_SET ( fileno ( stdin ) , & fdvar ) ; */
}
if (mySelect(b, (fd_set *)&a, ((void *)0 ), ((void *)0 ), ((void *)0 )) == -1) /* if (mySelect(maxFD, (fd_set *)&fdvar, ((void *)0 ), ((void *)0 ), ((void *)0 )) == -1) */ {
perror("Error selecting FD to read from"); /* perror ( "Error selecting FD to read from" ) ; */
exit(1); /* exit ( 1 ) ; */
}
if (FD_ISSET(global_c, &a)) /* if (FD_ISSET(socketFD, &fdvar)) */ {
return global_c; /* return socketFD */
}
if (global_a&&FD_ISSET(fileno(stdin), &a)) /* if (isConnected&&FD_ISSET(fileno(stdin), &fdvar)) */ {
return fileno(stdin); /* return fileno(stdin) */
}
return -1; /* return -1 */
}
void process() /* void process ( ) { */{
int a; /* int fdToProcess ; */
while (1) /* while (1) */ {
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
a = getFDForInput(); /* fdToProcess = getFDForInput ( ) ; */
if (a == fileno(stdin)) /* if (fdToProcess == fileno(stdin)) */ {
processInput(); /* processInput ( ) ; */
} else {
printf("\n"); /* printf ( "\n" ) ; */
processPacket(); /* processPacket ( ) ; */
}
}
}
void processPacket() /* void processPacket ( ) { */{
uint8_t *a = malloc(1000); /* uint8_t * packet = malloc ( 1000 ) ; */
uint8_t *b = a; /* uint8_t * toFree = packet ; */
int c = 0; /* int numBytes = 0 ; */
if ((c = myRecv(global_c, a, 1000, 0) )<0) /* if ((numBytes = myRecv(socketFD, packet, 1000, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (c == 0) /* if (numBytes == 0) */ {
printf("Server terminated\n"); /* printf ( "Server terminated\n" ) ; */
close(global_c); /* close ( socketFD ) ; */
exit(1); /* exit ( 1 ) ; */
}
while (c>0) /* while (numBytes>0) */ {
switch (((header *)a )->flag) /* switch (((header *)packet )->flag) */ {
case 2: 
printf("Connected to server\n"); /* printf ( "Connected to server\n" ) ; */
global_a = 1; /* isConnected = 1 ; */
break;
case 3: 
printf("Handle already in use: %s\n", global_b); /* printf ( "Handle already in use: %s\n" , myHandle ) ; */
exit(1); /* exit ( 1 ) ; */
break;
case 9: 
printf("Goodbye!\n"); /* printf ( "Goodbye!\n" ) ; */
exit(0); /* exit ( 0 ) ; */
break;
case 4: 
printBroadcast(a); /* printBroadcast ( packet ) ; */
break;
case 7: 
printf("No such handle connected to the server\n"); /* printf ( "No such handle connected to the server\n" ) ; */
break;
case 5: 
printMessage(a); /* printMessage ( packet ) ; */
break;
case 12: 
printHandle(a); /* printHandle ( packet ) ; */
break;
case 11: 
printf("Number of clients: %u\n", packetHandleAckGetNum((packetHandleAck *)a)); /* printf ( "Number of clients: %u\n" , packetHandleAckGetNum ( ( packetHandleAck * ) packet ) ) ; */
break;
default: 
printf("unknown packet %d with size %d\n", ((header *)a )->flag, c); /* printf ( "unknown packet %d with size %d\n" , ( ( header * ) packet ) -> flag , numBytes ) ; */
break;
}
a += headerGetLen((header *)a) + sizeof(header); /* packet += headerGetLen ( ( header * ) packet ) + sizeof ( header ) ; */
c -= headerGetLen((header *)a) + sizeof(header); /* numBytes -= headerGetLen ( ( header * ) packet ) + sizeof ( header ) ; */
}
free(b); /* free ( toFree ) ; */
}
void processInput() /* void processInput ( ) { */{
int a = 0; /* int didWork = 0 ; */
unsigned long b = -1; /* unsigned long inputNum = - 1 ; */
char *c = ((void *)0 ); /* char * line = ( ( void * ) 0 ) ; */
char *d = ((void *)0 ); /* char * lineToFree = ( ( void * ) 0 ) ; */
char *e = ((void *)0 ); /* char * cmd = ( ( void * ) 0 ) ; */
char *f = ((void *)0 ); /* char * mssg = ( ( void * ) 0 ) ; */
char *g = ((void *)0 ); /* char * dst = ( ( void * ) 0 ) ; */
getline(&c, (size_t *)&b, stdin); /* getline ( & line , ( size_t * ) & inputNum , stdin ) ; */
d = c; /* lineToFree = line ; */
e = strtok(c, " \n\t"); /* cmd = strtok ( line , " \n\t" ) ; */
if (e != ((void *)0 )) /* if (cmd != ((void *)0 )) */ {
if (strcmp(e, "%E") == 0 || strcmp(e, "%e") == 0) /* if (strcmp(cmd, "%E") == 0 || strcmp(cmd, "%e") == 0) */ {
sendPacketFlag(8, global_c); /* sendPacketFlag ( 8 , socketFD ) ; */
a = 1; /* didWork = 1 ; */
}
if (strcmp(e, "%B") == 0 || strcmp(e, "%b") == 0) /* if (strcmp(cmd, "%B") == 0 || strcmp(cmd, "%b") == 0) */ {
f = strtok(((void *)0 ), "\n"); /* mssg = strtok ( ( ( void * ) 0 ) , "\n" ) ; */
if (f == ((void *)0 )) /* if (mssg == ((void *)0 )) */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
} else {
sendPacketBroadCast(f, global_c); /* sendPacketBroadCast ( mssg , socketFD ) ; */
}
a = 1; /* didWork = 1 ; */
}
if (strcmp(e, "%M") == 0 || strcmp(e, "%m") == 0) /* if (strcmp(cmd, "%M") == 0 || strcmp(cmd, "%m") == 0) */ {
g = strtok(((void *)0 ), " \n\t"); /* dst = strtok ( ( ( void * ) 0 ) , " \n\t" ) ; */
if (g == ((void *)0 )) /* if (dst == ((void *)0 )) */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
} else {
f = strtok(((void *)0 ), "\n"); /* mssg = strtok ( ( ( void * ) 0 ) , "\n" ) ; */
if (f == ((void *)0 )) /* if (mssg == ((void *)0 )) */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
} else {
sendPacketMssg(f, g, global_c); /* sendPacketMssg ( mssg , dst , socketFD ) ; */
}
}
a = 1; /* didWork = 1 ; */
}
if (strcmp(e, "%L") == 0 || strcmp(e, "%l") == 0) /* if (strcmp(cmd, "%L") == 0 || strcmp(cmd, "%l") == 0) */ {
sendPacketFlag(10, global_c); /* sendPacketFlag ( 10 , socketFD ) ; */
a = 1; /* didWork = 1 ; */
}
}
free(d); /* free ( lineToFree ) ; */
if (a == 0) /* if (didWork == 0) */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
}
}
void processHandles(int a) /* void processHandles ( int handleNum ) { */{
int b = 0; /* int i = 0 ; */
global_a = 0; /* isConnected = 0 ; */
for (b = 0; b<a; b++) {
getFDForInput(); /* getFDForInput ( ) ; */
processPacket(); /* processPacket ( ) ; */
}
global_a = 1; /* isConnected = 1 ; */
}
void printBroadcast(void *a) /* void printBroadcast ( void * packet ) { */{
char *b = ((void *)0 ); /* char * sender = ( ( void * ) 0 ) ; */
char *c = ((void *)0 ); /* char * mssg = ( ( void * ) 0 ) ; */
getFirstHandle(a, &b); /* getFirstHandle ( packet , & sender ) ; */
getBroadCastMssg(a, &c); /* getBroadCastMssg ( packet , & mssg ) ; */
printf("%s: %s\n", b, c); /* printf ( "%s: %s\n" , sender , mssg ) ; */
free(c); /* free ( mssg ) ; */
free(b); /* free ( sender ) ; */
}
void printMessage(void *a) /* void printMessage ( void * packet ) { */{
char *b = ((void *)0 ); /* char * sender = ( ( void * ) 0 ) ; */
char *c = ((void *)0 ); /* char * mssg = ( ( void * ) 0 ) ; */
getSrcHandle(a, &b); /* getSrcHandle ( packet , & sender ) ; */
getMssg(a, &c); /* getMssg ( packet , & mssg ) ; */
printf("%s: %s\n", b, c); /* printf ( "%s: %s\n" , sender , mssg ) ; */
free(c); /* free ( mssg ) ; */
free(b); /* free ( sender ) ; */
}
void printHandle(void *a) /* void printHandle ( void * packet ) { */{
char *b = ((void *)0 ); /* char * handle = ( ( void * ) 0 ) ; */
getFirstHandle(a, &b); /* getFirstHandle ( packet , & handle ) ; */
printf("  -%s\n", b); /* printf ( "  -%s\n" , handle ) ; */
free(b); /* free ( handle ) ; */
}
