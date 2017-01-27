int global_a; /* int sock ; */
int global_e; /* int bodySize ; */
uint8_t global_k; /* uint8_t handleLen ; */
uint8_t *global_i; /* uint8_t * handle ; */
char *global_g; /* char * sendBuf ; */
uint8_t global_d[3]; /* uint8_t headBuf [ 3 ] ; */
uint8_t *global_b; /* uint8_t * bodyBuf ; */
char *global_f; /* char * readBuf ; */
char global_c[3]; /* char commandBuf [ 3 ] ; */
char *global_h; /* char * handleBuf ; */
int global_j; /* int hlen ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int sendInit() /* int sendInit ( ) { */{
size_t a = 4 + global_k; /* size_t sendLen = 4 + handleLen ; */
global_g = (char *)calloc(a, sizeof(char)); /* sendBuf = ( char * ) calloc ( sendLen , sizeof ( char ) ) ; */
((uint16_t *)global_g )[0] = htons(a); /* ( ( uint16_t * ) sendBuf ) [ 0 ] = htons ( sendLen ) ; */
global_g[2] = 1; /* sendBuf [ 2 ] = 1 ; */
global_g[3] = global_k; /* sendBuf [ 3 ] = handleLen ; */
memcpy(global_g + 4, global_i, global_k); /* memcpy ( sendBuf + 4 , handle , handleLen ) ; */
mySend(global_a, global_g, a, 0); /* mySend ( sock , sendBuf , sendLen , 0 ) ; */
free(global_g); /* free ( sendBuf ) ; */
if (myRecv(global_a, global_d, 3, MSG_WAITALL)<3) /* if (myRecv(sock, headBuf, 3, MSG_WAITALL)<3) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (global_d[2] == 2) /* if (headBuf[2] == 2) */ {
return 1; /* return 1 */
} else if (global_d[2] == 3) /* if (headBuf[2] == 3) */ {
printf("Handle already in use: %s\n", global_i); /* printf ( "Handle already in use: %s\n" , handle ) ; */
return 0; /* return 0 */
} else {
perror("wrong flag"); /* perror ( "wrong flag" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
int newCommand() /* int newCommand ( ) { */{
scanf("%s", global_c); /* scanf ( "%s" , commandBuf ) ; */
global_c[2] = '\0'; /* commandBuf [ 2 ] = '\0' ; */
if (global_c[0] != '%') /* if (commandBuf[0] != '%') */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
return 0; /* return 0 */
}
if (global_c[1] == 'e' || global_c[1] == 'E') /* if (commandBuf[1] == 'e' || commandBuf[1] == 'E') */ {
global_g = (char *)calloc(3, sizeof(char)); /* sendBuf = ( char * ) calloc ( 3 , sizeof ( char ) ) ; */
((uint16_t *)global_g )[0] = htons(3); /* ( ( uint16_t * ) sendBuf ) [ 0 ] = htons ( 3 ) ; */
global_g[2] = 8; /* sendBuf [ 2 ] = 8 ; */
mySend(global_a, global_g, 3, 0); /* mySend ( sock , sendBuf , 3 , 0 ) ; */
free(global_g); /* free ( sendBuf ) ; */
} else if (global_c[1] == 'b' || global_c[1] == 'B') /* if (commandBuf[1] == 'b' || commandBuf[1] == 'B') */ {
global_f = (char *)calloc(1001, sizeof(char)); /* readBuf = ( char * ) calloc ( 1001 , sizeof ( char ) ) ; */
fgets(global_f, 1000, stdin); /* fgets ( readBuf , 1000 , stdin ) ; */
global_f[strlen(global_f) - 1] = '\0'; /* readBuf [ strlen ( readBuf ) - 1 ] = '\0' ; */
int a = 4 + global_k + strlen(global_f); /* int sendLen = 4 + handleLen + strlen ( readBuf ) ; */
global_g = (char *)calloc(a, sizeof(char)); /* sendBuf = ( char * ) calloc ( sendLen , sizeof ( char ) ) ; */
((uint16_t *)global_g )[0] = htons(a); /* ( ( uint16_t * ) sendBuf ) [ 0 ] = htons ( sendLen ) ; */
global_g[2] = 4; /* sendBuf [ 2 ] = 4 ; */
global_g[3] = global_k; /* sendBuf [ 3 ] = handleLen ; */
memcpy(global_g + 4, global_i, global_k); /* memcpy ( sendBuf + 4 , handle , handleLen ) ; */
memcpy(global_g + 4 + global_k, global_f, strlen(global_f)); /* memcpy ( sendBuf + 4 + handleLen , readBuf , strlen ( readBuf ) ) ; */
mySend(global_a, global_g, a, 0); /* mySend ( sock , sendBuf , sendLen , 0 ) ; */
free(global_g); /* free ( sendBuf ) ; */
free(global_f); /* free ( readBuf ) ; */
} else if (global_c[1] == 'm' || global_c[1] == 'M') /* if (commandBuf[1] == 'm' || commandBuf[1] == 'M') */ {
global_h = (char *)calloc(256, sizeof(char)); /* handleBuf = ( char * ) calloc ( 256 , sizeof ( char ) ) ; */
scanf("%s", global_h); /* scanf ( "%s" , handleBuf ) ; */
global_j = strlen(global_h); /* hlen = strlen ( handleBuf ) ; */
global_f = (char *)calloc(1001, sizeof(char)); /* readBuf = ( char * ) calloc ( 1001 , sizeof ( char ) ) ; */
fgets(global_f, 1000, stdin); /* fgets ( readBuf , 1000 , stdin ) ; */
global_f[strlen(global_f) - 1] = '\0'; /* readBuf [ strlen ( readBuf ) - 1 ] = '\0' ; */
int b = 4 + global_j + 1 + global_k + strlen(global_f); /* int sendLen = 4 + hlen + 1 + handleLen + strlen ( readBuf ) ; */
global_g = (char *)calloc(b, sizeof(char)); /* sendBuf = ( char * ) calloc ( sendLen , sizeof ( char ) ) ; */
((uint16_t *)global_g )[0] = htons(b); /* ( ( uint16_t * ) sendBuf ) [ 0 ] = htons ( sendLen ) ; */
global_g[2] = 5; /* sendBuf [ 2 ] = 5 ; */
global_g[3] = global_j; /* sendBuf [ 3 ] = hlen ; */
memcpy(global_g + 4, global_h, global_j); /* memcpy ( sendBuf + 4 , handleBuf , hlen ) ; */
global_g[4 + global_j] = global_k; /* sendBuf [ 4 + hlen ] = handleLen ; */
memcpy(global_g + 5 + global_j, global_i, global_k); /* memcpy ( sendBuf + 5 + hlen , handle , handleLen ) ; */
memcpy(global_g + 5 + global_j + global_k, global_f, strlen(global_f)); /* memcpy ( sendBuf + 5 + hlen + handleLen , readBuf , strlen ( readBuf ) ) ; */
mySend(global_a, global_g, b, 0); /* mySend ( sock , sendBuf , sendLen , 0 ) ; */
free(global_g); /* free ( sendBuf ) ; */
free(global_f); /* free ( readBuf ) ; */
} else if (global_c[1] == 'l' || global_c[1] == 'L') /* if (commandBuf[1] == 'l' || commandBuf[1] == 'L') */ {
global_g = (char *)calloc(3, sizeof(char)); /* sendBuf = ( char * ) calloc ( 3 , sizeof ( char ) ) ; */
((uint16_t *)global_g )[0] = htons(3); /* ( ( uint16_t * ) sendBuf ) [ 0 ] = htons ( 3 ) ; */
global_g[2] = 10; /* sendBuf [ 2 ] = 10 ; */
mySend(global_a, global_g, 3, 0); /* mySend ( sock , sendBuf , 3 , 0 ) ; */
free(global_g); /* free ( sendBuf ) ; */
} else {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
}
return 0; /* return 0 */
}
int readIn() /* int readIn ( ) { */{
int a; /* int slen ; */
int b; /* int itr ; */
int c; /* int nHandles ; */
int d = myRecv(global_a, global_d, 3, MSG_DONTWAIT|MSG_PEEK); /* int exitCheck = myRecv ( sock , headBuf , 3 , MSG_DONTWAIT | MSG_PEEK ) ; */
if (d<0) /* if (exitCheck<0) */ {
perror("recv exit check"); /* perror ( "recv exit check" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (!d) /* if (!exitCheck) */ {
printf("Server Terminated\n"); /* printf ( "Server Terminated\n" ) ; */
free(global_i); /* free ( handle ) ; */
close(global_a); /* close ( sock ) ; */
return 0; /* return 0 */
} else {
if (myRecv(global_a, global_d, 3, MSG_WAITALL)<3) /* if (myRecv(sock, headBuf, 3, MSG_WAITALL)<3) */ {
perror("recv header"); /* perror ( "recv header" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
global_e = ntohs(*((uint16_t *)global_d )) - 3; /* bodySize = ntohs ( * ( ( uint16_t * ) headBuf ) ) - 3 ; */
if (global_e>0) /* if (bodySize>0) */ {
global_b = (uint8_t *)calloc(global_e, sizeof(uint8_t)); /* bodyBuf = ( uint8_t * ) calloc ( bodySize , sizeof ( uint8_t ) ) ; */
if (myRecv(global_a, global_b, global_e, MSG_WAITALL)<global_e) /* if (myRecv(sock, bodyBuf, bodySize, MSG_WAITALL)<bodySize) */ {
perror("recv body"); /* perror ( "recv body" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
switch (global_d[2]) /* switch (headBuf[2]) */ {
case 4: global_j = global_b[0]; /* hlen = bodyBuf [ 0 ] ; */
printf("\n%s:%s\n", global_h, global_f); /* printf ( "\n%s:%s\n" , handleBuf , readBuf ) ; */
memcpy(global_h, global_b + 1, global_j); /* memcpy ( handleBuf , bodyBuf + 1 , hlen ) ; */
free(global_h); /* free ( handleBuf ) ; */
memcpy(global_f, global_b + 1 + global_j, global_e - global_j - 1); /* memcpy ( readBuf , bodyBuf + 1 + hlen , bodySize - hlen - 1 ) ; */
global_h = (char *)calloc(global_j + 1, sizeof(char)); /* handleBuf = ( char * ) calloc ( hlen + 1 , sizeof ( char ) ) ; */
global_f = (char *)calloc(global_e - global_j, sizeof(char)); /* readBuf = ( char * ) calloc ( bodySize - hlen , sizeof ( char ) ) ; */
break;
case 5: a = global_b[0]; /* slen = bodyBuf [ 0 ] ; */
printf("\n%s:%s\n", global_h, global_f); /* printf ( "\n%s:%s\n" , handleBuf , readBuf ) ; */
memcpy(global_h, global_b + a + 2, global_j); /* memcpy ( handleBuf , bodyBuf + slen + 2 , hlen ) ; */
free(global_h); /* free ( handleBuf ) ; */
memcpy(global_f, global_b + a + 2 + global_j, global_e - global_j - 2 - a); /* memcpy ( readBuf , bodyBuf + slen + 2 + hlen , bodySize - hlen - 2 - slen ) ; */
global_j = global_b[a + 1]; /* hlen = bodyBuf [ slen + 1 ] ; */
global_f = (char *)calloc(global_e - global_j - a - 1, sizeof(char)); /* readBuf = ( char * ) calloc ( bodySize - hlen - slen - 1 , sizeof ( char ) ) ; */
global_h = (char *)calloc(global_j + 1, sizeof(char)); /* handleBuf = ( char * ) calloc ( hlen + 1 , sizeof ( char ) ) ; */
break;
case 7: global_j = global_b[0]; /* hlen = bodyBuf [ 0 ] ; */
printf("\nClient with handle %s does not exit.\n", global_h); /* printf ( "\nClient with handle %s does not exit.\n" , handleBuf ) ; */
memcpy(global_h, global_b + 1, global_j); /* memcpy ( handleBuf , bodyBuf + 1 , hlen ) ; */
global_h = (char *)calloc(global_j + 1, sizeof(char)); /* handleBuf = ( char * ) calloc ( hlen + 1 , sizeof ( char ) ) ; */
break;
case 9: printf("\n"); /* printf ( "\n" ) ; */
return 1; /* return 1 */
break;
case 11: c = ntohl(*((uint32_t *)global_b )); /* nHandles = ntohl ( * ( ( uint32_t * ) bodyBuf ) ) ; */
printf("\nNumber of clients: %d\n", c); /* printf ( "\nNumber of clients: %d\n" , nHandles ) ; */
for (b = 0; b<c; b++) {
if (myRecv(global_a, global_d, 3, MSG_WAITALL)<3) /* if (myRecv(sock, headBuf, 3, MSG_WAITALL)<3) */ {
perror("recv header"); /* perror ( "recv header" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
global_e = ntohs(*((uint16_t *)global_d )) - 3; /* bodySize = ntohs ( * ( ( uint16_t * ) headBuf ) ) - 3 ; */
global_b = (uint8_t *)calloc(global_e, sizeof(uint8_t)); /* bodyBuf = ( uint8_t * ) calloc ( bodySize , sizeof ( uint8_t ) ) ; */
if (myRecv(global_a, global_b, global_e, MSG_WAITALL)<global_e) /* if (myRecv(sock, bodyBuf, bodySize, MSG_WAITALL)<bodySize) */ {
perror("recv body"); /* perror ( "recv body" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
global_j = global_b[0]; /* hlen = bodyBuf [ 0 ] ; */
global_h = (char *)calloc(global_j + 1, sizeof(char)); /* handleBuf = ( char * ) calloc ( hlen + 1 , sizeof ( char ) ) ; */
memcpy(global_h, global_b + 1, global_j); /* memcpy ( handleBuf , bodyBuf + 1 , hlen ) ; */
printf("  %s\n", global_h); /* printf ( "  %s\n" , handleBuf ) ; */
}
break;
default: break;
}
}
return 0; /* return 0 */
}
int runClient() /* int runClient ( ) { */{
fd_set a; /* fd_set set ; */
fd_set b; /* fd_set staticSet ; */
FD_ZERO(&b); /* FD_ZERO ( & staticSet ) ; */
FD_SET(0, &b); /* FD_SET ( 0 , & staticSet ) ; */
FD_SET(global_a, &b); /* FD_SET ( sock , & staticSet ) ; */
while (1) /* while (1) */ {
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
memcpy(&a, &b, sizeof (b )); /* memcpy ( & set , & staticSet , sizeof ( staticSet ) ) ; */
mySelect(4, &a, ((void *)0 ), ((void *)0 ), ((void *)0 )); /* mySelect ( 4 , & set , ( ( void * ) 0 ) , ( ( void * ) 0 ) , ( ( void * ) 0 ) ) ; */
if (FD_ISSET(0, &a)) /* if (FD_ISSET(0, &set)) */ {
newCommand(); /* newCommand ( ) ; */
} else {
if (readIn()) /* if (readIn()) */ {
return 0; /* return 0 */
}
}
}
return 0; /* return 0 */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
if (a != 4) /* if (argc != 4) */ {
printf("usage: %s handle server-name server-port \n", b[0]); /* printf ( "usage: %s handle server-name server-port \n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
}
global_k = strlen(b[1]); /* handleLen = strlen ( argv [ 1 ] ) ; */
global_i = (uint8_t *)calloc(global_k + 1, sizeof(char)); /* handle = ( uint8_t * ) calloc ( handleLen + 1 , sizeof ( char ) ) ; */
memcpy(global_i, b[1], global_k); /* memcpy ( handle , argv [ 1 ] , handleLen ) ; */
global_a = tcpClientSetup(b[2], b[3]); /* sock = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
if (sendInit()) /* if (sendInit()) */ {
runClient(); /* runClient ( ) ; */
}
return 0; /* return 0 */
}
