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
char *global_handle; /* char * global_handle ; */
int max_socket = 3; /* int max_socket = 3 ; */
int list_flag = 0; /* int list_flag = 0 ; */
int actual_clients = 0; /* int actual_clients = 0 ; */
int count_clients = 0; /* int count_clients = 0 ; */

unsigned int __bswap_32(unsigned int __bsx) /*  */{
return __builtin_bswap32(__bsx); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t __bsx) /*  */{
return __builtin_bswap64(__bsx); /* return __builtin_bswap64(__bsx) */
}
int main(int argc, char *argv) /* int main ( int argc , char * argv [ ] ) { */{
int socket_fd = 0; /* int socket_fd = 0 ; */
if (argc != 4) /* if (argc != 4) */ {
fprintf(stderr, "Error - must include IP address, port number, and client handle name\n"); /* fprintf ( stderr , "Error - must include IP address, port number, and client handle name\n" ) ; */
exit(1); /* exit ( 1 ) ; */
}
socket_fd = tcpClientSetup(argv[2], argv[3]); /* socket_fd = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
initPacket(argv[1], socket_fd); /* initPacket ( argv [ 1 ] , socket_fd ) ; */
startUp(socket_fd); /* startUp ( socket_fd ) ; */
close(socket_fd); /* close ( socket_fd ) ; */
return 0; /* return 0 */
}
void startUp(int socket_fd) /* void startUp ( int socket_fd ) { */{
fd_set fdvar; /* fd_set fdvar ; */
int max_socket = socket_fd; /* int max_socket = socket_fd ; */
FD_ZERO(&fdvar); /* FD_ZERO ( & fdvar ) ; */
FD_SET(socket_fd, &fdvar); /* FD_SET ( socket_fd , & fdvar ) ; */
while (1) /* while (1) */ {
if (!list_flag) /* if (!list_flag) */ {
printPrompt(); /* printPrompt ( ) ; */
}
while (1) /* while (1) */ {
FD_ZERO(&fdvar); /* FD_ZERO ( & fdvar ) ; */
FD_SET(socket_fd, &fdvar); /* FD_SET ( socket_fd , & fdvar ) ; */
FD_SET(0, &fdvar); /* FD_SET ( 0 , & fdvar ) ; */
if (mySelect(max_socket + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, ((void *)0 ))>0) /* if (mySelect(max_socket + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, ((void *)0 ))>0) */ {
if (FD_ISSET(socket_fd, &fdvar)) /* if (FD_ISSET(socket_fd, &fdvar)) */ {
transmitCheck(socket_fd); /* transmitCheck ( socket_fd ) ; */
}
if (!list_flag) /* if (!list_flag) */ {
if (FD_ISSET(0, &fdvar)) /* if (FD_ISSET(0, &fdvar)) */ {
promptUser(socket_fd); /* promptUser ( socket_fd ) ; */
}
}
break;
}
break;
}
}
}
void transmitCheck(int socket_fd) /* void transmitCheck ( int socket_fd ) { */{
uint8_t flag; /* uint8_t flag ; */
char *packet = ((void *)0 ); /* char * packet = ( ( void * ) 0 ) ; */
int bytes_recv = -1; /* int bytes_recv = - 1 ; */
struct timeval timeout; /* static struct timeval timeout ; */
timeout.tv_usec = 0; /* timeout . tv_usec = 0 ; */
timeout.tv_sec = 1; /* timeout . tv_sec = 1 ; */
packet = malloc(1024); /* packet = malloc ( 1024 ) ; */
bytes_recv = myRecv(socket_fd, packet, 1024, 0); /* bytes_recv = myRecv ( socket_fd , packet , 1024 , 0 ) ; */
if (bytes_recv != -1) /* if (bytes_recv != -1) */ {
flag = *(packet + 2 ); /* flag = * ( packet + 2 ) ; */
switch (flag) /* switch (flag) */ {
case 5: 
receivingMessage(packet); /* receivingMessage ( packet ) ; */
break;
case 7: 
errorMessage(packet); /* errorMessage ( packet ) ; */
break;
case 9: 
terminateClient(packet); /* terminateClient ( packet ) ; */
break;
case 11: 
receiveNumHandles(packet, socket_fd); /* receiveNumHandles ( packet , socket_fd ) ; */
break;
case 12: 
count_clients++; /* count_clients ++ ; */
printHandles(packet, socket_fd); /* printHandles ( packet , socket_fd ) ; */
if (count_clients == actual_clients) /* if (count_clients == actual_clients) */ {
list_flag = 0; /* list_flag = 0 ; */
count_clients = 0; /* count_clients = 0 ; */
}
break;
default: 
break;
}
}
}
void terminateClient(char *packet) /* void terminateClient ( char * packet ) { */{
printf("\n"); /* printf ( "\n" ) ; */
exit(0); /* exit ( 0 ) ; */
}
void printHandles(char *packet, int socket_fd) /* void printHandles ( char * packet , int socket_fd ) { */{
uint16_t packet_len, gar_len = 3; /* uint16_t packet_len , gar_len = 3 ; */
uint8_t flag; /* uint8_t flag ; */
char *handle, *garbage = malloc(3); /* char * handle , * garbage = malloc ( 3 ) ; */
uint8_t handle_len; /* uint8_t handle_len ; */
flag = 0; /* flag = 0 ; */
packet_len = *(uint16_t *)packet; /* packet_len = * ( uint16_t * ) packet ; */
handle_len = *((uint8_t *)(packet + 3 ) ); /* handle_len = * ( ( uint8_t * ) ( packet + 3 ) ) ; */
handle = packet + 3 + 1; /* handle = packet + 3 + 1 ; */
printf("\t%s\n", handle); /* printf ( "\t%s\n" , handle ) ; */
memcpy(garbage, &gar_len, 2); /* memcpy ( garbage , & gar_len , 2 ) ; */
memcpy(garbage + 2, &flag, 1); /* memcpy ( garbage + 2 , & flag , 1 ) ; */
mySend(socket_fd, garbage, gar_len, 0); /* mySend ( socket_fd , garbage , gar_len , 0 ) ; */
}
void receiveNumHandles(char *packet, int socket_fd) /* void receiveNumHandles ( char * packet , int socket_fd ) { */{
int flag = 0; /* int flag = 0 ; */
uint32_t num_clients; /* uint32_t num_clients ; */
char *garbage = malloc(3); /* char * garbage = malloc ( 3 ) ; */
uint16_t gar_len = 3; /* uint16_t gar_len = 3 ; */
list_flag = 1; /* list_flag = 1 ; */
num_clients = *((uint32_t *)(packet + 3 ) ); /* num_clients = * ( ( uint32_t * ) ( packet + 3 ) ) ; */
actual_clients = num_clients; /* actual_clients = num_clients ; */
printf("\nNumber of clients: %d\n", num_clients); /* printf ( "\nNumber of clients: %d\n" , num_clients ) ; */
memcpy(garbage, &gar_len, 2); /* memcpy ( garbage , & gar_len , 2 ) ; */
memcpy(garbage + 2, &flag, 1); /* memcpy ( garbage + 2 , & flag , 1 ) ; */
mySend(socket_fd, garbage, gar_len, 0); /* mySend ( socket_fd , garbage , gar_len , 0 ) ; */
}
void receivingMessage(char *packet) /* void receivingMessage ( char * packet ) { */{
char *sender, *text, *buf; /* char * sender , * text , * buf ; */
uint16_t packet_len; /* uint16_t packet_len ; */
uint8_t sender_len; /* uint8_t sender_len ; */
packet_len = *((uint16_t *)packet ); /* packet_len = * ( ( uint16_t * ) packet ) ; */
sender_len = *(packet + 3 ); /* sender_len = * ( packet + 3 ) ; */
text = packet + 3 + 1 + sender_len; /* text = packet + 3 + 1 + sender_len ; */
sender = packet + 3 + 1; /* sender = packet + 3 + 1 ; */
buf = malloc(sender_len); /* buf = malloc ( sender_len ) ; */
memcpy(buf, sender, sender_len); /* memcpy ( buf , sender , sender_len ) ; */
printf("\n%s: %s\n", buf, text); /* printf ( "\n%s: %s\n" , buf , text ) ; */
free(buf); /* free ( buf ) ; */
}
void initPacket(char *client_handle, int socket_fd) /* void initPacket ( char * client_handle , int socket_fd ) { */{
int sent; /* int sent ; */
fd_set fdvar; /* fd_set fdvar ; */
int nfds, msg_len; /* int nfds , msg_len ; */
uint16_t packet_len = 3 + 1 + strlen(client_handle); /* uint16_t packet_len = 3 + 1 + strlen ( client_handle ) ; */
char *packet = malloc(packet_len); /* char * packet = malloc ( packet_len ) ; */
uint8_t flag = 1, handle_len = strlen(client_handle); /* uint8_t flag = 1 , handle_len = strlen ( client_handle ) ; */
memcpy(packet, &packet_len, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(packet + 2, &flag, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(packet + 2 + 1, &handle_len, 1); /* memcpy ( packet + 2 + 1 , & handle_len , 1 ) ; */
memcpy(packet + 2 + 2, client_handle, handle_len); /* memcpy ( packet + 2 + 2 , client_handle , handle_len ) ; */
sent = mySend(socket_fd, packet, packet_len, 0); /* sent = mySend ( socket_fd , packet , packet_len , 0 ) ; */
packet = ((void *)0 ); /* packet = ( ( void * ) 0 ) ; */
packet = malloc(3); /* packet = malloc ( 3 ) ; */
FD_ZERO(&fdvar); /* FD_ZERO ( & fdvar ) ; */
FD_SET(socket_fd, &fdvar); /* FD_SET ( socket_fd , & fdvar ) ; */
nfds = mySelect(socket_fd + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, ((void *)0 )); /* nfds = mySelect ( socket_fd + 1 , ( fd_set * ) & fdvar , ( fd_set * ) 0 , ( fd_set * ) 0 , ( ( void * ) 0 ) ) ; */
if (FD_ISSET(socket_fd, &fdvar)) /* if (FD_ISSET(socket_fd, &fdvar)) */ {
global_handle = client_handle; /* global_handle = client_handle ; */
msg_len = myRecv(socket_fd, packet, 1024, 0); /* msg_len = myRecv ( socket_fd , packet , 1024 , 0 ) ; */
packet_len = *(uint16_t *)packet; /* packet_len = * ( uint16_t * ) packet ; */
flag = *((uint8_t *)packet + 2 ); /* flag = * ( ( uint8_t * ) packet + 2 ) ; */
if (flag == 3) /* if (flag == 3) */ {
fprintf(stderr, "Handle already in use: <%s>\n", client_handle); /* fprintf ( stderr , "Handle already in use: <%s>\n" , client_handle ) ; */
exit(1); /* exit ( 1 ) ; */
}
free(packet); /* free ( packet ) ; */
}
}
void promptUser(int socket_fd) /* void promptUser ( int socket_fd ) { */{
int send_len = 0; /* int send_len = 0 ; */
char *send_buf = ((void *)0 ), command; /* char * send_buf = ( ( void * ) 0 ) , command ; */
send_buf = (char *)malloc(1024); /* send_buf = ( char * ) malloc ( 1024 ) ; */
send_len = 0; /* send_len = 0 ; */
while ((send_buf[send_len] = getchar() ) != '\n'&&send_len<1024 - 1) /* while ((send_buf[send_len] = getchar() ) != '\n'&&send_len<1024 - 1) */ {
send_len++; /* send_len ++ ; */
}
send_buf[send_len] = '\0'; /* send_buf [ send_len ] = '\0' ; */
if (send_buf[0] != '%') /* if (send_buf[0] != '%') */ {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
} else {
command = send_buf[1]; /* command = send_buf [ 1 ] ; */
switch (tolower(command)) /* switch (tolower(command)) */ {
case 'm': 
message(send_buf, socket_fd, send_len); /* message ( send_buf , socket_fd , send_len ) ; */
break;
case 'b': 
broadcast(send_buf, socket_fd, send_len); /* broadcast ( send_buf , socket_fd , send_len ) ; */
break;
case 'l': 
handleList(send_buf, socket_fd, send_len); /* handleList ( send_buf , socket_fd , send_len ) ; */
break;
case 'e': 
exitClient(send_buf, socket_fd, send_len); /* exitClient ( send_buf , socket_fd , send_len ) ; */
break;
default: 
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
break;
}
}
}
void message(char *send_buf, int socket_fd, int send_len) /* void message ( char * send_buf , int socket_fd , int send_len ) { */{
uint16_t total_len = 0; /* uint16_t total_len = 0 ; */
char *delim = " ", flag = 5; /* const char * delim = " " , flag = 5 ; */
uint8_t handle_len, sending_len; /* uint8_t handle_len , sending_len ; */
int sent, txt_len = 0; /* int sent , txt_len = 0 ; */
char *token = send_buf, *handle, *packet, *text; /* char * token = send_buf , * handle , * packet , * text ; */
token = strtok(token, delim); /* token = strtok ( token , delim ) ; */
handle = strtok(((void *)0 ), delim); /* handle = strtok ( ( ( void * ) 0 ) , delim ) ; */
handle_len = strlen(handle); /* handle_len = strlen ( handle ) ; */
sending_len = strlen(global_handle); /* sending_len = strlen ( global_handle ) ; */
txt_len = send_len - 4 - handle_len; /* txt_len = send_len - 4 - handle_len ; */
text = malloc(txt_len + 1); /* text = malloc ( txt_len + 1 ) ; */
while ((token = strtok(((void *)0 ), delim) ) != ((void *)0 )) /* while ((token = strtok(((void *)0 ), delim) ) != ((void *)0 )) */ {
strcat(text, token); /* strcat ( text , token ) ; */
strcat(text, " "); /* strcat ( text , " " ) ; */
}
strcat(text, "\0"); /* strcat ( text , "\0" ) ; */
total_len = 3 + 1 + handle_len + 1 + sending_len + txt_len; /* total_len = 3 + 1 + handle_len + 1 + sending_len + txt_len ; */
packet = malloc(total_len); /* packet = malloc ( total_len ) ; */
memcpy(packet, &total_len, 2); /* memcpy ( packet , & total_len , 2 ) ; */
memcpy(packet + 2, &flag, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(packet + 2 + 1, &handle_len, 1); /* memcpy ( packet + 2 + 1 , & handle_len , 1 ) ; */
memcpy(packet + 2 + 1 + 1, handle, handle_len); /* memcpy ( packet + 2 + 1 + 1 , handle , handle_len ) ; */
memcpy(packet + 2 + 1 + 1 + handle_len, &sending_len, 1); /* memcpy ( packet + 2 + 1 + 1 + handle_len , & sending_len , 1 ) ; */
memcpy(packet + 2 + 1 + 1 + handle_len + 1, global_handle, sending_len); /* memcpy ( packet + 2 + 1 + 1 + handle_len + 1 , global_handle , sending_len ) ; */
memcpy(packet + 2 + 1 + 1 + handle_len + 1 + sending_len, text, txt_len + 1); /* memcpy ( packet + 2 + 1 + 1 + handle_len + 1 + sending_len , text , txt_len + 1 ) ; */
sent = mySend(socket_fd, packet, total_len, 0); /* sent = mySend ( socket_fd , packet , total_len , 0 ) ; */
}
void broadcast(char *input, int socket_fd, int send_len) /* void broadcast ( char * input , int socket_fd , int send_len ) { */{
uint8_t sending_len, flag = 4; /* uint8_t sending_len , flag = 4 ; */
int total_len, txt_len, sent; /* int total_len , txt_len , sent ; */
char *token = input, *handle, *packet, *text; /* char * token = input , * handle , * packet , * text ; */
handle = global_handle; /* handle = global_handle ; */
token = strtok(token, " "); /* token = strtok ( token , " " ) ; */
sending_len = strlen(global_handle); /* sending_len = strlen ( global_handle ) ; */
txt_len = send_len - 3; /* txt_len = send_len - 3 ; */
text = malloc(txt_len + 1); /* text = malloc ( txt_len + 1 ) ; */
while ((token = strtok(((void *)0 ), " ") ) != ((void *)0 )) /* while ((token = strtok(((void *)0 ), " ") ) != ((void *)0 )) */ {
strcat(text, token); /* strcat ( text , token ) ; */
strcat(text, " "); /* strcat ( text , " " ) ; */
}
strcat(text, "\0"); /* strcat ( text , "\0" ) ; */
strcat(text, "\0"); /* strcat ( text , "\0" ) ; */
total_len = 3 + 1 + sending_len + txt_len; /* total_len = 3 + 1 + sending_len + txt_len ; */
packet = malloc(total_len); /* packet = malloc ( total_len ) ; */
memcpy(packet, &total_len, 2); /* memcpy ( packet , & total_len , 2 ) ; */
memcpy(packet + 2, &flag, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(packet + 2 + 1, &sending_len, 1); /* memcpy ( packet + 2 + 1 , & sending_len , 1 ) ; */
memcpy(packet + 2 + 1 + 1, handle, sending_len); /* memcpy ( packet + 2 + 1 + 1 , handle , sending_len ) ; */
memcpy(packet + 2 + 1 + 1 + sending_len, text, txt_len + 1); /* memcpy ( packet + 2 + 1 + 1 + sending_len , text , txt_len + 1 ) ; */
sent = mySend(socket_fd, packet, total_len, 0); /* sent = mySend ( socket_fd , packet , total_len , 0 ) ; */
}
void handleList(char *input, int socket_fd, int send_len) /* void handleList ( char * input , int socket_fd , int send_len ) { */{
int sent; /* int sent ; */
char *packet; /* char * packet ; */
uint8_t flag = 10; /* uint8_t flag = 10 ; */
uint16_t packet_len = 3; /* uint16_t packet_len = 3 ; */
packet = malloc(packet_len); /* packet = malloc ( packet_len ) ; */
memcpy(packet, &packet_len, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(packet + 2, &flag, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
sent = mySend(socket_fd, packet, packet_len, 0); /* sent = mySend ( socket_fd , packet , packet_len , 0 ) ; */
}
void exitClient(char *input, int socket_fd, int send_len) /* void exitClient ( char * input , int socket_fd , int send_len ) { */{
uint16_t packet_len; /* uint16_t packet_len ; */
char *packet; /* char * packet ; */
uint8_t flag = 8; /* uint8_t flag = 8 ; */
int sent; /* int sent ; */
packet_len = 3; /* packet_len = 3 ; */
packet = malloc(packet_len); /* packet = malloc ( packet_len ) ; */
memcpy(packet, &packet_len, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(packet + 2, &flag, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
sent = mySend(socket_fd, packet, packet_len, 0); /* sent = mySend ( socket_fd , packet , packet_len , 0 ) ; */
}
void errorMessage(char *packet) /* void errorMessage ( char * packet ) { */{
char *invalid, *client; /* char * invalid , * client ; */
int invalid_len = *(packet + 3 ); /* int invalid_len = * ( packet + 3 ) ; */
client = packet + 3 + 1; /* client = packet + 3 + 1 ; */
invalid = malloc(invalid_len); /* invalid = malloc ( invalid_len ) ; */
memcpy(invalid, client, invalid_len); /* memcpy ( invalid , client , invalid_len ) ; */
fprintf(stderr, "\nClient with handle %s does not exist.\n", invalid); /* fprintf ( stderr , "\nClient with handle %s does not exist.\n" , invalid ) ; */
free(invalid); /* free ( invalid ) ; */
}
void printPrompt() /* void printPrompt ( ) { */{
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
