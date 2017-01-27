int global_a = 0; /* int maxClients = 0 ; */
int global_e = 0; /* int numClients = 0 ; */
char *global_d; /* char * send_packet ; */
char *global_f; /* char * rec_packet ; */
int global_g = 0, global_b = 0; /* int sendLen = 0 , recLen = 0 ; */
fd_set global_c; /* fd_set server_fds ; */
int PacketBuilder(int flag, struct client *theClient, char *packet); /* int PacketBuilder(int flag, struct client {
int socketNum;
char *handle;
unsigned short handleLen;
int isAlive;
} *theClient, char *packet) */
int SetupServer(struct client *client_socket, char *portNum); /* int SetupServer(struct client *client_socket, char *portNum) */
void RunServer(int server_socket, struct client *client_socket); /* void RunServer(int server_socket, struct client *client_socket) */
int CheckNewClientRequest(int server_socket, struct client *client_socket, fd_set *server_fds); /* int CheckNewClientRequest(int server_socket, struct client *client_socket, fd_set *server_fds) */
int CheckIncomingPacket(int server_socket, struct client *client_socket, fd_set *server_fds); /* int CheckIncomingPacket(int server_socket, struct client *client_socket, fd_set *server_fds) */
void ResetFDs(int server_socket, struct client *client_socket, fd_set *server_fds); /* void ResetFDs(int server_socket, struct client *client_socket, fd_set *server_fds) */
int ProcessFlag(char *packet); /* int ProcessFlag(char *packet) */
int ProcessLength(char *packet); /* int ProcessLength(char *packet) */
void RespondToPacket(int flag, int clNdx, char *packet, struct client *client_socket); /* void RespondToPacket(int flag, int clNdx, char *packet, struct client *client_socket) */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
struct client c[10]; /* struct client client_sockets [ 10 ] ; */
int d = 0; /* int server_socket = 0 ; */
if (a>1) /* if (argc>1) */ {
d = SetupServer(c, b[1]); /* server_socket = SetupServer ( client_sockets , argv [ 1 ] ) ; */
} else {
d = SetupServer(c, ((void *)0 )); /* server_socket = SetupServer ( client_sockets , ( ( void * ) 0 ) ) ; */
}
RunServer(d, c); /* RunServer ( server_socket , client_sockets ) ; */
return 0; /* return 0 */
}
int PacketBuilder(int a, struct client *b, char *c) /* int PacketBuilder ( int flag , struct client * theClient , char * packet ) { */{
char *d = calloc(3, sizeof(char)); /* char * cHeadBuf = calloc ( 3 , sizeof ( char ) ) ; */
struct chatHeader {
unsigned short length;
char flag;
} e; /* struct chatHeader cHead ; */
char f; /* char handleLen ; */
char *g; /* char * handle ; */
global_g = 0; /* sendLen = 0 ; */
global_d = calloc(1024, sizeof(char)); /* send_packet = calloc ( 1024 , sizeof ( char ) ) ; */
e.flag = 0; /* cHead . flag = 0 ; */
e.length = 0; /* cHead . length = 0 ; */
g = calloc(255, sizeof(char)); /* handle = calloc ( 255 , sizeof ( char ) ) ; */
switch (a) /* switch (flag) */ {
case 2: e.flag |= a; /* cHead . flag |= flag ; */
memcpy(global_d, d, e.length); /* memcpy ( send_packet , cHeadBuf , cHead . length ) ; */
memcpy(d, &e.length, sizeof(short)); /* memcpy ( cHeadBuf , & cHead . length , sizeof ( short ) ) ; */
global_g = e.length; /* sendLen = cHead . length ; */
d[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
e.length = 3; /* cHead . length = 3 ; */
break;
case 3: e.flag |= a; /* cHead . flag |= flag ; */
memcpy(global_d, d, e.length); /* memcpy ( send_packet , cHeadBuf , cHead . length ) ; */
memcpy(d, &e.length, sizeof(short)); /* memcpy ( cHeadBuf , & cHead . length , sizeof ( short ) ) ; */
d[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
e.length = 3; /* cHead . length = 3 ; */
break;
case 7: memcpy(&f, &global_f[3], sizeof(char)); /* memcpy ( & handleLen , & rec_packet [ 3 ] , sizeof ( char ) ) ; */
memcpy(global_d, d, e.length); /* memcpy ( send_packet , cHeadBuf , cHead . length ) ; */
memcpy(g, &global_f[4], f); /* memcpy ( handle , & rec_packet [ 4 ] , handleLen ) ; */
memcpy(d, &e.length, sizeof(short)); /* memcpy ( cHeadBuf , & cHead . length , sizeof ( short ) ) ; */
memcpy(&global_d[4], g, f); /* memcpy ( & send_packet [ 4 ] , handle , handleLen ) ; */
memcpy(&global_d[3], &f, sizeof(char)); /* memcpy ( & send_packet [ 3 ] , & handleLen , sizeof ( char ) ) ; */
d[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
e.flag |= a; /* cHead . flag |= flag ; */
e.length = 3 + 1 + f; /* cHead . length = 3 + 1 + handleLen ; */
break;
case 9: e.flag |= a; /* cHead . flag |= flag ; */
memcpy(global_d, d, e.length); /* memcpy ( send_packet , cHeadBuf , cHead . length ) ; */
memcpy(d, &e.length, sizeof(short)); /* memcpy ( cHeadBuf , & cHead . length , sizeof ( short ) ) ; */
global_e--; /* numClients -- ; */
d[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
e.length = 3; /* cHead . length = 3 ; */
break;
case 11: e.flag |= a; /* cHead . flag |= flag ; */
memcpy(global_d, d, 3); /* memcpy ( send_packet , cHeadBuf , 3 ) ; */
memcpy(d, &e.length, sizeof(short)); /* memcpy ( cHeadBuf , & cHead . length , sizeof ( short ) ) ; */
memcpy(&global_d[3], &global_e, sizeof(unsigned)); /* memcpy ( & send_packet [ 3 ] , & numClients , sizeof ( unsigned int ) ) ; */
global_g = e.length; /* sendLen = cHead . length ; */
d[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
e.length = 3 + 4; /* cHead . length = 3 + 4 ; */
break;
case 12: e.flag |= a; /* cHead . flag |= flag ; */
memcpy(global_d, d, 3); /* memcpy ( send_packet , cHeadBuf , 3 ) ; */
memcpy(&global_d[4], b[0].handle, b[0].handleLen); /* memcpy ( & send_packet [ 4 ] , theClient [ 0 ] . handle , theClient [ 0 ] . handleLen ) ; */
memcpy(&global_d[3], &b[0].handleLen, sizeof(char)); /* memcpy ( & send_packet [ 3 ] , & theClient [ 0 ] . handleLen , sizeof ( char ) ) ; */
d[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
e.length = 3 + 1 + b[0].handleLen; /* cHead . length = 3 + 1 + theClient [ 0 ] . handleLen ; */
break;
}
global_g = e.length; /* sendLen = cHead . length ; */
return e.length; /* return cHead.length */
}
void RunServer(int a, struct client *b) /* void RunServer ( int server_socket , struct client * client_socket ) { */{
int c = 0, d = 0; /* int receivedClNdx = 0 , inFlag = 0 ; */
while (1) /* while (1) */ {
global_d = calloc(1024, sizeof(char)); /* send_packet = calloc ( 1024 , sizeof ( char ) ) ; */
global_f = calloc(1024, sizeof(char)); /* rec_packet = calloc ( 1024 , sizeof ( char ) ) ; */
ResetFDs(a, b, &global_c); /* ResetFDs ( server_socket , client_socket , & server_fds ) ; */
mySelect(1024, &global_c, ((void *)0 ), ((void *)0 ), ((void *)0 )); /* mySelect ( 1024 , & server_fds , ( ( void * ) 0 ) , ( ( void * ) 0 ) , ( ( void * ) 0 ) ) ; */
if (FD_ISSET(a, &global_c)) /* if (FD_ISSET(server_socket, &server_fds)) */ {
CheckNewClientRequest(a, b, &global_c); /* CheckNewClientRequest ( server_socket , client_socket , & server_fds ) ; */
} else {
d = 0; /* inFlag = 0 ; */
global_b = 0; /* recLen = 0 ; */
c = CheckIncomingPacket(a, b, &global_c); /* receivedClNdx = CheckIncomingPacket ( server_socket , client_socket , & server_fds ) ; */
d = ProcessFlag(global_f); /* inFlag = ProcessFlag ( rec_packet ) ; */
global_b = ProcessLength(global_f); /* recLen = ProcessLength ( rec_packet ) ; */
RespondToPacket(d, c, global_f, b); /* RespondToPacket ( inFlag , receivedClNdx , rec_packet , client_socket ) ; */
}
}
}
void RespondToPacket(int a, int b, char *c, struct client *d) /* void RespondToPacket ( int flag , int clNdx , char * packet , struct client * client_socket ) { */{
char *e; /* char * dstHandle ; */
char f; /* char dstHandleLen ; */
char g; /* char srcHandleLen ; */
int h = 0; /* int i = 0 ; */
char *i; /* char * srcHandle ; */
int j = 1; /* int uniqueHandle = 1 ; */
i = calloc(255, sizeof(char)); /* srcHandle = calloc ( 255 , sizeof ( char ) ) ; */
e = calloc(255, sizeof(char)); /* dstHandle = calloc ( 255 , sizeof ( char ) ) ; */
switch (a) /* switch (flag) */ {
case 1: d[global_a - 1].handle = calloc(255, sizeof(char)); /* client_socket [ maxClients - 1 ] . handle = calloc ( 255 , sizeof ( char ) ) ; */
memcpy(i, &global_f[4], g); /* memcpy ( srcHandle , & rec_packet [ 4 ] , srcHandleLen ) ; */
memcpy(&g, &global_f[3], sizeof(char)); /* memcpy ( & srcHandleLen , & rec_packet [ 3 ] , sizeof ( char ) ) ; */
global_d = calloc(1024, sizeof(char)); /* send_packet = calloc ( 1024 , sizeof ( char ) ) ; */
if (j) /* if (uniqueHandle) */ {
PacketBuilder(2, &d[global_a - 1], ((void *)0 )); /* PacketBuilder ( 2 , & client_socket [ maxClients - 1 ] , ( ( void * ) 0 ) ) ; */
mySend(d[global_a - 1].socketNum, global_d, global_g, 0); /* mySend ( client_socket [ maxClients - 1 ] . socketNum , send_packet , sendLen , 0 ) ; */
d[b].isAlive = 1; /* client_socket [ clNdx ] . isAlive = 1 ; */
d[b].handleLen = g; /* client_socket [ clNdx ] . handleLen = srcHandleLen ; */
memcpy(d[global_a - 1].handle, i, g); /* memcpy ( client_socket [ maxClients - 1 ] . handle , srcHandle , srcHandleLen ) ; */
} else {
global_a--; /* maxClients -- ; */
PacketBuilder(3, &d[global_a - 1], ((void *)0 )); /* PacketBuilder ( 3 , & client_socket [ maxClients - 1 ] , ( ( void * ) 0 ) ) ; */
mySend(d[global_a - 1].socketNum, global_d, global_g, 0); /* mySend ( client_socket [ maxClients - 1 ] . socketNum , send_packet , sendLen , 0 ) ; */
global_e--; /* numClients -- ; */
}
for (l = 0; l<global_a; l++) {
if (!strcmp(i, d[l].handle)) /* if (!strcmp(srcHandle, client_socket[i].handle)) */ {
j = 0; /* uniqueHandle = 0 ; */
} else {

}
}
break;
case 4: memcpy(&g, &global_f[3], sizeof(char)); /* memcpy ( & srcHandleLen , & rec_packet [ 3 ] , sizeof ( char ) ) ; */
memcpy(i, &global_f[4], g); /* memcpy ( srcHandle , & rec_packet [ 4 ] , srcHandleLen ) ; */
for (l = 0; l<global_a; l++) {
if (b == l) /* if (clNdx == i) */ {

} else {
if (d[l].isAlive) /* if (client_socket[i].isAlive) */ {
mySend(d[l].socketNum, global_f, global_b, 0); /* mySend ( client_socket [ i ] . socketNum , rec_packet , recLen , 0 ) ; */
}
}
}
break;
case 5: memcpy(&f, &global_f[3], sizeof(char)); /* memcpy ( & dstHandleLen , & rec_packet [ 3 ] , sizeof ( char ) ) ; */
int k = 0; /* int matched = 0 ; */
memcpy(e, &global_f[4], f); /* memcpy ( dstHandle , & rec_packet [ 4 ] , dstHandleLen ) ; */
if (k == 0) /* if (matched == 0) */ {
PacketBuilder(7, d, c); /* PacketBuilder ( 7 , client_socket , packet ) ; */
mySend(d[b].socketNum, global_d, global_g, 0); /* mySend ( client_socket [ clNdx ] . socketNum , send_packet , sendLen , 0 ) ; */
}
for (l = 0; l<global_a; l++) {
if (!strcmp(e, d[l].handle)) /* if (!strcmp(dstHandle, client_socket[i].handle)) */ {
if (d[l].isAlive) /* if (client_socket[i].isAlive) */ {
mySend(d[l].socketNum, global_f, global_b, 0); /* mySend ( client_socket [ i ] . socketNum , rec_packet , recLen , 0 ) ; */
k = 1; /* matched = 1 ; */
}
}
}
break;
case 8: global_d = calloc(1024, sizeof(char)); /* send_packet = calloc ( 1024 , sizeof ( char ) ) ; */
PacketBuilder(9, &d[b], ((void *)0 )); /* PacketBuilder ( 9 , & client_socket [ clNdx ] , ( ( void * ) 0 ) ) ; */
close(d[b].socketNum); /* close ( client_socket [ clNdx ] . socketNum ) ; */
global_e--; /* numClients -- ; */
mySend(d[b].socketNum, global_d, global_g, 0); /* mySend ( client_socket [ clNdx ] . socketNum , send_packet , sendLen , 0 ) ; */
d[b].isAlive = 0; /* client_socket [ clNdx ] . isAlive = 0 ; */
break;
case 10: global_d = calloc(1024, sizeof(char)); /* send_packet = calloc ( 1024 , sizeof ( char ) ) ; */
int l = 0; /* int i = 0 ; */
PacketBuilder(11, d, c); /* PacketBuilder ( 11 , client_socket , packet ) ; */
mySend(d[b].socketNum, global_d, global_g, 0); /* mySend ( client_socket [ clNdx ] . socketNum , send_packet , sendLen , 0 ) ; */
for (l = 0; l<global_a; l++) {
if (d[l].isAlive) /* if (client_socket[i].isAlive) */ {
global_d = calloc(1024, sizeof(char)); /* send_packet = calloc ( 1024 , sizeof ( char ) ) ; */
PacketBuilder(12, &d[l], c); /* PacketBuilder ( 12 , & client_socket [ i ] , packet ) ; */
mySend(d[b].socketNum, global_d, global_g, 0); /* mySend ( client_socket [ clNdx ] . socketNum , send_packet , sendLen , 0 ) ; */
}
}
break;
}
}
int ProcessLength(char *a) /* int ProcessLength ( char * packet ) { */{
int b = 0; /* int length = 0 ; */
memcpy(&b, a, sizeof(unsigned)); /* memcpy ( & length , packet , sizeof ( unsigned short ) ) ; */
return b; /* return length */
}
int ProcessFlag(char *a) /* int ProcessFlag ( char * packet ) { */{
unsigned short b = 0; /* unsigned short flag = 0 ; */
char *c = calloc(6, sizeof(char)); /* char * flagChar = calloc ( 6 , sizeof ( char ) ) ; */
memcpy(c, &a[2], 1); /* memcpy ( flagChar , & packet [ 2 ] , 1 ) ; */
b = *c; /* flag = * flagChar ; */
return b; /* return flag */
}
void ResetFDs(int a, struct client *b, fd_set *c) /* void ResetFDs ( int server_socket , struct client * client_socket , fd_set * server_fds ) { */{
int d, e = 0; /* int i , maxFd = 0 ; */
FD_ZERO(c); /* FD_ZERO ( server_fds ) ; */
FD_SET(a, c); /* FD_SET ( server_socket , server_fds ) ; */
for (d = 0; d<global_a; d++) {
if (b[d].isAlive) /* if (client_socket[i].isAlive) */ {
FD_SET(b[d].socketNum, c); /* FD_SET ( client_socket [ i ] . socketNum , server_fds ) ; */
}
}
}
int CheckIncomingPacket(int a, struct client *b, fd_set *c) /* int CheckIncomingPacket ( int server_socket , struct client * client_socket , fd_set * server_fds ) { */{
int d = 0, e = 0; /* int j = 0 , receivedClNdx = 0 ; */
for (d = 0; d<global_a; d++) {
if (FD_ISSET(b[d].socketNum, c)) /* if (FD_ISSET(client_socket[j].socketNum, server_fds)) */ {
global_b = 0; /* recLen = 0 ; */
if ((global_b = myRecv(b[d].socketNum, global_f, 1024, 0) )<0) /* if ((recLen = myRecv(client_socket[j].socketNum, rec_packet, 1024, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (global_b == 0) /* if (recLen == 0) */ {
close(b[d].socketNum); /* close ( client_socket [ j ] . socketNum ) ; */
b[d].isAlive = 0; /* client_socket [ j ] . isAlive = 0 ; */
}
e = d; /* receivedClNdx = j ; */
}
}
return e; /* return receivedClNdx */
}
int CheckNewClientRequest(int a, struct client *b, fd_set *c) /* int CheckNewClientRequest ( int server_socket , struct client * client_socket , fd_set * server_fds ) { */{
char *d; /* char * senderHandle ; */
global_e++; /* numClients ++ ; */
d = calloc(255, sizeof(char)); /* senderHandle = calloc ( 255 , sizeof ( char ) ) ; */
b[global_a].socketNum = tcpAccept(a); /* client_socket [ maxClients ] . socketNum = tcpAccept ( server_socket ) ; */
global_a++; /* maxClients ++ ; */
return global_a; /* return maxClients */
}
int SetupServer(struct client *a, char *b) /* int SetupServer ( struct client * client_socket , char * portNum ) { */{
int c = 0; /* int clNdx = 0 ; */
int d; /* int server_socket ; */
global_a++; /* maxClients ++ ; */
global_e++; /* numClients ++ ; */
if (b != ((void *)0 )) /* if (portNum != ((void *)0 )) */ {
d = tcpServerSetup(atoi(b)); /* server_socket = tcpServerSetup ( atoi ( portNum ) ) ; */
} else {
d = tcpServerSetup(); /* server_socket = tcpServerSetup ( ) ; */
}
a[c].socketNum = tcpAccept(d); /* client_socket [ clNdx ] . socketNum = tcpAccept ( server_socket ) ; */
return d; /* return server_socket */
}
