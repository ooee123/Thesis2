typedef struct Client {
int socket;
char *handle;
struct Client *next;
} Client;
Client *global_a = ((void *)0 ); /* static Client * client_list = ( ( void * ) 0 ) ; */
fd_set global_b; /* static fd_set fds ; */
int global_c; /* static int num_fds ; */
int global_d; /* static int server_socket ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int tcpServerSetup(uint16_t a) /* int tcpServerSetup ( uint16_t port ) { */{
int b = 0; /* int server_socket = 0 ; */
struct sockaddr_in c; /* struct sockaddr_in local ; */
socklen_t d = sizeof (c ); /* socklen_t len = sizeof ( local ) ; */
b = socket(2, SOCK_STREAM, 0); /* server_socket = socket ( 2 , SOCK_STREAM , 0 ) ; */
c.sin_family = 2; /* local . sin_family = 2 ; */
c.sin_addr.s_addr = ((in_addr_t)0 ); /* local . sin_addr . s_addr = ( ( in_addr_t ) 0x00000000 ) ; */
if (b<0) /* if (server_socket<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(1); /* exit ( 1 ) ; */
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
if (listen(b, 5)<0) /* if (listen(server_socket, 5)<0) */ {
perror("listen call"); /* perror ( "listen call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("Server is using port %d \n", ntohs(c.sin_port)); /* printf ( "Server is using port %d \n" , ntohs ( local . sin_port ) ) ; */
return b; /* return server_socket */
}
int tcpAccept(int a) /* int tcpAccept ( int server_socket ) { */{
int b = 0; /* int client_socket = 0 ; */
Client *c = malloc(sizeof (c )); /* Client * client = malloc ( sizeof ( client ) ) ; */
if ((b = accept(a, ((void *)0 ), ((void *)0 )) )<0) /* if ((client_socket = accept(server_socket, ((void *)0 ), ((void *)0 )) )<0) */ {
perror("accept call"); /* perror ( "accept call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
c->socket = b; /* client -> socket = client_socket ; */
if (global_a == ((void *)0 )) /* if (client_list == ((void *)0 )) */ {
global_a = c; /* client_list = client ; */
} else {
Client *d = client_list; /* Client * cur = client_list ; */
while (d->next != ((void *)0 )) /* while (cur->next != ((void *)0 )) */ {

}
d->next = c; /* cur -> next = client ; */
}
return (b ); /* return (client_socket ) */
}
void removeClient(int a) /* void removeClient ( int socket ) { */{
Client *b; /* Client * tmp ; */
if (global_a == ((void *)0 )) /* if (client_list == ((void *)0 )) */ {
return; /* return */
}
if (global_a->socket == a) /* if (client_list->socket == socket) */ {
b = global_a; /* tmp = client_list ; */
global_a = global_a->next; /* client_list = client_list -> next ; */
} else {
Client *c = client_list; /* Client * cur = client_list ; */
while (c->next != ((void *)0 )&&c->next->socket != a) /* while (cur->next != ((void *)0 )&&cur->next->socket != socket) */ {

}
if (c->next != ((void *)0 )) /* if (cur->next != ((void *)0 )) */ {
b = c->next; /* tmp = cur -> next ; */
c->next = c->next->next; /* cur -> next = cur -> next -> next ; */
}
}
free(b->handle); /* free ( tmp -> handle ) ; */
free(b); /* free ( tmp ) ; */
((void)(((&global_b )->__fds_bits )[((a ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((a ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( & fds ) -> __fds_bits ) [ ( ( socket ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( socket ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
close(a); /* close ( socket ) ; */
}
int isHandleUsed(int a, char *b) /* int isHandleUsed ( int socket , char * handle ) { */{
Client *c = client_list; /* Client * cur = client_list ; */
while (c != ((void *)0 )) /* while (cur != ((void *)0 )) */ {
if (c->socket != a&&strcmp(c->handle, b) == 0) /* if (cur->socket != socket&&strcmp(cur->handle, handle) == 0) */ {
return 1; /* return 1 */
}
c = c->next; /* cur = cur -> next ; */
}
return 0; /* return 0 */
}
void addClient(int a, int b) /* void addClient ( int socket , int len ) { */{
Header c; /* Header header ; */
uint8_t d[len]; /* uint8_t buffer [ len ] ; */
Client *e = client_list; /* Client * cur = client_list ; */
while (e->socket != a) /* while (cur->socket != socket) */ {

}
if (myRecv(a, d, b, 0)<0) /* if (myRecv(socket, buffer, len, 0)<0) */ {
perror("recv addClient"); /* perror ( "recv addClient" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
uint8_t f = *d; /* uint8_t handle_len = * buffer ; */
char *g = calloc(1, f + 1); /* char * handle = calloc ( 1 , handle_len + 1 ) ; */
memmove(g, d + 1, f); /* memmove ( handle , buffer + 1 , handle_len ) ; */
c.len = htons(3); /* header . len = htons ( 3 ) ; */
if (isHandleUsed(a, g)) /* if (isHandleUsed(socket, handle)) */ {
c.flag = 3; /* header . flag = 3 ; */
} else {
c.flag = 2; /* header . flag = 2 ; */
e->handle = g; /* cur -> handle = handle ; */
}
if (mySend(a, &c, 3, 0)<0) /* if (mySend(socket, &header, 3, 0)<0) */ {
perror("send addClient"); /* perror ( "send addClient" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (c.flag == 3) /* if (header.flag == 3) */ {
removeClient(a); /* removeClient ( socket ) ; */
}
}
void broadcast(int a, int b) /* void broadcast ( int socket , int len ) { */{
char c[3 + len]; /* char buffer [ 3 + len ] ; */
Client *d = client_list; /* Client * cur = client_list ; */
*((uint16_t *)c ) = htons(3 + b); /* * ( ( uint16_t * ) buffer ) = htons ( 3 + len ) ; */
*((uint8_t *)c + 2 ) = 4; /* * ( ( uint8_t * ) buffer + 2 ) = 4 ; */
if (myRecv(a, c + 3, b, 0)<0) /* if (myRecv(socket, buffer + 3, len, 0)<0) */ {
perror("recv broadcast"); /* perror ( "recv broadcast" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
while (d != ((void *)0 )) /* while (cur != ((void *)0 )) */ {
if (d->socket != a) /* if (cur->socket != socket) */ {
if (mySend(d->socket, c, 3 + b, 0)<0) /* if (mySend(cur->socket, buffer, 3 + len, 0)<0) */ {
perror("send broadcast"); /* perror ( "send broadcast" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
d = d->next; /* cur = cur -> next ; */
}
}
void message(int a, int b) /* void message ( int socket , int len ) { */{
char c[3 + handle_len + 1]; /* char buffer2 [ 3 + handle_len + 1 ] ; */
char d[3 + len]; /* char buffer [ 3 + len ] ; */
char e[handle_len + 1]; /* char dest_handle [ handle_len + 1 ] ; */
Client *f = client_list; /* Client * cur = client_list ; */
*((uint16_t *)d ) = htons(3 + b); /* * ( ( uint16_t * ) buffer ) = htons ( 3 + len ) ; */
*((uint8_t *)d + 2 ) = 5; /* * ( ( uint8_t * ) buffer + 2 ) = 5 ; */
if (myRecv(a, d + 3, b, 0)<0) /* if (myRecv(socket, buffer + 3, len, 0)<0) */ {
perror("recv message"); /* perror ( "recv message" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
uint8_t g = *((uint8_t *)d + 3 ); /* uint8_t handle_len = * ( ( uint8_t * ) buffer + 3 ) ; */
memmove(e, d + 3 + 1, g); /* memmove ( dest_handle , buffer + 3 + 1 , handle_len ) ; */
e[g] = '\0'; /* dest_handle [ handle_len ] = '\0' ; */
while (f != ((void *)0 )) /* while (cur != ((void *)0 )) */ {
if (strcmp(f->handle, e) == 0) /* if (strcmp(cur->handle, dest_handle) == 0) */ {
if (mySend(f->socket, d, 3 + b, 0)<0) /* if (mySend(cur->socket, buffer, 3 + len, 0)<0) */ {
perror("send message"); /* perror ( "send message" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return; /* return */
}
f = f->next; /* cur = cur -> next ; */
}
*((uint16_t *)c ) = htons(3 + g + 1); /* * ( ( uint16_t * ) buffer2 ) = htons ( 3 + handle_len + 1 ) ; */
*((uint8_t *)c + 2 ) = 7; /* * ( ( uint8_t * ) buffer2 + 2 ) = 7 ; */
memmove(c + 3, d + 3, g + 1); /* memmove ( buffer2 + 3 , buffer + 3 , handle_len + 1 ) ; */
if (mySend(a, c, 3 + g + 1, 0)<0) /* if (mySend(socket, buffer2, 3 + handle_len + 1, 0)<0) */ {
perror("send handle does not exist"); /* perror ( "send handle does not exist" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void exitClient(int a) /* void exitClient ( int socket ) { */{
Header b; /* Header header ; */
b.flag = 9; /* header . flag = 9 ; */
b.len = htons(3); /* header . len = htons ( 3 ) ; */
if (mySend(a, &b, 3, 0)<0) /* if (mySend(socket, &header, 3, 0)<0) */ {
perror("send exitClient"); /* perror ( "send exitClient" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
removeClient(a); /* removeClient ( socket ) ; */
}
void handles(int a) /* void handles ( int socket ) { */{
char b[3 + sizeof(uint32_t)]; /* char buffer [ 3 + sizeof ( uint32_t ) ] ; */
uint32_t c = 0; /* uint32_t num_handles = 0 ; */
char d[3 + 1 + 255]; /* char buffer2 [ 3 + 1 + 255 ] ; */
Client *e = client_list; /* Client * cur = client_list ; */
while (e != ((void *)0 )) /* while (cur != ((void *)0 )) */ {
c++; /* num_handles ++ ; */
e = e->next; /* cur = cur -> next ; */
}
c = htonl(c); /* num_handles = htonl ( num_handles ) ; */
*((uint16_t *)b ) = htons(3 + sizeof(uint32_t)); /* * ( ( uint16_t * ) buffer ) = htons ( 3 + sizeof ( uint32_t ) ) ; */
*((uint8_t *)b + 2 ) = 11; /* * ( ( uint8_t * ) buffer + 2 ) = 11 ; */
memmove(b + 3, &c, sizeof(uint32_t)); /* memmove ( buffer + 3 , & num_handles , sizeof ( uint32_t ) ) ; */
if (mySend(a, b, 3 + sizeof(uint32_t), 0)<0) /* if (mySend(socket, buffer, 3 + sizeof(uint32_t), 0)<0) */ {
perror("send ackHandles"); /* perror ( "send ackHandles" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
e = global_a; /* cur = client_list ; */
while (e != ((void *)0 )) /* while (cur != ((void *)0 )) */ {
int f = strlen(e->handle); /* int handle_len = strlen ( cur -> handle ) ; */
*((uint16_t *)d ) = htons(3 + 1 + f); /* * ( ( uint16_t * ) buffer2 ) = htons ( 3 + 1 + handle_len ) ; */
*((uint8_t *)d + 2 ) = 12; /* * ( ( uint8_t * ) buffer2 + 2 ) = 12 ; */
*((uint8_t *)d + 3 ) = f; /* * ( ( uint8_t * ) buffer2 + 3 ) = handle_len ; */
memmove(d + 3 + 1, e->handle, f); /* memmove ( buffer2 + 3 + 1 , cur -> handle , handle_len ) ; */
if (mySend(a, d, 3 + 1 + f, 0)<0) /* if (mySend(socket, buffer2, 3 + 1 + handle_len, 0)<0) */ {
perror("send singleHandle"); /* perror ( "send singleHandle" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
e = e->next; /* cur = cur -> next ; */
}
}
void processMessage(int a) /* void processMessage ( int socket ) { */{
Header b; /* Header header ; */
int c; /* int bytesRecv ; */
if ((c = myRecv(a, &b, 3, 0) )<0) /* if ((bytesRecv = myRecv(socket, &header, 3, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (c == 0) /* if (bytesRecv == 0) */ {
removeClient(a); /* removeClient ( socket ) ; */
} else if (b.flag == 1) /* if (header.flag == 1) */ {
addClient(a, ntohs(b.len) - 3); /* addClient ( socket , ntohs ( header . len ) - 3 ) ; */
} else if (b.flag == 4) /* if (header.flag == 4) */ {
broadcast(a, ntohs(b.len) - 3); /* broadcast ( socket , ntohs ( header . len ) - 3 ) ; */
} else if (b.flag == 5) /* if (header.flag == 5) */ {
message(a, b.len - 3); /* message ( socket , header . len - 3 ) ; */
} else if (b.flag == 8) /* if (header.flag == 8) */ {
exitClient(a); /* exitClient ( socket ) ; */
} else if (b.flag == 10) /* if (header.flag == 10) */ {
handles(a); /* handles ( socket ) ; */
}
}
void processInput() /* void processInput ( ) { */{
int a; /* int i ; */
fd_set b; /* fd_set temp_fds ; */
memmove(&b, &global_b, sizeof(fd_set)); /* memmove ( & temp_fds , & fds , sizeof ( fd_set ) ) ; */
if (mySelect(global_c + 1, &b, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (mySelect(num_fds + 1, &temp_fds, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select call"); /* perror ( "select call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
for (a = global_d; a<=global_c; ++a) {
if (FD_ISSET(a, &b)) /* if (FD_ISSET(i, &temp_fds)) */ {
if (a == global_d) /* if (i == server_socket) */ {
int c = tcpAccept(global_d); /* int new_client = tcpAccept ( server_socket ) ; */
global_c++; /* num_fds ++ ; */
FD_SET(c, &global_b); /* FD_SET ( new_client , & fds ) ; */
} else {
processMessage(a); /* processMessage ( i ) ; */
}
}
}
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
if (a != 1&&a != 2) /* if (argc != 1&&argc != 2) */ {
perror("Incorrect number of args.\n"); /* perror ( "Incorrect number of args.\n" ) ; */
}
global_d = tcpServerSetup(a == 2 ? atoi(b[1]) : 0); /* server_socket = tcpServerSetup ( argc == 2 ? atoi ( argv [ 1 ] ) : 0 ) ; */
global_c = global_d; /* num_fds = server_socket ; */
FD_ZERO(&global_b); /* FD_ZERO ( & fds ) ; */
FD_SET(global_d, &global_b); /* FD_SET ( server_socket , & fds ) ; */
while (1) /* while (1) */ {
processInput(); /* processInput ( ) ; */
}
close(global_d); /* close ( server_socket ) ; */
return 0; /* return 0 */
}
