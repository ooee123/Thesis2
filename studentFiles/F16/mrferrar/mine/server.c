fd_set global_b; /* fd_set set ; */
fd_set global_j; /* fd_set staticSet ; */
char **global_a; /* char * * handles ; */
char *global_g; /* char * handleBuf ; */
int global_c; /* int maxHandles ; */
int global_i; /* int curHandles ; */
int global_f; /* int bodySize ; */
int global_k; /* int server_socket ; */
uint8_t global_e[3]; /* uint8_t headBuf [ 3 ] ; */
uint8_t *global_d; /* uint8_t * bodyBuf ; */
uint8_t *global_h; /* uint8_t * sendBuf ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int handle1(int a) /* int handle1 ( int fd ) { */{
int b = 0; /* int found = 0 ; */
int c; /* int itr ; */
global_g = (char *)calloc(global_d[0] + 1, sizeof(uint8_t)); /* handleBuf = ( char * ) calloc ( bodyBuf [ 0 ] + 1 , sizeof ( uint8_t ) ) ; */
memcpy(global_g, global_d + 1, global_d[0]); /* memcpy ( handleBuf , bodyBuf + 1 , bodyBuf [ 0 ] ) ; */
for (c = 4; c<global_c; c++) {
if (global_a[c]&&!strcmp(global_a[c], global_g)) /* if (handles[itr]&&!strcmp(handles[itr], handleBuf)) */ {
b = 1; /* found = 1 ; */
printf("%s : %s\n", global_a[c], global_g); /* printf ( "%s : %s\n" , handles [ itr ] , handleBuf ) ; */
}
}
*((uint16_t *)global_e ) = htons(3); /* * ( ( uint16_t * ) headBuf ) = htons ( 3 ) ; */
if (b) /* if (found) */ {
global_e[2] = 3; /* headBuf [ 2 ] = 3 ; */
mySend(a, global_e, 3, 0); /* mySend ( fd , headBuf , 3 , 0 ) ; */
((void)(((&global_j )->__fds_bits )[((a ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((a ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( & staticSet ) -> __fds_bits ) [ ( ( fd ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( fd ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
free(global_g); /* free ( handleBuf ) ; */
close(a); /* close ( fd ) ; */
} else {
global_i++; /* curHandles ++ ; */
global_e[2] = 2; /* headBuf [ 2 ] = 2 ; */
global_a[a] = global_g; /* handles [ fd ] = handleBuf ; */
mySend(a, global_e, 3, 0); /* mySend ( fd , headBuf , 3 , 0 ) ; */
}
return 0; /* return 0 */
}
int handle4(int a) /* int handle4 ( int fd ) { */{
int b; /* int itr ; */
global_h = (uint8_t *)calloc(global_f + 3, sizeof(uint8_t)); /* sendBuf = ( uint8_t * ) calloc ( bodySize + 3 , sizeof ( uint8_t ) ) ; */
memcpy(global_h, global_e, 3); /* memcpy ( sendBuf , headBuf , 3 ) ; */
memcpy(global_h + 3, global_d, global_f); /* memcpy ( sendBuf + 3 , bodyBuf , bodySize ) ; */
for (b = 4; b<global_c; b++) {
if (global_a[b]&&b != a) /* if (handles[itr]&&itr != fd) */ {
mySend(b, global_h, global_f + 3, 0); /* mySend ( itr , sendBuf , bodySize + 3 , 0 ) ; */
}
}
free(global_h); /* free ( sendBuf ) ; */
return 0; /* return 0 */
}
int handle5(int a) /* int handle5 ( int fd ) { */{
int b; /* int itr ; */
global_g = (char *)calloc(global_d[0] + 1, sizeof(uint8_t)); /* handleBuf = ( char * ) calloc ( bodyBuf [ 0 ] + 1 , sizeof ( uint8_t ) ) ; */
memcpy(global_g, global_d + 1, global_d[0]); /* memcpy ( handleBuf , bodyBuf + 1 , bodyBuf [ 0 ] ) ; */
for (b = 4; b<global_c; b++) {
if (global_a[b]&&!strcmp(global_a[b], global_g)) /* if (handles[itr]&&!strcmp(handles[itr], handleBuf)) */ {
global_h = (uint8_t *)calloc(global_f + 3, sizeof(uint8_t)); /* sendBuf = ( uint8_t * ) calloc ( bodySize + 3 , sizeof ( uint8_t ) ) ; */
memcpy(global_h, global_e, 3); /* memcpy ( sendBuf , headBuf , 3 ) ; */
memcpy(global_h + 3, global_d, global_f); /* memcpy ( sendBuf + 3 , bodyBuf , bodySize ) ; */
mySend(b, global_h, global_f + 3, 0); /* mySend ( itr , sendBuf , bodySize + 3 , 0 ) ; */
free(global_h); /* free ( sendBuf ) ; */
return 0; /* return 0 */
}
}
global_h = (uint8_t *)calloc(4 + global_d[0], sizeof(uint8_t)); /* sendBuf = ( uint8_t * ) calloc ( 4 + bodyBuf [ 0 ] , sizeof ( uint8_t ) ) ; */
*((uint16_t *)global_h ) = htons(4 + global_d[0]); /* * ( ( uint16_t * ) sendBuf ) = htons ( 4 + bodyBuf [ 0 ] ) ; */
global_h[2] = 7; /* sendBuf [ 2 ] = 7 ; */
global_h[3] = global_d[0]; /* sendBuf [ 3 ] = bodyBuf [ 0 ] ; */
memcpy(global_h + 4, global_g, global_d[0]); /* memcpy ( sendBuf + 4 , handleBuf , bodyBuf [ 0 ] ) ; */
mySend(a, global_h, 4 + global_d[0], 0); /* mySend ( fd , sendBuf , 4 + bodyBuf [ 0 ] , 0 ) ; */
free(global_h); /* free ( sendBuf ) ; */
return 0; /* return 0 */
}
int handle8(int a) /* int handle8 ( int fd ) { */{
global_i--; /* curHandles -- ; */
*((uint16_t *)global_e ) = htons(3); /* * ( ( uint16_t * ) headBuf ) = htons ( 3 ) ; */
global_e[2] = 9; /* headBuf [ 2 ] = 9 ; */
mySend(a, global_e, 3, 0); /* mySend ( fd , headBuf , 3 , 0 ) ; */
free(global_a[a]); /* free ( handles [ fd ] ) ; */
global_a[a] = 0; /* handles [ fd ] = 0 ; */
((void)(((&global_j )->__fds_bits )[((a ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((a ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( & staticSet ) -> __fds_bits ) [ ( ( fd ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( fd ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
close(a); /* close ( fd ) ; */
return 0; /* return 0 */
}
int handle10(int a) /* int handle10 ( int fd ) { */{
int b; /* int itr ; */
global_h = (uint8_t *)calloc(7, sizeof(uint8_t)); /* sendBuf = ( uint8_t * ) calloc ( 7 , sizeof ( uint8_t ) ) ; */
*((uint16_t *)global_h ) = htons(7); /* * ( ( uint16_t * ) sendBuf ) = htons ( 7 ) ; */
global_h[2] = 11; /* sendBuf [ 2 ] = 11 ; */
*((uint32_t *)(global_h + 3 ) ) = htonl(global_i); /* * ( ( uint32_t * ) ( sendBuf + 3 ) ) = htonl ( curHandles ) ; */
mySend(a, global_h, 7, 0); /* mySend ( fd , sendBuf , 7 , 0 ) ; */
free(global_h); /* free ( sendBuf ) ; */
for (b = 4; b<global_c; b++) {
if (global_a[b]) /* if (handles[itr]) */ {
int c = 4 + strlen(global_a[b]); /* int sendLen = 4 + strlen ( handles [ itr ] ) ; */
global_h = (uint8_t *)calloc(c, sizeof(char)); /* sendBuf = ( uint8_t * ) calloc ( sendLen , sizeof ( char ) ) ; */
*((uint16_t *)global_h ) = htons(c); /* * ( ( uint16_t * ) sendBuf ) = htons ( sendLen ) ; */
global_h[2] = 12; /* sendBuf [ 2 ] = 12 ; */
global_h[3] = strlen(global_a[b]); /* sendBuf [ 3 ] = strlen ( handles [ itr ] ) ; */
memcpy(global_h + 4, global_a[b], strlen(global_a[b])); /* memcpy ( sendBuf + 4 , handles [ itr ] , strlen ( handles [ itr ] ) ) ; */
mySend(a, global_h, c, 0); /* mySend ( fd , sendBuf , sendLen , 0 ) ; */
free(global_h); /* free ( sendBuf ) ; */
}
}
return 0; /* return 0 */
}
int processSocket(int a) /* int processSocket ( int fd ) { */{
int b = myRecv(a, global_e, 3, MSG_DONTWAIT|MSG_PEEK); /* int exitCheck = myRecv ( fd , headBuf , 3 , MSG_DONTWAIT | MSG_PEEK ) ; */
if (b<0) /* if (exitCheck<0) */ {
perror("recv exit check"); /* perror ( "recv exit check" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else if (!b) /* if (!exitCheck) */ {
global_i--; /* curHandles -- ; */
free(global_a[a]); /* free ( handles [ fd ] ) ; */
global_a[a] = 0; /* handles [ fd ] = 0 ; */
((void)(((&global_j )->__fds_bits )[((a ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((a ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( & staticSet ) -> __fds_bits ) [ ( ( fd ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( fd ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
close(a); /* close ( fd ) ; */
} else {
if (myRecv(a, global_e, 3, MSG_WAITALL)<3) /* if (myRecv(fd, headBuf, 3, MSG_WAITALL)<3) */ {
perror("recv header"); /* perror ( "recv header" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
global_f = ntohs(*((uint16_t *)global_e )) - 3; /* bodySize = ntohs ( * ( ( uint16_t * ) headBuf ) ) - 3 ; */
if (global_f>0) /* if (bodySize>0) */ {
global_d = (uint8_t *)calloc(global_f, sizeof(uint8_t)); /* bodyBuf = ( uint8_t * ) calloc ( bodySize , sizeof ( uint8_t ) ) ; */
if (myRecv(a, global_d, global_f, MSG_WAITALL)<0) /* if (myRecv(fd, bodyBuf, bodySize, MSG_WAITALL)<0) */ {
perror("recv body"); /* perror ( "recv body" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
if (global_f>0) /* if (bodySize>0) */ {
free(global_d); /* free ( bodyBuf ) ; */
}
switch (global_e[2]) /* switch (headBuf[2]) */ {
case 1: handle1(a); /* handle1 ( fd ) ; */
break;
case 4: handle4(a); /* handle4 ( fd ) ; */
break;
case 5: handle5(a); /* handle5 ( fd ) ; */
break;
case 8: handle8(a); /* handle8 ( fd ) ; */
break;
case 10: handle10(a); /* handle10 ( fd ) ; */
break;
default: break;
}
}
return 0; /* return 0 */
}
int runServer() /* int runServer ( ) { */{
int a; /* int temp ; */
int b; /* int itr ; */
FD_ZERO(&global_j); /* FD_ZERO ( & staticSet ) ; */
FD_SET(global_k, &global_j); /* FD_SET ( server_socket , & staticSet ) ; */
while (1) /* while (1) */ {
memcpy(&global_b, &global_j, sizeof (global_j )); /* memcpy ( & set , & staticSet , sizeof ( staticSet ) ) ; */
mySelect(global_c + 1, &global_b, ((void *)0 ), ((void *)0 ), ((void *)0 )); /* mySelect ( maxHandles + 1 , & set , ( ( void * ) 0 ) , ( ( void * ) 0 ) , ( ( void * ) 0 ) ) ; */
if (FD_ISSET(global_k, &global_b)) /* if (FD_ISSET(server_socket, &set)) */ {
a = tcpAccept(global_k); /* temp = tcpAccept ( server_socket ) ; */
FD_SET(a, &global_j); /* FD_SET ( temp , & staticSet ) ; */
if (a == global_c) /* if (temp == maxHandles) */ {
global_c *= 2; /* maxHandles *= 2 ; */
global_a = (char **)realloc(global_a, global_c * sizeof(char *)); /* handles = ( char * * ) realloc ( handles , maxHandles * sizeof ( char * ) ) ; */
}
}
for (b = 4; b<global_c; b++) {
if (FD_ISSET(b, &global_b)) /* if (FD_ISSET(itr, &set)) */ {
processSocket(b); /* processSocket ( itr ) ; */
}
}
}
return 0; /* return 0 */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
global_i = 0; /* curHandles = 0 ; */
global_k = 0; /* server_socket = 0 ; */
global_c = 10; /* maxHandles = 10 ; */
global_k = tcpServerSetup(); /* server_socket = tcpServerSetup ( ) ; */
global_a = (char **)calloc(10, sizeof(char *)); /* handles = ( char * * ) calloc ( 10 , sizeof ( char * ) ) ; */
runServer(); /* runServer ( ) ; */
return 0; /* return 0 */
}
