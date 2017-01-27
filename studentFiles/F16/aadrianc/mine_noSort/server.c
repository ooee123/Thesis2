int global_b; /* int serverFD ; */
int *global_d = ((void *)0 ); /* int * clientFDs = ( ( void * ) 0 ) ; */
char **global_a = ((void *)0 ); /* char * * handles = ( ( void * ) 0 ) ; */
int global_c = 0; /* int clientNum = 0 ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
global_b = tcpServerSetup(0); /* serverFD = tcpServerSetup ( 0 ) ; */
process(); /* process ( ) ; */
return 0; /* return 0 */
}
void addClient(int a) /* void addClient ( int fd ) { */{
global_c++; /* clientNum ++ ; */
global_d = realloc(global_d, sizeof(int) * global_c); /* clientFDs = realloc ( clientFDs , sizeof ( int ) * clientNum ) ; */
global_d[global_c - 1] = a; /* clientFDs [ clientNum - 1 ] = fd ; */
global_a = realloc(global_a, sizeof(char *) * global_c); /* handles = realloc ( handles , sizeof ( char * ) * clientNum ) ; */
global_a[global_c - 1] = ((void *)0 ); /* handles [ clientNum - 1 ] = ( ( void * ) 0 ) ; */
}
void setClientName(void *a, int b) /* void setClientName ( void * packet , int fd ) { */{
int c = 0; /* int i = 0 ; */
char *d = ((void *)0 ); /* char * name = ( ( void * ) 0 ) ; */
getFirstHandle(a, &d); /* getFirstHandle ( packet , & name ) ; */
for (c = 0; c<global_c; c++) {
if (global_d[c] == b) /* if (clientFDs[i] == fd) */ {
if (getFDForName(d) != -1) /* if (getFDForName(name) != -1) */ {
free(d); /* free ( name ) ; */
sendPacketFlag(3, global_d[c]); /* sendPacketFlag ( 3 , clientFDs [ i ] ) ; */
removeClient(b); /* removeClient ( fd ) ; */
} else {
global_a[c] = d; /* handles [ i ] = name ; */
sendPacketFlag(2, global_d[c]); /* sendPacketFlag ( 2 , clientFDs [ i ] ) ; */
printf("Welcome: %s\n", global_a[c]); /* printf ( "Welcome: %s\n" , handles [ i ] ) ; */
}
return; /* return */
}
}
}
int getFDForName(char *a) /* int getFDForName ( char * name ) { */{
int b = 0; /* int i = 0 ; */
for (b = 0; b<global_c; b++) {
if (global_a[b] != ((void *)0 )&&strcmp(a, global_a[b]) == 0) /* if (handles[i] != ((void *)0 )&&strcmp(name, handles[i]) == 0) */ {
return global_d[b]; /* return clientFDs[i] */
}
}
return -1; /* return -1 */
}
void removeClient(int a) /* void removeClient ( int fd ) { */{
int b = 0; /* int i = 0 ; */
int *c; /* int * newClientFDs ; */
char **d; /* char * * newHandles ; */
int e = 0; /* int foundFD = 0 ; */
global_c--; /* clientNum -- ; */
c = (int *)malloc(sizeof(int) * global_c); /* newClientFDs = ( int * ) malloc ( sizeof ( int ) * clientNum ) ; */
d = (char **)malloc(sizeof(char *) * global_c); /* newHandles = ( char * * ) malloc ( sizeof ( char * ) * clientNum ) ; */
for (b = 0; b<=global_c; b++) {
if (global_d[b] != a) /* if (clientFDs[i] != fd) */ {
if (!e) /* if (!foundFD) */ {
c[b] = global_d[b]; /* newClientFDs [ i ] = clientFDs [ i ] ; */
d[b] = global_a[b]; /* newHandles [ i ] = handles [ i ] ; */
} else {
c[b - 1] = global_d[b]; /* newClientFDs [ i - 1 ] = clientFDs [ i ] ; */
d[b - 1] = global_a[b]; /* newHandles [ i - 1 ] = handles [ i ] ; */
}
} else {
e = 1; /* foundFD = 1 ; */
if (global_a[b] != ((void *)0 )) /* if (handles[i] != ((void *)0 )) */ {
printf("Bye %s\n", global_a[b]); /* printf ( "Bye %s\n" , handles [ i ] ) ; */
}
free(global_a[b]); /* free ( handles [ i ] ) ; */
close(global_d[b]); /* close ( clientFDs [ i ] ) ; */
}
}
free(global_a); /* free ( handles ) ; */
free(global_d); /* free ( clientFDs ) ; */
global_a = d; /* handles = newHandles ; */
global_d = c; /* clientFDs = newClientFDs ; */
}
int getFDForInput() /* int getFDForInput ( ) { */{
fd_set a; /* fd_set fdvar ; */
int b = 0; /* int i = 0 ; */
int c = global_b + 1; /* int maxFD = serverFD + 1 ; */
FD_ZERO(&a); /* FD_ZERO ( & fdvar ) ; */
FD_SET(global_b, &a); /* FD_SET ( serverFD , & fdvar ) ; */
for (b = 0; b<global_c; b++) {
FD_SET(global_d[b], &a); /* FD_SET ( clientFDs [ i ] , & fdvar ) ; */
}
if (global_c>0) /* if (clientNum>0) */ {
c = global_d[global_c - 1] + 1; /* maxFD = clientFDs [ clientNum - 1 ] + 1 ; */
}
if (mySelect(c, (fd_set *)&a, ((void *)0 ), ((void *)0 ), ((void *)0 )) == -1) /* if (mySelect(maxFD, (fd_set *)&fdvar, ((void *)0 ), ((void *)0 ), ((void *)0 )) == -1) */ {
perror("Error selecting FD to read from"); /* perror ( "Error selecting FD to read from" ) ; */
exit(1); /* exit ( 1 ) ; */
}
if (FD_ISSET(global_b, &a)) /* if (FD_ISSET(serverFD, &fdvar)) */ {
return global_b; /* return serverFD */
} else {
for (b = 0; b<global_c; b++) {
if (FD_ISSET(global_d[b], &a)) /* if (FD_ISSET(clientFDs[i], &fdvar)) */ {
return global_d[b]; /* return clientFDs[i] */
}
}
}
return -1; /* return -1 */
}
void acceptClient() /* void acceptClient ( ) { */{
int a = tcpAccept(global_b); /* int clientFD = tcpAccept ( serverFD ) ; */
addClient(a); /* addClient ( clientFD ) ; */
}
void process() /* void process ( ) { */{
int a; /* int fdToProcess ; */
while (1) /* while (1) */ {
a = getFDForInput(); /* fdToProcess = getFDForInput ( ) ; */
if (a == global_b) /* if (fdToProcess == serverFD) */ {
acceptClient(); /* acceptClient ( ) ; */
} else {
processPacket(a); /* processPacket ( fdToProcess ) ; */
}
}
}
void processPacket(int a) /* void processPacket ( int fd ) { */{
uint8_t *b = malloc(1000); /* uint8_t * packet = malloc ( 1000 ) ; */
int c = 0; /* int numBytes = 0 ; */
if ((c = myRecv(a, b, 1000, 0) )<0) /* if ((numBytes = myRecv(fd, packet, 1000, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (c == 0) /* if (numBytes == 0) */ {
removeClient(a); /* removeClient ( fd ) ; */
}
switch (((header *)b )->flag) /* switch (((header *)packet )->flag) */ {
case 1: 
setClientName(b, a); /* setClientName ( packet , fd ) ; */
break;
case 8: 
sendPacketFlag(9, a); /* sendPacketFlag ( 9 , fd ) ; */
removeClient(a); /* removeClient ( fd ) ; */
break;
case 4: 
broadCast(a, b); /* broadCast ( fd , packet ) ; */
break;
case 5: 
forwardMessage(a, b); /* forwardMessage ( fd , packet ) ; */
break;
case 10: 
sendPacketHandleAck(11, a); /* sendPacketHandleAck ( 11 , fd ) ; */
sendHandles(a); /* sendHandles ( fd ) ; */
break;
default: 
break;
}
free(b); /* free ( packet ) ; */
}
void broadCast(int a, void *b) /* void broadCast ( int senderFD , void * packet ) { */{
int c = 0; /* int i = 0 ; */
for (c = 0; c<global_c; c++) {
if (global_d[c] != a) /* if (clientFDs[i] != senderFD) */ {
if (sendPacket(b, global_d[c])<0) /* if (sendPacket(packet, clientFDs[i])<0) */ {
perror("Packet Broadcast"); /* perror ( "Packet Broadcast" ) ; */
exit(1); /* exit ( 1 ) ; */
}
}
}
}
void forwardMessage(int a, void *b) /* void forwardMessage ( int senderFD , void * packet ) { */{
int c = 0; /* int i = 0 ; */
char *d = ((void *)0 ); /* char * dest = ( ( void * ) 0 ) ; */
int e = 0; /* int sent = 0 ; */
getDestHandle(b, &d); /* getDestHandle ( packet , & dest ) ; */
for (c = 0; c<global_c; c++) {
if (strcmp(d, global_a[c]) == 0) /* if (strcmp(dest, handles[i]) == 0) */ {
if (sendPacket(b, global_d[c])<0) /* if (sendPacket(packet, clientFDs[i])<0) */ {
perror("Packet Broadcast"); /* perror ( "Packet Broadcast" ) ; */
exit(1); /* exit ( 1 ) ; */
}
e = 1; /* sent = 1 ; */
break;
}
}
if (e == 0) /* if (sent == 0) */ {
sendPacketFlag(7, a); /* sendPacketFlag ( 7 , senderFD ) ; */
}
free(d); /* free ( dest ) ; */
}
void sendPacketHandleAck(uint8_t a, int b) /* void sendPacketHandleAck ( uint8_t flag , int socket ) { */{
packetHandleAck c; /* packetHandleAck head ; */
headerSetLen((header *)&c, (uint16_t )(sizeof(packetHandleAck) - sizeof(header))); /* headerSetLen ( ( header * ) & head , ( uint16_t ) ( sizeof ( packetHandleAck ) - sizeof ( header ) ) ) ; */
packetHandleAckSetNum(&c, global_c); /* packetHandleAckSetNum ( & head , clientNum ) ; */
c.head.flag = a; /* head . head . flag = flag ; */
if (mySend(b, &c, sizeof(packetHandleAck), 0)<0) /* if (mySend(socket, &head, sizeof(packetHandleAck), 0)<0) */ {
perror("Flag Packet Send"); /* perror ( "Flag Packet Send" ) ; */
exit(1); /* exit ( 1 ) ; */
}
}
void sendPacketHandle(int a, char *b) /* void sendPacketHandle ( int socket , char * handle ) { */{
int c = strlen(b); /* int hLen = strlen ( handle ) ; */
void *d = makePacketHandle(12, c, b); /* void * packet = makePacketHandle ( 12 , hLen , handle ) ; */
if (sendPacket(d, a)<0) /* if (sendPacket(packet, socket)<0) */ {
perror("Packet Connection"); /* perror ( "Packet Connection" ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(d); /* free ( packet ) ; */
}
void sendHandles(int a) /* void sendHandles ( int socket ) { */{
int b = 0; /* int i = 0 ; */
for (b = 0; b<global_c; b++) {
sendPacketHandle(a, global_a[b]); /* sendPacketHandle ( socket , handles [ i ] ) ; */
}
}
