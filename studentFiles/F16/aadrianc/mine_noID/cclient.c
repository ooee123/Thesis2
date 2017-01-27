int socketFD = 0; /* int socketFD = 0 ; */
int isConnected = 0; /* int isConnected = 0 ; */
char *myHandle = ((void *)0 ); /* char * myHandle = ( ( void * ) 0 ) ; */

unsigned int __bswap_32(unsigned int __bsx) /*  */{
return __builtin_bswap32(__bsx); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t __bsx) /*  */{
return __builtin_bswap64(__bsx); /* return __builtin_bswap64(__bsx) */
}
int main(int argc, char *argv) /* int main ( int argc , char * argv [ ] ) { */{
if (argc != 4) /* if (argc != 4) */ {
printf("usage: %s handle host-name port-number \n", argv[0]); /* printf ( "usage: %s handle host-name port-number \n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
}
socketFD = tcpClientSetup(argv[2], argv[3]); /* socketFD = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
myHandle = argv[1]; /* myHandle = argv [ 1 ] ; */
sendPacketConnection(socketFD); /* sendPacketConnection ( socketFD ) ; */
process(); /* process ( ) ; */
return 0; /* return 0 */
}
void sendPacketConnection(int socket) /* void sendPacketConnection ( int socket ) { */{
int hLen = strlen(myHandle); /* int hLen = strlen ( myHandle ) ; */
void *packet = makePacketHandle(1, hLen, myHandle); /* void * packet = makePacketHandle ( 1 , hLen , myHandle ) ; */
if (sendPacket(packet, socket)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Connection"); /* perror ( "Packet Connection" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(packet); /* free ( packet ) ; */
}
void sendPacketBroadCast(char *mssg, int socket) /* void sendPacketBroadCast ( char * mssg , int socket ) { */{
int consumed = 0; /* int consumed = 0 ; */
void *packet; /* void * packet ; */
char *toSend; /* char * toSend ; */
int hLen = strlen(myHandle); /* int hLen = strlen ( myHandle ) ; */
int maxMssgLen = 1000 - hLen - sizeof(header) - 1; /* int maxMssgLen = 1000 - hLen - sizeof ( header ) - 1 ; */
int mssgNum = (strlen(mssg) + 1 ) / maxMssgLen; /* int mssgNum = ( strlen ( mssg ) + 1 ) / maxMssgLen ; */
int theRest = ((strlen(mssg) + 1 ) % maxMssgLen ) + mssgNum; /* int theRest = ( ( strlen ( mssg ) + 1 ) % maxMssgLen ) + mssgNum ; */
toSend = malloc(maxMssgLen); /* toSend = malloc ( maxMssgLen ) ; */
while (mssgNum>0) /* while (mssgNum>0) */ {
mssgNum--; /* mssgNum -- ; */
memcpy(toSend, mssg + consumed, maxMssgLen - 1); /* memcpy ( toSend , mssg + consumed , maxMssgLen - 1 ) ; */
toSend[maxMssgLen - 1] = 0; /* toSend [ maxMssgLen - 1 ] = 0 ; */
consumed += maxMssgLen - 1; /* consumed += maxMssgLen - 1 ; */
packet = makePacketBroadcast(4, hLen, myHandle, toSend); /* packet = makePacketBroadcast ( 4 , hLen , myHandle , toSend ) ; */
if (sendPacket(packet, socket)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Broadcast"); /* perror ( "Packet Broadcast" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(packet); /* free ( packet ) ; */
}
free(toSend); /* free ( toSend ) ; */
toSend = malloc(theRest); /* toSend = malloc ( theRest ) ; */
memcpy(toSend, mssg + consumed, theRest); /* memcpy ( toSend , mssg + consumed , theRest ) ; */
packet = makePacketBroadcast(4, hLen, myHandle, toSend); /* packet = makePacketBroadcast ( 4 , hLen , myHandle , toSend ) ; */
if (sendPacket(packet, socket)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Broadcast"); /* perror ( "Packet Broadcast" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(packet); /* free ( packet ) ; */
free(toSend); /* free ( toSend ) ; */
}
void sendPacketMssg(char *mssg, char *dest, int socket) /* void sendPacketMssg ( char * mssg , char * dest , int socket ) { */{
void *packet; /* void * packet ; */
int consumed = 0; /* int consumed = 0 ; */
char *toSend; /* char * toSend ; */
int srcLen = strlen(myHandle); /* int srcLen = strlen ( myHandle ) ; */
int destLen = strlen(dest); /* int destLen = strlen ( dest ) ; */
int maxMssgLen = 1000 - (srcLen + 1 ) - (destLen + 1 ) - sizeof(header); /* int maxMssgLen = 1000 - ( srcLen + 1 ) - ( destLen + 1 ) - sizeof ( header ) ; */
int mssgNum = (strlen(mssg) + 1 ) / maxMssgLen; /* int mssgNum = ( strlen ( mssg ) + 1 ) / maxMssgLen ; */
int theRest = ((strlen(mssg) + 1 ) % maxMssgLen ) + mssgNum; /* int theRest = ( ( strlen ( mssg ) + 1 ) % maxMssgLen ) + mssgNum ; */
toSend = malloc(maxMssgLen); /* toSend = malloc ( maxMssgLen ) ; */
while (mssgNum>0) /* while (mssgNum>0) */ {
memcpy(toSend, mssg + consumed, maxMssgLen - 1); /* memcpy ( toSend , mssg + consumed , maxMssgLen - 1 ) ; */
mssgNum--; /* mssgNum -- ; */
toSend[maxMssgLen - 1] = 0; /* toSend [ maxMssgLen - 1 ] = 0 ; */
consumed += maxMssgLen - 1; /* consumed += maxMssgLen - 1 ; */
packet = makePacketMssg(5, destLen, dest, srcLen, myHandle, toSend); /* packet = makePacketMssg ( 5 , destLen , dest , srcLen , myHandle , toSend ) ; */
if (sendPacket(packet, socket)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Message"); /* perror ( "Packet Message" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(packet); /* free ( packet ) ; */
}
free(toSend); /* free ( toSend ) ; */
toSend = malloc(theRest); /* toSend = malloc ( theRest ) ; */
memcpy(toSend, mssg + consumed, theRest); /* memcpy ( toSend , mssg + consumed , theRest ) ; */
packet = makePacketMssg(5, destLen, dest, srcLen, myHandle, toSend); /* packet = makePacketMssg ( 5 , destLen , dest , srcLen , myHandle , toSend ) ; */
if (sendPacket(packet, socket)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Message"); /* perror ( "Packet Message" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(packet); /* free ( packet ) ; */
free(toSend); /* free ( toSend ) ; */
}
int getFDForInput() /* int getFDForInput ( ) { */{
fd_set fdvar; /* fd_set fdvar ; */
int maxFD = socketFD + 1; /* int maxFD = socketFD + 1 ; */
FD_ZERO(&fdvar); /* FD_ZERO ( & fdvar ) ; */
FD_SET(socketFD, &fdvar); /* FD_SET ( socketFD , & fdvar ) ; */
if (isConnected) /* if (isConnected) */ {
FD_SET(fileno(stdin), &fdvar); /* FD_SET ( fileno ( stdin ) , & fdvar ) ; */
}
if (mySelect(maxFD, (fd_set *)&fdvar, ((void *)0 ), ((void *)0 ), ((void *)0 )) == -1) /* if (mySelect(maxFD, (fd_set *)&fdvar, ((void *)0 ), ((void *)0 ), ((void *)0 )) == -1) */ {
perror("Error selecting FD to read from"); /* perror ( "Error selecting FD to read from" ) ; */
exit(1); /* exit ( 1 ) ; */
}
if (FD_ISSET(socketFD, &fdvar)) /* if (FD_ISSET(socketFD, &fdvar)) */ {
return socketFD; /* return socketFD */
}
if (isConnected&&FD_ISSET(fileno(stdin), &fdvar)) /* if (isConnected&&FD_ISSET(fileno(stdin), &fdvar)) */ {
return fileno(stdin); /* return fileno(stdin) */
}
return -1; /* return -1 */
}
void process() /* void process ( ) { */{
int fdToProcess; /* int fdToProcess ; */
while (1) /* while (1) */ {
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
fdToProcess = getFDForInput(); /* fdToProcess = getFDForInput ( ) ; */
if (fdToProcess == fileno(stdin)) /* if (fdToProcess == fileno(stdin)) */ {
processInput(); /* processInput ( ) ; */
} else {
printf("\n"); /* printf ( "\n" ) ; */
processPacket(); /* processPacket ( ) ; */
}
}
}
void processPacket() /* void processPacket ( ) { */{
int numBytes = 0; /* int numBytes = 0 ; */
uint8_t *packet = malloc(1000); /* uint8_t * packet = malloc ( 1000 ) ; */
uint8_t *toFree = packet; /* uint8_t * toFree = packet ; */
if ((numBytes = myRecv(socketFD, packet, 1000, 0) )<0) /* if ((numBytes = myRecv(socketFD, packet, 1000, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (numBytes == 0) /* if (numBytes == 0) */ {
printf("Server terminated\n"); /* printf ( "Server terminated\n" ) ; */
close(socketFD); /* close ( socketFD ) ; */
exit(1); /* exit ( 1 ) ; */
}
while (numBytes>0) /* while (numBytes>0) */ {
switch (((header *)packet )->flag) /* switch (((header *)packet )->flag) */ {
case 2: 
printf("Connected to server\n"); /* printf ( "Connected to server\n" ) ; */
isConnected = 1; /* isConnected = 1 ; */
break;
case 3: 
printf("Handle already in use: %s\n", myHandle); /* printf ( "Handle already in use: %s\n" , myHandle ) ; */
exit(1); /* exit ( 1 ) ; */
break;
case 9: 
printf("Goodbye!\n"); /* printf ( "Goodbye!\n" ) ; */
exit(0); /* exit ( 0 ) ; */
break;
case 4: 
printBroadcast(packet); /* printBroadcast ( packet ) ; */
break;
case 7: 
printf("No such handle connected to the server\n"); /* printf ( "No such handle connected to the server\n" ) ; */
break;
case 5: 
printMessage(packet); /* printMessage ( packet ) ; */
break;
case 12: 
printHandle(packet); /* printHandle ( packet ) ; */
break;
case 11: 
printf("Number of clients: %u\n", packetHandleAckGetNum((packetHandleAck *)packet)); /* printf ( "Number of clients: %u\n" , packetHandleAckGetNum ( ( packetHandleAck * ) packet ) ) ; */
break;
default: 
printf("unknown packet %d with size %d\n", ((header *)packet )->flag, numBytes); /* printf ( "unknown packet %d with size %d\n" , ( ( header * ) packet ) -> flag , numBytes ) ; */
break;
}
packet += headerGetLen((header *)packet) + sizeof(header); /* packet += headerGetLen ( ( header * ) packet ) + sizeof ( header ) ; */
numBytes -= headerGetLen((header *)packet) + sizeof(header); /* numBytes -= headerGetLen ( ( header * ) packet ) + sizeof ( header ) ; */
}
free(toFree); /* free ( toFree ) ; */
}
void processInput() /* void processInput ( ) { */{
char *dst = ((void *)0 ); /* char * dst = ( ( void * ) 0 ) ; */
unsigned long inputNum = -1; /* unsigned long inputNum = - 1 ; */
char *line = ((void *)0 ); /* char * line = ( ( void * ) 0 ) ; */
char *cmd = ((void *)0 ); /* char * cmd = ( ( void * ) 0 ) ; */
char *mssg = ((void *)0 ); /* char * mssg = ( ( void * ) 0 ) ; */
int didWork = 0; /* int didWork = 0 ; */
char *lineToFree = ((void *)0 ); /* char * lineToFree = ( ( void * ) 0 ) ; */
getline(&line, (size_t *)&inputNum, stdin); /* getline ( & line , ( size_t * ) & inputNum , stdin ) ; */
lineToFree = line; /* lineToFree = line ; */
cmd = strtok(line, " \n\t"); /* cmd = strtok ( line , " \n\t" ) ; */
if (cmd != ((void *)0 )) /* if (cmd != ((void *)0 )) */ {
if (strcmp(cmd, "%E") == 0 || strcmp(cmd, "%e") == 0) /* if (strcmp(cmd, "%E") == 0 || strcmp(cmd, "%e") == 0) */ {
sendPacketFlag(8, socketFD); /* sendPacketFlag ( 8 , socketFD ) ; */
didWork = 1; /* didWork = 1 ; */
}
if (strcmp(cmd, "%B") == 0 || strcmp(cmd, "%b") == 0) /* if (strcmp(cmd, "%B") == 0 || strcmp(cmd, "%b") == 0) */ {
didWork = 1; /* didWork = 1 ; */
mssg = strtok(((void *)0 ), "\n"); /* mssg = strtok ( ( ( void * ) 0 ) , "\n" ) ; */
if (mssg == ((void *)0 )) /* if (mssg == ((void *)0 )) */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
} else {
sendPacketBroadCast(mssg, socketFD); /* sendPacketBroadCast ( mssg , socketFD ) ; */
}
}
if (strcmp(cmd, "%M") == 0 || strcmp(cmd, "%m") == 0) /* if (strcmp(cmd, "%M") == 0 || strcmp(cmd, "%m") == 0) */ {
didWork = 1; /* didWork = 1 ; */
dst = strtok(((void *)0 ), " \n\t"); /* dst = strtok ( ( ( void * ) 0 ) , " \n\t" ) ; */
if (dst == ((void *)0 )) /* if (dst == ((void *)0 )) */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
} else {
mssg = strtok(((void *)0 ), "\n"); /* mssg = strtok ( ( ( void * ) 0 ) , "\n" ) ; */
if (mssg == ((void *)0 )) /* if (mssg == ((void *)0 )) */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
} else {
sendPacketMssg(mssg, dst, socketFD); /* sendPacketMssg ( mssg , dst , socketFD ) ; */
}
}
}
if (strcmp(cmd, "%L") == 0 || strcmp(cmd, "%l") == 0) /* if (strcmp(cmd, "%L") == 0 || strcmp(cmd, "%l") == 0) */ {
sendPacketFlag(10, socketFD); /* sendPacketFlag ( 10 , socketFD ) ; */
didWork = 1; /* didWork = 1 ; */
}
}
free(lineToFree); /* free ( lineToFree ) ; */
if (didWork == 0) /* if (didWork == 0) */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
}
}
void processHandles(int handleNum) /* void processHandles ( int handleNum ) { */{
int i = 0; /* int i = 0 ; */
isConnected = 0; /* isConnected = 0 ; */
isConnected = 1; /* isConnected = 1 ; */
for (i = 0; i<handleNum; i++) {
getFDForInput(); /* getFDForInput ( ) ; */
processPacket(); /* processPacket ( ) ; */
}
}
void printBroadcast(void *packet) /* void printBroadcast ( void * packet ) { */{
char *sender = ((void *)0 ); /* char * sender = ( ( void * ) 0 ) ; */
char *mssg = ((void *)0 ); /* char * mssg = ( ( void * ) 0 ) ; */
getFirstHandle(packet, &sender); /* getFirstHandle ( packet , & sender ) ; */
getBroadCastMssg(packet, &mssg); /* getBroadCastMssg ( packet , & mssg ) ; */
printf("%s: %s\n", sender, mssg); /* printf ( "%s: %s\n" , sender , mssg ) ; */
free(mssg); /* free ( mssg ) ; */
free(sender); /* free ( sender ) ; */
}
void printMessage(void *packet) /* void printMessage ( void * packet ) { */{
char *sender = ((void *)0 ); /* char * sender = ( ( void * ) 0 ) ; */
char *mssg = ((void *)0 ); /* char * mssg = ( ( void * ) 0 ) ; */
getSrcHandle(packet, &sender); /* getSrcHandle ( packet , & sender ) ; */
getMssg(packet, &mssg); /* getMssg ( packet , & mssg ) ; */
printf("%s: %s\n", sender, mssg); /* printf ( "%s: %s\n" , sender , mssg ) ; */
free(mssg); /* free ( mssg ) ; */
free(sender); /* free ( sender ) ; */
}
void printHandle(void *packet) /* void printHandle ( void * packet ) { */{
char *handle = ((void *)0 ); /* char * handle = ( ( void * ) 0 ) ; */
getFirstHandle(packet, &handle); /* getFirstHandle ( packet , & handle ) ; */
printf("  -%s\n", handle); /* printf ( "  -%s\n" , handle ) ; */
free(handle); /* free ( handle ) ; */
}
