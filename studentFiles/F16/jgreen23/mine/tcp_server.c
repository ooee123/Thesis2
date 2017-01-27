typedef struct client {
char *handle;
int sock;
} client;
int global_a = 5; /* int gMaxClients = 5 ; */
int global_b = 0; /* int gClientCount = 0 ; */
client *global_e; /* client * gClients ; */
char *global_c; /* char * gBuff ; */
int global_d = 0; /* int gPort = 0 ; */
int global_f = 0; /* int gServerSock = 0 ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char **b) /* int main ( int argc , char * * argv ) { */{
global_e = (client *)malloc(sizeof(client) * global_a); /* gClients = ( client * ) malloc ( sizeof ( client ) * gMaxClients ) ; */
global_c = (char *)calloc(sizeof(char) * 1024, sizeof(char)); /* gBuff = ( char * ) calloc ( sizeof ( char ) * 1024 , sizeof ( char ) ) ; */
if (a>=2) /* if (argc>=2) */ {
global_d = atoi(b[1]); /* gPort = atoi ( argv [ 1 ] ) ; */
}
global_f = tcpSetup(global_d); /* gServerSock = tcpSetup ( gPort ) ; */
safeListen(); /* safeListen ( ) ; */
while (1) /* while (1) */ {
safeSelect(); /* safeSelect ( ) ; */
}
return 1; /* return 1 */
}
void safeSend(int a, char *b, int c) /* void safeSend ( int sock , char * buff , int len ) { */{
int d = 0; /* int ack = 0 ; */
d = mySend(a, b, c, 0); /* ack = mySend ( sock , buff , len , 0 ) ; */
if (d<0) /* if (ack<0) */ {
perror("send"); /* perror ( "send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
int tcpSetup(int a) /* int tcpSetup ( int port ) { */{
int b; /* int server_socket ; */
struct sockaddr_in c; /* struct sockaddr_in local ; */
socklen_t d = sizeof (c ); /* socklen_t len = sizeof ( local ) ; */
b = socket(2, SOCK_STREAM, 0); /* server_socket = socket ( 2 , SOCK_STREAM , 0 ) ; */
c.sin_family = 2; /* local . sin_family = 2 ; */
c.sin_addr.s_addr = ((in_addr_t)0 ); /* local . sin_addr . s_addr = ( ( in_addr_t ) 0x00000000 ) ; */
if (b<0) /* if (server_socket<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
c.sin_port = htons(a); /* local . sin_port = htons ( port ) ; */
if (myBind(b, (struct sockaddr *)&c, sizeof (c ))<0) /* if (myBind(server_socket, (struct sockaddr *)&local, sizeof (local ))<0) */ {
perror("bind call"); /* perror ( "bind call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (getsockname(b, (struct sockaddr *)&c, &d)<0) /* if (getsockname(server_socket, (struct sockaddr *)&local, &len)<0) */ {
perror("getsockname call"); /* perror ( "getsockname call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("Server is using port %d \n", ntohs(c.sin_port)); /* printf ( "Server is using port %d \n" , ntohs ( local . sin_port ) ) ; */
return b; /* return server_socket */
}
void safeListen() /* void safeListen ( ) { */{
if (listen(global_f, 5)<0) /* if (listen(gServerSock, 5)<0) */ {
perror("listen call"); /* perror ( "listen call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
int safeAccept() /* int safeAccept ( ) { */{
int a = 0; /* int ndx = 0 ; */
int b = 0; /* int clientSock = 0 ; */
if (((b = accept(global_f, (struct sockaddr *)0, (socklen_t *)0) )<0 )) /* if (((clientSock = accept(gServerSock, (struct sockaddr *)0, (socklen_t *)0) )<0 )) */ {
perror("accept"); /* perror ( "accept" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
global_e[global_b++].sock = b; /* gClients [ gClientCount ++ ] . sock = clientSock ; */
if (global_b == global_a) /* if (gClientCount == gMaxClients) */ {
client *c = gClients; /* client * tmp = gClients ; */
global_a *= 2; /* gMaxClients *= 2 ; */
global_e = realloc(global_e, sizeof(client) * global_a); /* gClients = realloc ( gClients , sizeof ( client ) * gMaxClients ) ; */
for (a = 0; a<global_a; a++) {
global_e[a] = c[a]; /* gClients [ ndx ] = tmp [ ndx ] ; */
}
}
return b; /* return clientSock */
}
void safeSelect() /* void safeSelect ( ) { */{
int a = 0; /* int ndx = 0 ; */
int b = 0; /* int socketMax = 0 ; */
fd_set c; /* fd_set fdx ; */
FD_ZERO(&c); /* FD_ZERO ( & fdx ) ; */
FD_SET(global_f, &c); /* FD_SET ( gServerSock , & fdx ) ; */
b = global_f; /* socketMax = gServerSock ; */
for (a = 0; a<=global_b; a++) {
FD_SET(global_e[a].sock, &c); /* FD_SET ( gClients [ ndx ] . sock , & fdx ) ; */
}
for (a = 0; a<global_b; a++) {
if (global_e[a].sock>b) /* if (gClients[ndx].sock>socketMax) */ {
b = global_e[a].sock; /* socketMax = gClients [ ndx ] . sock ; */
}
}
if (mySelect(b + 1, (fd_set *)&c, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (mySelect(socketMax + 1, (fd_set *)&fdx, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select"); /* perror ( "select" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (FD_ISSET(global_f, &c)) /* if (FD_ISSET(gServerSock, &fdx)) */ {
safeAccept(); /* safeAccept ( ) ; */
}
for (a = 0; a<global_b; a++) {
if (FD_ISSET(global_e[a].sock, &c)) /* if (FD_ISSET(gClients[ndx].sock, &fdx)) */ {
handle_message(global_e[a].sock); /* handle_message ( gClients [ ndx ] . sock ) ; */
}
}
}
int getClient(int a) /* int getClient ( int socknum ) { */{
int b; /* int ndx ; */
for (b = 0; b<global_b; b++) {
if (global_e[b].sock == a) /* if (gClients[ndx].sock == socknum) */ {
return b; /* return ndx */
}
}
return -1; /* return -1 */
}
int clientExists(char *a) /* int clientExists ( char * handle ) { */{
int b; /* int ndx ; */
for (b = 0; b<global_b; b++) {
if (global_e[b].handle&&strcmp(global_e[b].handle, a) == 0) /* if (gClients[ndx].handle&&strcmp(gClients[ndx].handle, handle) == 0) */ {
return global_e[b].sock; /* return gClients[ndx].sock */
}
}
return 0; /* return 0 */
}
void handle_message(int a) /* void handle_message ( int sock ) { */{
int b = 0, c = 0; /* int meslen = 0 , ndx = 0 ; */
int d = getClient(a); /* int clientsock = getClient ( sock ) ; */
bzero(global_c, 1024); /* bzero ( gBuff , 1024 ) ; */
if ((b = myRecv(a, global_c, 1024, 0) )<0) /* if ((meslen = myRecv(sock, gBuff, 1024, 0) )<0) */ {
perror("recv"); /* perror ( "recv" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (b == 0) /* if (meslen == 0) */ {
for (c = d; c<global_b + 1; c++) {
global_e[c] = global_e[c + 1]; /* gClients [ ndx ] = gClients [ ndx + 1 ] ; */
}
global_b--; /* gClientCount -- ; */
} else {
int e = global_c[2]; /* int flg = gBuff [ 2 ] ; */
if (e == 1) /* if (flg == 1) */ {
getInit(a, global_c, b); /* getInit ( sock , gBuff , meslen ) ; */
}
if (e == 4) /* if (flg == 4) */ {
getBroad(a, global_c, b); /* getBroad ( sock , gBuff , meslen ) ; */
}
if (e == 5) /* if (flg == 5) */ {
getMessage(a, global_c, b); /* getMessage ( sock , gBuff , meslen ) ; */
}
if (e == 8) /* if (flg == 8) */ {
s_exit(a, global_c, b); /* s_exit ( sock , gBuff , meslen ) ; */
}
if (e == 10) /* if (flg == 10) */ {
getList(a, global_c, b); /* getList ( sock , gBuff , meslen ) ; */
}
memset(&global_c[0], 0, sizeof (global_c )); /* memset ( & gBuff [ 0 ] , 0 , sizeof ( gBuff ) ) ; */
}
}
void getBroad(int a, char *b, int c) /* void getBroad ( int sock , char * buffer , int messlen ) { */{
char *d; /* char * outmsg ; */
int e = 0; /* int ndx = 0 ; */
char *f; /* char * handle ; */
char *g = buffer; /* char * temp = buffer ; */
int h = strlen(b); /* int handleLen = strlen ( buffer ) ; */
f = malloc(h + 1); /* handle = malloc ( handleLen + 1 ) ; */
memcpy(f, b + 3 + 1, h); /* memcpy ( handle , buffer + 3 + 1 , handleLen ) ; */
f[h] = '\0'; /* handle [ handleLen ] = '\0' ; */
b += 4 + h; /* buffer += 4 + handleLen ; */
d = malloc(1024); /* outmsg = malloc ( 1024 ) ; */
strcpy(d, b); /* strcpy ( outmsg , buffer ) ; */
for (e = 0; e<global_b; e++) {
if (global_e[e].sock != a) /* if (gClients[ndx].sock != sock) */ {
safeSend(global_e[e].sock, g, c); /* safeSend ( gClients [ ndx ] . sock , temp , messlen ) ; */
}
}
}
void getList(int a, char *b, int c) /* void getList ( int sock , char * buff , int messlen ) { */{
int d = 0; /* int ndx = 0 ; */
sendNumHandles(a); /* sendNumHandles ( sock ) ; */
for (d = 0; d<global_b; d++) {
sendHandle(a, global_e[d].handle); /* sendHandle ( sock , gClients [ ndx ] . handle ) ; */
}
}
void sendHandle(int a, char *b) /* void sendHandle ( int sock , char * handle ) { */{
int c = 0; /* int packetLen = 0 ; */
char *d; /* char * ptr ; */
char *e; /* char * packet ; */
c = 3 + 1 + strlen(b); /* packetLen = 3 + 1 + strlen ( handle ) ; */
e = calloc(c, sizeof(char)); /* packet = calloc ( packetLen , sizeof ( char ) ) ; */
d = e; /* ptr = packet ; */
*d = htons(c); /* * ptr = htons ( packetLen ) ; */
d += 2; /* ptr += 2 ; */
memset(d, 12, 1); /* memset ( ptr , 12 , 1 ) ; */
d++; /* ptr ++ ; */
memset(d, strlen(b), 1); /* memset ( ptr , strlen ( handle ) , 1 ) ; */
d++; /* ptr ++ ; */
memcpy(d, b, strlen(b) + 1); /* memcpy ( ptr , handle , strlen ( handle ) + 1 ) ; */
e[1] = htons(c)&255; /* packet [ 1 ] = htons ( packetLen ) & 0xff ; */
e[0] = htons(c) >> 8; /* packet [ 0 ] = htons ( packetLen ) >> 8 ; */
safeSend(a, e, c); /* safeSend ( sock , packet , packetLen ) ; */
free(e); /* free ( packet ) ; */
}
void sendNumHandles(int a) /* void sendNumHandles ( int sock ) { */{
char *b, *c; /* char * packet , * ptr ; */
int d = 3 + 4; /* int packLen = 3 + 4 ; */
b = malloc(d); /* packet = malloc ( packLen ) ; */
c = b; /* ptr = packet ; */
*c = htons(d); /* * ptr = htons ( packLen ) ; */
c += 2; /* ptr += 2 ; */
memset(c, 11, 1); /* memset ( ptr , 11 , 1 ) ; */
c++; /* ptr ++ ; */
*c = global_b; /* * ptr = gClientCount ; */
b[1] = htons(d)&255; /* packet [ 1 ] = htons ( packLen ) & 0xff ; */
b[0] = htons(d) >> 8; /* packet [ 0 ] = htons ( packLen ) >> 8 ; */
safeSend(a, b, d); /* safeSend ( sock , packet , packLen ) ; */
}
void getMessage(int a, char *b, int c) /* void getMessage ( int sender_sock , char * buff , int messlen ) { */{
int d; /* int dstSock ; */
char *e; /* char * dstHandle ; */
int f = (int)*(b + 3 ); /* int dstHandleLen = ( int ) * ( buff + 3 ) ; */
e = malloc(f + 1); /* dstHandle = malloc ( dstHandleLen + 1 ) ; */
memcpy(e, b + 4, f); /* memcpy ( dstHandle , buff + 4 , dstHandleLen ) ; */
d = clientExists(e); /* dstSock = clientExists ( dstHandle ) ; */
if (d != 0) /* if (dstSock != 0) */ {
safeSend(d, b, c); /* safeSend ( dstSock , buff , messlen ) ; */
} else {
int g = 3 + 1 + f; /* int packetLen = 3 + 1 + dstHandleLen ; */
char *h = malloc(g); /* char * packet = malloc ( packetLen ) ; */
char *i = packet; /* char * ptr = packet ; */
*i = htons(g); /* * ptr = htons ( packetLen ) ; */
i += 2; /* ptr += 2 ; */
memset(i, 7, 1); /* memset ( ptr , 7 , 1 ) ; */
i++; /* ptr ++ ; */
memset(i, f, 1); /* memset ( ptr , dstHandleLen , 1 ) ; */
i++; /* ptr ++ ; */
memcpy(i, e, f); /* memcpy ( ptr , dstHandle , dstHandleLen ) ; */
h[1] = htons(g)&255; /* packet [ 1 ] = htons ( packetLen ) & 0xff ; */
h[0] = htons(g) >> 8; /* packet [ 0 ] = htons ( packetLen ) >> 8 ; */
safeSend(a, h, g); /* safeSend ( sender_sock , packet , packetLen ) ; */
}
}
void s_exit(int a, char *b, int c) /* void s_exit ( int sock , char * buff , int messlen ) { */{
char *d, *e; /* char * packet , * ptr ; */
int f = htons(3); /* int packetLen = htons ( 3 ) ; */
d = malloc(4); /* packet = malloc ( 4 ) ; */
e = d; /* ptr = packet ; */
*e = f; /* * ptr = packetLen ; */
e += 2; /* ptr += 2 ; */
e[0] = 9; /* ptr [ 0 ] = 9 ; */
safeSend(a, d, f); /* safeSend ( sock , packet , packetLen ) ; */
}
void getInit(int a, char *b, int c) /* void getInit ( int sock , char * buffer , int messlen ) { */{
char *d, *e, *f; /* char * clientHandle , * packet , * ptr ; */
int g = 0; /* int ndx = 0 ; */
int h = 3; /* int packLen = 3 ; */
uint8_t i = (uint8_t ) * (b + 3 ); /* uint8_t handleLen = ( uint8_t ) * ( buffer + 3 ) ; */
d = (char *)malloc(i + 1); /* clientHandle = ( char * ) malloc ( handleLen + 1 ) ; */
memcpy(d, global_c + 4, i); /* memcpy ( clientHandle , gBuff + 4 , handleLen ) ; */
d[i] = '\0'; /* clientHandle [ handleLen ] = '\0' ; */
if (clientExists(d)) /* if (clientExists(clientHandle)) */ {
e = (char *)malloc(h); /* packet = ( char * ) malloc ( packLen ) ; */
f = e; /* ptr = packet ; */
*f = h; /* * ptr = packLen ; */
f += 2; /* ptr += 2 ; */
memset(f, 3, 1); /* memset ( ptr , 3 , 1 ) ; */
e[1] = htons(h)&255; /* packet [ 1 ] = htons ( packLen ) & 0xff ; */
e[0] = htons(h) >> 8; /* packet [ 0 ] = htons ( packLen ) >> 8 ; */
safeSend(a, e, h); /* safeSend ( sock , packet , packLen ) ; */
} else {
g = getClient(a); /* ndx = getClient ( sock ) ; */
global_e[g].handle = d; /* gClients [ ndx ] . handle = clientHandle ; */
strcpy(global_e[g].handle, d); /* strcpy ( gClients [ ndx ] . handle , clientHandle ) ; */
e = malloc(h); /* packet = malloc ( packLen ) ; */
f = e; /* ptr = packet ; */
*f = h; /* * ptr = packLen ; */
f += 2; /* ptr += 2 ; */
memset(f, 2, 1); /* memset ( ptr , 2 , 1 ) ; */
e[1] = h&255; /* packet [ 1 ] = packLen & 0xff ; */
e[0] = h >> 8; /* packet [ 0 ] = packLen >> 8 ; */
safeSend(a, e, h); /* safeSend ( sock , packet , packLen ) ; */
}
}
