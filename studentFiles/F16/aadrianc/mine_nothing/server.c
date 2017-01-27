int serverFD; /* int serverFD ; */
int *clientFDs = ((void *)0 ); /* int * clientFDs = ( ( void * ) 0 ) ; */
char **handles = ((void *)0 ); /* char * * handles = ( ( void * ) 0 ) ; */
int clientNum = 0; /* int clientNum = 0 ; */

unsigned int __bswap_32(unsigned int __bsx) /*  */{
return __builtin_bswap32(__bsx); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t __bsx) /*  */{
return __builtin_bswap64(__bsx); /* return __builtin_bswap64(__bsx) */
}
int main(int argc, char *argv) /* int main ( int argc , char * argv [ ] ) { */{
serverFD = tcpServerSetup(0); /* serverFD = tcpServerSetup ( 0 ) ; */
process(); /* process ( ) ; */
return 0; /* return 0 */
}
void addClient(int fd) /* void addClient ( int fd ) { */{
clientNum++; /* clientNum ++ ; */
clientFDs = realloc(clientFDs, sizeof(int) * clientNum); /* clientFDs = realloc ( clientFDs , sizeof ( int ) * clientNum ) ; */
clientFDs[clientNum - 1] = fd; /* clientFDs [ clientNum - 1 ] = fd ; */
handles = realloc(handles, sizeof(char *) * clientNum); /* handles = realloc ( handles , sizeof ( char * ) * clientNum ) ; */
handles[clientNum - 1] = ((void *)0 ); /* handles [ clientNum - 1 ] = ( ( void * ) 0 ) ; */
}
void setClientName(void *packet, int fd) /* void setClientName ( void * packet , int fd ) { */{
int i = 0; /* int i = 0 ; */
char *name = ((void *)0 ); /* char * name = ( ( void * ) 0 ) ; */
getFirstHandle(packet, &name); /* getFirstHandle ( packet , & name ) ; */
for (i = 0; i<clientNum; i++) {
if (clientFDs[i] == fd) /* if (clientFDs[i] == fd) */ {
if (getFDForName(name) != -1) /* if (getFDForName(name) != -1) */ {
free(name); /* free ( name ) ; */
sendPacketFlag(3, clientFDs[i]); /* sendPacketFlag ( 3 , clientFDs [ i ] ) ; */
removeClient(fd); /* removeClient ( fd ) ; */
} else {
handles[i] = name; /* handles [ i ] = name ; */
sendPacketFlag(2, clientFDs[i]); /* sendPacketFlag ( 2 , clientFDs [ i ] ) ; */
printf("Welcome: %s\n", handles[i]); /* printf ( "Welcome: %s\n" , handles [ i ] ) ; */
}
return; /* return */
}
}
}
int getFDForName(char *name) /* int getFDForName ( char * name ) { */{
int i = 0; /* int i = 0 ; */
for (i = 0; i<clientNum; i++) {
if (handles[i] != ((void *)0 )&&strcmp(name, handles[i]) == 0) /* if (handles[i] != ((void *)0 )&&strcmp(name, handles[i]) == 0) */ {
return clientFDs[i]; /* return clientFDs[i] */
}
}
return -1; /* return -1 */
}
void removeClient(int fd) /* void removeClient ( int fd ) { */{
int i = 0; /* int i = 0 ; */
int *newClientFDs; /* int * newClientFDs ; */
char **newHandles; /* char * * newHandles ; */
int foundFD = 0; /* int foundFD = 0 ; */
clientNum--; /* clientNum -- ; */
newClientFDs = (int *)malloc(sizeof(int) * clientNum); /* newClientFDs = ( int * ) malloc ( sizeof ( int ) * clientNum ) ; */
newHandles = (char **)malloc(sizeof(char *) * clientNum); /* newHandles = ( char * * ) malloc ( sizeof ( char * ) * clientNum ) ; */
for (i = 0; i<=clientNum; i++) {
if (clientFDs[i] != fd) /* if (clientFDs[i] != fd) */ {
if (!foundFD) /* if (!foundFD) */ {
newClientFDs[i] = clientFDs[i]; /* newClientFDs [ i ] = clientFDs [ i ] ; */
newHandles[i] = handles[i]; /* newHandles [ i ] = handles [ i ] ; */
} else {
newClientFDs[i - 1] = clientFDs[i]; /* newClientFDs [ i - 1 ] = clientFDs [ i ] ; */
newHandles[i - 1] = handles[i]; /* newHandles [ i - 1 ] = handles [ i ] ; */
}
} else {
foundFD = 1; /* foundFD = 1 ; */
if (handles[i] != ((void *)0 )) /* if (handles[i] != ((void *)0 )) */ {
printf("Bye %s\n", handles[i]); /* printf ( "Bye %s\n" , handles [ i ] ) ; */
}
free(handles[i]); /* free ( handles [ i ] ) ; */
close(clientFDs[i]); /* close ( clientFDs [ i ] ) ; */
}
}
free(handles); /* free ( handles ) ; */
free(clientFDs); /* free ( clientFDs ) ; */
handles = newHandles; /* handles = newHandles ; */
clientFDs = newClientFDs; /* clientFDs = newClientFDs ; */
}
int getFDForInput() /* int getFDForInput ( ) { */{
fd_set fdvar; /* fd_set fdvar ; */
int i = 0; /* int i = 0 ; */
int maxFD = serverFD + 1; /* int maxFD = serverFD + 1 ; */
FD_ZERO(&fdvar); /* FD_ZERO ( & fdvar ) ; */
FD_SET(serverFD, &fdvar); /* FD_SET ( serverFD , & fdvar ) ; */
for (i = 0; i<clientNum; i++) {
FD_SET(clientFDs[i], &fdvar); /* FD_SET ( clientFDs [ i ] , & fdvar ) ; */
}
if (clientNum>0) /* if (clientNum>0) */ {
maxFD = clientFDs[clientNum - 1] + 1; /* maxFD = clientFDs [ clientNum - 1 ] + 1 ; */
}
if (mySelect(maxFD, (fd_set *)&fdvar, ((void *)0 ), ((void *)0 ), ((void *)0 )) == -1) /* if (mySelect(maxFD, (fd_set *)&fdvar, ((void *)0 ), ((void *)0 ), ((void *)0 )) == -1) */ {
perror("Error selecting FD to read from"); /* perror ( "Error selecting FD to read from" ) ; */
exit(1); /* exit ( 1 ) ; */
}
if (FD_ISSET(serverFD, &fdvar)) /* if (FD_ISSET(serverFD, &fdvar)) */ {
return serverFD; /* return serverFD */
} else {
for (i = 0; i<clientNum; i++) {
if (FD_ISSET(clientFDs[i], &fdvar)) /* if (FD_ISSET(clientFDs[i], &fdvar)) */ {
return clientFDs[i]; /* return clientFDs[i] */
}
}
}
return -1; /* return -1 */
}
void acceptClient() /* void acceptClient ( ) { */{
int clientFD = tcpAccept(serverFD); /* int clientFD = tcpAccept ( serverFD ) ; */
addClient(clientFD); /* addClient ( clientFD ) ; */
}
void process() /* void process ( ) { */{
int fdToProcess; /* int fdToProcess ; */
while (1) /* while (1) */ {
fdToProcess = getFDForInput(); /* fdToProcess = getFDForInput ( ) ; */
if (fdToProcess == serverFD) /* if (fdToProcess == serverFD) */ {
acceptClient(); /* acceptClient ( ) ; */
} else {
processPacket(fdToProcess); /* processPacket ( fdToProcess ) ; */
}
}
}
void processPacket(int fd) /* void processPacket ( int fd ) { */{
uint8_t *packet = malloc(1000); /* uint8_t * packet = malloc ( 1000 ) ; */
int numBytes = 0; /* int numBytes = 0 ; */
if ((numBytes = myRecv(fd, packet, 1000, 0) )<0) /* if ((numBytes = myRecv(fd, packet, 1000, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (numBytes == 0) /* if (numBytes == 0) */ {
removeClient(fd); /* removeClient ( fd ) ; */
}
switch (((header *)packet )->flag) /* switch (((header *)packet )->flag) */ {
case 1: 
setClientName(packet, fd); /* setClientName ( packet , fd ) ; */
break;
case 8: 
sendPacketFlag(9, fd); /* sendPacketFlag ( 9 , fd ) ; */
removeClient(fd); /* removeClient ( fd ) ; */
break;
case 4: 
broadCast(fd, packet); /* broadCast ( fd , packet ) ; */
break;
case 5: 
forwardMessage(fd, packet); /* forwardMessage ( fd , packet ) ; */
break;
case 10: 
sendPacketHandleAck(11, fd); /* sendPacketHandleAck ( 11 , fd ) ; */
sendHandles(fd); /* sendHandles ( fd ) ; */
break;
default: 
break;
}
free(packet); /* free ( packet ) ; */
}
void broadCast(int senderFD, void *packet) /* void broadCast ( int senderFD , void * packet ) { */{
int i = 0; /* int i = 0 ; */
for (i = 0; i<clientNum; i++) {
if (clientFDs[i] != senderFD) /* if (clientFDs[i] != senderFD) */ {
if (sendPacket(packet, clientFDs[i])<0) /* if (sendPacket(packet, clientFDs[i])<0) */ {
perror("Packet Broadcast"); /* perror ( "Packet Broadcast" ) ; */
exit(1); /* exit ( 1 ) ; */
}
}
}
}
void forwardMessage(int senderFD, void *packet) /* void forwardMessage ( int senderFD , void * packet ) { */{
int i = 0; /* int i = 0 ; */
char *dest = ((void *)0 ); /* char * dest = ( ( void * ) 0 ) ; */
int sent = 0; /* int sent = 0 ; */
getDestHandle(packet, &dest); /* getDestHandle ( packet , & dest ) ; */
for (i = 0; i<clientNum; i++) {
if (strcmp(dest, handles[i]) == 0) /* if (strcmp(dest, handles[i]) == 0) */ {
if (sendPacket(packet, clientFDs[i])<0) /* if (sendPacket(packet, clientFDs[i])<0) */ {
perror("Packet Broadcast"); /* perror ( "Packet Broadcast" ) ; */
exit(1); /* exit ( 1 ) ; */
}
sent = 1; /* sent = 1 ; */
break;
}
}
if (sent == 0) /* if (sent == 0) */ {
sendPacketFlag(7, senderFD); /* sendPacketFlag ( 7 , senderFD ) ; */
}
free(dest); /* free ( dest ) ; */
}
void sendPacketHandleAck(uint8_t flag, int socket) /* void sendPacketHandleAck ( uint8_t flag , int socket ) { */{
packetHandleAck head; /* packetHandleAck head ; */
headerSetLen((header *)&head, (uint16_t )(sizeof(packetHandleAck) - sizeof(header))); /* headerSetLen ( ( header * ) & head , ( uint16_t ) ( sizeof ( packetHandleAck ) - sizeof ( header ) ) ) ; */
packetHandleAckSetNum(&head, clientNum); /* packetHandleAckSetNum ( & head , clientNum ) ; */
head.head.flag = flag; /* head . head . flag = flag ; */
if (mySend(socket, &head, sizeof(packetHandleAck), 0)<0) /* if (mySend(socket, &head, sizeof(packetHandleAck), 0)<0) */ {
perror("Flag Packet Send"); /* perror ( "Flag Packet Send" ) ; */
exit(1); /* exit ( 1 ) ; */
}
}
void sendPacketHandle(int socket, char *handle) /* void sendPacketHandle ( int socket , char * handle ) { */{
int hLen = strlen(handle); /* int hLen = strlen ( handle ) ; */
void *packet = makePacketHandle(12, hLen, handle); /* void * packet = makePacketHandle ( 12 , hLen , handle ) ; */
if (sendPacket(packet, socket)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Connection"); /* perror ( "Packet Connection" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(packet); /* free ( packet ) ; */
}
void sendHandles(int socket) /* void sendHandles ( int socket ) { */{
int i = 0; /* int i = 0 ; */
for (i = 0; i<clientNum; i++) {
sendPacketHandle(socket, handles[i]); /* sendPacketHandle ( socket , handles [ i ] ) ; */
}
}
