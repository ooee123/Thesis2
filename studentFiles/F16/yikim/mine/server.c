

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int CreateServer(int a, char *b, char **c) /* int CreateServer ( int argc , char * argv [ ] , char * * data_buf ) { */{
int d; /* int port_numb ; */
if (a == 2) /* if (argc == 2) */ {
d = (unsigned)strtod(b[1], 0); /* port_numb = ( unsigned short ) strtod ( argv [ 1 ] , 0 ) ; */
}
if ((*c = (char *)malloc(1029) ) == ((void *)0 )) /* if ((*data_buf = (char *)malloc(1029) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return tcpServerSetup(d); /* return tcpServerSetup(port_numb) */
}
int ResetFD(fd_set *a, node **b, int c, int d) /* int ResetFD ( fd_set * fdvar , node * * cli_sk_head , int cli_sk_count , int server_socket ) { */{
int e = 0; /* int i = 0 ; */
int f = server_socket; /* int max_fd = server_socket ; */
node *g = *b; /* node * cli_sk_cur = * cli_sk_head ; */
FD_ZERO(a); /* FD_ZERO ( fdvar ) ; */
FD_SET(d, a); /* FD_SET ( server_socket , fdvar ) ; */
f = d; /* max_fd = server_socket ; */
for (e = 0; e<c; e++) {
FD_SET(g->fd, a); /* FD_SET ( cli_sk_cur -> fd , fdvar ) ; */
if (g->fd>f) /* if (cli_sk_cur->fd>max_fd) */ {
f = g->fd; /* max_fd = cli_sk_cur -> fd ; */
}
}
return f; /* return max_fd */
}
int AcceptNewClient(int a, node **b, int *c) /* int AcceptNewClient ( int server_socket , node * * cli_sk_head , int * cli_sk_count ) { */{
char d[1029]; /* char data_buf [ 1029 ] ; */
int e; /* int new_client_sk ; */
int f; /* int message_len ; */
uint16_t g, h; /* uint16_t pack_len , pack_len_net ; */
char i[255 + 1]; /* char handle [ 255 + 1 ] ; */
uint8_t j, k; /* uint8_t flag , hand_len ; */
char *l = data_buf; /* char * temp = data_buf ; */
if ((e = accept(a, (struct sockaddr *)0, (socklen_t *)0) )<0) /* if ((new_client_sk = accept(server_socket, (struct sockaddr *)0, (socklen_t *)0) )<0) */ {
perror("accept error"); /* perror ( "accept error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
f = recv(e, d, 1029, 0); /* message_len = recv ( new_client_sk , data_buf , 1029 , 0 ) ; */
memcpy(&h, l, 2); /* memcpy ( & pack_len_net , temp , 2 ) ; */
g = ntohs(h); /* pack_len = ntohs ( pack_len_net ) ; */
l += 2; /* temp += 2 ; */
memcpy(&j, l, 1); /* memcpy ( & flag , temp , 1 ) ; */
l += 1; /* temp += 1 ; */
memcpy(&k, l, 1); /* memcpy ( & hand_len , temp , 1 ) ; */
l += 1; /* temp += 1 ; */
memcpy(i, l, k); /* memcpy ( handle , temp , hand_len ) ; */
i[k] = '\0'; /* handle [ hand_len ] = '\0' ; */
g = 2 + 1; /* pack_len = 2 + 1 ; */
if (findHandle(*b, i)) /* if (findHandle(*cli_sk_head, handle)) */ {
j = 3; /* flag = 3 ; */
} else {
j = 2; /* flag = 2 ; */
push(b, e, i); /* push ( cli_sk_head , new_client_sk , handle ) ; */
}
l = d; /* temp = data_buf ; */
h = htons(g); /* pack_len_net = htons ( pack_len ) ; */
memcpy(l, &h, sizeof (g )); /* memcpy ( temp , & pack_len_net , sizeof ( pack_len ) ) ; */
l += sizeof (g ); /* temp += sizeof ( pack_len ) ; */
memcpy(l, &j, sizeof (j )); /* memcpy ( temp , & flag , sizeof ( flag ) ) ; */
l += sizeof (j ); /* temp += sizeof ( flag ) ; */
if (send(e, d, g, 0)<0) /* if (send(new_client_sk, data_buf, pack_len, 0)<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (j == 3) /* if (flag == 3) */ {
close(e); /* close ( new_client_sk ) ; */
}
return e; /* return new_client_sk */
}
void Broadcast(char *a, node *b, int c, int d) /* void Broadcast ( char * data_buf , node * cli_sk_head , int pack_len , int fd_src ) { */{
int e; /* int client_sk ; */
node *f = cli_sk_head; /* node * temp = cli_sk_head ; */
while (f != ((void *)0 )&&f->fd != 0) /* while (temp != ((void *)0 )&&temp->fd != 0) */ {
e = f->fd; /* client_sk = temp -> fd ; */
if (e != d) /* if (client_sk != fd_src) */ {
if (send(e, a, c, 0)<0) /* if (send(client_sk, data_buf, pack_len, 0)<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
f = f->next; /* temp = temp -> next ; */
}
}
int CreateFlag7(char *a, char *b) /* int CreateFlag7 ( char * handle , char * data_buf ) { */{
uint8_t c = 7; /* uint8_t flag = 7 ; */
uint16_t d, e; /* uint16_t pack_len , pack_len_net ; */
char *f = data_buf; /* char * temp = data_buf ; */
uint8_t g = strlen(a); /* uint8_t handle_len = strlen ( handle ) ; */
d = 3 + sizeof (g ) + g; /* pack_len = 3 + sizeof ( handle_len ) + handle_len ; */
e = htons(d); /* pack_len_net = htons ( pack_len ) ; */
memcpy(f, &e, sizeof (d )); /* memcpy ( temp , & pack_len_net , sizeof ( pack_len ) ) ; */
f += sizeof (d ); /* temp += sizeof ( pack_len ) ; */
memcpy(f, &c, sizeof (c )); /* memcpy ( temp , & flag , sizeof ( flag ) ) ; */
f += sizeof (c ); /* temp += sizeof ( flag ) ; */
memcpy(f, &g, sizeof (g )); /* memcpy ( temp , & handle_len , sizeof ( handle_len ) ) ; */
f += sizeof (g ); /* temp += sizeof ( handle_len ) ; */
memcpy(f, a, g); /* memcpy ( temp , handle , handle_len ) ; */
return d; /* return pack_len */
}
void DirectMessage(char *a, node *b, int c) /* void DirectMessage ( char * data_buf , node * cli_sk_head , int pack_len ) { */{
char d[255]; /* char handle_dest [ 255 ] ; */
uint8_t e, f; /* uint8_t handleDest_len , handleSrc_len ; */
char g[255]; /* char handle_src [ 255 ] ; */
int h, i, j; /* int i , dest_fd , src_fd ; */
char *k = data_buf; /* char * temp = data_buf ; */
k += 3; /* temp += 3 ; */
e = *k; /* handleDest_len = * temp ; */
k += sizeof (e ); /* temp += sizeof ( handleDest_len ) ; */
for (h = 0; h<e; h++) {
k++; /* temp ++ ; */
d[h] = *k; /* handle_dest [ i ] = * temp ; */
}
d[h] = '\0'; /* handle_dest [ i ] = '\0' ; */
f = *k; /* handleSrc_len = * temp ; */
k += sizeof (f ); /* temp += sizeof ( handleSrc_len ) ; */
for (h = 0; h<f; h++) {
k++; /* temp ++ ; */
g[h] = *k; /* handle_src [ i ] = * temp ; */
}
g[h] = '\0'; /* handle_src [ i ] = '\0' ; */
if (findHandle(b, d)) /* if (findHandle(cli_sk_head, handle_dest)) */ {
i = getHandleFD(b, d); /* dest_fd = getHandleFD ( cli_sk_head , handle_dest ) ; */
if (send(i, a, c, 0)<0) /* if (send(dest_fd, data_buf, pack_len, 0)<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
} else {
c = CreateFlag7(d, a); /* pack_len = CreateFlag7 ( handle_dest , data_buf ) ; */
j = getHandleFD(b, g); /* src_fd = getHandleFD ( cli_sk_head , handle_src ) ; */
if (send(j, a, c, 0)<0) /* if (send(src_fd, data_buf, pack_len, 0)<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
}
void SendList(char *a, int b, int c, node *d) /* void SendList ( char * data_buf , int cli_sk_count , int src_fd , node * cli_sk_head ) { */{
int e; /* int i ; */
uint8_t f; /* uint8_t handleLen ; */
uint8_t g; /* uint8_t flag ; */
uint16_t h, i; /* uint16_t pack_len , pack_len_net ; */
char *j; /* char * handle12 ; */
char *k = data_buf; /* char * temp = data_buf ; */
uint32_t l = htons((uint32_t)b); /* uint32_t handle_numb = htons ( ( uint32_t ) cli_sk_count ) ; */
node *m = cli_sk_head; /* node * nodeCur = cli_sk_head ; */
h = sizeof (h ) + sizeof (g ) + sizeof (l ); /* pack_len = sizeof ( pack_len ) + sizeof ( flag ) + sizeof ( handle_numb ) ; */
g = 11; /* flag = 11 ; */
i = htons(h); /* pack_len_net = htons ( pack_len ) ; */
memcpy(k, &i, sizeof (h )); /* memcpy ( temp , & pack_len_net , sizeof ( pack_len ) ) ; */
k += sizeof (h ); /* temp += sizeof ( pack_len ) ; */
memcpy(k, &g, sizeof (g )); /* memcpy ( temp , & flag , sizeof ( flag ) ) ; */
k += sizeof (g ); /* temp += sizeof ( flag ) ; */
memcpy(k, &l, sizeof (l )); /* memcpy ( temp , & handle_numb , sizeof ( handle_numb ) ) ; */
if (send(c, a, h, 0)<0) /* if (send(src_fd, data_buf, pack_len, 0)<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
h = 0; /* pack_len = 0 ; */
g = 12; /* flag = 12 ; */
k = a; /* temp = data_buf ; */
i = htons(i); /* pack_len_net = htons ( pack_len_net ) ; */
memcpy(k, &i, sizeof (h )); /* memcpy ( temp , & pack_len_net , sizeof ( pack_len ) ) ; */
k += sizeof (h ); /* temp += sizeof ( pack_len ) ; */
memcpy(k, &g, sizeof (g )); /* memcpy ( temp , & flag , sizeof ( flag ) ) ; */
k += sizeof (g ); /* temp += sizeof ( flag ) ; */
h = sizeof (h ) + sizeof (g ); /* pack_len = sizeof ( pack_len ) + sizeof ( flag ) ; */
for (e = 0; e<b; e++) {
j = m->handle; /* handle12 = nodeCur -> handle ; */
f = strlen(j); /* handleLen = strlen ( handle12 ) ; */
h += sizeof (f ) + f; /* pack_len += sizeof ( handleLen ) + handleLen ; */
memcpy(k, &f, sizeof (f )); /* memcpy ( temp , & handleLen , sizeof ( handleLen ) ) ; */
k += sizeof (f ); /* temp += sizeof ( handleLen ) ; */
memcpy(k, j, f); /* memcpy ( temp , handle12 , handleLen ) ; */
k += f; /* temp += handleLen ; */
}
if (send(c, a, h, 0)<0) /* if (send(src_fd, data_buf, pack_len, 0)<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void EndConnection(char *a, int b, node **c, int *d) /* void EndConnection ( char * data_buf , int src_fd , node * * cli_sk_head , int * cli_sk_count ) { */{
uint8_t e = 9; /* uint8_t flag = 9 ; */
uint16_t f, g; /* uint16_t pack_len , pack_len_net ; */
char *h = data_buf; /* char * temp = data_buf ; */
f = sizeof (f ) + sizeof (e ); /* pack_len = sizeof ( pack_len ) + sizeof ( flag ) ; */
g = htons(f); /* pack_len_net = htons ( pack_len ) ; */
memcpy(h, &g, sizeof (f )); /* memcpy ( temp , & pack_len_net , sizeof ( pack_len ) ) ; */
h += sizeof (f ); /* temp += sizeof ( pack_len ) ; */
memcpy(h, &e, sizeof (e )); /* memcpy ( temp , & flag , sizeof ( flag ) ) ; */
if (send(b, a, f, 0)<0) /* if (send(src_fd, data_buf, pack_len, 0)<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (close(b)<0) /* if (close(src_fd)<0) */ {
perror("close call"); /* perror ( "close call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
removeNode(c, b); /* removeNode ( cli_sk_head , src_fd ) ; */
}
void InterpretData(char *a, node **b, int c, int *d) /* void InterpretData ( char * data_buf , node * * cli_sk_head , int fd_src , int * cli_sk_count ) { */{
uint8_t e = 0; /* uint8_t flag = 0 ; */
uint16_t f, g; /* uint16_t pack_len , pack_len_net ; */
char *h = data_buf; /* char * temp = data_buf ; */
memcpy(&g, h, sizeof (g )); /* memcpy ( & pack_len_net , temp , sizeof ( pack_len_net ) ) ; */
f = ntohs(g); /* pack_len = ntohs ( pack_len_net ) ; */
e = *h; /* flag = * temp ; */
if (e == 4) /* if (flag == 4) */ {
Broadcast(a, *b, f, c); /* Broadcast ( data_buf , * cli_sk_head , pack_len , fd_src ) ; */
} else if (e == 5) /* if (flag == 5) */ {
DirectMessage(a, *b, f); /* DirectMessage ( data_buf , * cli_sk_head , pack_len ) ; */
} else if (e == 10) /* if (flag == 10) */ {
SendList(a, *d, c, *b); /* SendList ( data_buf , * cli_sk_count , fd_src , * cli_sk_head ) ; */
} else if (e == 8) /* if (flag == 8) */ {
EndConnection(a, c, b, d); /* EndConnection ( data_buf , fd_src , cli_sk_head , cli_sk_count ) ; */
}
}
void IssetCheckClients(node **a, int *b, fd_set *c, char **d) /* void IssetCheckClients ( node * * cli_sk_head , int * cli_sk_count , fd_set * fdvar , char * * data_buf ) { */{
int e = 0; /* int message_len = 0 ; */
int f = 0; /* int i = 0 ; */
node *g = *a; /* node * cli_sk_cur = * cli_sk_head ; */
for (f = 0; f<*b; f++) {
if (FD_ISSET(g->fd, c)) /* if (FD_ISSET(cli_sk_cur->fd, fdvar)) */ {
if ((e = recv(g->fd, *d, 1029, 0) )<=0) /* if ((message_len = recv(cli_sk_cur->fd, *data_buf, 1029, 0) )<=0) */ {
if (e<0) /* if (message_len<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (close(g->fd)<0) /* if (close(cli_sk_cur->fd)<0) */ {
perror("close call"); /* perror ( "close call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
removeNode(a, g->fd); /* removeNode ( cli_sk_head , cli_sk_cur -> fd ) ; */
(*b )--; /* ( * cli_sk_count ) -- ; */
} else {
InterpretData(*d, a, g->fd, b); /* InterpretData ( * data_buf , cli_sk_head , cli_sk_cur -> fd , cli_sk_count ) ; */
}
}
}
}
void ConnectSendRecv(int a, fd_set *b, int c, node **d, int *e, char **f) /* void ConnectSendRecv ( int max_sk , fd_set * fdvar , int server_socket , node * * cli_sk_head , int * cli_sk_count , char * * data_buf ) { */{
if (select(a + 1, b, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (select(max_sk + 1, fdvar, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select error"); /* perror ( "select error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (FD_ISSET(c, b)) /* if (FD_ISSET(server_socket, fdvar)) */ {
AcceptNewClient(c, d, e); /* AcceptNewClient ( server_socket , cli_sk_head , cli_sk_count ) ; */
}
IssetCheckClients(d, e, b, f); /* IssetCheckClients ( cli_sk_head , cli_sk_count , fdvar , data_buf ) ; */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
node *c = malloc(sizeof(node)); /* node * cli_sk_head = malloc ( sizeof ( node ) ) ; */
char *d = ((void *)0 ); /* char * data_buf = ( ( void * ) 0 ) ; */
fd_set e; /* fd_set fdvar ; */
int f = 0, g = 0, h = server_socket; /* int cli_sk_count = 0 , server_socket = 0 , max_sk = server_socket ; */
g = CreateServer(a, b, &d); /* server_socket = CreateServer ( argc , argv , & data_buf ) ; */
while (1) /* while (1) */ {
h = ResetFD(&e, &c, f, g); /* max_sk = ResetFD ( & fdvar , & cli_sk_head , cli_sk_count , server_socket ) ; */
ConnectSendRecv(h, &e, g, &c, &f, &d); /* ConnectSendRecv ( max_sk , & fdvar , server_socket , & cli_sk_head , & cli_sk_count , & data_buf ) ; */
}
free(d); /* free ( data_buf ) ; */
free(c); /* free ( cli_sk_head ) ; */
close(g); /* close ( server_socket ) ; */
return 0; /* return 0 */
}
