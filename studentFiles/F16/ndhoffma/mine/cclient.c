char *global_j; /* static char * cHandle ; */
int global_h; /* static int commSocket ; */
fd_set global_i; /* static fd_set sSet ; */
int global_a; /* static int numSocks ; */
struct timeval global_k; /* static struct timeval time ; */
char global_e[2048]; /* static char buf [ 2048 ] ; */
char *global_c; /* static char * bp ; */
struct handle *global_l; /* static struct handle * hList ; */
uint32_t global_m; /* static uint32_t numHandles ; */
void global_b; /* void cReadCommSocket ( ) ; */
void global_g; /* void readUserInput ( ) ; */
void global_d; /* void sendFlag1 ( ) ; */
void recMessage(struct chat_header ch); /* void recMessage(struct chat_header {
uint16_t length;
uint8_t flag;
} ch) */
void global_n; /* void recNumHandles ( ) ; */
void global_f; /* void recHandleList ( ) ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void clearBuffer(int a) /* void clearBuffer ( int skt ) { */{
if (recv(a, global_e, 2048, 0)<0) /* if (recv(skt, buf, 2048, 0)<0) */ {
perror("rec call"); /* perror ( "rec call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void recMessage(struct chat_header a) /* void recMessage ( struct chat_header ch ) { */{
char *b; /* char * srcHandle ; */
ssize_t c; /* ssize_t bytes ; */
char *d; /* char * msg ; */
uint8_t e; /* uint8_t srcHandleLen ; */
if ((c = recv(global_h, global_e, a.length - sizeof(struct chat_header), 0) )<0) /* if ((bytes = recv(commSocket, buf, ch.length - sizeof(struct chat_header), 0) )<0) */ {
perror("Error in client receive: "); /* perror ( "Error in client receive: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (c>0) /* if (bytes>0) */ {
global_c = global_e; /* bp = buf ; */
e = e; /* srcHandleLen = srcHandleLen ; */
if (a.flag == 5) /* if (ch.flag == 5) */ {
global_c += strlen(global_j) + sizeof(uint8_t); /* bp += strlen ( cHandle ) + sizeof ( uint8_t ) ; */
}
memcpy(&e, global_c, sizeof(uint8_t)); /* memcpy ( & srcHandleLen , bp , sizeof ( uint8_t ) ) ; */
global_c += sizeof(uint8_t); /* bp += sizeof ( uint8_t ) ; */
b = calloc(e + 1, sizeof(uint8_t)); /* srcHandle = calloc ( srcHandleLen + 1 , sizeof ( uint8_t ) ) ; */
memcpy(b, global_c, e); /* memcpy ( srcHandle , bp , srcHandleLen ) ; */
global_c += e; /* bp += srcHandleLen ; */
b[e] = '\0'; /* srcHandle [ srcHandleLen ] = '\0' ; */
d = calloc(1024, sizeof(uint8_t)); /* msg = calloc ( 1024 , sizeof ( uint8_t ) ) ; */
strcpy(d, global_c); /* strcpy ( msg , bp ) ; */
printf("Message received from %s\nmsg: %s\n", b, d); /* printf ( "Message received from %s\nmsg: %s\n" , srcHandle , msg ) ; */
}
clearBuffer(global_h); /* clearBuffer ( commSocket ) ; */
}
void recNumHandles() /* void recNumHandles ( ) { */{
ssize_t a; /* ssize_t bytes ; */
if ((a = recv(global_h, global_e, sizeof(uint32_t), 0) )<0) /* if ((bytes = recv(commSocket, buf, sizeof(uint32_t), 0) )<0) */ {
perror("Error in client receive: "); /* perror ( "Error in client receive: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (a>0) /* if (bytes>0) */ {
memcpy(&global_m, global_e, sizeof(uint32_t)); /* memcpy ( & numHandles , buf , sizeof ( uint32_t ) ) ; */
global_m = ntohl(global_m); /* numHandles = ntohl ( numHandles ) ; */
global_l = calloc(global_m, sizeof(struct handle)); /* hList = calloc ( numHandles , sizeof ( struct handle ) ) ; */
}
clearBuffer(global_h); /* clearBuffer ( commSocket ) ; */
}
void recHandleList() /* void recHandleList ( ) { */{
char a[255]; /* char han [ 255 ] ; */
ssize_t b; /* ssize_t bytes ; */
uint8_t c; /* uint8_t hLen ; */
while (global_m) /* while (numHandles) */ {
global_m--; /* numHandles -- ; */
if ((b = recv(global_h, global_e, sizeof(uint8_t), 0) )<0) /* if ((bytes = recv(commSocket, buf, sizeof(uint8_t), 0) )<0) */ {
perror("Error in client receive: "); /* perror ( "Error in client receive: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (b>0) /* if (bytes>0) */ {
memcpy(&c, global_e, sizeof(uint8_t)); /* memcpy ( & hLen , buf , sizeof ( uint8_t ) ) ; */
}
if ((b = recv(global_h, global_e, c, 0) )<0) /* if ((bytes = recv(commSocket, buf, hLen, 0) )<0) */ {
perror("Error in client receive: "); /* perror ( "Error in client receive: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (b>0) /* if (bytes>0) */ {
memcpy(a, global_e, c); /* memcpy ( han , buf , hLen ) ; */
a[c] = '\0'; /* han [ hLen ] = '\0' ; */
}
printf("\t%s\n", a); /* printf ( "\t%s\n" , han ) ; */
}
clearBuffer(global_h); /* clearBuffer ( commSocket ) ; */
}
void sendFlag1() /* void sendFlag1 ( ) { */{
struct handle a; /* struct handle handle ; */
struct chat_header b; /* struct chat_header h ; */
global_c = global_e + sizeof(struct chat_header); /* bp = buf + sizeof ( struct chat_header ) ; */
b.flag = 1; /* h . flag = 0x01 ; */
b.length = sizeof(struct chat_header) + strlen(global_j) + 1; /* h . length = sizeof ( struct chat_header ) + strlen ( cHandle ) + 1 ; */
b.length = htons(b.length); /* h . length = htons ( h . length ) ; */
a.hLen = strlen(global_j); /* handle . hLen = strlen ( cHandle ) ; */
memcpy(a.handle, global_j, strlen(global_j)); /* memcpy ( handle . handle , cHandle , strlen ( cHandle ) ) ; */
memcpy(global_e, &b, sizeof(struct chat_header)); /* memcpy ( buf , & h , sizeof ( struct chat_header ) ) ; */
memcpy(global_c, &a, sizeof(struct handle)); /* memcpy ( bp , & handle , sizeof ( struct handle ) ) ; */
if (send(global_h, global_e, ntohs(b.length), 0)<0) /* if (send(commSocket, buf, ntohs(h.length), 0)<0) */ {
perror("Error in client send call: "); /* perror ( "Error in client send call: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void cReadCommSocket() /* void cReadCommSocket ( ) { */{
ssize_t a; /* ssize_t bytes ; */
struct chat_header b; /* struct chat_header cHead ; */
struct handle c; /* struct handle errHandle ; */
printf("receiving chat header for %s\n", global_j); /* printf ( "receiving chat header for %s\n" , cHandle ) ; */
if ((a = recv(global_h, global_e, sizeof(struct chat_header), 0) )<0) /* if ((bytes = recv(commSocket, buf, sizeof(struct chat_header), 0) )<0) */ {
perror("Error in recv() call: "); /* perror ( "Error in recv() call: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (a>0) /* if (bytes>0) */ {
memcpy(&b, global_e, sizeof(struct chat_header)); /* memcpy ( & cHead , buf , sizeof ( struct chat_header ) ) ; */
global_c = global_e + sizeof(struct chat_header); /* bp = buf + sizeof ( struct chat_header ) ; */
b.length = ntohs(b.length); /* cHead . length = ntohs ( cHead . length ) ; */
printf("received chat header for %s\nlength: %d flag: %d\n", global_j, b.length, b.flag); /* printf ( "received chat header for %s\nlength: %d flag: %d\n" , cHandle , cHead . length , cHead . flag ) ; */
switch (b.flag) /* switch (cHead.flag) */ {
case 2: printf("Flag 2 received\n"); /* printf ( "Flag 2 received\n" ) ; */
break;
case 3: printf("Flag 3 received. Handle already exists, exiting program\n"); /* printf ( "Flag 3 received. Handle already exists, exiting program\n" ) ; */
exit(0); /* exit ( 0 ) ; */
break;
case 4: printf("flag 4 %s\n", global_j); /* printf ( "flag 4 %s\n" , cHandle ) ; */
recMessage(b); /* recMessage ( cHead ) ; */
break;
case 5: recMessage(b); /* recMessage ( cHead ) ; */
break;
case 7: if ((a = recv(global_h, global_e, sizeof(struct handle), 0) )<0) /* if ((bytes = recv(commSocket, buf, sizeof(struct handle {
uint8_t hLen;
uint8_t handle;
}), 0) )<0) */ {
perror("Error in recv() call: "); /* perror ( "Error in recv() call: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (a>0) /* if (bytes>0) */ {
memcpy(&c, global_e, b.length - sizeof(struct chat_header)); /* memcpy ( & errHandle , buf , cHead . length - sizeof ( struct chat_header ) ) ; */
}
printf("Flag 7 received. Destination handle <%.*s> does not exist\n", c.hLen, c.handle); /* printf ( "Flag 7 received. Destination handle <%.*s> does not exist\n" , errHandle . hLen , errHandle . handle ) ; */
break;
case 9: printf("Flag 9 received. Exiting program.\n"); /* printf ( "Flag 9 received. Exiting program.\n" ) ; */
exit(0); /* exit ( 0 ) ; */
break;
case 11: global_n(); /* recNumHandles ( ) ; */
break;
case 12: global_f(); /* recHandleList ( ) ; */
break;
}
}
}
void readUserInput() /* void readUserInput ( ) { */{
char a[255 + 1]; /* char handle [ 255 + 1 ] ; */
struct handle b; /* struct handle dHan ; */
char c[1024]; /* char msg [ 1024 ] ; */
char d[3]; /* char cmd [ 3 ] ; */
char *e; /* char * ptr ; */
char f[2048]; /* char buffer [ 2048 ] ; */
struct chat_header g; /* struct chat_header ch ; */
fgets(d, 3, stdin); /* fgets ( cmd , 3 , stdin ) ; */
d[2] = '\0'; /* cmd [ 2 ] = '\0' ; */
switch (d[1]) /* switch (cmd[1]) */ {
case 'M': case 'm': fgets(f, 255 + 1024 + 1, stdin); /* fgets ( buffer , 255 + 1024 + 1 , stdin ) ; */
memcpy(c, e, 1024); /* memcpy ( msg , ptr , 1024 ) ; */
strcpy(a, f); /* strcpy ( handle , buffer ) ; */
memcpy(global_c, c, strlen(c) + 1); /* memcpy ( bp , msg , strlen ( msg ) + 1 ) ; */
memcpy(global_e, &g, sizeof(struct chat_header)); /* memcpy ( buf , & ch , sizeof ( struct chat_header ) ) ; */
memcpy(global_c, &b, b.hLen + 1); /* memcpy ( bp , & dHan , dHan . hLen + 1 ) ; */
memcpy(b.handle, a, b.hLen); /* memcpy ( dHan . handle , handle , dHan . hLen ) ; */
e += 1024; /* ptr += 1024 ; */
e = strchr(f, ' '); /* ptr = strchr ( buffer , ' ' ) ; */
b.hLen = strlen(a); /* dHan . hLen = strlen ( handle ) ; */
c[1024 - 1] = '\0'; /* msg [ 1024 - 1 ] = '\0' ; */
*e++ = '\0'; /* * ptr ++ = '\0' ; */
global_c = global_e + sizeof(struct chat_header); /* bp = buf + sizeof ( struct chat_header ) ; */
global_c += b.hLen + 1; /* bp += dHan . hLen + 1 ; */
g.flag = 5; /* ch . flag = 0x05 ; */
g.length = sizeof(struct chat_header) + b.hLen + strlen(c) + 2; /* ch . length = sizeof ( struct chat_header ) + dHan . hLen + strlen ( msg ) + 2 ; */
if (send(global_h, global_e, g.length, 0)<0) /* if (send(commSocket, buf, ch.length, 0)<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
break;
case 'B': case 'b': break;
}
}
void loop() /* void loop ( ) { */{
while (1) /* while (1) */ {
FD_ZERO(&global_i); /* FD_ZERO ( & sSet ) ; */
FD_SET(global_h, &global_i); /* FD_SET ( commSocket , & sSet ) ; */
FD_SET(0, &global_i); /* FD_SET ( 0 , & sSet ) ; */
global_a = select(1024, &global_i, (fd_set *)0, (fd_set *)0, &global_k); /* numSocks = select ( 1024 , & sSet , ( fd_set * ) 0 , ( fd_set * ) 0 , & time ) ; */
if (global_a<0) /* if (numSocks<0) */ {
perror("Error in select call: "); /* perror ( "Error in select call: " ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (FD_ISSET(global_h, &global_i)) /* if (FD_ISSET(commSocket, &sSet)) */ {
printf("comm sock | %s\n", global_j); /* printf ( "comm sock | %s\n" , cHandle ) ; */
global_b(); /* cReadCommSocket ( ) ; */
} else if (FD_ISSET(0, &global_i)) /* if (FD_ISSET(0, &sSet)) */ {
printf("usr input\n"); /* printf ( "usr input\n" ) ; */
global_g(); /* readUserInput ( ) ; */
}
}
}
void init(int a, char *b) /* void init ( int argc , char * argv [ ] ) { */{
global_h = parseClientArgs(a, b, &global_j); /* commSocket = parseClientArgs ( argc , argv , & cHandle ) ; */
global_d(); /* sendFlag1 ( ) ; */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
loop(); /* loop ( ) ; */
init(a, b); /* init ( argc , argv ) ; */
return 0; /* return 0 */
}
