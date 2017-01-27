uint16_t global_b; /* static uint16_t port ; */
int global_d; /* static int server_socket ; */
c_handle *global_a; /* static c_handle * clients ; */
uint16_t global_c; /* static uint16_t num_clients ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void print_usage(char **a) /* static void print_usage ( const char * * argv ) { */{
fprintf(stderr, "usage: %s [port]\n", *a); /* fprintf ( stderr , "usage: %s [port]\n" , * argv ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
void parse_args(int a, char **b) /* static void parse_args ( int argc , const char * * argv ) { */{
if (a>2) /* if (argc>2) */ {
print_usage(b); /* print_usage ( argv ) ; */
}
if (a == 2) /* if (argc == 2) */ {
global_b = strtol(*(b + 1 ), ((void *)0 ), 10); /* port = strtol ( * ( argv + 1 ) , ( ( void * ) 0 ) , 10 ) ; */
if (!global_b) /* if (!port) */ {
fprintf(stderr, "error: invalid port number\n"); /* fprintf ( stderr , "error: invalid port number\n" ) ; */
print_usage(b); /* print_usage ( argv ) ; */
}
} else {
global_b = 0; /* port = 0 ; */
}
}
void init() /* static void init ( ) { */{
global_d = tcpServerSetup(global_b); /* server_socket = tcpServerSetup ( port ) ; */
global_a = ((void *)0 ); /* clients = ( ( void * ) 0 ) ; */
}
int next_readfd(fd_set *a, c_handle *b, uint16_t c) /* static int next_readfd ( fd_set * set , c_handle * head , uint16_t index ) { */{
uint16_t d; /* uint16_t i ; */
int e; /* int tmp_sock ; */
for (d = c; ; d++) {
e = getHandleByIndex(b, d); /* tmp_sock = getHandleByIndex ( head , i ) ; */
if (FD_ISSET(e, a)) /* if (FD_ISSET(tmp_sock, set)) */ {
return e; /* return tmp_sock */
}
}
}
void reset_readfd(fd_set *a, c_handle *b) /* static void reset_readfd ( fd_set * set , c_handle * head ) { */{
int c; /* int tmp_sock ; */
uint16_t d; /* uint16_t i ; */
FD_ZERO(a); /* FD_ZERO ( set ) ; */
FD_SET(global_d, a); /* FD_SET ( server_socket , set ) ; */
for (d = 0; d<global_c; d++) {
c = getHandleByIndex(b, d); /* tmp_sock = getHandleByIndex ( head , i ) ; */
FD_SET(c, a); /* FD_SET ( tmp_sock , set ) ; */
}
}
void send_message(int a, rdy_packet *b, uint16_t c) /* static void send_message ( int socket , rdy_packet * packet , uint16_t len ) { */{
write(a, b, c); /* write ( socket , packet , len ) ; */
}
void broadcast_message(rdy_packet *a, uint16_t b, int c) /* static void broadcast_message ( rdy_packet * packet , uint16_t len , int exclude ) { */{
int d; /* int i ; */
int e; /* int send_sock ; */
for (d = 0; d<global_c; d++) {
e = getHandleByIndex(global_a, d); /* send_sock = getHandleByIndex ( clients , i ) ; */
if (e != c) /* if (send_sock != exclude) */ {
send_message(e, a, b); /* send_message ( send_sock , packet , len ) ; */
}
}
}
void send_state(int a, int b, char *c) /* static void send_state ( int err_socket , int state , char * dest_name ) { */{
raw_packet *d = create_raw_packet(); /* raw_packet * raw = create_raw_packet ( ) ; */
rdy_packet *e = ((void *)0 ); /* rdy_packet * rdy = ( ( void * ) 0 ) ; */
switch (b) /* switch (state) */ {
case -2: d->packet_len = 3; /* raw -> packet_len = 0x03 ; */
d->type = FLAG2; /* raw -> type = FLAG2 ; */
break;
case -3: d->packet_len = 3; /* raw -> packet_len = 0x03 ; */
d->type = FLAG3; /* raw -> type = FLAG3 ; */
break;
case -7: d->type = FLAG7; /* raw -> type = FLAG7 ; */
memcpy(d->dest_handle_name, c, strlen(c)); /* memcpy ( raw -> dest_handle_name , dest_name , strlen ( dest_name ) ) ; */
d->dest_handle_len = strlen(c); /* raw -> dest_handle_len = strlen ( dest_name ) ; */
d->dest_handle_name = calloc(strlen(c) + 1, sizeof(char)); /* raw -> dest_handle_name = calloc ( strlen ( dest_name ) + 1 , sizeof ( char ) ) ; */
d->packet_len = 4 + strlen(c); /* raw -> packet_len = 0x04 + strlen ( dest_name ) ; */
break;
case -9: d->packet_len = 3; /* raw -> packet_len = 0x03 ; */
d->type = FLAG9; /* raw -> type = FLAG9 ; */
break;
case -11: d->packet_len = 3 + sizeof(uint32_t); /* raw -> packet_len = 0x03 + sizeof ( uint32_t ) ; */
d->to_be_sent = global_c; /* raw -> to_be_sent = num_clients ; */
d->type = FLAG11; /* raw -> type = FLAG11 ; */
break;
case -12: d->type = FLAG12; /* raw -> type = FLAG12 ; */
memcpy(d->dest_handle_name, c, strlen(c)); /* memcpy ( raw -> dest_handle_name , dest_name , strlen ( dest_name ) ) ; */
d->dest_handle_len = strlen(c); /* raw -> dest_handle_len = strlen ( dest_name ) ; */
d->dest_handle_name = calloc(strlen(c) + 1, sizeof(char)); /* raw -> dest_handle_name = calloc ( strlen ( dest_name ) + 1 , sizeof ( char ) ) ; */
d->packet_len = 4 + strlen(c); /* raw -> packet_len = 0x04 + strlen ( dest_name ) ; */
break;
default: break;
}
e = create_rdy_packet(d->packet_len); /* rdy = create_rdy_packet ( raw -> packet_len ) ; */
construct_packet(e, d); /* construct_packet ( rdy , raw ) ; */
send_message(a, e, d->packet_len); /* send_message ( err_socket , rdy , raw -> packet_len ) ; */
destroy_raw_packet(d); /* destroy_raw_packet ( raw ) ; */
destroy_rdy_packet(e); /* destroy_rdy_packet ( rdy ) ; */
}
void process_client(int a) /* static void process_client ( int client ) { */{
int b; /* int i ; */
rdy_packet *c = create_rdy_packet(1515); /* rdy_packet * rec_rdy = create_rdy_packet ( 1515 ) ; */
rdy_packet *d = create_rdy_packet(1515); /* rdy_packet * send_rdy = create_rdy_packet ( 1515 ) ; */
char *e; /* char * name ; */
int f; /* int status ; */
raw_packet *g = create_raw_packet(); /* raw_packet * rec_raw = create_raw_packet ( ) ; */
int h; /* int tmp ; */
int i = 0; /* int recipient = 0 ; */
f = read(a, c, 1515); /* status = read ( client , rec_rdy , 1515 ) ; */
if (f<0) /* if (status<0) */ {
perror("read call"); /* perror ( "read call" ) ; */
} else if (!f) /* if (!status) */ {
global_c--; /* num_clients -- ; */
shutdown(a, SHUT_RDWR); /* shutdown ( client , SHUT_RDWR ) ; */
removeHandle(&global_a, a); /* removeHandle ( & clients , client ) ; */
} else {
f = deconstruct_packet(g, c); /* status = deconstruct_packet ( rec_raw , rec_rdy ) ; */
if (f) /* if (status) */ {
fprintf(stderr, "error: could not read incoming packet\n"); /* fprintf ( stderr , "error: could not read incoming packet\n" ) ; */
exit(-12); /* exit ( - 12 ) ; */
} else {
deconstruct_packet(g, c); /* deconstruct_packet ( rec_raw , rec_rdy ) ; */
switch (g->type) /* switch (rec_raw->type) */ {
case FLAG4: h = getHandleSocket(global_a, g->src_handle_name); /* tmp = getHandleSocket ( clients , rec_raw -> src_handle_name ) ; */
broadcast_message(c, g->packet_len, h); /* broadcast_message ( rec_rdy , rec_raw -> packet_len , tmp ) ; */
break;
case FLAG5: i = getHandleSocket(global_a, g->dest_handle_name); /* recipient = getHandleSocket ( clients , rec_raw -> dest_handle_name ) ; */
if (-1 != i) /* if (-1 != recipient) */ {
send_message(i, c, g->packet_len); /* send_message ( recipient , rec_rdy , rec_raw -> packet_len ) ; */
} else {
send_state(a, -7, g->dest_handle_name); /* send_state ( client , - 7 , rec_raw -> dest_handle_name ) ; */
}
break;
case FLAG8: send_state(a, -9, ((void *)0 )); /* send_state ( client , - 9 , ( ( void * ) 0 ) ) ; */
break;
case FLAG10: send_state(a, -11, ((void *)0 )); /* send_state ( client , - 11 , ( ( void * ) 0 ) ) ; */
for (b = 0; b<global_c; b++) {
h = getHandleByIndex(global_a, b); /* tmp = getHandleByIndex ( clients , i ) ; */
e = getHandleName(global_a, h); /* name = getHandleName ( clients , tmp ) ; */
send_state(a, -12, e); /* send_state ( client , - 12 , name ) ; */
}
break;
default: break;
}
}
}
destroy_rdy_packet(c); /* destroy_rdy_packet ( rec_rdy ) ; */
destroy_raw_packet(g); /* destroy_raw_packet ( rec_raw ) ; */
destroy_rdy_packet(d); /* destroy_rdy_packet ( send_rdy ) ; */
}
void accept_client(c_handle **a) /* static void accept_client ( c_handle * * clients ) { */{
int b; /* int status ; */
rdy_packet *c = create_rdy_packet(1515); /* rdy_packet * rec_rdy = create_rdy_packet ( 1515 ) ; */
raw_packet *d = create_raw_packet(); /* raw_packet * rec_raw = create_raw_packet ( ) ; */
rdy_packet *e = create_rdy_packet(1515); /* rdy_packet * send_rdy = create_rdy_packet ( 1515 ) ; */
int f; /* int client_socket ; */
raw_packet *g = create_raw_packet(); /* raw_packet * send_raw = create_raw_packet ( ) ; */
f = tcpServerAccept(global_d); /* client_socket = tcpServerAccept ( server_socket ) ; */
b = read(f, c, 1515); /* status = read ( client_socket , rec_rdy , 1515 ) ; */
if (b<0) /* if (status<0) */ {
perror("read call"); /* perror ( "read call" ) ; */
}
b = deconstruct_packet(d, c); /* status = deconstruct_packet ( rec_raw , rec_rdy ) ; */
if (b) /* if (status) */ {
fprintf(stderr, "error: could not read incoming packet\n"); /* fprintf ( stderr , "error: could not read incoming packet\n" ) ; */
} else {
b = addHandle(a, f, d->src_handle_name); /* status = addHandle ( clients , client_socket , rec_raw -> src_handle_name ) ; */
if (-1 == b) /* if (-1 == status) */ {
send_state(f, -3, ((void *)0 )); /* send_state ( client_socket , - 3 , ( ( void * ) 0 ) ) ; */
} else if (!b) /* if (!status) */ {
global_c++; /* num_clients ++ ; */
send_state(f, -2, ((void *)0 )); /* send_state ( client_socket , - 2 , ( ( void * ) 0 ) ) ; */
}
}
destroy_rdy_packet(c); /* destroy_rdy_packet ( rec_rdy ) ; */
destroy_raw_packet(d); /* destroy_raw_packet ( rec_raw ) ; */
destroy_rdy_packet(e); /* destroy_rdy_packet ( send_rdy ) ; */
destroy_raw_packet(g); /* destroy_raw_packet ( send_raw ) ; */
}
void test_receive() /* static void test_receive ( ) { */{
int a; /* int client_sock ; */
fd_set b; /* fd_set readfd ; */
int c = 0; /* int status = 0 ; */
int d; /* int i ; */
while (1) /* while (1) */ {
reset_readfd(&b, global_a); /* reset_readfd ( & readfd , clients ) ; */
c = mySelect(1024, &b, ((void *)0 ), ((void *)0 ), ((void *)0 )); /* status = mySelect ( 1024 , & readfd , ( ( void * ) 0 ) , ( ( void * ) 0 ) , ( ( void * ) 0 ) ) ; */
if (!c) /* if (!status) */ {
fprintf(stderr, "select returned prematurely\n"); /* fprintf ( stderr , "select returned prematurely\n" ) ; */
} else {
if (FD_ISSET(global_d, &b)) /* if (FD_ISSET(server_socket, &readfd)) */ {
accept_client(&global_a); /* accept_client ( & clients ) ; */
c--; /* status -- ; */
}
for (d = 0; d<c; d++) {
a = next_readfd(&b, global_a, d); /* client_sock = next_readfd ( & readfd , clients , i ) ; */
process_client(a); /* process_client ( client_sock ) ; */
}
}
}
}
int main(int a, char **b) /* int main ( int argc , const char * * argv ) { */{
test_receive(); /* test_receive ( ) ; */
parse_args(a, b); /* parse_args ( argc , argv ) ; */
init(); /* init ( ) ; */
return 0; /* return 0 */
}
