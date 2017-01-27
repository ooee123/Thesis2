int PacketBuilder(int flag, struct client *dstClient, struct client *srcClient, char *message, int messageLen); /* int PacketBuilder(int flag, struct client {
int socketNum;
char *handle;
unsigned short handleLen;
int isAlive;
} *dstClient, struct client *srcClient, char *message, int messageLen) */
void SetupClient(int argc, char *argv, struct client *thisClient); /* void SetupClient(int argc, char *argv, struct client *thisClient) */
void ProcessInput(struct client *thisClient); /* void ProcessInput(struct client *thisClient) */
int ParseInput(char *in_buf, int in_len, struct client *thisClient); /* int ParseInput(char *in_buf, int in_len, struct client *thisClient) */
int ProcessFlag(char *packet); /* int ProcessFlag(char *packet) */
int ProcessLength(char *packet); /* int ProcessLength(char *packet) */
void RespondToPacket(int flag, int inLen, char *packet, int received_socket, struct client *client_socket); /* void RespondToPacket(int flag, int inLen, char *packet, int received_socket, struct client *client_socket) */
char *global_b = ((void *)0 ); /* char * send_packet = ( ( void * ) 0 ) ; */
char *global_c = ((void *)0 ); /* char * rec_packet = ( ( void * ) 0 ) ; */
int global_e = 0; /* int sendLen = 0 ; */
int global_a = 0; /* int recLen = 0 ; */
fd_set global_f; /* fd_set openFDs ; */
int global_d = 0; /* int numClients = 0 ; */

