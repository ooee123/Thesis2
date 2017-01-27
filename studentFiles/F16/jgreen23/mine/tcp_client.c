char *global_b; /* char * gClientHandle ; */
char *global_c; /* char * gServerName ; */
int global_a[2]; /* int gSocks [ 2 ] ; */
int global_f; /* int gSocketNum ; */
int global_d; /* int gLength ; */
char *global_e; /* char * gCBuff ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
global_e = (char *)calloc(1024, sizeof(char)); /* gCBuff = ( char * ) calloc ( 1024 , sizeof ( char ) ) ; */
if (a != 4) /* if (argc != 4) */ {
printf("usage: %s [handle] [host] [port] \n", b[0]); /* printf ( "usage: %s [handle] [host] [port] \n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
}
getHandle(a, b); /* getHandle ( argc , argv ) ; */
getServer(a, b); /* getServer ( argc , argv ) ; */
global_f = tcpClientSetup(b[2], b[3]); /* gSocketNum = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
startupClient(); /* startupClient ( ) ; */
return 0; /* return 0 */
}
void startupClient() /* void startupClient ( ) { */{
global_a[0] = 2; /* gSocks [ 0 ] = 2 ; */
global_a[1] = global_f; /* gSocks [ 1 ] = gSocketNum ; */
sendInit(); /* sendInit ( ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
while (1) /* while (1) */ {
clientSelect(); /* clientSelect ( ) ; */
free(global_e); /* free ( gCBuff ) ; */
global_e = (char *)calloc(1024, sizeof(char)); /* gCBuff = ( char * ) calloc ( 1024 , sizeof ( char ) ) ; */
}
close(global_f); /* close ( gSocketNum ) ; */
}
void sendInit() /* void sendInit ( ) { */{
char *a; /* char * packet ; */
char *b; /* char * ptr ; */
uint8_t c = strlen(global_b); /* uint8_t handleLen = strlen ( gClientHandle ) ; */
uint16_t d = 3 + 1 + c; /* uint16_t packLength = 3 + 1 + handleLen ; */
a = malloc(d); /* packet = malloc ( packLength ) ; */
b = a; /* ptr = packet ; */
*b = htons(d); /* * ptr = htons ( packLength ) ; */
b += 2; /* ptr += 2 ; */
memset(b, 1, 1); /* memset ( ptr , 1 , 1 ) ; */
b += 1; /* ptr += 1 ; */
memset(b, c, 1); /* memset ( ptr , handleLen , 1 ) ; */
b += 1; /* ptr += 1 ; */
memcpy(b, global_b, c); /* memcpy ( ptr , gClientHandle , handleLen ) ; */
b += c; /* ptr += handleLen ; */
a[1] = htons(d)&255; /* packet [ 1 ] = htons ( packLength ) & 0xff ; */
a[0] = htons(d) >> 8; /* packet [ 0 ] = htons ( packLength ) >> 8 ; */
safeSend(a, d); /* safeSend ( packet , packLength ) ; */
initGet(); /* initGet ( ) ; */
}
void initGet() /* void initGet ( ) { */{
char a; /* char flag ; */
int b; /* int messlen ; */
if ((b = myRecv(global_f, global_e, 1024, 0) )<0) /* if ((messlen = myRecv(gSocketNum, gCBuff, 1024, 0) )<0) */ {
perror("recv"); /* perror ( "recv" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
a = global_e[2]; /* flag = gCBuff [ 2 ] ; */
if (a == 3) /* if (flag == 3) */ {
printf("Handle already in use: %s\n", global_b); /* printf ( "Handle already in use: %s\n" , gClientHandle ) ; */
close(global_f); /* close ( gSocketNum ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (a != 2) /* if (flag != 2) */ {
printf("bad flag: %d\n", a); /* printf ( "bad flag: %d\n" , flag ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void safeSend(char *a, int b) /* void safeSend ( char * packet , int packetLength ) { */{
int c; /* int ack ; */
c = mySend(global_f, a, b, 0); /* ack = mySend ( gSocketNum , packet , packetLength , 0 ) ; */
if (c<0) /* if (ack<0) */ {
perror("send"); /* perror ( "send" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void clientSelect() /* void clientSelect ( ) { */{
char *a; /* char * inbuff ; */
fd_set b; /* fd_set fdx ; */
int c; /* int in_len ; */
FD_ZERO(&b); /* FD_ZERO ( & fdx ) ; */
FD_SET(global_a[0], &b); /* FD_SET ( gSocks [ 0 ] , & fdx ) ; */
FD_SET(global_a[1], &b); /* FD_SET ( gSocks [ 1 ] , & fdx ) ; */
if (mySelect(global_f + 1, (fd_set *)&b, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (mySelect(gSocketNum + 1, (fd_set *)&fdx, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select"); /* perror ( "select" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (FD_ISSET(global_a[0], &b)) /* if (FD_ISSET(gSocks[0], &fdx)) */ {
c = 0; /* in_len = 0 ; */
a = (char *)malloc(1024); /* inbuff = ( char * ) malloc ( 1024 ) ; */
while ((a[c] = getchar() ) != '\n') /* while ((inbuff[in_len] = getchar() ) != '\n') */ {

}
a[c] = '\0'; /* inbuff [ in_len ] = '\0' ; */
if (a[0] != '%') /* if (inbuff[0] != '%') */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
} else if (a[0] == '%'&&!((*__ctype_b_loc() )[(int)((a[1] ) )]&(unsigned)_ISspace )) /* if (inbuff[0] == '%'&&!((*__ctype_b_loc() )[(int)((inbuff[1] ) )]&(unsigned)_ISspace )) */ {
if ((a[1] == 'B' || a[1] == 'b' )&&((*__ctype_b_loc() )[(int)((a[2] ) )]&(unsigned)_ISspace )) /* if ((inbuff[1] == 'B' || inbuff[1] == 'b' )&&((*__ctype_b_loc() )[(int)((inbuff[2] ) )]&(unsigned)_ISspace )) */ {
broadcast(a); /* broadcast ( inbuff ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
if ((a[1] == 'E' || a[1] == 'e' )) /* if ((inbuff[1] == 'E' || inbuff[1] == 'e' )) */ {
c_exit(a); /* c_exit ( inbuff ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
if ((a[1] == 'M' || a[1] == 'm' )) /* if ((inbuff[1] == 'M' || inbuff[1] == 'm' )) */ {
message(a); /* message ( inbuff ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
if ((a[1] == 'L' || a[1] == 'l' )) /* if ((inbuff[1] == 'L' || inbuff[1] == 'l' )) */ {
c_list(a); /* c_list ( inbuff ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
}
memset(&a[0], 0, sizeof (a )); /* memset ( & inbuff [ 0 ] , 0 , sizeof ( inbuff ) ) ; */
}
if (FD_ISSET(global_a[1], &b)) /* if (FD_ISSET(gSocks[1], &fdx)) */ {
serverListen(); /* serverListen ( ) ; */
}
}
void getHandle(int a, char *b) /* void getHandle ( int argc , char * argv [ ] ) { */{
global_b = malloc(strlen(b[1]) + 1); /* gClientHandle = malloc ( strlen ( argv [ 1 ] ) + 1 ) ; */
strcpy(global_b, b[1]); /* strcpy ( gClientHandle , argv [ 1 ] ) ; */
}
void getServer(int a, char *b) /* void getServer ( int argc , char * argv [ ] ) { */{
global_c = malloc(strlen(b[2]) + 1); /* gServerName = malloc ( strlen ( argv [ 2 ] ) + 1 ) ; */
strcpy(global_c, b[2]); /* strcpy ( gServerName , argv [ 2 ] ) ; */
}
void serverListen() /* void serverListen ( ) { */{
int a; /* int flag ; */
int b = 0; /* int handleLen = 0 ; */
char *c; /* char * handle ; */
int d; /* int messlen ; */
if ((d = myRecv(global_f, global_e, 1024, 0) )<0) /* if ((messlen = myRecv(gSocketNum, gCBuff, 1024, 0) )<0) */ {
perror("recv"); /* perror ( "recv" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (d == 0) /* if (messlen == 0) */ {
printf("Server terminated\n"); /* printf ( "Server terminated\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
a = global_e[2]; /* flag = gCBuff [ 2 ] ; */
if (a == 7) /* if (flag == 7) */ {
b = (int)*(global_e + 3 ); /* handleLen = ( int ) * ( gCBuff + 3 ) ; */
c = malloc(b + 1); /* handle = malloc ( handleLen + 1 ) ; */
memcpy(c, global_e + 4, b); /* memcpy ( handle , gCBuff + 4 , handleLen ) ; */
c[b] = '\0'; /* handle [ handleLen ] = '\0' ; */
printf("Client with handle %s does not exist\n", c); /* printf ( "Client with handle %s does not exist\n" , handle ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
} else if (a == 4 || a == 0) /* if (flag == 4 || flag == 0) */ {
getBroadcast(global_e, d); /* getBroadcast ( gCBuff , messlen ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
} else if (a == 9) /* if (flag == 9) */ {
close(global_f); /* close ( gSocketNum ) ; */
exit(1); /* exit ( 1 ) ; */
} else if (a == 5) /* if (flag == 5) */ {
getMessage(global_e, d); /* getMessage ( gCBuff , messlen ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
memset(&global_e[0], 0, sizeof (global_e )); /* memset ( & gCBuff [ 0 ] , 0 , sizeof ( gCBuff ) ) ; */
} else if (a == 11) /* if (flag == 11) */ {
getListCount(global_e, d); /* getListCount ( gCBuff , messlen ) ; */
} else if (a == 12) /* if (flag == 12) */ {
listHandles(); /* listHandles ( ) ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
} else {
printf("bad flag is %d\n", a); /* printf ( "bad flag is %d\n" , flag ) ; */
}
memset(&global_e[0], 0, sizeof (global_e )); /* memset ( & gCBuff [ 0 ] , 0 , sizeof ( gCBuff ) ) ; */
}
void getBroadcast(char *a, int b) /* void getBroadcast ( char * buffer , int messlen ) { */{
char *c; /* char * msg ; */
char *d; /* char * handle ; */
int e = (int)*(a + 3 ); /* int handleLen = ( int ) * ( buffer + 3 ) ; */
d = malloc(e + 1); /* handle = malloc ( handleLen + 1 ) ; */
memcpy(d, a + 4, e); /* memcpy ( handle , buffer + 4 , handleLen ) ; */
d[e] = '\0'; /* handle [ handleLen ] = '\0' ; */
a += 3 + e; /* buffer += 3 + handleLen ; */
c = malloc(b); /* msg = malloc ( messlen ) ; */
strcpy(c, a); /* strcpy ( msg , buffer ) ; */
printf("\n%s: %s\n", d, c + 1); /* printf ( "\n%s: %s\n" , handle , msg + 1 ) ; */
}
void c_exit(char *a) /* void c_exit ( char * buffer ) { */{
char *b, *c; /* char * packet , * ptr ; */
int d = 3; /* int packetLen = 3 ; */
b = malloc(d); /* packet = malloc ( packetLen ) ; */
c = b; /* ptr = packet ; */
*c = htons(d); /* * ptr = htons ( packetLen ) ; */
c += 2; /* ptr += 2 ; */
memset(c, 8, 1); /* memset ( ptr , 8 , 1 ) ; */
b[1] = htons(d)&255; /* packet [ 1 ] = htons ( packetLen ) & 0xff ; */
b[0] = htons(d) >> 8; /* packet [ 0 ] = htons ( packetLen ) >> 8 ; */
safeSend(b, d); /* safeSend ( packet , packetLen ) ; */
}
void c_list(char *a) /* void c_list ( char * buffer ) { */{
int b = 3; /* int packetLen = 3 ; */
char *c, *d; /* char * packet , * ptr ; */
c = malloc(b); /* packet = malloc ( packetLen ) ; */
d = c; /* ptr = packet ; */
*d = htons(b); /* * ptr = htons ( packetLen ) ; */
d += 2; /* ptr += 2 ; */
memset(d, 10, 1); /* memset ( ptr , 10 , 1 ) ; */
c[1] = htons(b)&255; /* packet [ 1 ] = htons ( packetLen ) & 0xff ; */
c[0] = htons(b) >> 8; /* packet [ 0 ] = htons ( packetLen ) >> 8 ; */
safeSend(c, b); /* safeSend ( packet , packetLen ) ; */
}
void broadcast(char *a) /* void broadcast ( char * buffer ) { */{
int b = 0, c = 0, d = 0; /* int handleLen = 0 , textLen = 0 , packetLen = 0 ; */
char *e; /* char * input ; */
char *f, *g; /* char * packet , * ptr ; */
e = malloc(strlen(a) + 1); /* input = malloc ( strlen ( buffer ) + 1 ) ; */
strcpy(e, a); /* strcpy ( input , buffer ) ; */
e += 3; /* input += 3 ; */
if (strlen(e)>1024) /* if (strlen(input)>1024) */ {
printf("ERR: Message to long.\n"); /* printf ( "ERR: Message to long.\n" ) ; */
} else {
b = strlen(global_b); /* handleLen = strlen ( gClientHandle ) ; */
c = strlen(e) + 1; /* textLen = strlen ( input ) + 1 ; */
d = 3 + 1 + b + c; /* packetLen = 3 + 1 + handleLen + textLen ; */
f = malloc(d); /* packet = malloc ( packetLen ) ; */
uint16_t h = packetLen; /* uint16_t packetLen2BYTES = packetLen ; */
g = f; /* ptr = packet ; */
*g = htons(h); /* * ptr = htons ( packetLen2BYTES ) ; */
g += 2; /* ptr += 2 ; */
memset(g, 4, 1); /* memset ( ptr , 4 , 1 ) ; */
g++; /* ptr ++ ; */
memset(g, b, 1); /* memset ( ptr , handleLen , 1 ) ; */
g++; /* ptr ++ ; */
memcpy(g, global_b, b); /* memcpy ( ptr , gClientHandle , handleLen ) ; */
g += b; /* ptr += handleLen ; */
strcpy(g, e); /* strcpy ( ptr , input ) ; */
f[1] = htons(d)&255; /* packet [ 1 ] = htons ( packetLen ) & 0xff ; */
f[0] = htons(d) >> 8; /* packet [ 0 ] = htons ( packetLen ) >> 8 ; */
safeSend(f, d); /* safeSend ( packet , packetLen ) ; */
}
}
void message(char *a) /* void message ( char * buffer ) { */{
char *b, *c; /* char * packet , * ptr ; */
char *d, *e, *f; /* char * input , * handle , * msg ; */
int g = 0, h = 0, i = 0, j = 0; /* int myHandleLen = 0 , dstHandleLen = 0 , textLen = 0 , packetLen = 0 ; */
d = malloc(strlen(a) + 1); /* input = malloc ( strlen ( buffer ) + 1 ) ; */
strcpy(d, a); /* strcpy ( input , buffer ) ; */
strtok(d, " "); /* strtok ( input , " " ) ; */
e = strtok(((void *)0 ), " "); /* handle = strtok ( ( ( void * ) 0 ) , " " ) ; */
if (e == ((void *)0 )) /* if (handle == ((void *)0 )) */ {
printf("ERR: Must provide handle\n"); /* printf ( "ERR: Must provide handle\n" ) ; */
return; /* return */
} else {
f = a; /* msg = buffer ; */
}
f += 3; /* msg += 3 ; */
if (strlen(f)>1024) /* if (strlen(msg)>1024) */ {
printf("ERR: Message to long.\n"); /* printf ( "ERR: Message to long.\n" ) ; */
} else {
g = strlen(global_b); /* myHandleLen = strlen ( gClientHandle ) ; */
h = strlen(e); /* dstHandleLen = strlen ( handle ) ; */
d += h; /* input += dstHandleLen ; */
i = strlen(f) + 1; /* textLen = strlen ( msg ) + 1 ; */
j = 3 + 1 + g + i + h; /* packetLen = 3 + 1 + myHandleLen + textLen + dstHandleLen ; */
b = malloc(j); /* packet = malloc ( packetLen ) ; */
uint16_t k = packetLen; /* uint16_t packetLen2BYTES = packetLen ; */
c = b; /* ptr = packet ; */
*c = htons(k); /* * ptr = htons ( packetLen2BYTES ) ; */
c += 2; /* ptr += 2 ; */
memset(c, 5, 1); /* memset ( ptr , 5 , 1 ) ; */
c++; /* ptr ++ ; */
memset(c, h, 1); /* memset ( ptr , dstHandleLen , 1 ) ; */
c++; /* ptr ++ ; */
memcpy(c, e, h); /* memcpy ( ptr , handle , dstHandleLen ) ; */
c += h; /* ptr += dstHandleLen ; */
memset(c, g, 1); /* memset ( ptr , myHandleLen , 1 ) ; */
c++; /* ptr ++ ; */
memcpy(c, global_b, g); /* memcpy ( ptr , gClientHandle , myHandleLen ) ; */
c += g; /* ptr += myHandleLen ; */
strcpy(c, f + g); /* strcpy ( ptr , msg + myHandleLen ) ; */
b[1] = htons(j)&255; /* packet [ 1 ] = htons ( packetLen ) & 0xff ; */
b[0] = htons(j) >> 8; /* packet [ 0 ] = htons ( packetLen ) >> 8 ; */
safeSend(b, j); /* safeSend ( packet , packetLen ) ; */
memset(&f[0], 0, sizeof (f )); /* memset ( & msg [ 0 ] , 0 , sizeof ( msg ) ) ; */
}
}
void getMessage(char *a, int b) /* void getMessage ( char * buffer , int messlen ) { */{
int c; /* int cliHandleLen ; */
char *d, *e; /* char * cliHandle , * dstHandle ; */
char *f = ((void *)0 ); /* char * msg = ( ( void * ) 0 ) ; */
int g = (int)*(a + 3 ); /* int dstHandleLen = ( int ) * ( buffer + 3 ) ; */
e = malloc(g + 1); /* dstHandle = malloc ( dstHandleLen + 1 ) ; */
e[g] = '\0'; /* dstHandle [ dstHandleLen ] = '\0' ; */
a += 4 + g; /* buffer += 4 + dstHandleLen ; */
c = (int)*a; /* cliHandleLen = ( int ) * buffer ; */
d = malloc(c + 1); /* cliHandle = malloc ( cliHandleLen + 1 ) ; */
memcpy(d, a + 1, c); /* memcpy ( cliHandle , buffer + 1 , cliHandleLen ) ; */
d[c] = '\0'; /* cliHandle [ cliHandleLen ] = '\0' ; */
a += 1 + c; /* buffer += 1 + cliHandleLen ; */
f = malloc(b); /* msg = malloc ( messlen ) ; */
strcpy(f, a); /* strcpy ( msg , buffer ) ; */
a = ((void *)0 ); /* buffer = ( ( void * ) 0 ) ; */
printf("\n%s: %s\n", d, f); /* printf ( "\n%s: %s\n" , cliHandle , msg ) ; */
free(f); /* free ( msg ) ; */
}
void getListCount(char *a, int b) /* void getListCount ( char * buff , int messlen ) { */{
int c = 0, d; /* int handlecount = 0 , ndx ; */
a += 3; /* buff += 3 ; */
memcpy(&c, a, 4); /* memcpy ( & handlecount , buff , 4 ) ; */
printf("There are %d handles\n", c); /* printf ( "There are %d handles\n" , handlecount ) ; */
for (d = 0; d<c; d++) {

}
}
void listHandles() /* void listHandles ( ) { */{
char *a; /* char * handle ; */
int b; /* int handleLen ; */
b = *(global_e + 3 ); /* handleLen = * ( gCBuff + 3 ) ; */
a = malloc(b + 1); /* handle = malloc ( handleLen + 1 ) ; */
memcpy(a, global_e + 4, b); /* memcpy ( handle , gCBuff + 4 , handleLen ) ; */
printf("   %s\n", a); /* printf ( "   %s\n" , handle ) ; */
}
