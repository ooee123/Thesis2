int32_t global_g; /* static int32_t arr_size ; */
int32_t global_b; /* static int32_t max ; */
int32_t global_h; /* static int32_t server_socket ; */
int8_t *global_d; /* static int8_t * rec_buf ; */
int8_t *global_a; /* static int8_t * send_buf ; */
struct timeval global_e; /* static struct timeval time ; */
Client *global_f; /* static Client * sockets ; */
fd_set global_c; /* static fd_set socket_fds ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void initialize_sockets(int32_t a) /* void initialize_sockets ( int32_t size ) { */{
int32_t b; /* int32_t count ; */
Client c; /* Client c ; */
if (((void *)0 ) == (global_f = calloc(1, a) )) /* if (((void *)0 ) == (sockets = calloc(1, size) )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(1); /* exit ( 1 ) ; */
}
for (b = 0; b<a; b++) {
global_f[b] = c; /* sockets [ count ] = c ; */
}
}
void initialize_fd_set() /* void initialize_fd_set ( ) { */{
int32_t a; /* int32_t count ; */
FD_ZERO(&global_c); /* FD_ZERO ( & socket_fds ) ; */
FD_SET(global_h, &global_c); /* FD_SET ( server_socket , & socket_fds ) ; */
for (a = 1; a<global_b + 1; a++) {
if (0 != global_f[a].socket) /* if (0 != sockets[count].socket) */ {
FD_SET(global_f[a].socket, &global_c); /* FD_SET ( sockets [ count ] . socket , & socket_fds ) ; */
}
}
}
void add_socket(int32_t a) /* void add_socket ( int32_t accept_socket ) { */{
Client b; /* Client client ; */
b.handle = ((void *)0 ); /* client . handle = ( ( void * ) 0 ) ; */
b.handle_len = 0; /* client . handle_len = 0 ; */
b.socket = a; /* client . socket = accept_socket ; */
while (a>=global_g) /* while (accept_socket>=arr_size) */ {
global_g += 10; /* arr_size += 10 ; */
if (((void *)0 ) == (global_f = realloc(global_f, global_g + 10) )) /* if (((void *)0 ) == (sockets = realloc(sockets, arr_size + 10) )) */ {
perror("realloc() error"); /* perror ( "realloc() error" ) ; */
exit(1); /* exit ( 1 ) ; */
}
}
global_f[a] = b; /* sockets [ accept_socket ] = client ; */
if (a>global_b) /* if (accept_socket>max) */ {
global_b = a; /* max = accept_socket ; */
}
}
int32_t handle_exists(int32_t a) /* int32_t handle_exists ( int32_t index ) { */{
int32_t b; /* int32_t count ; */
int8_t *c = &global_d[3 + 1]; /* int8_t * temp = & rec_buf [ 3 + 1 ] ; */
int8_t d = global_d[3]; /* int8_t size = rec_buf [ 3 ] ; */
for (b = 0; b<global_b + 1; b++) {
if (((void *)0 ) != global_f[b].handle&&!strncmp((char *)(global_f[b].handle ), (char *)c, d)) /* if (((void *)0 ) != sockets[count].handle&&!strncmp((char *)(sockets[count].handle ), (char *)temp, size)) */ {
return 1; /* return 1 */
}
}
return 0; /* return 0 */
}
void process_new_client(int32_t a) /* void process_new_client ( int32_t index ) { */{
if (handle_exists(a)) /* if (handle_exists(index)) */ {
chat_header_setup(&global_a, 3, 3); /* chat_header_setup ( & send_buf , 3 , 3 ) ; */
send_data(&global_a, global_f[a].socket, 3); /* send_data ( & send_buf , sockets [ index ] . socket , 3 ) ; */
} else {
chat_header_setup(&global_a, 2, 3); /* chat_header_setup ( & send_buf , 2 , 3 ) ; */
send_data(&global_a, global_f[a].socket, 3); /* send_data ( & send_buf , sockets [ index ] . socket , 3 ) ; */
global_f[a].handle_len = global_d[3]; /* sockets [ index ] . handle_len = rec_buf [ 3 ] ; */
create_buffer((int8_t **)&(global_f[a].handle ), global_f[a].handle_len); /* create_buffer ( ( int8_t * * ) & ( sockets [ index ] . handle ) , sockets [ index ] . handle_len ) ; */
memcpy(global_f[a].handle, &global_d[3 + 1], global_f[a].handle_len); /* memcpy ( sockets [ index ] . handle , & rec_buf [ 3 + 1 ] , sockets [ index ] . handle_len ) ; */
}
}
void server_handles(int32_t a) /* void server_handles ( int32_t index ) { */{
int32_t b = 0, c, d, e = 3; /* int32_t count = 0 , ndx , size , increment = 3 ; */
d = 3 + 4; /* size = 3 + 4 ; */
for (c = 0; c<global_b + 1; c++) {
if (((void *)0 ) != global_f[c].handle) /* if (((void *)0 ) != sockets[ndx].handle) */ {
++b; /* ++ count ; */
}
}
b = htonl(b); /* count = htonl ( count ) ; */
chat_header_setup(&global_a, 11, 3); /* chat_header_setup ( & send_buf , 11 , 3 ) ; */
memcpy(global_a + 3, &b, 4); /* memcpy ( send_buf + 3 , & count , 4 ) ; */
send_data(&global_a, global_f[a].socket, d); /* send_data ( & send_buf , sockets [ index ] . socket , size ) ; */
reset(global_a, 1620); /* reset ( send_buf , 1620 ) ; */
chat_header_setup(&global_a, 12, 0); /* chat_header_setup ( & send_buf , 12 , 0 ) ; */
for (c = 0; c<global_b + 1; c++) {
if (((void *)0 ) != global_f[c].handle) /* if (((void *)0 ) != sockets[ndx].handle) */ {
memcpy(&global_a[e], &global_f[c].handle_len, 1); /* memcpy ( & send_buf [ increment ] , & sockets [ ndx ] . handle_len , 1 ) ; */
e += 1; /* increment += 1 ; */
memcpy(&global_a[e], global_f[c].handle, global_f[c].handle_len); /* memcpy ( & send_buf [ increment ] , sockets [ ndx ] . handle , sockets [ ndx ] . handle_len ) ; */
e += global_f[c].handle_len; /* increment += sockets [ ndx ] . handle_len ; */
}
}
send_data(&global_a, global_f[a].socket, e); /* send_data ( & send_buf , sockets [ index ] . socket , increment ) ; */
reset(global_a, 1620); /* reset ( send_buf , 1620 ) ; */
}
void broadcast_server(int32_t a) /* void broadcast_server ( int32_t index ) { */{
int32_t b; /* int32_t count ; */
for (b = 0; b<global_b + 1; b++) {
if (b != a&&((void *)0 ) != global_f[b].handle) /* if (count != index&&((void *)0 ) != sockets[count].handle) */ {
if ('\n' == global_d[0]) /* if ('\n' == rec_buf[0]) */ {
printf("Terminating %s...\n", global_f[b].handle); /* printf ( "Terminating %s...\n" , sockets [ count ] . handle ) ; */
chat_header_setup(&global_a, 9, 3); /* chat_header_setup ( & send_buf , 9 , 3 ) ; */
send_data(&global_a, global_f[a].socket, 3); /* send_data ( & send_buf , sockets [ index ] . socket , 3 ) ; */
if (global_f[a].handle != ((void *)0 )) /* if (sockets[index].handle != ((void *)0 )) */ {
free(global_f[a].handle); /* free ( sockets [ index ] . handle ) ; */
}
} else {
send_data(&global_d, global_f[b].socket, 1620); /* send_data ( & rec_buf , sockets [ count ] . socket , 1620 ) ; */
}
}
}
}
void message_server(int32_t a) /* void message_server ( int32_t index ) { */{
int32_t b; /* int32_t count ; */
int8_t *c; /* int8_t * temp ; */
int8_t d = global_d[3]; /* int8_t size = rec_buf [ 3 ] ; */
create_buffer(&c, 255 + 1); /* create_buffer ( & temp , 255 + 1 ) ; */
memcpy((char *)c, (char *)&global_d[3 + 1], d); /* memcpy ( ( char * ) temp , ( char * ) & rec_buf [ 3 + 1 ] , size ) ; */
for (b = 0; b<global_b + 1; b++) {
if (((void *)0 ) != global_f[b].handle&&!strncmp((char *)global_f[b].handle, (char *)c, d)) /* if (((void *)0 ) != sockets[count].handle&&!strncmp((char *)sockets[count].handle, (char *)temp, size)) */ {
break;
}
}
if ('\n' == global_d[0]) /* if ('\n' == rec_buf[0]) */ {
printf("Terminating %s...\n", global_f[a].handle); /* printf ( "Terminating %s...\n" , sockets [ index ] . handle ) ; */
chat_header_setup(&global_a, 9, 3); /* chat_header_setup ( & send_buf , 9 , 3 ) ; */
send_data(&global_a, global_f[a].socket, 3); /* send_data ( & send_buf , sockets [ index ] . socket , 3 ) ; */
if (global_f[a].handle != ((void *)0 )) /* if (sockets[index].handle != ((void *)0 )) */ {
free(global_f[a].handle); /* free ( sockets [ index ] . handle ) ; */
}
} else {
if (!handle_exists(a)) /* if (!handle_exists(index)) */ {
memcpy(&global_a[3], &d, 1); /* memcpy ( & send_buf [ 3 ] , & size , 1 ) ; */
memcpy(&global_a[3 + 1], c, d); /* memcpy ( & send_buf [ 3 + 1 ] , temp , size ) ; */
d = 3 + 1 + global_d[3]; /* size = 3 + 1 + rec_buf [ 3 ] ; */
chat_header_setup(&global_a, 7, d); /* chat_header_setup ( & send_buf , 7 , size ) ; */
send_data(&global_a, global_f[a].socket, 1620); /* send_data ( & send_buf , sockets [ index ] . socket , 1620 ) ; */
} else {
send_data(&global_d, global_f[b].socket, 1620); /* send_data ( & rec_buf , sockets [ count ] . socket , 1620 ) ; */
}
}
free(c); /* free ( temp ) ; */
}
void process_flag_server(int8_t a, int32_t b) /* void process_flag_server ( int8_t flag , int32_t index ) { */{
if (1 == a) /* if (1 == flag) */ {
process_new_client(b); /* process_new_client ( index ) ; */
} else if (4 == a) /* if (4 == flag) */ {
broadcast_server(b); /* broadcast_server ( index ) ; */
} else if (5 == a) /* if (5 == flag) */ {
message_server(b); /* message_server ( index ) ; */
} else if (8 == a) /* if (8 == flag) */ {
chat_header_setup(&global_a, 9, 3); /* chat_header_setup ( & send_buf , 9 , 3 ) ; */
send_data(&global_a, global_f[b].socket, 3); /* send_data ( & send_buf , sockets [ index ] . socket , 3 ) ; */
} else if (10 == a) /* if (10 == flag) */ {
server_handles(b); /* server_handles ( index ) ; */
}
reset(global_d, 1620); /* reset ( rec_buf , 1620 ) ; */
reset(global_a, 1620); /* reset ( send_buf , 1620 ) ; */
}
void handle_returned_sockets(int32_t a) /* void handle_returned_sockets ( int32_t num_sockets ) { */{
int8_t b; /* int8_t flag ; */
int32_t c, d, e; /* int32_t accept_socket , count , temp ; */
if (a<0) /* if (num_sockets<0) */ {
perror("select call"); /* perror ( "select call" ) ; */
exit(1); /* exit ( 1 ) ; */
} else if (0<a) /* if (0<num_sockets) */ {
if (FD_ISSET(global_h, &global_c)) /* if (FD_ISSET(server_socket, &socket_fds)) */ {
c = tcpAccept(global_h); /* accept_socket = tcpAccept ( server_socket ) ; */
add_socket(c); /* add_socket ( accept_socket ) ; */
}
for (d = 1; d<global_b + 1; d++) {
e = global_f[d].socket; /* temp = sockets [ count ] . socket ; */
if (0 != e&&FD_ISSET(e, &global_c)) /* if (0 != temp&&FD_ISSET(temp, &socket_fds)) */ {
b = check_socket(global_d, &global_f[d], 1); /* flag = check_socket ( rec_buf , & sockets [ count ] , 1 ) ; */
if (0 != b) /* if (0 != flag) */ {
process_flag_server(b, d); /* process_flag_server ( flag , count ) ; */
}
}
}
}
}
void server_loop() /* void server_loop ( ) { */{
int32_t a; /* int32_t num_sockets ; */
while (1) /* while (1) */ {
initialize_fd_set(global_b); /* initialize_fd_set ( max ) ; */
global_e.tv_usec = 0; /* time . tv_usec = 0 ; */
global_e.tv_sec = 0; /* time . tv_sec = 0 ; */
a = tcp_select(&global_c, global_b, global_e); /* num_sockets = tcp_select ( & socket_fds , max , time ) ; */
handle_returned_sockets(a); /* handle_returned_sockets ( num_sockets ) ; */
}
}
void initialize_global_variables() /* void initialize_global_variables ( ) { */{
create_buffer(&global_d, 1620); /* create_buffer ( & rec_buf , 1620 ) ; */
create_buffer(&global_a, 1620); /* create_buffer ( & send_buf , 1620 ) ; */
initialize_sockets(10); /* initialize_sockets ( 10 ) ; */
global_g = 10; /* arr_size = 10 ; */
global_b = global_h; /* max = server_socket ; */
}
int main(int a, char **b) /* int main ( int argc , char * * argv ) { */{
global_h = tcpServerSetup(); /* server_socket = tcpServerSetup ( ) ; */
initialize_global_variables(); /* initialize_global_variables ( ) ; */
server_loop(global_h); /* server_loop ( server_socket ) ; */
close(global_h); /* close ( server_socket ) ; */
return 0; /* return 0 */
}
