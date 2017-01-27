void receiveData(int client_socket); /* void receiveData(int client_socket) */
void initialPacket(char *packet, int client_socket); /* void initialPacket(char *packet, int client_socket) */
void sendMessage(char *packet, int client_socket); /* void sendMessage(char *packet, int client_socket) */
void destinationError(int client_socket, char *invalid, char *send_to, int send_to_len, char *send_from, int send_from_len); /* void destinationError(int client_socket, char *invalid, char *send_to, int send_to_len, char *send_from, int send_from_len) */
int compareClients(char *send_to, int send_to_len); /* int compareClients(char *send_to, int send_to_len) */
void selectCall(int server_socket); /* void selectCall(int server_socket) */
void broadcastMessage(char *packet, int client_socket); /* void broadcastMessage(char *packet, int client_socket) */
void listAllHandles(char *packet, int client_socket); /* void listAllHandles(char *packet, int client_socket) */
void sendHandle(int sending_index); /* void sendHandle(int sending_index) */
void exitClient(char *packet, int client_socket); /* void exitClient(char *packet, int client_socket) */
int global_e = 1; /* int variable_elements = 1 ; */
char **global_a; /* char * * handle_list ; */
int *global_d; /* int * handle_list_fds ; */
int global_c = 0; /* int num_handles_name = 0 ; */
int global_g = 0; /* int num_handles_fd = 0 ; */
int global_b = 3; /* int max_socket = 3 ; */
int global_f; /* int global_socket ; */
int global_h = 0; /* int list_flag = 0 ; */
int global_i = 0; /* int sending_index = 0 ; */
int global_j; /* int list_client ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
int c = tcpServerSetup(); /* int server_socket = tcpServerSetup ( ) ; */
global_a = malloc(global_e * sizeof(char *)); /* handle_list = malloc ( variable_elements * sizeof ( char * ) ) ; */
global_d = malloc(global_e * sizeof(char)); /* handle_list_fds = malloc ( variable_elements * sizeof ( char ) ) ; */
global_f = c; /* global_socket = server_socket ; */
selectCall(c); /* selectCall ( server_socket ) ; */
close(c); /* close ( server_socket ) ; */
return 0; /* return 0 */
}
void selectCall(int a) /* void selectCall ( int server_socket ) { */{
int b; /* int client_socket ; */
fd_set c; /* fd_set fdvar ; */
FD_ZERO(&c); /* FD_ZERO ( & fdvar ) ; */
FD_SET(a, &c); /* FD_SET ( server_socket , & fdvar ) ; */
global_d[global_g] = a; /* handle_list_fds [ num_handles_fd ] = server_socket ; */
global_g++; /* num_handles_fd ++ ; */
while (mySelect(global_b + 1, (fd_set *)&c, (fd_set *)0, (fd_set *)0, ((void *)0 ))) /* while (mySelect(max_socket + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, ((void *)0 ))) */ {
int d; /* int i ; */
for (d = 0; d<global_g; d++) {
int e = global_d[d]; /* int current_fd = handle_list_fds [ i ] ; */
if (FD_ISSET(e, &c)) /* if (FD_ISSET(current_fd, &fdvar)) */ {
if (e == a) /* if (current_fd == server_socket) */ {
b = tcpAccept(a); /* client_socket = tcpAccept ( server_socket ) ; */
FD_SET(b, &c); /* FD_SET ( client_socket , & fdvar ) ; */
global_d[global_g] = b; /* handle_list_fds [ num_handles_fd ] = client_socket ; */
global_g++; /* num_handles_fd ++ ; */
if (b>global_b) /* if (client_socket>max_socket) */ {
global_b = b; /* max_socket = client_socket ; */
}
}
receiveData(e); /* receiveData ( current_fd ) ; */
}
}
FD_ZERO(&c); /* FD_ZERO ( & fdvar ) ; */
for (d = 0; d<global_g; d++) {
FD_SET(global_d[d], &c); /* FD_SET ( handle_list_fds [ i ] , & fdvar ) ; */
}
}
}
void receiveData(int a) /* void receiveData ( int client_socket ) { */{
int b; /* int message_len ; */
char *c = ((void *)0 ); /* char * packet = ( ( void * ) 0 ) ; */
uint16_t d; /* uint16_t packet_len ; */
uint8_t e; /* uint8_t flag ; */
c = malloc(1024); /* packet = malloc ( 1024 ) ; */
b = myRecv(a, c, 1024, 0); /* message_len = myRecv ( client_socket , packet , 1024 , 0 ) ; */
if (!global_h) /* if (!list_flag) */ {
d = *(uint16_t *)c; /* packet_len = * ( uint16_t * ) packet ; */
e = *((uint8_t *)(c + 2 ) ); /* flag = * ( ( uint8_t * ) ( packet + 2 ) ) ; */
switch (e) /* switch (flag) */ {
case 1: 
initialPacket(c, a); /* initialPacket ( packet , client_socket ) ; */
break;
case 5: 
sendMessage(c, a); /* sendMessage ( packet , client_socket ) ; */
break;
case 4: 
broadcastMessage(c, a); /* broadcastMessage ( packet , client_socket ) ; */
break;
case 8: 
exitClient(c, a); /* exitClient ( packet , client_socket ) ; */
break;
case 10: 
listAllHandles(c, a); /* listAllHandles ( packet , client_socket ) ; */
break;
}
} else {
if (global_i == global_c) /* if (sending_index == num_handles_name) */ {
global_i = 0; /* sending_index = 0 ; */
global_h = 0; /* list_flag = 0 ; */
} else {
sendHandle(global_i); /* sendHandle ( sending_index ) ; */
}
}
}
void exitClient(char *a, int b) /* void exitClient ( char * packet , int client_socket ) { */{
char *c; /* char * ret_packet ; */
uint16_t d = 3; /* uint16_t packet_len = 3 ; */
uint8_t e = 9; /* uint8_t flag = 9 ; */
int f; /* int sent ; */
c = malloc(3); /* ret_packet = malloc ( 3 ) ; */
memcpy(c, &d, 2); /* memcpy ( ret_packet , & packet_len , 2 ) ; */
memcpy(c + 2, &e, 1); /* memcpy ( ret_packet + 2 , & flag , 1 ) ; */
int g, h = -1; /* int i , index = - 1 ; */
for (g = 0; g<global_g; g++) {
if (global_d[g] == b) /* if (handle_list_fds[i] == client_socket) */ {
h = g; /* index = i ; */
}
}
for (g = h; g<global_g - 1; g++) {
global_d[g] = global_d[g + 1]; /* handle_list_fds [ i ] = handle_list_fds [ i + 1 ] ; */
}
for (g = h - 1; g<global_c - 1; g++) {
global_a[g] = global_a[g + 1]; /* handle_list [ i ] = handle_list [ i + 1 ] ; */
}
global_c--; /* num_handles_name -- ; */
global_g--; /* num_handles_fd -- ; */
f = mySend(b, c, d, 0); /* sent = mySend ( client_socket , ret_packet , packet_len , 0 ) ; */
}
void listAllHandles(char *a, int b) /* void listAllHandles ( char * packet , int client_socket ) { */{
char *c; /* char * ret_packet ; */
uint16_t d; /* uint16_t packet_len ; */
uint8_t e = 11; /* uint8_t flag = 11 ; */
int f; /* int sent ; */
d = 3 + 4; /* packet_len = 3 + 4 ; */
c = malloc(d); /* ret_packet = malloc ( packet_len ) ; */
memcpy(c, &d, 2); /* memcpy ( ret_packet , & packet_len , 2 ) ; */
memcpy(c + 2, &e, 1); /* memcpy ( ret_packet + 2 , & flag , 1 ) ; */
memcpy(c + 3, &global_c, 4); /* memcpy ( ret_packet + 3 , & num_handles_name , 4 ) ; */
f = mySend(b, c, d, 0); /* sent = mySend ( client_socket , ret_packet , packet_len , 0 ) ; */
free(c); /* free ( ret_packet ) ; */
global_h = 1; /* list_flag = 1 ; */
global_j = b; /* list_client = client_socket ; */
}
void sendHandle(int a) /* void sendHandle ( int handle_index ) { */{
char *b, *c; /* char * ret_packet , * handle ; */
uint8_t d, e = 12; /* uint8_t handle_len , flag = 12 ; */
uint16_t f; /* uint16_t packet_len ; */
int g; /* int sent ; */
c = global_a[a]; /* handle = handle_list [ handle_index ] ; */
d = strlen(global_a[a]); /* handle_len = strlen ( handle_list [ handle_index ] ) ; */
f = 3 + 1 + d; /* packet_len = 3 + 1 + handle_len ; */
b = malloc(f); /* ret_packet = malloc ( packet_len ) ; */
memcpy(b, &f, 2); /* memcpy ( ret_packet , & packet_len , 2 ) ; */
memcpy(b + 2, &e, 1); /* memcpy ( ret_packet + 2 , & flag , 1 ) ; */
memcpy(b + 3, &d, 1); /* memcpy ( ret_packet + 3 , & handle_len , 1 ) ; */
memcpy(b + 3 + 1, c, d); /* memcpy ( ret_packet + 3 + 1 , handle , handle_len ) ; */
g = mySend(global_j, b, f, 0); /* sent = mySend ( list_client , ret_packet , packet_len , 0 ) ; */
free(b); /* free ( ret_packet ) ; */
global_i++; /* sending_index ++ ; */
}
void initialPacket(char *a, int b) /* void initialPacket ( char * packet , int client_socket ) { */{
char *c = malloc(3), *d; /* char * ret_packet = malloc ( 3 ) , * cmp2 ; */
char e = *(a + 3 ); /* char handle_length = * ( packet + 3 ) ; */
char *f = a + 4; /* char * handle = packet + 4 ; */
int g, h, i; /* int packet_len , flag , sent ; */
f = strtok(f, " "); /* handle = strtok ( handle , " " ) ; */
g = 3; /* packet_len = 3 ; */
h = 2; /* flag = 2 ; */
int j; /* int i ; */
for (j = 0; j<global_c; j++) {
d = malloc(e); /* cmp2 = malloc ( handle_length ) ; */
memcpy(d, f, e); /* memcpy ( cmp2 , handle , handle_length ) ; */
if (strcmp(global_a[j], d) == 0) /* if (strcmp(handle_list[i], cmp2) == 0) */ {
h = 3; /* flag = 3 ; */
}
free(d); /* free ( cmp2 ) ; */
}
if (h == 2) /* if (flag == 2) */ {
if (global_c == global_e) /* if (num_handles_name == variable_elements) */ {
global_e *= 2; /* variable_elements *= 2 ; */
char *k = realloc(global_a, global_e); /* char * tmp = realloc ( handle_list , variable_elements ) ; */
if (k == ((void *)0 )) /* if (tmp == ((void *)0 )) */ {
printf("Error reallocating memory!\n"); /* printf ( "Error reallocating memory!\n" ) ; */
}
int *l = malloc(global_e); /* int * tmp_arr = malloc ( variable_elements ) ; */
int m; /* int i ; */
for (m = 0; m<global_g; m++) {
l[m] = global_d[m]; /* tmp_arr [ i ] = handle_list_fds [ i ] ; */
}
global_d = l; /* handle_list_fds = tmp_arr ; */
}
global_a[global_c] = malloc(e); /* handle_list [ num_handles_name ] = malloc ( handle_length ) ; */
memcpy(global_a[global_c], f, e); /* memcpy ( handle_list [ num_handles_name ] , handle , handle_length ) ; */
global_c++; /* num_handles_name ++ ; */
}
memcpy(c, &g, 2); /* memcpy ( ret_packet , & packet_len , 2 ) ; */
memcpy(c + 2, &h, 1); /* memcpy ( ret_packet + 2 , & flag , 1 ) ; */
i = mySend(b, c, 3, 0); /* sent = mySend ( client_socket , ret_packet , 3 , 0 ) ; */
}
void sendMessage(char *a, int b) /* void sendMessage ( char * packet , int client_socket ) { */{
char *c, *d, *e, *f; /* char * send_to , * send_from , * text , * invalid ; */
uint8_t g, h, i; /* uint8_t send_to_len , send_from_len , flag ; */
int j = -1, k, l, m; /* int fd_index = - 1 , fd_send , sent , text_len ; */
uint16_t n; /* uint16_t packet_len ; */
g = *(a + 3 ); /* send_to_len = * ( packet + 3 ) ; */
c = a + 3 + 1; /* send_to = packet + 3 + 1 ; */
h = *(a + 3 + 1 + g ); /* send_from_len = * ( packet + 3 + 1 + send_to_len ) ; */
d = a + 3 + 1 + g + 1; /* send_from = packet + 3 + 1 + send_to_len + 1 ; */
e = a + 3 + 1 + g + 1 + h; /* text = packet + 3 + 1 + send_to_len + 1 + send_from_len ; */
m = *(uint16_t *)a - 3 - 1 - g - 1 - h; /* text_len = * ( uint16_t * ) packet - 3 - 1 - send_to_len - 1 - send_from_len ; */
if (strlen(e) == 0) /* if (strlen(text) == 0) */ {
e = malloc(1); /* text = malloc ( 1 ) ; */
memcpy(e, " ", 1); /* memcpy ( text , " " , 1 ) ; */
m = 1; /* text_len = 1 ; */
}
f = malloc(g); /* invalid = malloc ( send_to_len ) ; */
memcpy(f, c, g); /* memcpy ( invalid , send_to , send_to_len ) ; */
j = compareClients(c, g); /* fd_index = compareClients ( send_to , send_to_len ) ; */
i = 5; /* flag = 5 ; */
n = 3 + 1 + h + m; /* packet_len = 3 + 1 + send_from_len + text_len ; */
a = malloc(n); /* packet = malloc ( packet_len ) ; */
memcpy(a, &n, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(a + 2, &i, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(a + 2 + 1, &h, 1); /* memcpy ( packet + 2 + 1 , & send_from_len , 1 ) ; */
memcpy(a + 2 + 1 + 1, d, h); /* memcpy ( packet + 2 + 1 + 1 , send_from , send_from_len ) ; */
memcpy(a + 2 + 1 + 1 + h, e, m); /* memcpy ( packet + 2 + 1 + 1 + send_from_len , text , text_len ) ; */
if (j != -1) /* if (fd_index != -1) */ {
k = global_d[j + 1]; /* fd_send = handle_list_fds [ fd_index + 1 ] ; */
l = mySend(k, a, n, 0); /* sent = mySend ( fd_send , packet , packet_len , 0 ) ; */
} else {
destinationError(b, f, c, g, d, h); /* destinationError ( client_socket , invalid , send_to , send_to_len , send_from , send_from_len ) ; */
}
free(f); /* free ( invalid ) ; */
}
int compareClients(char *a, int b) /* int compareClients ( char * send_to , int send_to_len ) { */{
int c, d = -1, e, f; /* int i , fd_index = - 1 , cmp , handle_len ; */
char *g, *h; /* char * hand , * tmp ; */
for (c = 0; c<global_c; c++) {
f = strlen(global_a[c]); /* handle_len = strlen ( handle_list [ i ] ) ; */
g = malloc(b); /* hand = malloc ( send_to_len ) ; */
h = malloc(b); /* tmp = malloc ( send_to_len ) ; */
memcpy(g, a, b); /* memcpy ( hand , send_to , send_to_len ) ; */
memcpy(h, global_a[c], f); /* memcpy ( tmp , handle_list [ i ] , handle_len ) ; */
e = strcmp(h, g); /* cmp = strcmp ( tmp , hand ) ; */
if (e == 0) /* if (cmp == 0) */ {
return c; /* return i */
}
}
return d; /* return fd_index */
}
void broadcastMessage(char *a, int b) /* void broadcastMessage ( char * packet , int client_socket ) { */{
char *c, *d, *e; /* char * packet_new , * send_from , * text ; */
uint16_t f, g, h; /* uint16_t packet_len , text_len , packet_new_len ; */
uint8_t i, j = 5; /* uint8_t send_from_len , flag = 5 ; */
f = *((uint16_t *)a ); /* packet_len = * ( ( uint16_t * ) packet ) ; */
i = *((uint8_t *)(a + 3 ) ); /* send_from_len = * ( ( uint8_t * ) ( packet + 3 ) ) ; */
d = a + 3 + 1; /* send_from = packet + 3 + 1 ; */
e = a + 3 + 1 + i; /* text = packet + 3 + 1 + send_from_len ; */
g = f - 3 - 1 - i; /* text_len = packet_len - 3 - 1 - send_from_len ; */
if (strlen(e) == 0) /* if (strlen(text) == 0) */ {
e = malloc(1); /* text = malloc ( 1 ) ; */
memcpy(e, " ", 1); /* memcpy ( text , " " , 1 ) ; */
g = 1; /* text_len = 1 ; */
}
h = 3 + 1 + i + g; /* packet_new_len = 3 + 1 + send_from_len + text_len ; */
c = malloc(h); /* packet_new = malloc ( packet_new_len ) ; */
memcpy(c, &h, 2); /* memcpy ( packet_new , & packet_new_len , 2 ) ; */
memcpy(c + 2, &j, 1); /* memcpy ( packet_new + 2 , & flag , 1 ) ; */
memcpy(c + 3, &i, 1); /* memcpy ( packet_new + 3 , & send_from_len , 1 ) ; */
memcpy(c + 3 + 1, d, i); /* memcpy ( packet_new + 3 + 1 , send_from , send_from_len ) ; */
memcpy(c + 3 + 1 + i, e, g); /* memcpy ( packet_new + 3 + 1 + send_from_len , text , text_len ) ; */
int k, l, m; /* int i , cur_fd , sent ; */
for (k = 0; k<global_g; k++) {
l = global_d[k]; /* cur_fd = handle_list_fds [ i ] ; */
if (l != global_f&&l != b) /* if (cur_fd != global_socket&&cur_fd != client_socket) */ {
m = mySend(l, c, f, 0); /* sent = mySend ( cur_fd , packet_new , packet_len , 0 ) ; */
}
}
}
void destinationError(int a, char *b, char *c, int d, char *e, int f) /* void destinationError ( int client_socket , char * invalid , char * send_to , int send_to_len , char * send_from , int send_from_len ) { */{
char *g; /* char * packet ; */
uint16_t h; /* uint16_t packet_len ; */
uint8_t i = 7, j = d; /* uint8_t flag = 7 , len = send_to_len ; */
int k; /* int sent ; */
h = 3 + 1 + d; /* packet_len = 3 + 1 + send_to_len ; */
g = malloc(h); /* packet = malloc ( packet_len ) ; */
memcpy(g, &h, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(g + 2, &i, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(g + 2 + 1, &j, 1); /* memcpy ( packet + 2 + 1 , & len , 1 ) ; */
memcpy(g + 2 + 1 + 1, c, j); /* memcpy ( packet + 2 + 1 + 1 , send_to , len ) ; */
k = mySend(a, g, h, 0); /* sent = mySend ( client_socket , packet , packet_len , 0 ) ; */
}