unsigned int __bswap_32(unsigned int a) /* length = 3 ; cHeadBuf [ 2 ] |= flag ; */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /* length , sizeof ( short ) ) ; memcpy */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
struct client c; /* struct client thisClient ; */
c.isAlive = 0; /* thisClient . isAlive = 0 ; */
c.handleLen = 0; /* thisClient . handleLen = 0 ; */
SetupClient(a, b, &c); /* SetupClient ( argc , argv , & thisClient ) ; */
ProcessInput(&c); /* ProcessInput ( & thisClient ) ; */
return 0; /* return 0 */
}
void ProcessInput(struct client *a) /* void ProcessInput ( struct client * thisClient ) { */{
char *b = ((void *)0 ); /* char * in_buf = ( ( void * ) 0 ) ; */
int c = 0, d = 0, e = 0; /* int in_len = 0 , flag = 0 , sent = 0 ; */
global_e = 0; /* sendLen = 0 ; */
printf("\n$: "); /* printf ( "\n$: " ) ; */
while (a->isAlive) /* while (thisClient->isAlive) */ {
printf("\n$: "); /* printf ( "\n$: " ) ; */
global_b = calloc(1024, sizeof(char)); /* send_packet = calloc ( 1024 , sizeof ( char ) ) ; */
global_c = calloc(1024, sizeof(char)); /* rec_packet = calloc ( 1024 , sizeof ( char ) ) ; */
FD_ZERO(&global_f); /* FD_ZERO ( & openFDs ) ; */
FD_SET(a->socketNum, &global_f); /* FD_SET ( thisClient -> socketNum , & openFDs ) ; */
FD_SET(0, &global_f); /* FD_SET ( 0 , & openFDs ) ; */
if (mySelect(1024, &global_f, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (mySelect(1024, &openFDs, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("Error with client select\n"); /* perror ( "Error with client select\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (FD_ISSET(a->socketNum, &global_f)) /* if (FD_ISSET(thisClient->socketNum, &openFDs)) */ {
global_a = 0; /* recLen = 0 ; */
global_c = calloc(1024, sizeof(char)); /* rec_packet = calloc ( 1024 , sizeof ( char ) ) ; */
if ((global_a = myRecv(a->socketNum, global_c, 1024, 0) )<0) /* if ((recLen = myRecv(thisClient->socketNum, rec_packet, 1024, 0) )<0) */ {
printf("error receiving reply"); /* printf ( "error receiving reply" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (global_a == 0) /* if (recLen == 0) */ {
close(a->socketNum); /* close ( thisClient -> socketNum ) ; */
a->isAlive = 0; /* thisClient -> isAlive = 0 ; */
printf("Server Terminated.\n"); /* printf ( "Server Terminated.\n" ) ; */
}
d = ProcessFlag(global_c); /* flag = ProcessFlag ( rec_packet ) ; */
RespondToPacket(d, global_a, global_c, a->socketNum, a); /* RespondToPacket ( flag , recLen , rec_packet , thisClient -> socketNum , thisClient ) ; */
} else if (FD_ISSET(0, &global_f)) /* if (FD_ISSET(0, &openFDs)) */ {
c = 0; /* in_len = 0 ; */
b = calloc(1024, sizeof(char)); /* in_buf = calloc ( 1024 , sizeof ( char ) ) ; */
while ((b[c] = getchar() ) != '\n') /* while ((in_buf[in_len] = getchar() ) != '\n') */ {
c++; /* in_len ++ ; */
}
b[c] = '\0'; /* in_buf [ in_len ] = '\0' ; */
d = ParseInput(b, c, a); /* flag = ParseInput ( in_buf , in_len , thisClient ) ; */
e = mySend(a->socketNum, global_b, global_e, 0); /* sent = mySend ( thisClient -> socketNum , send_packet , sendLen , 0 ) ; */
if (e<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
free(global_b); /* free ( send_packet ) ; */
free(global_c); /* free ( rec_packet ) ; */
}
close(a->socketNum); /* close ( thisClient -> socketNum ) ; */
}
void RespondToPacket(int a, int b, char *c, int d, struct client *e) /* void RespondToPacket ( int flag , int inLen , char * packet , int received_socket , struct client * client_socket ) { */{
char f; /* char handleLen ; */
char *g; /* char * senderHandle ; */
int h; /* int recFlag ; */
int i = 0; /* int messageLen = 0 ; */
char *j; /* char * destHandle ; */
char k; /* char senderHandleLen ; */
char *l; /* char * msgBuffer ; */
switch (a) /* switch (flag) */ {
case 2: break;
case 3: g = calloc(255, sizeof(char)); /* senderHandle = calloc ( 255 , sizeof ( char ) ) ; */
printf("\nHandle already in use: %s\n", e->handle); /* printf ( "\nHandle already in use: %s\n" , client_socket -> handle ) ; */
memcpy(g, &global_c[4], f); /* memcpy ( senderHandle , & rec_packet [ 4 ] , handleLen ) ; */
close(e->socketNum); /* close ( client_socket -> socketNum ) ; */
memcpy(&f, &global_c[3], sizeof(char)); /* memcpy ( & handleLen , & rec_packet [ 3 ] , sizeof ( char ) ) ; */
e->isAlive = 0; /* client_socket -> isAlive = 0 ; */
break;
case 4: g = calloc(255, sizeof(char)); /* senderHandle = calloc ( 255 , sizeof ( char ) ) ; */
printf("%s: %s\n", g, l); /* printf ( "%s: %s\n" , senderHandle , msgBuffer ) ; */
memcpy(g, &global_c[4], f); /* memcpy ( senderHandle , & rec_packet [ 4 ] , handleLen ) ; */
memcpy(l, &global_c[4 + f], i); /* memcpy ( msgBuffer , & rec_packet [ 4 + handleLen ] , messageLen ) ; */
memcpy(&f, &global_c[3], sizeof(char)); /* memcpy ( & handleLen , & rec_packet [ 3 ] , sizeof ( char ) ) ; */
f = 0; /* handleLen = 0 ; */
l = calloc(1000, sizeof(char)); /* msgBuffer = calloc ( 1000 , sizeof ( char ) ) ; */
i = global_a - (4 + f ); /* messageLen = recLen - ( 4 + handleLen ) ; */
break;
case 5: g = calloc(255, sizeof(char)); /* senderHandle = calloc ( 255 , sizeof ( char ) ) ; */
printf("\n%s: %s\n", g, l); /* printf ( "\n%s: %s\n" , senderHandle , msgBuffer ) ; */
memcpy(g, &global_c[5 + f], k); /* memcpy ( senderHandle , & rec_packet [ 5 + handleLen ] , senderHandleLen ) ; */
memcpy(l, &global_c[5 + f + k], i); /* memcpy ( msgBuffer , & rec_packet [ 5 + handleLen + senderHandleLen ] , messageLen ) ; */
memcpy(&f, &global_c[3], sizeof(char)); /* memcpy ( & handleLen , & rec_packet [ 3 ] , sizeof ( char ) ) ; */
memcpy(&k, &global_c[4 + f], sizeof(char)); /* memcpy ( & senderHandleLen , & rec_packet [ 4 + handleLen ] , sizeof ( char ) ) ; */
f = 0; /* handleLen = 0 ; */
l = calloc(1000, sizeof(char)); /* msgBuffer = calloc ( 1000 , sizeof ( char ) ) ; */
i = global_a - (5 + f + k ); /* messageLen = recLen - ( 5 + handleLen + senderHandleLen ) ; */
break;
case 7: f = 0; /* handleLen = 0 ; */
printf("\nClient with handle %s does not exist.\n", j); /* printf ( "\nClient with handle %s does not exist.\n" , destHandle ) ; */
memcpy(j, &global_c[4], f); /* memcpy ( destHandle , & rec_packet [ 4 ] , handleLen ) ; */
memcpy(&f, &global_c[3], sizeof(char)); /* memcpy ( & handleLen , & rec_packet [ 3 ] , sizeof ( char ) ) ; */
j = calloc(255, sizeof(char)); /* destHandle = calloc ( 255 , sizeof ( char ) ) ; */
break;
case 9: close(e->socketNum); /* close ( client_socket -> socketNum ) ; */
e->isAlive = 0; /* client_socket -> isAlive = 0 ; */
break;
case 11: global_d = 0; /* numClients = 0 ; */
int m = 0; /* int i = 0 ; */
printf("\nNumber of clients: %d\n", global_d); /* printf ( "\nNumber of clients: %d\n" , numClients ) ; */
memcpy(&global_d, &global_c[3], sizeof(unsigned)); /* memcpy ( & numClients , & rec_packet [ 3 ] , sizeof ( unsigned int ) ) ; */
for (m = 0; m<global_d; m++) {
global_a = 0; /* recLen = 0 ; */
global_c = calloc(1024, sizeof(char)); /* rec_packet = calloc ( 1024 , sizeof ( char ) ) ; */
if ((global_a = myRecv(d, global_c, 1024, 0) )<0) /* if ((recLen = myRecv(received_socket, rec_packet, 1024, 0) )<0) */ {
printf("error receiving reply"); /* printf ( "error receiving reply" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
h = ProcessFlag(global_c); /* recFlag = ProcessFlag ( rec_packet ) ; */
RespondToPacket(h, global_a, global_c, e->socketNum, e); /* RespondToPacket ( recFlag , recLen , rec_packet , client_socket -> socketNum , client_socket ) ; */
}
break;
case 12: g = calloc(255, sizeof(char)); /* senderHandle = calloc ( 255 , sizeof ( char ) ) ; */
printf("\t %s\n", g); /* printf ( "\t %s\n" , senderHandle ) ; */
memcpy(g, &global_c[4], f); /* memcpy ( senderHandle , & rec_packet [ 4 ] , handleLen ) ; */
memcpy(&f, &global_c[3], sizeof(char)); /* memcpy ( & handleLen , & rec_packet [ 3 ] , sizeof ( char ) ) ; */
f = 0; /* handleLen = 0 ; */
break;
default: printf("\nUnknown incoming packet type:"); /* printf ( "\nUnknown incoming packet type:" ) ; */
}
printf("$: "); /* printf ( "$: " ) ; */
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
int ParseInput(char *a, int b, struct client *c) /* int ParseInput ( char * in_buf , int in_len , struct client * thisClient ) { */{
int d = 0, e = 0, f = 0; /* int flag = 0 , inNdx = 0 , msgNdx = 0 ; */
char *g, *h, *i; /* char * type , * dstHandle , * message ; */
g = calloc(1, sizeof(char)); /* type = calloc ( 1 , sizeof ( char ) ) ; */
while ((a[e] != '%' )&&(e<b )) /* while ((in_buf[inNdx] != '%' )&&(inNdx<in_len )) */ {
e++; /* inNdx ++ ; */
}
e++; /* inNdx ++ ; */
g[0] = a[e]; /* type [ 0 ] = in_buf [ inNdx ] ; */
if (*g == 'B' || *g == 'b') /* if (*type == 'B' || *type == 'b') */ {
e++; /* inNdx ++ ; */
d = 4; /* flag = 4 ; */
i = calloc(b, sizeof(char)); /* message = calloc ( in_len , sizeof ( char ) ) ; */
while ((a[e] == ' ' )&&(e<b )) /* while ((in_buf[inNdx] == ' ' )&&(inNdx<in_len )) */ {
e++; /* inNdx ++ ; */
}
while (e<b) /* while (inNdx<in_len) */ {
e++; /* inNdx ++ ; */
i[f] = a[e]; /* message [ msgNdx ] = in_buf [ inNdx ] ; */
f++; /* msgNdx ++ ; */
}
global_e = PacketBuilder(d, c, ((void *)0 ), i, f); /* sendLen = PacketBuilder ( flag , thisClient , ( ( void * ) 0 ) , message , msgNdx ) ; */
} else if (*g == 'M' || *g == 'm') /* if (*type == 'M' || *type == 'm') */ {
struct client j; /* struct client destClient ; */
int k = 0; /* int hndlNdx = 0 ; */
e++; /* inNdx ++ ; */
d = 5; /* flag = 5 ; */
j.handleLen = 0; /* destClient . handleLen = 0 ; */
j.handle = calloc(255, sizeof(char)); /* destClient . handle = calloc ( 255 , sizeof ( char ) ) ; */
h = calloc(255, sizeof(char)); /* dstHandle = calloc ( 255 , sizeof ( char ) ) ; */
i = calloc(b, sizeof(char)); /* message = calloc ( in_len , sizeof ( char ) ) ; */
while ((a[e] == ' ' )&&(e<b )) /* while ((in_buf[inNdx] == ' ' )&&(inNdx<in_len )) */ {
e++; /* inNdx ++ ; */
}
while ((a[e] != ' ' )&&(e<b )) /* while ((in_buf[inNdx] != ' ' )&&(inNdx<in_len )) */ {
e++; /* inNdx ++ ; */
j.handleLen++; /* destClient . handleLen ++ ; */
j.handle[k] = a[e]; /* destClient . handle [ hndlNdx ] = in_buf [ inNdx ] ; */
k++; /* hndlNdx ++ ; */
}
e++; /* inNdx ++ ; */
while ((a[e] == ' ' )&&(e<b )) /* while ((in_buf[inNdx] == ' ' )&&(inNdx<in_len )) */ {
e++; /* inNdx ++ ; */
}
while (e<b) /* while (inNdx<in_len) */ {
e++; /* inNdx ++ ; */
i[f] = a[e]; /* message [ msgNdx ] = in_buf [ inNdx ] ; */
f++; /* msgNdx ++ ; */
}
global_e = PacketBuilder(d, c, &j, i, f); /* sendLen = PacketBuilder ( flag , thisClient , & destClient , message , msgNdx ) ; */
} else if (*g == 'E' || *g == 'e') /* if (*type == 'E' || *type == 'e') */ {
d = 8; /* flag = 8 ; */
global_e = PacketBuilder(d, c, ((void *)0 ), ((void *)0 ), 0); /* sendLen = PacketBuilder ( flag , thisClient , ( ( void * ) 0 ) , ( ( void * ) 0 ) , 0 ) ; */
} else if (*g == 'L' || *g == 'l') /* if (*type == 'L' || *type == 'l') */ {
d = 10; /* flag = 10 ; */
global_e = PacketBuilder(d, c, ((void *)0 ), ((void *)0 ), 0); /* sendLen = PacketBuilder ( flag , thisClient , ( ( void * ) 0 ) , ( ( void * ) 0 ) , 0 ) ; */
} else {
printf("\nInvalid Command."); /* printf ( "\nInvalid Command." ) ; */
}
return d; /* return flag */
}
void SetupClient(int a, char *b, struct client *c) /* void SetupClient ( int argc , char * argv [ ] , struct client * thisClient ) { */{
int d; /* int flag ; */
int e = 0, f = 0, g; /* int hndlNdx = 0 , handleLen = 0 , sent ; */
global_a = 0; /* recLen = 0 ; */
if (a != 4) /* if (argc != 4) */ {
printf("usage: %s handle host-name port-number \n", b[0]); /* printf ( "usage: %s handle host-name port-number \n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
}
c->handle = calloc(255, sizeof(char)); /* thisClient -> handle = calloc ( 255 , sizeof ( char ) ) ; */
f = strlen(b[1]); /* handleLen = strlen ( argv [ 1 ] ) ; */
while ((e<f )) /* while ((hndlNdx<handleLen )) */ {
c->handle[e] = b[1][e]; /* thisClient -> handle [ hndlNdx ] = argv [ 1 ] [ hndlNdx ] ; */
e++; /* hndlNdx ++ ; */
}
c->handleLen = f; /* thisClient -> handleLen = handleLen ; */
c->socketNum = tcpClientSetup(b[2], b[3]); /* thisClient -> socketNum = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
c->isAlive = 1; /* thisClient -> isAlive = 1 ; */
free(global_b); /* free ( send_packet ) ; */
global_b = calloc(1024, sizeof(char)); /* send_packet = calloc ( 1024 , sizeof ( char ) ) ; */
global_e = PacketBuilder(1, c, ((void *)0 ), ((void *)0 ), 0); /* sendLen = PacketBuilder ( 1 , thisClient , ( ( void * ) 0 ) , ( ( void * ) 0 ) , 0 ) ; */
g = mySend(c->socketNum, global_b, global_e, 0); /* sent = mySend ( thisClient -> socketNum , send_packet , sendLen , 0 ) ; */
if (g<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
global_c = calloc(1024, sizeof(char)); /* rec_packet = calloc ( 1024 , sizeof ( char ) ) ; */
if ((global_a = myRecv(c->socketNum, global_c, 1024, 0) )<0) /* if ((recLen = myRecv(thisClient->socketNum, rec_packet, 1024, 0) )<0) */ {
printf("error receiving reply"); /* printf ( "error receiving reply" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
d = ProcessFlag(global_c); /* flag = ProcessFlag ( rec_packet ) ; */
RespondToPacket(d, global_a, global_c, c->socketNum, c); /* RespondToPacket ( flag , recLen , rec_packet , thisClient -> socketNum , thisClient ) ; */
}
int PacketBuilder(int a, struct client *b, struct client *c, char *d, int e) /* int PacketBuilder ( int flag , struct client * srcClient , struct client * dstClient , char * message , int messageLen ) { */{
struct chatHeader {
unsigned short length;
char flag;
} f; /* struct chatHeader cHead ; */
char *g = calloc(3, sizeof(char)); /* char * cHeadBuf = calloc ( 3 , sizeof ( char ) ) ; */
global_e = 0; /* sendLen = 0 ; */
f.length = 0; /* cHead . length = 0 ; */
global_b = calloc(1024, sizeof(char)); /* send_packet = calloc ( 1024 , sizeof ( char ) ) ; */
f.flag = 0; /* cHead . flag = 0 ; */
switch (a) /* switch (flag) */ {
case 1: f.flag |= a; /* cHead . flag |= flag ; */
memcpy(global_b, g, 3); /* memcpy ( send_packet , cHeadBuf , 3 ) ; */
memcpy(g, &f.length, sizeof(short)); /* memcpy ( cHeadBuf , & cHead . length , sizeof ( short ) ) ; */
memcpy(&global_b[4], b->handle, b->handleLen); /* memcpy ( & send_packet [ 4 ] , srcClient -> handle , srcClient -> handleLen ) ; */
memcpy(&global_b[3], &b->handleLen, sizeof(char)); /* memcpy ( & send_packet [ 3 ] , & srcClient -> handleLen , sizeof ( char ) ) ; */
g[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
f.length = 3 + 1 + b->handleLen; /* cHead . length = 3 + 1 + srcClient -> handleLen ; */
break;
case 4: f.flag |= a; /* cHead . flag |= flag ; */
memcpy(global_b, g, 3); /* memcpy ( send_packet , cHeadBuf , 3 ) ; */
memcpy(g, &f.length, sizeof(short)); /* memcpy ( cHeadBuf , & cHead . length , sizeof ( short ) ) ; */
memcpy(&global_b[4], b->handle, b->handleLen * sizeof(char)); /* memcpy ( & send_packet [ 4 ] , srcClient -> handle , srcClient -> handleLen * sizeof ( char ) ) ; */
memcpy(&global_b[3], &b->handleLen, sizeof(char)); /* memcpy ( & send_packet [ 3 ] , & srcClient -> handleLen , sizeof ( char ) ) ; */
memcpy(&global_b[4 + b->handleLen], d, e); /* memcpy ( & send_packet [ 4 + srcClient -> handleLen ] , message , messageLen ) ; */
g[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
f.length = 3 + 1 + b->handleLen + e; /* cHead . length = 3 + 1 + srcClient -> handleLen + messageLen ; */
break;
case 5: f.flag |= a; /* cHead . flag |= flag ; */
memcpy(global_b, g, 3); /* memcpy ( send_packet , cHeadBuf , 3 ) ; */
memcpy(g, &f.length, sizeof(short)); /* memcpy ( cHeadBuf , & cHead . length , sizeof ( short ) ) ; */
memcpy(&global_b[4], c->handle, c->handleLen); /* memcpy ( & send_packet [ 4 ] , dstClient -> handle , dstClient -> handleLen ) ; */
memcpy(&global_b[3], &c->handleLen, sizeof(char)); /* memcpy ( & send_packet [ 3 ] , & dstClient -> handleLen , sizeof ( char ) ) ; */
memcpy(&global_b[4 + c->handleLen + 1], b->handle, b->handleLen); /* memcpy ( & send_packet [ 4 + dstClient -> handleLen + 1 ] , srcClient -> handle , srcClient -> handleLen ) ; */
memcpy(&global_b[5 + c->handleLen + b->handleLen], d, e); /* memcpy ( & send_packet [ 5 + dstClient -> handleLen + srcClient -> handleLen ] , message , messageLen ) ; */
memcpy(&global_b[4 + c->handleLen], &b->handleLen, sizeof(char)); /* memcpy ( & send_packet [ 4 + dstClient -> handleLen ] , & srcClient -> handleLen , sizeof ( char ) ) ; */
g[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
f.length = 3 + 1 + b->handleLen + 1 + c->handleLen + e; /* cHead . length = 3 + 1 + srcClient -> handleLen + 1 + dstClient -> handleLen + messageLen ; */
break;
case 8: f.flag |= a; /* cHead . flag |= flag ; */
memcpy(global_b, g, f.length); /* memcpy ( send_packet , cHeadBuf , cHead . length ) ; */
memcpy(g, &f.length, sizeof(short)); /* memcpy ( cHeadBuf , & cHead . length , sizeof ( short ) ) ; */
g[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
f.length = 3; /* cHead . length = 3 ; */
break;
case 10: f.flag |= a; /* cHead . flag |= flag ; */
memcpy(global_b, g, f.length); /* memcpy ( send_packet , cHeadBuf , cHead . length ) ; */
memcpy(g, &f.length, sizeof(short)); /* memcpy ( cHeadBuf , & cHead . length , sizeof ( short ) ) ; */
g[2] |= a; /* cHeadBuf [ 2 ] |= flag ; */
f.length = 3; /* cHead . length = 3 ; */
break;
}
return f.length; /* return cHead.length */
}
