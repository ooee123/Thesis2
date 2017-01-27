

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void messagePackets(int a, char *b) /* void messagePackets ( int socket_num , char * myHandle ) { */{
char **c = malloc(sizeof(char *)); /* char * * buffers = malloc ( sizeof ( char * ) ) ; */
int d = 1; /* int numbuffs = 1 ; */
char *e = ((void *)0 ); /* char * send_buf = ( ( void * ) 0 ) ; */
int f; /* int i ; */
char *g = malloc(1000); /* char * text = malloc ( 1000 ) ; */
int h = 0; /* int sent = 0 ; */
uint8_t i = 5; /* uint8_t flag = 5 ; */
int j = 0; /* int textlen = 0 ; */
char *k = malloc(255); /* char * destHandle = malloc ( 255 ) ; */
*c = g; /* * buffers = text ; */
scanf("%s", k); /* scanf ( "%s" , destHandle ) ; */
while ((g[j] = getchar() ) != '\n') /* while ((text[textlen] = getchar() ) != '\n') */ {
j++; /* textlen ++ ; */
if (j == 999) /* if (textlen == 999) */ {
d++; /* numbuffs ++ ; */
g[j] = '\0'; /* text [ textlen ] = '\0' ; */
c = (char **)realloc(c, sizeof(char *) * d); /* buffers = ( char * * ) realloc ( buffers , sizeof ( char * ) * numbuffs ) ; */
g = (char *)malloc(1000); /* text = ( char * ) malloc ( 1000 ) ; */
*(c + (d - 1 ) ) = g; /* * ( buffers + ( numbuffs - 1 ) ) = text ; */
}
}
g[j] = '\0'; /* text [ textlen ] = '\0' ; */
uint16_t l = 5 + strlen(k) + 1000 + strlen(b) + 1; /* uint16_t packetLen = 5 + strlen ( destHandle ) + 1000 + strlen ( myHandle ) + 1 ; */
uint16_t m = htons(l); /* uint16_t npacketLen = htons ( packetLen ) ; */
if ((e = malloc(l) ) == ((void *)0 )) /* if ((send_buf = malloc(packetLen) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
char *n = send_buf; /* char * mover = send_buf ; */
memcpy(n, &m, 2); /* memcpy ( mover , & npacketLen , 2 ) ; */
n += 2; /* mover += 2 ; */
memcpy(n, &i, 1); /* memcpy ( mover , & flag , 1 ) ; */
n += 1; /* mover += 1 ; */
uint8_t o = strlen(k); /* uint8_t destLen = strlen ( destHandle ) ; */
memcpy(n, &o, 1); /* memcpy ( mover , & destLen , 1 ) ; */
n += 1; /* mover += 1 ; */
memcpy(n, k, strlen(k)); /* memcpy ( mover , destHandle , strlen ( destHandle ) ) ; */
n += strlen(k); /* mover += strlen ( destHandle ) ; */
uint8_t p = strlen(b); /* uint8_t myLen = strlen ( myHandle ) ; */
memcpy(n, &p, 1); /* memcpy ( mover , & myLen , 1 ) ; */
n += 1; /* mover += 1 ; */
memcpy(n, b, strlen(b)); /* memcpy ( mover , myHandle , strlen ( myHandle ) ) ; */
n += strlen(b); /* mover += strlen ( myHandle ) ; */
for (f = 0; f<d; f++) {
memcpy(n, c[f], strlen(c[f])); /* memcpy ( mover , buffers [ i ] , strlen ( buffers [ i ] ) ) ; */
h = mySend(a, e, l, 0); /* sent = mySend ( socket_num , send_buf , packetLen , 0 ) ; */
}
}
void broadcastMessage(int a, char *b) /* void broadcastMessage ( int socket_num , char * myHandle ) { */{
uint8_t c = 4; /* uint8_t flag = 4 ; */
int d = 0; /* int sent = 0 ; */
char *e = malloc(1000); /* char * text = malloc ( 1000 ) ; */
int f = 0; /* int textlen = 0 ; */
char *g = ((void *)0 ); /* char * send_buf = ( ( void * ) 0 ) ; */
while ((e[f] = getchar() ) != '\n') /* while ((text[textlen] = getchar() ) != '\n') */ {

}
e[f] = '\0'; /* text [ textlen ] = '\0' ; */
uint16_t h = 4 + strlen(b) + strlen(e) + 1; /* uint16_t packetLen = 4 + strlen ( myHandle ) + strlen ( text ) + 1 ; */
uint16_t i = htons(h); /* uint16_t npacketLen = htons ( packetLen ) ; */
if ((g = malloc(h) ) == ((void *)0 )) /* if ((send_buf = malloc(packetLen) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
char *j = send_buf; /* char * mover = send_buf ; */
memcpy(j, &i, 2); /* memcpy ( mover , & npacketLen , 2 ) ; */
j += 2; /* mover += 2 ; */
memcpy(j, &c, 1); /* memcpy ( mover , & flag , 1 ) ; */
j += 1; /* mover += 1 ; */
uint8_t k = strlen(b); /* uint8_t sendlen = strlen ( myHandle ) ; */
memcpy(j, &k, 1); /* memcpy ( mover , & sendlen , 1 ) ; */
j += 1; /* mover += 1 ; */
memcpy(j, b, strlen(b)); /* memcpy ( mover , myHandle , strlen ( myHandle ) ) ; */
j += strlen(b); /* mover += strlen ( myHandle ) ; */
strcpy(j, e); /* strcpy ( mover , text ) ; */
d = mySend(a, g, h, 0); /* sent = mySend ( socket_num , send_buf , packetLen , 0 ) ; */
if (d<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void listHandles(int a, char *b) /* void listHandles ( int socket_num , char * myHandle ) { */{
uint8_t c = 10; /* uint8_t flag = 10 ; */
uint16_t d = htons(3); /* uint16_t len = htons ( 3 ) ; */
char *e = malloc(3); /* char * buf = malloc ( 3 ) ; */
memcpy(e, &d, 2); /* memcpy ( buf , & len , 2 ) ; */
memcpy(e + 2, &c, 1); /* memcpy ( buf + 2 , & flag , 1 ) ; */
mySend(a, e, 3, 0); /* mySend ( socket_num , buf , 3 , 0 ) ; */
}
void exitProgram(int a) /* void exitProgram ( int socket_num ) { */{
char *b = malloc(3); /* char * buf = malloc ( 3 ) ; */
uint16_t c = 3; /* uint16_t packetLen = 3 ; */
uint8_t d = 8; /* uint8_t flag = 8 ; */
uint16_t e = htons(c); /* uint16_t npacketLen = htons ( packetLen ) ; */
memcpy(b, &e, 2); /* memcpy ( buf , & npacketLen , 2 ) ; */
memcpy(b + 2, &d, 1); /* memcpy ( buf + 2 , & flag , 1 ) ; */
mySend(a, b, c, 0); /* mySend ( socket_num , buf , packetLen , 0 ) ; */
}
void parseCommands(int a, char *b) /* void parseCommands ( int socket_num , char * handle ) { */{
char *c = malloc(3); /* char * command = malloc ( 3 ) ; */
char *d = malloc(1024); /* char * buf = malloc ( 1024 ) ; */
uint32_t e; /* uint32_t numListHandles ; */
fd_set f; /* fd_set fdvar ; */
FD_ZERO(&f); /* FD_ZERO ( & fdvar ) ; */
FD_SET(0, &f); /* FD_SET ( 0 , & fdvar ) ; */
FD_SET(a, &f); /* FD_SET ( socket_num , & fdvar ) ; */
while (1) /* while (1) */ {
printf("$:"); /* printf ( "$:" ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
mySelect(4, &f, ((void *)0 ), ((void *)0 ), ((void *)0 )); /* mySelect ( 4 , & fdvar , ( ( void * ) 0 ) , ( ( void * ) 0 ) , ( ( void * ) 0 ) ) ; */
if (FD_ISSET(0, &f)) /* if (FD_ISSET(0, &fdvar)) */ {
scanf("%s", c); /* scanf ( "%s" , command ) ; */
if ((*c != '%' ) || ((*(c + 1 ) != 'M' )&&(*(c + 1 ) != 'm' )&&(*(c + 1 ) != 'B' )&&(*(c + 1 ) != 'b' )&&(*(c + 1 ) != 'L' )&&(*(c + 1 ) != 'l' )&&(*(c + 1 ) != 'E' )&&(*(c + 1 ) != 'e' ) )) /* if ((*command != '%' ) || ((*(command + 1 ) != 'M' )&&(*(command + 1 ) != 'm' )&&(*(command + 1 ) != 'B' )&&(*(command + 1 ) != 'b' )&&(*(command + 1 ) != 'L' )&&(*(command + 1 ) != 'l' )&&(*(command + 1 ) != 'E' )&&(*(command + 1 ) != 'e' ) )) */ {
printf("Invalid Command\n"); /* printf ( "Invalid Command\n" ) ; */
} else if (strcmp(c, "%M") == 0 || strcmp(c, "%m") == 0) /* if (strcmp(command, "%M") == 0 || strcmp(command, "%m") == 0) */ {
messagePackets(a, b); /* messagePackets ( socket_num , handle ) ; */
} else if (strcmp(c, "%B") == 0 || strcmp(c, "%b") == 0) /* if (strcmp(command, "%B") == 0 || strcmp(command, "%b") == 0) */ {
broadcastMessage(a, b); /* broadcastMessage ( socket_num , handle ) ; */
} else if (strcmp(c, "%L") == 0 || strcmp(c, "%l") == 0) /* if (strcmp(command, "%L") == 0 || strcmp(command, "%l") == 0) */ {
listHandles(a, b); /* listHandles ( socket_num , handle ) ; */
} else if (strcmp(c, "%E") == 0 || strcmp(c, "%e") == 0) /* if (strcmp(command, "%E") == 0 || strcmp(command, "%e") == 0) */ {
exitProgram(a); /* exitProgram ( socket_num ) ; */
}
} else if (FD_ISSET(a, &f)) /* if (FD_ISSET(socket_num, &fdvar)) */ {
uint16_t g; /* uint16_t packetLen ; */
uint8_t h; /* uint8_t flag ; */
int i = myRecv(a, d, 3, 0); /* int rec = myRecv ( socket_num , buf , 3 , 0 ) ; */
memcpy(&g, d, 2); /* memcpy ( & packetLen , buf , 2 ) ; */
uint16_t j = ntohs(g); /* uint16_t pLen = ntohs ( packetLen ) ; */
memcpy(&h, d + 2, 1); /* memcpy ( & flag , buf + 2 , 1 ) ; */
if (h == 5) /* if (flag == 5) */ {
uint8_t k; /* uint8_t senderHandle ; */
char *l = malloc(1000); /* char * text = malloc ( 1000 ) ; */
memcpy(&k, d + strlen(b) + 1, 1); /* memcpy ( & senderHandle , buf + strlen ( handle ) + 1 , 1 ) ; */
char *m = malloc(k); /* char * sender = malloc ( senderHandle ) ; */
memcpy(m, d + 2 + strlen(b), k); /* memcpy ( sender , buf + 2 + strlen ( handle ) , senderHandle ) ; */
strcpy(l, d + 2 + strlen(b) + k); /* strcpy ( text , buf + 2 + strlen ( handle ) + senderHandle ) ; */
printf("\n%s: %s\n", m, l); /* printf ( "\n%s: %s\n" , sender , text ) ; */
} else if (h == 4) /* if (flag == 4) */ {
char *n = malloc(1000); /* char * text = malloc ( 1000 ) ; */
uint8_t o; /* uint8_t sendlen ; */
memcpy(&o, d, 1); /* memcpy ( & sendlen , buf , 1 ) ; */
char *p = malloc(o); /* char * s = malloc ( sendlen ) ; */
memcpy(p, d + 1, o); /* memcpy ( s , buf + 1 , sendlen ) ; */
strcpy(n, d + 1 + o); /* strcpy ( text , buf + 1 + sendlen ) ; */
printf("\n%s: %s\n", p, n); /* printf ( "\n%s: %s\n" , s , text ) ; */
} else if (h == 7) /* if (flag == 7) */ {
uint8_t q; /* uint8_t handlelen ; */
memcpy(&q, d, 1); /* memcpy ( & handlelen , buf , 1 ) ; */
char *r = malloc(q); /* char * toprint = malloc ( handlelen ) ; */
memcpy(r, d + 1, q); /* memcpy ( toprint , buf + 1 , handlelen ) ; */
printf("\nClient with handle %s does not exist.\n", r); /* printf ( "\nClient with handle %s does not exist.\n" , toprint ) ; */
} else if (h == 9) /* if (flag == 9) */ {
exit(0); /* exit ( 0 ) ; */
} else if (h == 11) /* if (flag == 11) */ {
uint32_t s; /* uint32_t num ; */
memcpy(&s, d, 4); /* memcpy ( & num , buf , 4 ) ; */
e = ntohl(s); /* numListHandles = ntohl ( num ) ; */
} else if (h == 12) /* if (flag == 12) */ {
char *t; /* char * string ; */
uint8_t u; /* uint8_t len ; */
int v; /* int i ; */
printf("\nNumber of clients: %lu\n", (unsigned)e); /* printf ( "\nNumber of clients: %lu\n" , ( unsigned long ) numListHandles ) ; */
for (v = 0; v<e; v++) {
myRecv(a, &u, 1, 0); /* myRecv ( socket_num , & len , 1 , 0 ) ; */
t = malloc(u); /* string = malloc ( len ) ; */
myRecv(a, t, u, 0); /* myRecv ( socket_num , string , len , 0 ) ; */
printf("\t%s\n", t); /* printf ( "\t%s\n" , string ) ; */
}
}
if (h != 12) /* if (flag != 12) */ {
i = myRecv(a, d, j - 3, 0); /* rec = myRecv ( socket_num , buf , pLen - 3 , 0 ) ; */
}
}
FD_ZERO(&f); /* FD_ZERO ( & fdvar ) ; */
FD_SET(0, &f); /* FD_SET ( 0 , & fdvar ) ; */
FD_SET(a, &f); /* FD_SET ( socket_num , & fdvar ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
}
void sendInitialPacket(char *a, int b) /* void sendInitialPacket ( char * handle , int socket_num ) { */{
uint8_t c = 1; /* uint8_t flag = 1 ; */
char *d = (char *)malloc(1024); /* char * buf = ( char * ) malloc ( 1024 ) ; */
int e, f; /* int sent , message_len ; */
uint8_t g; /* uint8_t responseFlag ; */
uint8_t h = strlen(a); /* uint8_t handleLen = strlen ( handle ) ; */
int i = h + 4; /* int send_len = handleLen + 4 ; */
char *j = malloc(i); /* char * send_buf = malloc ( send_len ) ; */
char *k = send_buf; /* char * mover = send_buf ; */
uint16_t l = htons(i); /* uint16_t var = htons ( send_len ) ; */
memcpy(k, &l, 2); /* memcpy ( mover , & var , 2 ) ; */
k += 2; /* mover += 2 ; */
memcpy(k, &c, 1); /* memcpy ( mover , & flag , 1 ) ; */
k += 1; /* mover += 1 ; */
memcpy(k, &h, 1); /* memcpy ( mover , & handleLen , 1 ) ; */
k += 1; /* mover += 1 ; */
memcpy(k, a, h); /* memcpy ( mover , handle , handleLen ) ; */
e = mySend(b, j, i, 0); /* sent = mySend ( socket_num , send_buf , send_len , 0 ) ; */
if (e<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((f = myRecv(b, d, 1024, 0) )<0) /* if ((message_len = myRecv(socket_num, buf, 1024, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy(&g, d + 2, 1); /* memcpy ( & responseFlag , buf + 2 , 1 ) ; */
if (g == 2) /* if (responseFlag == 2) */ {
parseCommands(b, a); /* parseCommands ( socket_num , handle ) ; */
} else if (g == 3) /* if (responseFlag == 3) */ {
printf("Handle already in use: %s\n", a); /* printf ( "Handle already in use: %s\n" , handle ) ; */
exit(0); /* exit ( 0 ) ; */
}
}
int main(int a, char **b) /* int main ( int argc , char * * argv ) { */{
int c = 0; /* int socket_num = 0 ; */
if (a<4) /* if (argc<4) */ {
printf("Invalid command line arguments.\n"); /* printf ( "Invalid command line arguments.\n" ) ; */
return 0; /* return 0 */
}
c = tcpClientSetup(b[2], b[3]); /* socket_num = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
sendInitialPacket(b[1], c); /* sendInitialPacket ( argv [ 1 ] , socket_num ) ; */
return 0; /* return 0 */
}
