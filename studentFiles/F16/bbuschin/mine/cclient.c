int global_d; /* static int server_socket ; */
uint8_t global_b; /* static uint8_t handle_len ; */
char *global_c; /* static char * handle ; */
int global_a = 0; /* static int num_handles = 0 ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void checkArgs(int a, char **b) /* void checkArgs ( int argc , char * * argv ) { */{
if (a != 4) /* if (argc != 4) */ {
printf("usage: %s handle host-name port-number \n", b[0]); /* printf ( "usage: %s handle host-name port-number \n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
} else if (strlen(b[1])>255) /* if (strlen(argv[1])>255) */ {
perror("Handle must be <= 255 characters\n"); /* perror ( "Handle must be <= 255 characters\n" ) ; */
exit(1); /* exit ( 1 ) ; */
}
global_c = b[1]; /* handle = argv [ 1 ] ; */
global_b = strlen(global_c); /* handle_len = strlen ( handle ) ; */
}
int tcpClientSetup(char *a, char *b) /* int tcpClientSetup ( char * host_name , char * port ) { */{
struct hostent *c; /* struct hostent * hp ; */
int d; /* int socket_num ; */
struct sockaddr_in e; /* struct sockaddr_in remote ; */
e.sin_family = 2; /* remote . sin_family = 2 ; */
if ((d = socket(2, SOCK_STREAM, 0) )<0) /* if ((socket_num = socket(2, SOCK_STREAM, 0) )<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((c = gethostbyname(a) ) == ((void *)0 )) /* if ((hp = gethostbyname(host_name) ) == ((void *)0 )) */ {
printf("Error getting hostname: %s\n", a); /* printf ( "Error getting hostname: %s\n" , host_name ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy((char *)&e.sin_addr, (char *)c->h_addr_list[0], c->h_length); /* memcpy ( ( char * ) & remote . sin_addr , ( char * ) hp -> h_addr_list [ 0 ] , hp -> h_length ) ; */
e.sin_port = htons(atoi(b)); /* remote . sin_port = htons ( atoi ( port ) ) ; */
if (connect(d, (struct sockaddr *)&e, sizeof(struct sockaddr_in))<0) /* if (connect(socket_num, (struct sockaddr *)&remote, sizeof(struct sockaddr_in))<0) */ {
perror("connect call"); /* perror ( "connect call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return d; /* return socket_num */
}
void initClient() /* void initClient ( ) { */{
Header a; /* Header header ; */
int b = 3 + global_b + 1; /* int packet_len = 3 + handle_len + 1 ; */
char *c = malloc(b); /* char * buffer = malloc ( packet_len ) ; */
*((uint16_t *)c ) = b; /* * ( ( uint16_t * ) buffer ) = packet_len ; */
*((uint8_t *)c + 2 ) = 1; /* * ( ( uint8_t * ) buffer + 2 ) = 1 ; */
*((uint8_t *)c + 3 ) = global_b; /* * ( ( uint8_t * ) buffer + 3 ) = handle_len ; */
memmove(c + 4, global_c, global_b); /* memmove ( buffer + 4 , handle , handle_len ) ; */
if (mySend(global_d, c, b, 0)<0) /* if (mySend(server_socket, buffer, packet_len, 0)<0) */ {
perror("send initClient"); /* perror ( "send initClient" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (myRecv(global_d, &a, 3, 0)<0) /* if (myRecv(server_socket, &header, 3, 0)<0) */ {
perror("recv initClient"); /* perror ( "recv initClient" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (a.flag == 3) /* if (header.flag == 3) */ {
printf("Handle already in use: %s\n", global_c); /* printf ( "Handle already in use: %s\n" , handle ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(c); /* free ( buffer ) ; */
}
void recvMessage(int a) /* void recvMessage ( int len ) { */{
char *b; /* char * msg ; */
uint8_t c; /* uint8_t src_handle_len ; */
char d[len]; /* char buffer [ len ] ; */
uint8_t e; /* uint8_t dst_handle_len ; */
char *f; /* char * src_handle ; */
if (myRecv(global_d, d, a, 0)<0) /* if (myRecv(server_socket, buffer, len, 0)<0) */ {
perror("recv recvMessage"); /* perror ( "recv recvMessage" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
e = *((uint8_t *)d ); /* dst_handle_len = * ( ( uint8_t * ) buffer ) ; */
c = *((uint8_t *)d + 1 + e ); /* src_handle_len = * ( ( uint8_t * ) buffer + 1 + dst_handle_len ) ; */
f = calloc(1, c + 1); /* src_handle = calloc ( 1 , src_handle_len + 1 ) ; */
memmove(f, d + 2 + e, c); /* memmove ( src_handle , buffer + 2 + dst_handle_len , src_handle_len ) ; */
b = malloc(a - c - e - 2); /* msg = malloc ( len - src_handle_len - dst_handle_len - 2 ) ; */
memmove(b, d + 2 + c + e, a - c - e - 2); /* memmove ( msg , buffer + 2 + src_handle_len + dst_handle_len , len - src_handle_len - dst_handle_len - 2 ) ; */
printf("\n%s: %s\n", f, b); /* printf ( "\n%s: %s\n" , src_handle , msg ) ; */
free(f); /* free ( src_handle ) ; */
free(b); /* free ( msg ) ; */
}
void recvBroadcast(int a) /* void recvBroadcast ( int len ) { */{
char *b; /* char * msg ; */
char *c; /* char * handle ; */
char d[len]; /* char buffer [ len ] ; */
uint8_t e; /* uint8_t handle_len ; */
if (myRecv(global_d, d, a, 0)<0) /* if (myRecv(server_socket, buffer, len, 0)<0) */ {
perror("recv recvMessage"); /* perror ( "recv recvMessage" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
e = *((uint8_t *)d ); /* handle_len = * ( ( uint8_t * ) buffer ) ; */
c = calloc(1, e + 1); /* handle = calloc ( 1 , handle_len + 1 ) ; */
memmove(c, d + 1, e); /* memmove ( handle , buffer + 1 , handle_len ) ; */
b = malloc(a - e - 1); /* msg = malloc ( len - handle_len - 1 ) ; */
memmove(b, d + 1 + e, a - e - 1); /* memmove ( msg , buffer + 1 + handle_len , len - handle_len - 1 ) ; */
printf("\n%s: %s\n", c, b); /* printf ( "\n%s: %s\n" , handle , msg ) ; */
free(c); /* free ( handle ) ; */
free(b); /* free ( msg ) ; */
}
void recvHandles(int a) /* void recvHandles ( int len ) { */{
char b[len]; /* char buffer [ len ] ; */
if (myRecv(global_d, b, a, 0)<0) /* if (myRecv(server_socket, buffer, len, 0)<0) */ {
perror("recv recvHandles"); /* perror ( "recv recvHandles" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
global_a = ntohl(*((uint32_t *)b )); /* num_handles = ntohl ( * ( ( uint32_t * ) buffer ) ) ; */
printf("\nNumber of clients: %d\n", global_a); /* printf ( "\nNumber of clients: %d\n" , num_handles ) ; */
}
void recvHandle(int a) /* void recvHandle ( int len ) { */{
char b[len]; /* char buffer [ len ] ; */
char c[handle_len + 1]; /* char h_buf [ handle_len + 1 ] ; */
global_a--; /* num_handles -- ; */
if (myRecv(global_d, b, a, 0)<0) /* if (myRecv(server_socket, buffer, len, 0)<0) */ {
perror("recv recvHandle"); /* perror ( "recv recvHandle" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
int d = *((uint8_t *)b ); /* int handle_len = * ( ( uint8_t * ) buffer ) ; */
c[d] = '\0'; /* h_buf [ handle_len ] = '\0' ; */
memmove(c, b + 1, d); /* memmove ( h_buf , buffer + 1 , handle_len ) ; */
printf("\t%s\n", c); /* printf ( "\t%s\n" , h_buf ) ; */
}
void recvHandleDNE(int a) /* void recvHandleDNE ( int len ) { */{
char b[len]; /* char buffer [ len ] ; */
char c[handle_len + 1]; /* char h_buf [ handle_len + 1 ] ; */
if (myRecv(global_d, b, a, 0)<0) /* if (myRecv(server_socket, buffer, len, 0)<0) */ {
perror("recv recvHandle"); /* perror ( "recv recvHandle" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
int d = *((uint8_t *)b ); /* int handle_len = * ( ( uint8_t * ) buffer ) ; */
c[d] = '\0'; /* h_buf [ handle_len ] = '\0' ; */
memmove(c, b + 1, d); /* memmove ( h_buf , buffer + 1 , handle_len ) ; */
printf("Client with handle %s does not exist.\n", c); /* printf ( "Client with handle %s does not exist.\n" , h_buf ) ; */
}
void processServer() /* void processServer ( ) { */{
Header a; /* Header header ; */
int b; /* int bytesRecv ; */
if ((b = myRecv(global_d, &a, 3, 0) )<0) /* if ((bytesRecv = myRecv(server_socket, &header, 3, 0) )<0) */ {
perror("recv processServer"); /* perror ( "recv processServer" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (b == 0) /* if (bytesRecv == 0) */ {
printf("\nServer Terminated\n"); /* printf ( "\nServer Terminated\n" ) ; */
exit(0); /* exit ( 0 ) ; */
}
int c = ntohs(a.len); /* int len = ntohs ( header . len ) ; */
if (a.flag == 4) /* if (header.flag == 4) */ {
recvBroadcast(c - 3); /* recvBroadcast ( len - 3 ) ; */
} else if (a.flag == 5) /* if (header.flag == 5) */ {
recvMessage(c - 3); /* recvMessage ( len - 3 ) ; */
} else if (a.flag == 9) /* if (header.flag == 9) */ {
close(global_d); /* close ( server_socket ) ; */
exit(0); /* exit ( 0 ) ; */
} else if (a.flag == 11) /* if (header.flag == 11) */ {
recvHandles(c - 3); /* recvHandles ( len - 3 ) ; */
} else if (a.flag == 12) /* if (header.flag == 12) */ {
recvHandle(c - 3); /* recvHandle ( len - 3 ) ; */
} else if (a.flag == 7) /* if (header.flag == 7) */ {
recvHandleDNE(c - 3); /* recvHandleDNE ( len - 3 ) ; */
} else {
printf("Unknown flag: %d\n", a.flag); /* printf ( "Unknown flag: %d\n" , header . flag ) ; */
}
}
void sendMessage(int a, char *b) /* void sendMessage ( int len , char * message ) { */{
char c[256]; /* char dest [ 256 ] ; */
char d[3 + 2 + handle_len + dst_handle_len + 1001]; /* char buffer [ 3 + 2 + handle_len + dst_handle_len + 1001 ] ; */
sscanf(b, "%s ", c); /* sscanf ( message , "%s " , dest ) ; */
int e = strlen(c); /* int dst_handle_len = strlen ( dest ) ; */
b += e + 1; /* message += dst_handle_len + 1 ; */
a -= e + 1; /* len -= dst_handle_len + 1 ; */
*((uint8_t *)d + 2 ) = 5; /* * ( ( uint8_t * ) buffer + 2 ) = 5 ; */
*((uint8_t *)d + 3 ) = e; /* * ( ( uint8_t * ) buffer + 3 ) = dst_handle_len ; */
memmove(d + 3 + 1, c, e); /* memmove ( buffer + 3 + 1 , dest , dst_handle_len ) ; */
*((uint8_t *)d + 3 + e + 1 ) = global_b; /* * ( ( uint8_t * ) buffer + 3 + dst_handle_len + 1 ) = handle_len ; */
memmove(d + 3 + e + 2, global_c, global_b); /* memmove ( buffer + 3 + dst_handle_len + 2 , handle , handle_len ) ; */
while (a>0) /* while (len>0) */ {
int f = a>1000 ? 1001 : a + 1; /* int msg_len = len > 1000 ? 1001 : len + 1 ; */
a -= 1000; /* len -= 1000 ; */
b += 1000; /* message += 1000 ; */
*((uint16_t *)d ) = htons(3 + 2 + global_b + e + f); /* * ( ( uint16_t * ) buffer ) = htons ( 3 + 2 + handle_len + dst_handle_len + msg_len ) ; */
memmove(d + 3 + 2 + global_b + e, b, f); /* memmove ( buffer + 3 + 2 + handle_len + dst_handle_len , message , msg_len ) ; */
d[3 + 2 + global_b + e + f + 1] = '\0'; /* buffer [ 3 + 2 + handle_len + dst_handle_len + msg_len + 1 ] = '\0' ; */
if (mySend(global_d, d, 3 + 2 + global_b + e + f + 1, 0)<0) /* if (mySend(server_socket, buffer, 3 + 2 + handle_len + dst_handle_len + msg_len + 1, 0)<0) */ {
perror("send sendMessage\n"); /* perror ( "send sendMessage\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
}
void sendBroadcast(int a, char *b) /* void sendBroadcast ( int len , char * message ) { */{
char c[packet_len]; /* char buffer [ packet_len ] ; */
int d = 3 + 1 + global_b + a; /* int packet_len = 3 + 1 + handle_len + len ; */
*((uint16_t *)c ) = htons(d); /* * ( ( uint16_t * ) buffer ) = htons ( packet_len ) ; */
*((uint8_t *)c + 2 ) = 4; /* * ( ( uint8_t * ) buffer + 2 ) = 4 ; */
*((uint8_t *)c + 3 ) = global_b; /* * ( ( uint8_t * ) buffer + 3 ) = handle_len ; */
memmove(c + 3 + 1, global_c, global_b); /* memmove ( buffer + 3 + 1 , handle , handle_len ) ; */
while (a>0) /* while (len>0) */ {
int e = a>1000 ? 1000 : a; /* int msg_len = len > 1000 ? 1000 : len ; */
memmove(c + 3 + 1 + global_b, b, e); /* memmove ( buffer + 3 + 1 + handle_len , message , msg_len ) ; */
a -= 1000; /* len -= 1000 ; */
b += 1000; /* message += 1000 ; */
if (mySend(global_d, c, 3 + 1 + global_b + e, 0)<0) /* if (mySend(server_socket, buffer, 3 + 1 + handle_len + msg_len, 0)<0) */ {
perror("send broadcast"); /* perror ( "send broadcast" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
}
void sendHandles() /* void sendHandles ( ) { */{
Header a; /* Header header ; */
a.flag = 10; /* header . flag = 10 ; */
a.len = htons(3); /* header . len = htons ( 3 ) ; */
if (mySend(global_d, &a, 3, 0)<0) /* if (mySend(server_socket, &header, 3, 0)<0) */ {
perror("send sendHandles"); /* perror ( "send sendHandles" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void sendExit() /* void sendExit ( ) { */{
Header a; /* Header header ; */
a.flag = 8; /* header . flag = 8 ; */
a.len = htons(3); /* header . len = htons ( 3 ) ; */
if (mySend(global_d, &a, 3, 0)<0) /* if (mySend(server_socket, &header, 3, 0)<0) */ {
perror("send sendExit"); /* perror ( "send sendExit" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void processUser() /* void processUser ( ) { */{
size_t a; /* size_t size ; */
char *b = ((void *)0 ); /* char * line = ( ( void * ) 0 ) ; */
if (getline(&b, &a, stdin)<0) /* if (getline(&line, &size, stdin)<0) */ {
perror("getline"); /* perror ( "getline" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
int c = strlen(b) + 1; /* int len = strlen ( line ) + 1 ; */
if ((strncmp("%M", (b ), 2) == 0 || strncmp("%m", (b ), 2) == 0 )) /* if ((strncmp("%M", (line ), 2) == 0 || strncmp("%m", (line ), 2) == 0 )) */ {
sendMessage(c - 3, b + 3); /* sendMessage ( len - 3 , line + 3 ) ; */
} else if ((strncmp("%B", (b ), 2) == 0 || strncmp("%b", (b ), 2) == 0 )) /* if ((strncmp("%B", (line ), 2) == 0 || strncmp("%b", (line ), 2) == 0 )) */ {
sendBroadcast(c - 3, b + 3); /* sendBroadcast ( len - 3 , line + 3 ) ; */
} else if ((strncmp("%L", (b ), 2) == 0 || strncmp("%l", (b ), 2) == 0 )) /* if ((strncmp("%L", (line ), 2) == 0 || strncmp("%l", (line ), 2) == 0 )) */ {
sendHandles(); /* sendHandles ( ) ; */
} else if ((strncmp("%E", (b ), 2) == 0 || strncmp("%e", (b ), 2) == 0 )) /* if ((strncmp("%E", (line ), 2) == 0 || strncmp("%e", (line ), 2) == 0 )) */ {
sendExit(); /* sendExit ( ) ; */
} else {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
}
free(b); /* free ( line ) ; */
}
void processInput() /* void processInput ( ) { */{
fd_set a; /* fd_set fds ; */
while (1) /* while (1) */ {
if (global_a == 0) /* if (num_handles == 0) */ {
printf("$"); /* printf ( "$" ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
FD_ZERO(&a); /* FD_ZERO ( & fds ) ; */
FD_SET(0, &a); /* FD_SET ( 0 , & fds ) ; */
FD_SET(global_d, &a); /* FD_SET ( server_socket , & fds ) ; */
if (mySelect(global_d + 1, &a, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (mySelect(server_socket + 1, &fds, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select call"); /* perror ( "select call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (FD_ISSET(global_d, &a)) /* if (FD_ISSET(server_socket, &fds)) */ {
processServer(); /* processServer ( ) ; */
}
if (FD_ISSET(0, &a)) /* if (FD_ISSET(0, &fds)) */ {
processUser(); /* processUser ( ) ; */
}
}
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
processInput(); /* processInput ( ) ; */
checkArgs(a, b); /* checkArgs ( argc , argv ) ; */
global_d = tcpClientSetup(b[2], b[3]); /* server_socket = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
initClient(); /* initClient ( ) ; */
return 0; /* return 0 */
}
