typedef struct {
char **handles;
int current_handles;
int current_max;
} handle_list;
void main_op(int port_addr); /* void main_op(int port_addr) */
void handle_client(int port_addr, handle_list *hdl_list, fd_set *active_fds); /* void handle_client(int port_addr, handle_list *hdl_list, fd_set *active_fds) */
void new_connection(int port_addr, handle_list *hdl_list, fd_set *active_fds); /* void new_connection(int port_addr, handle_list *hdl_list, fd_set *active_fds) */
int send_message(int port_addr, char *data, handle_list *hdl_list, fd_set *active_fds); /* int send_message(int port_addr, char *data, handle_list *hdl_list, fd_set *active_fds) */
void close_client(int port_addr, handle_list *hdl_list, fd_set *active_fds); /* void close_client(int port_addr, handle_list *hdl_list, fd_set *active_fds) */
int find_handle(handle_list *hdl_list, char *toFind); /* int find_handle(handle_list *hdl_list, char *toFind) */
void broadcast(int port_addr, char *data, handle_list *hdl_list, fd_set *active_fds); /* void broadcast(int port_addr, char *data, handle_list *hdl_list, fd_set *active_fds) */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
fd_set c, d; /* fd_set active_fds , read_fds ; */
char **e = (char **)calloc(20, sizeof(char *)); /* char * * temp = ( char * * ) calloc ( 20 , sizeof ( char * ) ) ; */
int f, g; /* int port_addr , i ; */
for (g = 0; g<20; g++) {
e[g] = calloc(256, sizeof(char)); /* temp [ i ] = calloc ( 256 , sizeof ( char ) ) ; */
}
handle_list h = {e,0,20}; /* handle_list hdl_list = { . handles = temp , . current_handles = 0 , . current_max = 20 } ; */
FD_ZERO(&c); /* FD_ZERO ( & active_fds ) ; */
f = (a == 2 ) ? atoi(b[1]) : 0; /* port_addr = ( argc == 2 ) ? atoi ( argv [ 1 ] ) : 0 ; */
f = tcpServerSetup(f); /* port_addr = tcpServerSetup ( port_addr ) ; */
FD_SET(f, &c); /* FD_SET ( port_addr , & active_fds ) ; */
while (1) /* while (1) */ {
d = c; /* read_fds = active_fds ; */
if (mySelect(1024, &d, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (mySelect(1024, &read_fds, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select"); /* perror ( "select" ) ; */
return -1; /* return -1 */
} else {
for (g = 0; g<1024; ++g) {
if (FD_ISSET(g, &d)) /* if (FD_ISSET(i, &read_fds)) */ {
if (g == f) /* if (i == port_addr) */ {
new_connection(f, &h, &c); /* new_connection ( port_addr , & hdl_list , & active_fds ) ; */
FD_ZERO(&d); /* FD_ZERO ( & read_fds ) ; */
} else {
handle_client(g, &h, &c); /* handle_client ( i , & hdl_list , & active_fds ) ; */
}
}
}
}
}
close(f); /* close ( port_addr ) ; */
return 0; /* return 0 */
}
void new_connection(int a, handle_list *b, fd_set *c) /* void new_connection ( int port_addr , handle_list * hdl_list , fd_set * active_fds ) { */{
char **d; /* char * * temp ; */
uint8_t e = 0; /* uint8_t handle_len = 0 ; */
uint8_t f[1024]; /* uint8_t data [ 1024 ] ; */
char g[256]; /* char handle [ 256 ] ; */
uint8_t h; /* uint8_t flag ; */
int i; /* int i ; */
uint8_t j[1024]; /* uint8_t send_buf [ 1024 ] ; */
uint16_t k; /* uint16_t data_len ; */
int l = tcpAccept(a); /* int client_port = tcpAccept ( port_addr ) ; */
if (myRecv(l, f, 1024, 0)<0) /* if (myRecv(client_port, data, 1024, 0)<0) */ {
printf("data not sent\n"); /* printf ( "data not sent\n" ) ; */
return; /* return */
}
memcpy(&k, f, 2); /* memcpy ( & data_len , data , 2 ) ; */
k = ntohs(k); /* data_len = ntohs ( data_len ) ; */
e = f[3]; /* handle_len = data [ 3 ] ; */
h = f[2]; /* flag = data [ 2 ] ; */
((h == 1 ) ? (void)(0 ) : __assert_fail("flag == 0x01", "<stdin>", 113, __PRETTY_FUNCTION__) ); /* ( ( flag == 0x01 ) ? ( void ) ( 0 ) : __assert_fail ( "flag == 0x01" , "<stdin>" , 113 , __PRETTY_FUNCTION__ ) ) ; */
((k>3 ) ? (void)(0 ) : __assert_fail("data_len > 3", "<stdin>", 114, __PRETTY_FUNCTION__) ); /* ( ( data_len > 3 ) ? ( void ) ( 0 ) : __assert_fail ( "data_len > 3" , "<stdin>" , 114 , __PRETTY_FUNCTION__ ) ) ; */
e = f[3]; /* handle_len = data [ 3 ] ; */
((e>0 ) ? (void)(0 ) : __assert_fail("handle_len > 0", "<stdin>", 118, __PRETTY_FUNCTION__) ); /* ( ( handle_len > 0 ) ? ( void ) ( 0 ) : __assert_fail ( "handle_len > 0" , "<stdin>" , 118 , __PRETTY_FUNCTION__ ) ) ; */
if (b->current_max<=l - 4) /* if (hdl_list->current_max<=client_port - 4) */ {
d = realloc(b->handles, (b->current_max + 20 ) * sizeof(char *)); /* temp = realloc ( hdl_list -> handles , ( hdl_list -> current_max + 20 ) * sizeof ( char * ) ) ; */
for (i = 0; i<20; i++) {
d[i + b->current_max] = calloc(256, sizeof(char)); /* temp [ i + hdl_list -> current_max ] = calloc ( 256 , sizeof ( char ) ) ; */
}
b->handles = d; /* hdl_list -> handles = temp ; */
b->current_max += 20; /* hdl_list -> current_max += 20 ; */
}
memcpy(g, f + 4, e); /* memcpy ( handle , data + 4 , handle_len ) ; */
g[e] = '\0'; /* handle [ handle_len ] = '\0' ; */
if (find_handle(b, g) == -1) /* if (find_handle(hdl_list, handle) == -1) */ {
memcpy(b->handles[l - 4], f + 4, e); /* memcpy ( hdl_list -> handles [ client_port - 4 ] , data + 4 , handle_len ) ; */
b->handles[l - 4][e] = '\0'; /* hdl_list -> handles [ client_port - 4 ] [ handle_len ] = '\0' ; */
b->current_handles++; /* hdl_list -> current_handles ++ ; */
FD_SET(l, c); /* FD_SET ( client_port , active_fds ) ; */
j[2] = 2; /* send_buf [ 2 ] = 0x02 ; */
} else {
j[2] = 3; /* send_buf [ 2 ] = 0x03 ; */
}
j[0] = 3; /* send_buf [ 0 ] = 3 ; */
j[1] = 0; /* send_buf [ 1 ] = 0 ; */
mySend(l, j, 3, 0); /* mySend ( client_port , send_buf , 3 , 0 ) ; */
return; /* return */
}
void handle_client(int a, handle_list *b, fd_set *c) /* void handle_client ( int port_addr , handle_list * hdl_list , fd_set * active_fds ) { */{
uint16_t d; /* uint16_t data_len ; */
char e[1024]; /* char data [ 1024 ] ; */
uint8_t f; /* uint8_t flag ; */
int g; /* int num_bytes ; */
g = myRecv(a, e, 1024, 0); /* num_bytes = myRecv ( port_addr , data , 1024 , 0 ) ; */
if (g<0) /* if (num_bytes<0) */ {
printf("data not sent\n"); /* printf ( "data not sent\n" ) ; */
return; /* return */
} else if (g == 0) /* if (num_bytes == 0) */ {
close(a); /* close ( port_addr ) ; */
((void)(((c )->__fds_bits )[((a ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((a ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( active_fds ) -> __fds_bits ) [ ( ( port_addr ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( port_addr ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
b->handles[a - 4][0] = '\0'; /* hdl_list -> handles [ port_addr - 4 ] [ 0 ] = '\0' ; */
b->current_handles--; /* hdl_list -> current_handles -- ; */
return; /* return */
}
memcpy(&d, e, 2); /* memcpy ( & data_len , data , 2 ) ; */
d = ntohs(e[1]); /* data_len = ntohs ( data [ 1 ] ) ; */
f = e[2]; /* flag = data [ 2 ] ; */
switch (f) /* switch (flag) */ {
case 1: break;
case 2: break;
case 3: break;
case 4: broadcast(a, e, b, c); /* broadcast ( port_addr , data , hdl_list , active_fds ) ; */
break;
case 5: send_message(a, e, b, c); /* send_message ( port_addr , data , hdl_list , active_fds ) ; */
break;
case 8: close_client(a, b, c); /* close_client ( port_addr , hdl_list , active_fds ) ; */
break;
case 9: break;
case 10: printf("Listing clients not supported\n"); /* printf ( "Listing clients not supported\n" ) ; */
break;
case 11: break;
case 12: break;
default: printf("Unknown flag\n"); /* printf ( "Unknown flag\n" ) ; */
printf("flag: %x\n", f); /* printf ( "flag: %x\n" , flag ) ; */
}
return; /* return */
}
void close_client(int a, handle_list *b, fd_set *c) /* void close_client ( int port_addr , handle_list * hdl_list , fd_set * active_fds ) { */{
char d[] = "309"; /* char close_mesg [ ] = "309" ; */
mySend(a, d, 3, 0); /* mySend ( port_addr , close_mesg , 3 , 0 ) ; */
close(a); /* close ( port_addr ) ; */
((void)(((c )->__fds_bits )[((a ) / (8 * (int)sizeof(__fd_mask) ) )] &= ~((__fd_mask )('U' << ((a ) % (8 * (int)sizeof(__fd_mask) ) )) ) ) ); /* ( ( void ) ( ( ( active_fds ) -> __fds_bits ) [ ( ( port_addr ) / ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ] &= ~ ( ( __fd_mask ) ( 1UL << ( ( port_addr ) % ( 8 * ( int ) sizeof ( __fd_mask ) ) ) ) ) ) ) ; */
b->handles[a - 4][0] = '\0'; /* hdl_list -> handles [ port_addr - 4 ] [ 0 ] = '\0' ; */
return; /* return */
}
int send_message(int a, char *b, handle_list *c, fd_set *d) /* int send_message ( int port_addr , char * data , handle_list * hdl_list , fd_set * active_fds ) { */{
int e, f; /* int dest_handle_len , dest_port ; */
uint16_t g; /* uint16_t data_len ; */
char h[256]; /* char dest_handle [ 256 ] ; */
memcpy(&g, b, 2); /* memcpy ( & data_len , data , 2 ) ; */
g = ntohs(g); /* data_len = ntohs ( data_len ) ; */
e = b[3]; /* dest_handle_len = data [ 3 ] ; */
memcpy(h, b + 4, e); /* memcpy ( dest_handle , data + 4 , dest_handle_len ) ; */
h[e] = '\0'; /* dest_handle [ dest_handle_len ] = '\0' ; */
f = find_handle(c, h); /* dest_port = find_handle ( hdl_list , dest_handle ) ; */
if (f != -1) /* if (dest_port != -1) */ {
mySend(f, b, g, 0); /* mySend ( dest_port , data , data_len , 0 ) ; */
} else {
g = ntohs(3 + strlen(h)); /* data_len = ntohs ( 3 + strlen ( dest_handle ) ) ; */
memcpy(b, &g, 2); /* memcpy ( data , & data_len , 2 ) ; */
b[2] = 7; /* data [ 2 ] = 0x07 ; */
mySend(a, b, 4 + strlen(h), 0); /* mySend ( port_addr , data , 4 + strlen ( dest_handle ) , 0 ) ; */
}
return 0; /* return 0 */
}
void broadcast(int a, char *b, handle_list *c, fd_set *d) /* void broadcast ( int port_addr , char * data , handle_list * hdl_list , fd_set * active_fds ) { */{
uint16_t e; /* uint16_t data_len ; */
int f, g; /* int src_handle_len , i ; */
char h[256]; /* char src_handle [ 256 ] ; */
memcpy(&e, b, 2); /* memcpy ( & data_len , data , 2 ) ; */
e = ntohs(e); /* data_len = ntohs ( data_len ) ; */
f = b[3]; /* src_handle_len = data [ 3 ] ; */
memcpy(h, b + 4, f); /* memcpy ( src_handle , data + 4 , src_handle_len ) ; */
h[f] = '\0'; /* src_handle [ src_handle_len ] = '\0' ; */
for (g = 0; g<c->current_handles; g++) {
if ((strcmp(c->handles[g], h) != 0 )&&c->handles[g][0] != '\0') /* if ((strcmp(hdl_list->handles[i], src_handle) != 0 )&&hdl_list->handles[i][0] != '\0') */ {
mySend(g + 4, b, e, 0); /* mySend ( i + 4 , data , data_len , 0 ) ; */
}
}
return; /* return */
}
int find_handle(handle_list *a, char *b) /* int find_handle ( handle_list * hdl_list , char * toFind ) { */{
int c; /* int i ; */
for (c = 0; c<a->current_handles; c++) {
if (strcmp(a->handles[c], b) == 0) /* if (strcmp(hdl_list->handles[i], toFind) == 0) */ {
return c + 4; /* return i + 4 */
}
}
return -1; /* return -1 */
}
