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
fd_set b; /* fd_set fdvar ; */
int c = a; /* int max_socket = socket_fd ; */
FD_ZERO(&b); /* FD_ZERO ( & fdvar ) ; */
FD_SET(a, &b); /* FD_SET ( socket_fd , & fdvar ) ; */
while (1) /* while (1) */ {
if (!global_e) /* if (!list_flag) */ {
printPrompt(); /* printPrompt ( ) ; */
}
while (1) /* while (1) */ {
FD_ZERO(&b); /* FD_ZERO ( & fdvar ) ; */
FD_SET(a, &b); /* FD_SET ( socket_fd , & fdvar ) ; */
FD_SET(0, &b); /* FD_SET ( 0 , & fdvar ) ; */
if (mySelect(c + 1, (fd_set *)&b, (fd_set *)0, (fd_set *)0, ((void *)0 ))>0) /* if (mySelect(max_socket + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, ((void *)0 ))>0) */ {
if (FD_ISSET(a, &b)) /* if (FD_ISSET(socket_fd, &fdvar)) */ {
transmitCheck(a); /* transmitCheck ( socket_fd ) ; */
}
if (!global_e) /* if (!list_flag) */ {
if (FD_ISSET(0, &b)) /* if (FD_ISSET(0, &fdvar)) */ {
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
uint8_t b; /* uint8_t flag ; */
char *c = ((void *)0 ); /* char * packet = ( ( void * ) 0 ) ; */
int d = -1; /* int bytes_recv = - 1 ; */
struct timeval e; /* static struct timeval timeout ; */
e.tv_usec = 0; /* timeout . tv_usec = 0 ; */
e.tv_sec = 1; /* timeout . tv_sec = 1 ; */
c = malloc(1024); /* packet = malloc ( 1024 ) ; */
d = myRecv(a, c, 1024, 0); /* bytes_recv = myRecv ( socket_fd , packet , 1024 , 0 ) ; */
if (d != -1) /* if (bytes_recv != -1) */ {
b = *(c + 2 ); /* flag = * ( packet + 2 ) ; */
switch (b) /* switch (flag) */ {
case 5: 
receivingMessage(c); /* receivingMessage ( packet ) ; */
break;
case 7: 
errorMessage(c); /* errorMessage ( packet ) ; */
break;
case 9: 
terminateClient(c); /* terminateClient ( packet ) ; */
break;
case 11: 
receiveNumHandles(c, a); /* receiveNumHandles ( packet , socket_fd ) ; */
break;
case 12: 
global_d++; /* count_clients ++ ; */
printHandles(c, a); /* printHandles ( packet , socket_fd ) ; */
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
char *f, *g = malloc(3); /* char * handle , * garbage = malloc ( 3 ) ; */
uint8_t h; /* uint8_t handle_len ; */
e = 0; /* flag = 0 ; */
c = *(uint16_t *)a; /* packet_len = * ( uint16_t * ) packet ; */
h = *((uint8_t *)(a + 3 ) ); /* handle_len = * ( ( uint8_t * ) ( packet + 3 ) ) ; */
f = a + 3 + 1; /* handle = packet + 3 + 1 ; */
printf("\t%s\n", f); /* printf ( "\t%s\n" , handle ) ; */
memcpy(g, &d, 2); /* memcpy ( garbage , & gar_len , 2 ) ; */
memcpy(g + 2, &e, 1); /* memcpy ( garbage + 2 , & flag , 1 ) ; */
mySend(b, g, d, 0); /* mySend ( socket_fd , garbage , gar_len , 0 ) ; */
}
void receiveNumHandles(char *a, int b) /* void receiveNumHandles ( char * packet , int socket_fd ) { */{
int c = 0; /* int flag = 0 ; */
uint32_t d; /* uint32_t num_clients ; */
char *e = malloc(3); /* char * garbage = malloc ( 3 ) ; */
uint16_t f = 3; /* uint16_t gar_len = 3 ; */
global_e = 1; /* list_flag = 1 ; */
d = *((uint32_t *)(a + 3 ) ); /* num_clients = * ( ( uint32_t * ) ( packet + 3 ) ) ; */
global_a = d; /* actual_clients = num_clients ; */
printf("\nNumber of clients: %d\n", d); /* printf ( "\nNumber of clients: %d\n" , num_clients ) ; */
memcpy(e, &f, 2); /* memcpy ( garbage , & gar_len , 2 ) ; */
memcpy(e + 2, &c, 1); /* memcpy ( garbage + 2 , & flag , 1 ) ; */
mySend(b, e, f, 0); /* mySend ( socket_fd , garbage , gar_len , 0 ) ; */
}
void receivingMessage(char *a) /* void receivingMessage ( char * packet ) { */{
char *b, *c, *d; /* char * sender , * text , * buf ; */
uint16_t e; /* uint16_t packet_len ; */
uint8_t f; /* uint8_t sender_len ; */
e = *((uint16_t *)a ); /* packet_len = * ( ( uint16_t * ) packet ) ; */
f = *(a + 3 ); /* sender_len = * ( packet + 3 ) ; */
c = a + 3 + 1 + f; /* text = packet + 3 + 1 + sender_len ; */
b = a + 3 + 1; /* sender = packet + 3 + 1 ; */
d = malloc(f); /* buf = malloc ( sender_len ) ; */
memcpy(d, b, f); /* memcpy ( buf , sender , sender_len ) ; */
printf("\n%s: %s\n", d, c); /* printf ( "\n%s: %s\n" , buf , text ) ; */
free(d); /* free ( buf ) ; */
}
void initPacket(char *a, int b) /* void initPacket ( char * client_handle , int socket_fd ) { */{
int c; /* int sent ; */
fd_set d; /* fd_set fdvar ; */
int e, f; /* int nfds , msg_len ; */
uint16_t g = 3 + 1 + strlen(a); /* uint16_t packet_len = 3 + 1 + strlen ( client_handle ) ; */
char *h = malloc(g); /* char * packet = malloc ( packet_len ) ; */
uint8_t i = 1, j = strlen(a); /* uint8_t flag = 1 , handle_len = strlen ( client_handle ) ; */
memcpy(h, &g, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(h + 2, &i, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(h + 2 + 1, &j, 1); /* memcpy ( packet + 2 + 1 , & handle_len , 1 ) ; */
memcpy(h + 2 + 2, a, j); /* memcpy ( packet + 2 + 2 , client_handle , handle_len ) ; */
c = mySend(b, h, g, 0); /* sent = mySend ( socket_fd , packet , packet_len , 0 ) ; */
h = ((void *)0 ); /* packet = ( ( void * ) 0 ) ; */
h = malloc(3); /* packet = malloc ( 3 ) ; */
FD_ZERO(&d); /* FD_ZERO ( & fdvar ) ; */
FD_SET(b, &d); /* FD_SET ( socket_fd , & fdvar ) ; */
e = mySelect(b + 1, (fd_set *)&d, (fd_set *)0, (fd_set *)0, ((void *)0 )); /* nfds = mySelect ( socket_fd + 1 , ( fd_set * ) & fdvar , ( fd_set * ) 0 , ( fd_set * ) 0 , ( ( void * ) 0 ) ) ; */
if (FD_ISSET(b, &d)) /* if (FD_ISSET(socket_fd, &fdvar)) */ {
global_c = a; /* global_handle = client_handle ; */
f = myRecv(b, h, 1024, 0); /* msg_len = myRecv ( socket_fd , packet , 1024 , 0 ) ; */
g = *(uint16_t *)h; /* packet_len = * ( uint16_t * ) packet ; */
i = *((uint8_t *)h + 2 ); /* flag = * ( ( uint8_t * ) packet + 2 ) ; */
if (i == 3) /* if (flag == 3) */ {
fprintf(stderr, "Handle already in use: <%s>\n", a); /* fprintf ( stderr , "Handle already in use: <%s>\n" , client_handle ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(h); /* free ( packet ) ; */
}
}
void promptUser(int a) /* void promptUser ( int socket_fd ) { */{
int b = 0; /* int send_len = 0 ; */
char *c = ((void *)0 ), d; /* char * send_buf = ( ( void * ) 0 ) , command ; */
c = (char *)malloc(1024); /* send_buf = ( char * ) malloc ( 1024 ) ; */
b = 0; /* send_len = 0 ; */
while ((c[b] = getchar() ) != '\n'&&b<1024 - 1) /* while ((send_buf[send_len] = getchar() ) != '\n'&&send_len<1024 - 1) */ {
b++; /* send_len ++ ; */
}
c[b] = '\0'; /* send_buf [ send_len ] = '\0' ; */
if (c[0] != '%') /* if (send_buf[0] != '%') */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
} else {
d = c[1]; /* command = send_buf [ 1 ] ; */
switch (tolower(d)) /* switch (tolower(command)) */ {
case 'm': 
message(c, a, b); /* message ( send_buf , socket_fd , send_len ) ; */
break;
case 'b': 
broadcast(c, a, b); /* broadcast ( send_buf , socket_fd , send_len ) ; */
break;
case 'l': 
handleList(c, a, b); /* handleList ( send_buf , socket_fd , send_len ) ; */
break;
case 'e': 
exitClient(c, a, b); /* exitClient ( send_buf , socket_fd , send_len ) ; */
break;
default: 
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
break;
}
}
}
void message(char *a, int b, int c) /* void message ( char * send_buf , int socket_fd , int send_len ) { */{
uint16_t d = 0; /* uint16_t total_len = 0 ; */
char *e = " ", f = 5; /* const char * delim = " " , flag = 5 ; */
uint8_t g, h; /* uint8_t handle_len , sending_len ; */
int i, j = 0; /* int sent , txt_len = 0 ; */
char *k = a, *l, *m, *n; /* char * token = send_buf , * handle , * packet , * text ; */
k = strtok(k, e); /* token = strtok ( token , delim ) ; */
l = strtok(((void *)0 ), e); /* handle = strtok ( ( ( void * ) 0 ) , delim ) ; */
g = strlen(l); /* handle_len = strlen ( handle ) ; */
h = strlen(global_c); /* sending_len = strlen ( global_handle ) ; */
j = c - 4 - g; /* txt_len = send_len - 4 - handle_len ; */
n = malloc(j + 1); /* text = malloc ( txt_len + 1 ) ; */
while ((k = strtok(((void *)0 ), e) ) != ((void *)0 )) /* while ((token = strtok(((void *)0 ), delim) ) != ((void *)0 )) */ {
strcat(n, k); /* strcat ( text , token ) ; */
strcat(n, " "); /* strcat ( text , " " ) ; */
}
strcat(n, "\0"); /* strcat ( text , "\0" ) ; */
d = 3 + 1 + g + 1 + h + j; /* total_len = 3 + 1 + handle_len + 1 + sending_len + txt_len ; */
m = malloc(d); /* packet = malloc ( total_len ) ; */
memcpy(m, &d, 2); /* memcpy ( packet , & total_len , 2 ) ; */
memcpy(m + 2, &f, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(m + 2 + 1, &g, 1); /* memcpy ( packet + 2 + 1 , & handle_len , 1 ) ; */
memcpy(m + 2 + 1 + 1, l, g); /* memcpy ( packet + 2 + 1 + 1 , handle , handle_len ) ; */
memcpy(m + 2 + 1 + 1 + g, &h, 1); /* memcpy ( packet + 2 + 1 + 1 + handle_len , & sending_len , 1 ) ; */
memcpy(m + 2 + 1 + 1 + g + 1, global_c, h); /* memcpy ( packet + 2 + 1 + 1 + handle_len + 1 , global_handle , sending_len ) ; */
memcpy(m + 2 + 1 + 1 + g + 1 + h, n, j + 1); /* memcpy ( packet + 2 + 1 + 1 + handle_len + 1 + sending_len , text , txt_len + 1 ) ; */
i = mySend(b, m, d, 0); /* sent = mySend ( socket_fd , packet , total_len , 0 ) ; */
}
void broadcast(char *a, int b, int c) /* void broadcast ( char * input , int socket_fd , int send_len ) { */{
uint8_t d, e = 4; /* uint8_t sending_len , flag = 4 ; */
int f, g, h; /* int total_len , txt_len , sent ; */
char *i = a, *j, *k, *l; /* char * token = input , * handle , * packet , * text ; */
j = global_c; /* handle = global_handle ; */
i = strtok(i, " "); /* token = strtok ( token , " " ) ; */
d = strlen(global_c); /* sending_len = strlen ( global_handle ) ; */
g = c - 3; /* txt_len = send_len - 3 ; */
l = malloc(g + 1); /* text = malloc ( txt_len + 1 ) ; */
while ((i = strtok(((void *)0 ), " ") ) != ((void *)0 )) /* while ((token = strtok(((void *)0 ), " ") ) != ((void *)0 )) */ {
strcat(l, i); /* strcat ( text , token ) ; */
strcat(l, " "); /* strcat ( text , " " ) ; */
}
strcat(l, "\0"); /* strcat ( text , "\0" ) ; */
strcat(l, "\0"); /* strcat ( text , "\0" ) ; */
f = 3 + 1 + d + g; /* total_len = 3 + 1 + sending_len + txt_len ; */
k = malloc(f); /* packet = malloc ( total_len ) ; */
memcpy(k, &f, 2); /* memcpy ( packet , & total_len , 2 ) ; */
memcpy(k + 2, &e, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(k + 2 + 1, &d, 1); /* memcpy ( packet + 2 + 1 , & sending_len , 1 ) ; */
memcpy(k + 2 + 1 + 1, j, d); /* memcpy ( packet + 2 + 1 + 1 , handle , sending_len ) ; */
memcpy(k + 2 + 1 + 1 + d, l, g + 1); /* memcpy ( packet + 2 + 1 + 1 + sending_len , text , txt_len + 1 ) ; */
h = mySend(b, k, f, 0); /* sent = mySend ( socket_fd , packet , total_len , 0 ) ; */
}
void handleList(char *a, int b, int c) /* void handleList ( char * input , int socket_fd , int send_len ) { */{
int d; /* int sent ; */
char *e; /* char * packet ; */
uint8_t f = 10; /* uint8_t flag = 10 ; */
uint16_t g = 3; /* uint16_t packet_len = 3 ; */
e = malloc(g); /* packet = malloc ( packet_len ) ; */
memcpy(e, &g, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(e + 2, &f, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
d = mySend(b, e, g, 0); /* sent = mySend ( socket_fd , packet , packet_len , 0 ) ; */
}
void exitClient(char *a, int b, int c) /* void exitClient ( char * input , int socket_fd , int send_len ) { */{
uint16_t d; /* uint16_t packet_len ; */
char *e; /* char * packet ; */
uint8_t f = 8; /* uint8_t flag = 8 ; */
int g; /* int sent ; */
d = 3; /* packet_len = 3 ; */
e = malloc(d); /* packet = malloc ( packet_len ) ; */
memcpy(e, &d, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(e + 2, &f, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
g = mySend(b, e, d, 0); /* sent = mySend ( socket_fd , packet , packet_len , 0 ) ; */
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
