void startUp(int socket_fd); /* void startUp(int socket_fd) */
void printPrompt(void); /* void printPrompt(void) */
void initPacket(char *client_handle, int socket_fd); /* void initPacket(char *client_handle, int socket_fd) */
void promptUser(int socket_fd); /* void promptUser(int socket_fd) */
void message(char *input, int socket_fd, int send_len); /* void message(char *input, int socket_fd, int send_len) */
void broadcast(char *input, int socket_fd, int send_len); /* void broadcast(char *input, int socket_fd, int send_len) */
void handleList(char *input, int socket_fd, int send_len); /* void handleList(char *input, int socket_fd, int send_len) */
void exitClient(char *input, int socket_fd, int send_len); /* void exitClient(char *input, int socket_fd, int send_len) */
void transmitCheck(int socket_fd); /* void transmitCheck(int socket_fd) */
void receivingMessage(char *packet); /* void receivingMessage(char *packet) */
void errorMessage(char *packet); /* void errorMessage(char *packet) */
void receiveNumHandles(char *packet, int socket_fd); /* void receiveNumHandles(char *packet, int socket_fd) */
void printHandles(char *packet, int socket_fd); /* void printHandles(char *packet, int socket_fd) */
void terminateClient(char *packet); /* void terminateClient(char *packet) */
char *global_c; /* char * global_handle ; */
int global_b = 3; /* int max_socket = 3 ; */
int global_e = 0; /* int list_flag = 0 ; */
int global_a = 0; /* int actual_clients = 0 ; */
int global_d = 0; /* int count_clients = 0 ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
int c = 0; /* int socket_fd = 0 ; */
if (a != 4) /* if (argc != 4) */ {
fprintf(stderr, "Error - must include IP address, port number, and client handle name\n"); /* fprintf ( stderr , "Error - must include IP address, port number, and client handle name\n" ) ; */
exit(1); /* exit ( 1 ) ; */
}
c = tcpClientSetup(b[2], b[3]); /* socket_fd = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
initPacket(b[1], c); /* initPacket ( argv [ 1 ] , socket_fd ) ; */
startUp(c); /* startUp ( socket_fd ) ; */
close(c); /* close ( socket_fd ) ; */
return 0; /* return 0 */
}
void startUp(int a) /* void startUp ( int socket_fd ) { */{
int b = a; /* int max_socket = socket_fd ; */
fd_set c; /* fd_set fdvar ; */
FD_ZERO(&c); /* FD_ZERO ( & fdvar ) ; */
FD_SET(a, &c); /* FD_SET ( socket_fd , & fdvar ) ; */
while (1) /* while (1) */ {
if (!global_e) /* if (!list_flag) */ {
printPrompt(); /* printPrompt ( ) ; */
}
while (1) /* while (1) */ {
FD_ZERO(&c); /* FD_ZERO ( & fdvar ) ; */
FD_SET(a, &c); /* FD_SET ( socket_fd , & fdvar ) ; */
FD_SET(0, &c); /* FD_SET ( 0 , & fdvar ) ; */
if (mySelect(b + 1, (fd_set *)&c, (fd_set *)0, (fd_set *)0, ((void *)0 ))>0) /* if (mySelect(max_socket + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, ((void *)0 ))>0) */ {
if (FD_ISSET(a, &c)) /* if (FD_ISSET(socket_fd, &fdvar)) */ {
transmitCheck(a); /* transmitCheck ( socket_fd ) ; */
}
if (!global_e) /* if (!list_flag) */ {
if (FD_ISSET(0, &c)) /* if (FD_ISSET(0, &fdvar)) */ {
promptUser(a); /* promptUser ( socket_fd ) ; */
}
}
break;
}
break;
}
}
}
void transmitCheck(int a) /* void transmitCheck ( int socket_fd ) { */{
char *b = ((void *)0 ); /* char * packet = ( ( void * ) 0 ) ; */
int c = -1; /* int bytes_recv = - 1 ; */
uint8_t d; /* uint8_t flag ; */
struct timeval e; /* static struct timeval timeout ; */
e.tv_sec = 1; /* timeout . tv_sec = 1 ; */
e.tv_usec = 0; /* timeout . tv_usec = 0 ; */
b = malloc(1024); /* packet = malloc ( 1024 ) ; */
c = myRecv(a, b, 1024, 0); /* bytes_recv = myRecv ( socket_fd , packet , 1024 , 0 ) ; */
if (c != -1) /* if (bytes_recv != -1) */ {
d = *(b + 2 ); /* flag = * ( packet + 2 ) ; */
switch (d) /* switch (flag) */ {
case 5: 
receivingMessage(b); /* receivingMessage ( packet ) ; */
break;
case 7: 
errorMessage(b); /* errorMessage ( packet ) ; */
break;
case 9: 
terminateClient(b); /* terminateClient ( packet ) ; */
break;
case 11: 
receiveNumHandles(b, a); /* receiveNumHandles ( packet , socket_fd ) ; */
break;
case 12: 
global_d++; /* count_clients ++ ; */
printHandles(b, a); /* printHandles ( packet , socket_fd ) ; */
if (global_d == global_a) /* if (count_clients == actual_clients) */ {
global_e = 0; /* list_flag = 0 ; */
global_d = 0; /* count_clients = 0 ; */
}
break;
default: 
break;
}
}
}
void terminateClient(char *a) /* void terminateClient ( char * packet ) { */{
printf("\n"); /* printf ( "\n" ) ; */
exit(0); /* exit ( 0 ) ; */
}
void printHandles(char *a, int b) /* void printHandles ( char * packet , int socket_fd ) { */{
uint16_t c, d = 3; /* uint16_t packet_len , gar_len = 3 ; */
uint8_t e; /* uint8_t flag ; */
uint8_t f; /* uint8_t handle_len ; */
char *g, *h = malloc(3); /* char * handle , * garbage = malloc ( 3 ) ; */
c = *(uint16_t *)a; /* packet_len = * ( uint16_t * ) packet ; */
f = *((uint8_t *)(a + 3 ) ); /* handle_len = * ( ( uint8_t * ) ( packet + 3 ) ) ; */
g = a + 3 + 1; /* handle = packet + 3 + 1 ; */
printf("\t%s\n", g); /* printf ( "\t%s\n" , handle ) ; */
e = 0; /* flag = 0 ; */
memcpy(h, &d, 2); /* memcpy ( garbage , & gar_len , 2 ) ; */
memcpy(h + 2, &e, 1); /* memcpy ( garbage + 2 , & flag , 1 ) ; */
mySend(b, h, d, 0); /* mySend ( socket_fd , garbage , gar_len , 0 ) ; */
}
void receiveNumHandles(char *a, int b) /* void receiveNumHandles ( char * packet , int socket_fd ) { */{
uint32_t c; /* uint32_t num_clients ; */
char *d = malloc(3); /* char * garbage = malloc ( 3 ) ; */
uint16_t e = 3; /* uint16_t gar_len = 3 ; */
int f = 0; /* int flag = 0 ; */
c = *((uint32_t *)(a + 3 ) ); /* num_clients = * ( ( uint32_t * ) ( packet + 3 ) ) ; */
global_a = c; /* actual_clients = num_clients ; */
printf("\nNumber of clients: %d\n", c); /* printf ( "\nNumber of clients: %d\n" , num_clients ) ; */
global_e = 1; /* list_flag = 1 ; */
memcpy(d, &e, 2); /* memcpy ( garbage , & gar_len , 2 ) ; */
memcpy(d + 2, &f, 1); /* memcpy ( garbage + 2 , & flag , 1 ) ; */
mySend(b, d, e, 0); /* mySend ( socket_fd , garbage , gar_len , 0 ) ; */
}
void receivingMessage(char *a) /* void receivingMessage ( char * packet ) { */{
char *b, *c, *d; /* char * sender , * text , * buf ; */
uint16_t e; /* uint16_t packet_len ; */
uint8_t f; /* uint8_t sender_len ; */
e = *((uint16_t *)a ); /* packet_len = * ( ( uint16_t * ) packet ) ; */
f = *(a + 3 ); /* sender_len = * ( packet + 3 ) ; */
b = a + 3 + 1; /* sender = packet + 3 + 1 ; */
c = a + 3 + 1 + f; /* text = packet + 3 + 1 + sender_len ; */
d = malloc(f); /* buf = malloc ( sender_len ) ; */
memcpy(d, b, f); /* memcpy ( buf , sender , sender_len ) ; */
printf("\n%s: %s\n", d, c); /* printf ( "\n%s: %s\n" , buf , text ) ; */
free(d); /* free ( buf ) ; */
}
void initPacket(char *a, int b) /* void initPacket ( char * client_handle , int socket_fd ) { */{
uint16_t c = 3 + 1 + strlen(a); /* uint16_t packet_len = 3 + 1 + strlen ( client_handle ) ; */
char *d = malloc(c); /* char * packet = malloc ( packet_len ) ; */
uint8_t e = 1, f = strlen(a); /* uint8_t flag = 1 , handle_len = strlen ( client_handle ) ; */
int g; /* int sent ; */
memcpy(d, &c, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(d + 2, &e, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(d + 2 + 1, &f, 1); /* memcpy ( packet + 2 + 1 , & handle_len , 1 ) ; */
memcpy(d + 2 + 2, a, f); /* memcpy ( packet + 2 + 2 , client_handle , handle_len ) ; */
g = mySend(b, d, c, 0); /* sent = mySend ( socket_fd , packet , packet_len , 0 ) ; */
d = ((void *)0 ); /* packet = ( ( void * ) 0 ) ; */
d = malloc(3); /* packet = malloc ( 3 ) ; */
fd_set h; /* fd_set fdvar ; */
int i, j; /* int nfds , msg_len ; */
FD_ZERO(&h); /* FD_ZERO ( & fdvar ) ; */
FD_SET(b, &h); /* FD_SET ( socket_fd , & fdvar ) ; */
i = mySelect(b + 1, (fd_set *)&h, (fd_set *)0, (fd_set *)0, ((void *)0 )); /* nfds = mySelect ( socket_fd + 1 , ( fd_set * ) & fdvar , ( fd_set * ) 0 , ( fd_set * ) 0 , ( ( void * ) 0 ) ) ; */
if (FD_ISSET(b, &h)) /* if (FD_ISSET(socket_fd, &fdvar)) */ {
j = myRecv(b, d, 1024, 0); /* msg_len = myRecv ( socket_fd , packet , 1024 , 0 ) ; */
c = *(uint16_t *)d; /* packet_len = * ( uint16_t * ) packet ; */
e = *((uint8_t *)d + 2 ); /* flag = * ( ( uint8_t * ) packet + 2 ) ; */
if (e == 3) /* if (flag == 3) */ {
fprintf(stderr, "Handle already in use: <%s>\n", a); /* fprintf ( stderr , "Handle already in use: <%s>\n" , client_handle ) ; */
exit(1); /* exit ( 1 ) ; */
}
global_c = a; /* global_handle = client_handle ; */
free(d); /* free ( packet ) ; */
}
}
void promptUser(int a) /* void promptUser ( int socket_fd ) { */{
char *b = ((void *)0 ), c; /* char * send_buf = ( ( void * ) 0 ) , command ; */
int d = 0; /* int send_len = 0 ; */
b = (char *)malloc(1024); /* send_buf = ( char * ) malloc ( 1024 ) ; */
d = 0; /* send_len = 0 ; */
while ((b[d] = getchar() ) != '\n'&&d<1024 - 1) /* while ((send_buf[send_len] = getchar() ) != '\n'&&send_len<1024 - 1) */ {
d++; /* send_len ++ ; */
}
b[d] = '\0'; /* send_buf [ send_len ] = '\0' ; */
if (b[0] != '%') /* if (send_buf[0] != '%') */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
} else {
c = b[1]; /* command = send_buf [ 1 ] ; */
switch (tolower(c)) /* switch (tolower(command)) */ {
case 'm': 
message(b, a, d); /* message ( send_buf , socket_fd , send_len ) ; */
break;
case 'b': 
broadcast(b, a, d); /* broadcast ( send_buf , socket_fd , send_len ) ; */
break;
case 'l': 
handleList(b, a, d); /* handleList ( send_buf , socket_fd , send_len ) ; */
break;
case 'e': 
exitClient(b, a, d); /* exitClient ( send_buf , socket_fd , send_len ) ; */
break;
default: 
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
break;
}
}
}
void message(char *a, int b, int c) /* void message ( char * send_buf , int socket_fd , int send_len ) { */{
char *d = a, *e, *f, *g; /* char * token = send_buf , * handle , * packet , * text ; */
char *h = " ", i = 5; /* const char * delim = " " , flag = 5 ; */
int j, k = 0; /* int sent , txt_len = 0 ; */
uint16_t l = 0; /* uint16_t total_len = 0 ; */
uint8_t m, n; /* uint8_t handle_len , sending_len ; */
d = strtok(d, h); /* token = strtok ( token , delim ) ; */
e = strtok(((void *)0 ), h); /* handle = strtok ( ( ( void * ) 0 ) , delim ) ; */
m = strlen(e); /* handle_len = strlen ( handle ) ; */
n = strlen(global_c); /* sending_len = strlen ( global_handle ) ; */
k = c - 4 - m; /* txt_len = send_len - 4 - handle_len ; */
g = malloc(k + 1); /* text = malloc ( txt_len + 1 ) ; */
while ((d = strtok(((void *)0 ), h) ) != ((void *)0 )) /* while ((token = strtok(((void *)0 ), delim) ) != ((void *)0 )) */ {
strcat(g, d); /* strcat ( text , token ) ; */
strcat(g, " "); /* strcat ( text , " " ) ; */
}
strcat(g, "\0"); /* strcat ( text , "\0" ) ; */
l = 3 + 1 + m + 1 + n + k; /* total_len = 3 + 1 + handle_len + 1 + sending_len + txt_len ; */
f = malloc(l); /* packet = malloc ( total_len ) ; */
memcpy(f, &l, 2); /* memcpy ( packet , & total_len , 2 ) ; */
memcpy(f + 2, &i, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(f + 2 + 1, &m, 1); /* memcpy ( packet + 2 + 1 , & handle_len , 1 ) ; */
memcpy(f + 2 + 1 + 1, e, m); /* memcpy ( packet + 2 + 1 + 1 , handle , handle_len ) ; */
memcpy(f + 2 + 1 + 1 + m, &n, 1); /* memcpy ( packet + 2 + 1 + 1 + handle_len , & sending_len , 1 ) ; */
memcpy(f + 2 + 1 + 1 + m + 1, global_c, n); /* memcpy ( packet + 2 + 1 + 1 + handle_len + 1 , global_handle , sending_len ) ; */
memcpy(f + 2 + 1 + 1 + m + 1 + n, g, k + 1); /* memcpy ( packet + 2 + 1 + 1 + handle_len + 1 + sending_len , text , txt_len + 1 ) ; */
j = mySend(b, f, l, 0); /* sent = mySend ( socket_fd , packet , total_len , 0 ) ; */
}
void broadcast(char *a, int b, int c) /* void broadcast ( char * input , int socket_fd , int send_len ) { */{
char *d = a, *e, *f, *g; /* char * token = input , * handle , * packet , * text ; */
uint8_t h, i = 4; /* uint8_t sending_len , flag = 4 ; */
int j, k, l; /* int total_len , txt_len , sent ; */
d = strtok(d, " "); /* token = strtok ( token , " " ) ; */
h = strlen(global_c); /* sending_len = strlen ( global_handle ) ; */
k = c - 3; /* txt_len = send_len - 3 ; */
g = malloc(k + 1); /* text = malloc ( txt_len + 1 ) ; */
while ((d = strtok(((void *)0 ), " ") ) != ((void *)0 )) /* while ((token = strtok(((void *)0 ), " ") ) != ((void *)0 )) */ {
strcat(g, d); /* strcat ( text , token ) ; */
strcat(g, " "); /* strcat ( text , " " ) ; */
}
strcat(g, "\0"); /* strcat ( text , "\0" ) ; */
e = global_c; /* handle = global_handle ; */
strcat(g, "\0"); /* strcat ( text , "\0" ) ; */
j = 3 + 1 + h + k; /* total_len = 3 + 1 + sending_len + txt_len ; */
f = malloc(j); /* packet = malloc ( total_len ) ; */
memcpy(f, &j, 2); /* memcpy ( packet , & total_len , 2 ) ; */
memcpy(f + 2, &i, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(f + 2 + 1, &h, 1); /* memcpy ( packet + 2 + 1 , & sending_len , 1 ) ; */
memcpy(f + 2 + 1 + 1, e, h); /* memcpy ( packet + 2 + 1 + 1 , handle , sending_len ) ; */
memcpy(f + 2 + 1 + 1 + h, g, k + 1); /* memcpy ( packet + 2 + 1 + 1 + sending_len , text , txt_len + 1 ) ; */
l = mySend(b, f, j, 0); /* sent = mySend ( socket_fd , packet , total_len , 0 ) ; */
}
void handleList(char *a, int b, int c) /* void handleList ( char * input , int socket_fd , int send_len ) { */{
char *d; /* char * packet ; */
uint16_t e = 3; /* uint16_t packet_len = 3 ; */
uint8_t f = 10; /* uint8_t flag = 10 ; */
int g; /* int sent ; */
d = malloc(e); /* packet = malloc ( packet_len ) ; */
memcpy(d, &e, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(d + 2, &f, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
g = mySend(b, d, e, 0); /* sent = mySend ( socket_fd , packet , packet_len , 0 ) ; */
}
void exitClient(char *a, int b, int c) /* void exitClient ( char * input , int socket_fd , int send_len ) { */{
uint16_t d; /* uint16_t packet_len ; */
uint8_t e = 8; /* uint8_t flag = 8 ; */
char *f; /* char * packet ; */
int g; /* int sent ; */
d = 3; /* packet_len = 3 ; */
f = malloc(d); /* packet = malloc ( packet_len ) ; */
memcpy(f, &d, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(f + 2, &e, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
g = mySend(b, f, d, 0); /* sent = mySend ( socket_fd , packet , packet_len , 0 ) ; */
}
void errorMessage(char *a) /* void errorMessage ( char * packet ) { */{
char *b, *c; /* char * invalid , * client ; */
int d = *(a + 3 ); /* int invalid_len = * ( packet + 3 ) ; */
c = a + 3 + 1; /* client = packet + 3 + 1 ; */
b = malloc(d); /* invalid = malloc ( invalid_len ) ; */
memcpy(b, c, d); /* memcpy ( invalid , client , invalid_len ) ; */
fprintf(stderr, "\nClient with handle %s does not exist.\n", b); /* fprintf ( stderr , "\nClient with handle %s does not exist.\n" , invalid ) ; */
free(b); /* free ( invalid ) ; */
}
void printPrompt() /* void printPrompt ( ) { */{
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
