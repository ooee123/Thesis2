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
int variable_elements = 1; /* int variable_elements = 1 ; */
char **handle_list; /* char * * handle_list ; */
int *handle_list_fds; /* int * handle_list_fds ; */
int num_handles_name = 0; /* int num_handles_name = 0 ; */
int num_handles_fd = 0; /* int num_handles_fd = 0 ; */
int max_socket = 3; /* int max_socket = 3 ; */
int global_socket; /* int global_socket ; */
int list_flag = 0; /* int list_flag = 0 ; */
int sending_index = 0; /* int sending_index = 0 ; */
int list_client; /* int list_client ; */

unsigned int __bswap_32(unsigned int __bsx) /*  */{
return __builtin_bswap32(__bsx); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t __bsx) /*  */{
return __builtin_bswap64(__bsx); /* return __builtin_bswap64(__bsx) */
}
int main(int argc, char *argv) /* int main ( int argc , char * argv [ ] ) { */{
int server_socket = tcpServerSetup(); /* int server_socket = tcpServerSetup ( ) ; */
handle_list = malloc(variable_elements * sizeof(char *)); /* handle_list = malloc ( variable_elements * sizeof ( char * ) ) ; */
handle_list_fds = malloc(variable_elements * sizeof(char)); /* handle_list_fds = malloc ( variable_elements * sizeof ( char ) ) ; */
global_socket = server_socket; /* global_socket = server_socket ; */
selectCall(server_socket); /* selectCall ( server_socket ) ; */
close(server_socket); /* close ( server_socket ) ; */
return 0; /* return 0 */
}
void selectCall(int server_socket) /* void selectCall ( int server_socket ) { */{
int client_socket; /* int client_socket ; */
fd_set fdvar; /* fd_set fdvar ; */
FD_ZERO(&fdvar); /* FD_ZERO ( & fdvar ) ; */
FD_SET(server_socket, &fdvar); /* FD_SET ( server_socket , & fdvar ) ; */
handle_list_fds[num_handles_fd] = server_socket; /* handle_list_fds [ num_handles_fd ] = server_socket ; */
num_handles_fd++; /* num_handles_fd ++ ; */
while (mySelect(max_socket + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, ((void *)0 ))) /* while (mySelect(max_socket + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, ((void *)0 ))) */ {
int i; /* int i ; */
for (i = 0; i<num_handles_fd; i++) {
int current_fd = handle_list_fds[i]; /* int current_fd = handle_list_fds [ i ] ; */
if (FD_ISSET(current_fd, &fdvar)) /* if (FD_ISSET(current_fd, &fdvar)) */ {
if (current_fd == server_socket) /* if (current_fd == server_socket) */ {
client_socket = tcpAccept(server_socket); /* client_socket = tcpAccept ( server_socket ) ; */
FD_SET(client_socket, &fdvar); /* FD_SET ( client_socket , & fdvar ) ; */
handle_list_fds[num_handles_fd] = client_socket; /* handle_list_fds [ num_handles_fd ] = client_socket ; */
num_handles_fd++; /* num_handles_fd ++ ; */
if (client_socket>max_socket) /* if (client_socket>max_socket) */ {
max_socket = client_socket; /* max_socket = client_socket ; */
}
}
receiveData(current_fd); /* receiveData ( current_fd ) ; */
}
}
FD_ZERO(&fdvar); /* FD_ZERO ( & fdvar ) ; */
for (i = 0; i<num_handles_fd; i++) {
FD_SET(handle_list_fds[i], &fdvar); /* FD_SET ( handle_list_fds [ i ] , & fdvar ) ; */
}
}
}
void receiveData(int client_socket) /* void receiveData ( int client_socket ) { */{
uint16_t packet_len; /* uint16_t packet_len ; */
int message_len; /* int message_len ; */
uint8_t flag; /* uint8_t flag ; */
char *packet = ((void *)0 ); /* char * packet = ( ( void * ) 0 ) ; */
packet = malloc(1024); /* packet = malloc ( 1024 ) ; */
message_len = myRecv(client_socket, packet, 1024, 0); /* message_len = myRecv ( client_socket , packet , 1024 , 0 ) ; */
if (!list_flag) /* if (!list_flag) */ {
flag = *((uint8_t *)(packet + 2 ) ); /* flag = * ( ( uint8_t * ) ( packet + 2 ) ) ; */
switch (flag) /* switch (flag) */ {
case 1: 
initialPacket(packet, client_socket); /* initialPacket ( packet , client_socket ) ; */
break;
case 5: 
sendMessage(packet, client_socket); /* sendMessage ( packet , client_socket ) ; */
break;
case 4: 
broadcastMessage(packet, client_socket); /* broadcastMessage ( packet , client_socket ) ; */
break;
case 8: 
exitClient(packet, client_socket); /* exitClient ( packet , client_socket ) ; */
break;
case 10: 
listAllHandles(packet, client_socket); /* listAllHandles ( packet , client_socket ) ; */
break;
}
} else {
if (sending_index == num_handles_name) /* if (sending_index == num_handles_name) */ {
sending_index = 0; /* sending_index = 0 ; */
list_flag = 0; /* list_flag = 0 ; */
} else {
sendHandle(sending_index); /* sendHandle ( sending_index ) ; */
}
}
}
void exitClient(char *packet, int client_socket) /* void exitClient ( char * packet , int client_socket ) { */{
int i, index = -1; /* int i , index = - 1 ; */
int sent; /* int sent ; */
uint8_t flag = 9; /* uint8_t flag = 9 ; */
uint16_t packet_len = 3; /* uint16_t packet_len = 3 ; */
char *ret_packet; /* char * ret_packet ; */
ret_packet = malloc(3); /* ret_packet = malloc ( 3 ) ; */
memcpy(ret_packet, &packet_len, 2); /* memcpy ( ret_packet , & packet_len , 2 ) ; */
memcpy(ret_packet + 2, &flag, 1); /* memcpy ( ret_packet + 2 , & flag , 1 ) ; */
for (i = 0; i<num_handles_fd; i++) {
if (handle_list_fds[i] == client_socket) /* if (handle_list_fds[i] == client_socket) */ {
index = i; /* index = i ; */
}
}
for (i = index; i<num_handles_fd - 1; i++) {
handle_list_fds[i] = handle_list_fds[i + 1]; /* handle_list_fds [ i ] = handle_list_fds [ i + 1 ] ; */
}
num_handles_fd--; /* num_handles_fd -- ; */
for (i = index - 1; i<num_handles_name - 1; i++) {
handle_list[i] = handle_list[i + 1]; /* handle_list [ i ] = handle_list [ i + 1 ] ; */
}
num_handles_name--; /* num_handles_name -- ; */
sent = mySend(client_socket, ret_packet, packet_len, 0); /* sent = mySend ( client_socket , ret_packet , packet_len , 0 ) ; */
}
void listAllHandles(char *packet, int client_socket) /* void listAllHandles ( char * packet , int client_socket ) { */{
int sent; /* int sent ; */
char *ret_packet; /* char * ret_packet ; */
uint16_t packet_len; /* uint16_t packet_len ; */
uint8_t flag = 11; /* uint8_t flag = 11 ; */
list_flag = 1; /* list_flag = 1 ; */
list_client = client_socket; /* list_client = client_socket ; */
packet_len = 3 + 4; /* packet_len = 3 + 4 ; */
ret_packet = malloc(packet_len); /* ret_packet = malloc ( packet_len ) ; */
memcpy(ret_packet, &packet_len, 2); /* memcpy ( ret_packet , & packet_len , 2 ) ; */
memcpy(ret_packet + 2, &flag, 1); /* memcpy ( ret_packet + 2 , & flag , 1 ) ; */
memcpy(ret_packet + 3, &num_handles_name, 4); /* memcpy ( ret_packet + 3 , & num_handles_name , 4 ) ; */
sent = mySend(client_socket, ret_packet, packet_len, 0); /* sent = mySend ( client_socket , ret_packet , packet_len , 0 ) ; */
free(ret_packet); /* free ( ret_packet ) ; */
}
void sendHandle(int handle_index) /* void sendHandle ( int handle_index ) { */{
uint8_t handle_len, flag = 12; /* uint8_t handle_len , flag = 12 ; */
int sent; /* int sent ; */
uint16_t packet_len; /* uint16_t packet_len ; */
char *ret_packet, *handle; /* char * ret_packet , * handle ; */
sending_index++; /* sending_index ++ ; */
handle = handle_list[handle_index]; /* handle = handle_list [ handle_index ] ; */
handle_len = strlen(handle_list[handle_index]); /* handle_len = strlen ( handle_list [ handle_index ] ) ; */
packet_len = 3 + 1 + handle_len; /* packet_len = 3 + 1 + handle_len ; */
ret_packet = malloc(packet_len); /* ret_packet = malloc ( packet_len ) ; */
memcpy(ret_packet, &packet_len, 2); /* memcpy ( ret_packet , & packet_len , 2 ) ; */
memcpy(ret_packet + 2, &flag, 1); /* memcpy ( ret_packet + 2 , & flag , 1 ) ; */
memcpy(ret_packet + 3, &handle_len, 1); /* memcpy ( ret_packet + 3 , & handle_len , 1 ) ; */
memcpy(ret_packet + 3 + 1, handle, handle_len); /* memcpy ( ret_packet + 3 + 1 , handle , handle_len ) ; */
sent = mySend(list_client, ret_packet, packet_len, 0); /* sent = mySend ( list_client , ret_packet , packet_len , 0 ) ; */
free(ret_packet); /* free ( ret_packet ) ; */
}
void initialPacket(char *packet, int client_socket) /* void initialPacket ( char * packet , int client_socket ) { */{
int packet_len, flag, sent; /* int packet_len , flag , sent ; */
int i; /* int i ; */
char *ret_packet = malloc(3), *cmp2; /* char * ret_packet = malloc ( 3 ) , * cmp2 ; */
char *handle = packet + 4; /* char * handle = packet + 4 ; */
char handle_length = *(packet + 3 ); /* char handle_length = * ( packet + 3 ) ; */
flag = 2; /* flag = 2 ; */
packet_len = 3; /* packet_len = 3 ; */
handle = strtok(handle, " "); /* handle = strtok ( handle , " " ) ; */
for (i = 0; i<num_handles_name; i++) {
cmp2 = malloc(handle_length); /* cmp2 = malloc ( handle_length ) ; */
memcpy(cmp2, handle, handle_length); /* memcpy ( cmp2 , handle , handle_length ) ; */
if (strcmp(handle_list[i], cmp2) == 0) /* if (strcmp(handle_list[i], cmp2) == 0) */ {
flag = 3; /* flag = 3 ; */
}
free(cmp2); /* free ( cmp2 ) ; */
}
if (flag == 2) /* if (flag == 2) */ {
if (num_handles_name == variable_elements) /* if (num_handles_name == variable_elements) */ {
int i; /* int i ; */
variable_elements *= 2; /* variable_elements *= 2 ; */
char *tmp = realloc(handle_list, variable_elements); /* char * tmp = realloc ( handle_list , variable_elements ) ; */
if (tmp == ((void *)0 )) /* if (tmp == ((void *)0 )) */ {
printf("Error reallocating memory!\n"); /* printf ( "Error reallocating memory!\n" ) ; */
}
int *tmp_arr = malloc(variable_elements); /* int * tmp_arr = malloc ( variable_elements ) ; */
for (i = 0; i<num_handles_fd; i++) {
tmp_arr[i] = handle_list_fds[i]; /* tmp_arr [ i ] = handle_list_fds [ i ] ; */
}
handle_list_fds = tmp_arr; /* handle_list_fds = tmp_arr ; */
}
handle_list[num_handles_name] = malloc(handle_length); /* handle_list [ num_handles_name ] = malloc ( handle_length ) ; */
memcpy(handle_list[num_handles_name], handle, handle_length); /* memcpy ( handle_list [ num_handles_name ] , handle , handle_length ) ; */
num_handles_name++; /* num_handles_name ++ ; */
}
memcpy(ret_packet, &packet_len, 2); /* memcpy ( ret_packet , & packet_len , 2 ) ; */
memcpy(ret_packet + 2, &flag, 1); /* memcpy ( ret_packet + 2 , & flag , 1 ) ; */
sent = mySend(client_socket, ret_packet, 3, 0); /* sent = mySend ( client_socket , ret_packet , 3 , 0 ) ; */
}
void sendMessage(char *packet, int client_socket) /* void sendMessage ( char * packet , int client_socket ) { */{
uint16_t packet_len; /* uint16_t packet_len ; */
uint8_t send_to_len, send_from_len, flag; /* uint8_t send_to_len , send_from_len , flag ; */
int fd_index = -1, fd_send, sent, text_len; /* int fd_index = - 1 , fd_send , sent , text_len ; */
char *send_to, *send_from, *text, *invalid; /* char * send_to , * send_from , * text , * invalid ; */
flag = 5; /* flag = 5 ; */
send_to_len = *(packet + 3 ); /* send_to_len = * ( packet + 3 ) ; */
send_from = packet + 3 + 1 + send_to_len + 1; /* send_from = packet + 3 + 1 + send_to_len + 1 ; */
send_to = packet + 3 + 1; /* send_to = packet + 3 + 1 ; */
send_from_len = *(packet + 3 + 1 + send_to_len ); /* send_from_len = * ( packet + 3 + 1 + send_to_len ) ; */
text = packet + 3 + 1 + send_to_len + 1 + send_from_len; /* text = packet + 3 + 1 + send_to_len + 1 + send_from_len ; */
text_len = *(uint16_t *)packet - 3 - 1 - send_to_len - 1 - send_from_len; /* text_len = * ( uint16_t * ) packet - 3 - 1 - send_to_len - 1 - send_from_len ; */
if (strlen(text) == 0) /* if (strlen(text) == 0) */ {
text_len = 1; /* text_len = 1 ; */
text = malloc(1); /* text = malloc ( 1 ) ; */
memcpy(text, " ", 1); /* memcpy ( text , " " , 1 ) ; */
}
invalid = malloc(send_to_len); /* invalid = malloc ( send_to_len ) ; */
memcpy(invalid, send_to, send_to_len); /* memcpy ( invalid , send_to , send_to_len ) ; */
fd_index = compareClients(send_to, send_to_len); /* fd_index = compareClients ( send_to , send_to_len ) ; */
packet_len = 3 + 1 + send_from_len + text_len; /* packet_len = 3 + 1 + send_from_len + text_len ; */
packet = malloc(packet_len); /* packet = malloc ( packet_len ) ; */
memcpy(packet, &packet_len, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(packet + 2, &flag, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(packet + 2 + 1, &send_from_len, 1); /* memcpy ( packet + 2 + 1 , & send_from_len , 1 ) ; */
memcpy(packet + 2 + 1 + 1, send_from, send_from_len); /* memcpy ( packet + 2 + 1 + 1 , send_from , send_from_len ) ; */
memcpy(packet + 2 + 1 + 1 + send_from_len, text, text_len); /* memcpy ( packet + 2 + 1 + 1 + send_from_len , text , text_len ) ; */
if (fd_index != -1) /* if (fd_index != -1) */ {
fd_send = handle_list_fds[fd_index + 1]; /* fd_send = handle_list_fds [ fd_index + 1 ] ; */
sent = mySend(fd_send, packet, packet_len, 0); /* sent = mySend ( fd_send , packet , packet_len , 0 ) ; */
} else {
destinationError(client_socket, invalid, send_to, send_to_len, send_from, send_from_len); /* destinationError ( client_socket , invalid , send_to , send_to_len , send_from , send_from_len ) ; */
}
free(invalid); /* free ( invalid ) ; */
}
int compareClients(char *send_to, int send_to_len) /* int compareClients ( char * send_to , int send_to_len ) { */{
char *hand, *tmp; /* char * hand , * tmp ; */
int i, fd_index = -1, cmp, handle_len; /* int i , fd_index = - 1 , cmp , handle_len ; */
for (i = 0; i<num_handles_name; i++) {
handle_len = strlen(handle_list[i]); /* handle_len = strlen ( handle_list [ i ] ) ; */
hand = malloc(send_to_len); /* hand = malloc ( send_to_len ) ; */
tmp = malloc(send_to_len); /* tmp = malloc ( send_to_len ) ; */
memcpy(hand, send_to, send_to_len); /* memcpy ( hand , send_to , send_to_len ) ; */
memcpy(tmp, handle_list[i], handle_len); /* memcpy ( tmp , handle_list [ i ] , handle_len ) ; */
cmp = strcmp(tmp, hand); /* cmp = strcmp ( tmp , hand ) ; */
if (cmp == 0) /* if (cmp == 0) */ {
return i; /* return i */
}
}
return fd_index; /* return fd_index */
}
void broadcastMessage(char *packet, int client_socket) /* void broadcastMessage ( char * packet , int client_socket ) { */{
int i, cur_fd, sent; /* int i , cur_fd , sent ; */
char *packet_new, *send_from, *text; /* char * packet_new , * send_from , * text ; */
uint8_t send_from_len, flag = 5; /* uint8_t send_from_len , flag = 5 ; */
uint16_t packet_len, text_len, packet_new_len; /* uint16_t packet_len , text_len , packet_new_len ; */
packet_len = *((uint16_t *)packet ); /* packet_len = * ( ( uint16_t * ) packet ) ; */
send_from_len = *((uint8_t *)(packet + 3 ) ); /* send_from_len = * ( ( uint8_t * ) ( packet + 3 ) ) ; */
text_len = packet_len - 3 - 1 - send_from_len; /* text_len = packet_len - 3 - 1 - send_from_len ; */
text = packet + 3 + 1 + send_from_len; /* text = packet + 3 + 1 + send_from_len ; */
send_from = packet + 3 + 1; /* send_from = packet + 3 + 1 ; */
if (strlen(text) == 0) /* if (strlen(text) == 0) */ {
text_len = 1; /* text_len = 1 ; */
text = malloc(1); /* text = malloc ( 1 ) ; */
memcpy(text, " ", 1); /* memcpy ( text , " " , 1 ) ; */
}
packet_new_len = 3 + 1 + send_from_len + text_len; /* packet_new_len = 3 + 1 + send_from_len + text_len ; */
packet_new = malloc(packet_new_len); /* packet_new = malloc ( packet_new_len ) ; */
memcpy(packet_new, &packet_new_len, 2); /* memcpy ( packet_new , & packet_new_len , 2 ) ; */
memcpy(packet_new + 2, &flag, 1); /* memcpy ( packet_new + 2 , & flag , 1 ) ; */
memcpy(packet_new + 3, &send_from_len, 1); /* memcpy ( packet_new + 3 , & send_from_len , 1 ) ; */
memcpy(packet_new + 3 + 1, send_from, send_from_len); /* memcpy ( packet_new + 3 + 1 , send_from , send_from_len ) ; */
memcpy(packet_new + 3 + 1 + send_from_len, text, text_len); /* memcpy ( packet_new + 3 + 1 + send_from_len , text , text_len ) ; */
for (i = 0; i<num_handles_fd; i++) {
cur_fd = handle_list_fds[i]; /* cur_fd = handle_list_fds [ i ] ; */
if (cur_fd != global_socket&&cur_fd != client_socket) /* if (cur_fd != global_socket&&cur_fd != client_socket) */ {
sent = mySend(cur_fd, packet_new, packet_len, 0); /* sent = mySend ( cur_fd , packet_new , packet_len , 0 ) ; */
}
}
}
void destinationError(int client_socket, char *invalid, char *send_to, int send_to_len, char *send_from, int send_from_len) /* void destinationError ( int client_socket , char * invalid , char * send_to , int send_to_len , char * send_from , int send_from_len ) { */{
int sent; /* int sent ; */
char *packet; /* char * packet ; */
uint16_t packet_len; /* uint16_t packet_len ; */
uint8_t flag = 7, len = send_to_len; /* uint8_t flag = 7 , len = send_to_len ; */
packet_len = 3 + 1 + send_to_len; /* packet_len = 3 + 1 + send_to_len ; */
packet = malloc(packet_len); /* packet = malloc ( packet_len ) ; */
memcpy(packet, &packet_len, 2); /* memcpy ( packet , & packet_len , 2 ) ; */
memcpy(packet + 2, &flag, 1); /* memcpy ( packet + 2 , & flag , 1 ) ; */
memcpy(packet + 2 + 1, &len, 1); /* memcpy ( packet + 2 + 1 , & len , 1 ) ; */
memcpy(packet + 2 + 1 + 1, send_to, len); /* memcpy ( packet + 2 + 1 + 1 , send_to , len ) ; */
sent = mySend(client_socket, packet, packet_len, 0); /* sent = mySend ( client_socket , packet , packet_len , 0 ) ; */
}
