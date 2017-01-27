void CreateSendFlag1(char *handle, int server_socket); /* void CreateSendFlag1(char *handle, int server_socket) */
int ServerReply(int server_socket); /* int ServerReply(int server_socket) */
int my_Select(int server_socket, char *handle, char *data_buf); /* int my_Select(int server_socket, char *handle, char *data_buf) */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int ConnectionSetup(int a, char *b, char **c, int *d, char **e) /* int ConnectionSetup ( int argc , char * argv [ ] , char * * data_buf , int * server_socket , char * * handle ) { */{
int f = 0; /* int flag = 0 ; */
if (a != 4) /* if (argc != 4) */ {
printf("usage: %s handle server-name server-port \n", b[0]); /* printf ( "usage: %s handle server-name server-port \n" , argv [ 0 ] ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((*c = (char *)malloc(1259) ) == ((void *)0 )) /* if ((*data_buf = (char *)malloc(1259) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((*e = (char *)malloc(255) ) == ((void *)0 )) /* if ((*handle = (char *)malloc(255) ) == ((void *)0 )) */ {
perror("malloc() error"); /* perror ( "malloc() error" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
strcpy(*e, b[1]); /* strcpy ( * handle , argv [ 1 ] ) ; */
*d = tcpClientSetup(b[2], b[3]); /* * server_socket = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
CreateSendFlag1(*e, *d); /* CreateSendFlag1 ( * handle , * server_socket ) ; */
f = ServerReply(*d); /* flag = ServerReply ( * server_socket ) ; */
return f; /* return flag */
}
int ServerReply(int a) /* int ServerReply ( int server_socket ) { */{
fd_set b; /* fd_set fdvar ; */
uint16_t c; /* uint16_t pack_len ; */
struct timeval d; /* struct timeval tv ; */
char e[1259]; /* char data_buf [ 1259 ] ; */
int f; /* int message_len ; */
uint8_t g; /* uint8_t flag ; */
char *h = data_buf; /* char * temp = data_buf ; */
FD_ZERO(&b); /* FD_ZERO ( & fdvar ) ; */
FD_SET(a, &b); /* FD_SET ( server_socket , & fdvar ) ; */
d.tv_usec = 0; /* tv . tv_usec = 0 ; */
d.tv_sec = 5; /* tv . tv_sec = 5 ; */
if (select(a + 1, &b, ((void *)0 ), ((void *)0 ), &d)<0) /* if (select(server_socket + 1, &fdvar, ((void *)0 ), ((void *)0 ), &tv)<0) */ {
perror("select call"); /* perror ( "select call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((f = recv(a, e, 1259, 0) )<=0) /* if ((message_len = recv(server_socket, data_buf, 1259, 0) )<=0) */ {
if (f<0) /* if (message_len<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (close(a)<0) /* if (close(server_socket)<0) */ {
perror("close call"); /* perror ( "close call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
h += sizeof (c ); /* temp += sizeof ( pack_len ) ; */
memcpy(&g, h, sizeof (g )); /* memcpy ( & flag , temp , sizeof ( flag ) ) ; */
return g; /* return flag */
}
void CreateSendFlag1(char *a, int b) /* void CreateSendFlag1 ( char * handle , int server_socket ) { */{
uint16_t c, d; /* uint16_t pack_length , pack_len_net ; */
uint8_t e = 1; /* uint8_t flag = 1 ; */
char *f = malloc(1259); /* char * data_buf = malloc ( 1259 ) ; */
uint8_t g = strlen(a); /* uint8_t hand_length = strlen ( handle ) ; */
char *h = data_buf; /* char * temp = data_buf ; */
c = sizeof (c ) + sizeof (e ) + sizeof (g ) + g; /* pack_length = sizeof ( pack_length ) + sizeof ( flag ) + sizeof ( hand_length ) + hand_length ; */
d = htons(c); /* pack_len_net = htons ( pack_length ) ; */
memcpy(h, &d, sizeof (c )); /* memcpy ( temp , & pack_len_net , sizeof ( pack_length ) ) ; */
h += sizeof (c ); /* temp += sizeof ( pack_length ) ; */
memcpy(h, &e, sizeof (e )); /* memcpy ( temp , & flag , sizeof ( flag ) ) ; */
h += sizeof (e ); /* temp += sizeof ( flag ) ; */
memcpy(h, &g, sizeof (g )); /* memcpy ( temp , & hand_length , sizeof ( hand_length ) ) ; */
h += sizeof (g ); /* temp += sizeof ( hand_length ) ; */
memcpy(h, a, g); /* memcpy ( temp , handle , hand_length ) ; */
send(b, f, c, 0); /* send ( server_socket , data_buf , pack_length , 0 ) ; */
free(f); /* free ( data_buf ) ; */
}
void AddChatHeader(char *a, int b, int c, uint8_t d) /* void AddChatHeader ( char * data_buf , int message_len , int header_len , uint8_t flag ) { */{
uint16_t e, f; /* uint16_t pack_len , pack_len_net ; */
char *g = data_buf; /* char * temp = data_buf ; */
e = b + c; /* pack_len = message_len + header_len ; */
f = htons(e); /* pack_len_net = htons ( pack_len ) ; */
memcpy(g, &f, sizeof (e )); /* memcpy ( temp , & pack_len_net , sizeof ( pack_len ) ) ; */
g += sizeof (e ); /* temp += sizeof ( pack_len ) ; */
memcpy(g, &d, sizeof (d )); /* memcpy ( temp , & flag , sizeof ( flag ) ) ; */
}
int CreateFlag5(char *a, char *b, char *c, int d) /* int CreateFlag5 ( char * handle_dest , char * handle_src , char * data_buf , int message_len ) { */{
int e; /* int header_len ; */
char *f = data_buf; /* char * temp = data_buf ; */
uint8_t g = strlen(a); /* uint8_t handleLen_dest = strlen ( handle_dest ) ; */
uint8_t h = strlen(b); /* uint8_t handleLen_src = strlen ( handle_src ) ; */
e = 3 + 1 + 1 + g + h; /* header_len = 3 + 1 + 1 + handleLen_dest + handleLen_src ; */
AddChatHeader(c, d, e, 5); /* AddChatHeader ( data_buf , message_len , header_len , 5 ) ; */
f += 3; /* temp += 3 ; */
memcpy(f, &g, 1); /* memcpy ( temp , & handleLen_dest , 1 ) ; */
f += 1; /* temp += 1 ; */
memcpy(f, a, g); /* memcpy ( temp , handle_dest , handleLen_dest ) ; */
f += g; /* temp += handleLen_dest ; */
memcpy(f, &h, 1); /* memcpy ( temp , & handleLen_src , 1 ) ; */
f += 1; /* temp += 1 ; */
memcpy(f, b, h); /* memcpy ( temp , handle_src , handleLen_src ) ; */
return e; /* return header_len */
}
int CreateFlag4(char *a, char *b, int c) /* int CreateFlag4 ( char * handle , char * data_buf , int message_len ) { */{
int d; /* int header_len ; */
char *e = data_buf; /* char * temp = data_buf ; */
uint8_t f = strlen(a); /* uint8_t handle_len = strlen ( handle ) ; */
d = 3 + 1 + f; /* header_len = 3 + 1 + handle_len ; */
AddChatHeader(b, c, d, 4); /* AddChatHeader ( data_buf , message_len , header_len , 4 ) ; */
e += 3; /* temp += 3 ; */
memcpy(e, &f, sizeof (f )); /* memcpy ( temp , & handle_len , sizeof ( handle_len ) ) ; */
e += 1; /* temp += 1 ; */
memcpy(e, a, f); /* memcpy ( temp , handle , handle_len ) ; */
return d; /* return header_len */
}
int GetMessage(char **a) /* int GetMessage ( char * * message_buf ) { */{
int b = 0; /* int message_len = 0 ; */
while (((*a )[b] = getchar() ) != '\n'&&b<1000) /* while (((*message_buf )[message_len] = getchar() ) != '\n'&&message_len<1000) */ {
if ((*a )[b] != ' ') /* if ((*message_buf )[message_len] != ' ') */ {
b++; /* message_len ++ ; */
}
}
(*a )[b++] = '\0'; /* ( * message_buf ) [ message_len ++ ] = '\0' ; */
return b; /* return message_len */
}
int GetInput(char *a, char **b) /* int GetInput ( char * handle , char * * data_buf ) { */{
char c[3 + 1]; /* char command [ 3 + 1 ] ; */
char *d = malloc(1000 + 1); /* char * message_buf = malloc ( 1000 + 1 ) ; */
int e, f, g; /* int headerLen , message_len , pack_len ; */
char h[255]; /* char handle_dest [ 255 ] ; */
f = 0; /* message_len = 0 ; */
e = 0; /* headerLen = 0 ; */
scanf("%s", c); /* scanf ( "%s" , command ) ; */
if (c[0] == '%') /* if (command[0] == '%') */ {
if (c[1] == 'B' || c[1] == 'b') /* if (command[1] == 'B' || command[1] == 'b') */ {
f = GetMessage(&d); /* message_len = GetMessage ( & message_buf ) ; */
e = CreateFlag4(a, *b, f); /* headerLen = CreateFlag4 ( handle , * data_buf , message_len ) ; */
memcpy((*b ) + e, d, f); /* memcpy ( ( * data_buf ) + headerLen , message_buf , message_len ) ; */
} else if (c[1] == 'M' || c[1] == 'm') /* if (command[1] == 'M' || command[1] == 'm') */ {
scanf("%s", h); /* scanf ( "%s" , handle_dest ) ; */
f = GetMessage(&d); /* message_len = GetMessage ( & message_buf ) ; */
e = CreateFlag5(h, a, *b, f); /* headerLen = CreateFlag5 ( handle_dest , handle , * data_buf , message_len ) ; */
memcpy((*b ) + e, d, f); /* memcpy ( ( * data_buf ) + headerLen , message_buf , message_len ) ; */
} else if (c[1] == 'L' || c[1] == 'l') /* if (command[1] == 'L' || command[1] == 'l') */ {
f = 0; /* message_len = 0 ; */
e = 3; /* headerLen = 3 ; */
AddChatHeader(*b, f, 3, 10); /* AddChatHeader ( * data_buf , message_len , 3 , 10 ) ; */
} else if (c[1] == 'E' || c[1] == 'e') /* if (command[1] == 'E' || command[1] == 'e') */ {
f = 0; /* message_len = 0 ; */
e = 3; /* headerLen = 3 ; */
AddChatHeader(*b, f, 3, 8); /* AddChatHeader ( * data_buf , message_len , 3 , 8 ) ; */
} else {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
}
} else {
printf("Invalid command\n"); /* printf ( "Invalid command\n" ) ; */
}
free(d); /* free ( message_buf ) ; */
g = e + f; /* pack_len = headerLen + message_len ; */
return g; /* return pack_len */
}
void InterpreteFlag4(char *a) /* void InterpreteFlag4 ( char * data_buf ) { */{
int b; /* int i ; */
uint8_t c; /* uint8_t handle_len ; */
char d[255 + 1]; /* char handle [ 255 + 1 ] ; */
c = *a; /* handle_len = * data_buf ; */
a += sizeof (c ); /* data_buf += sizeof ( handle_len ) ; */
for (b = 0; b<c; b++) {
a++; /* data_buf ++ ; */
d[b] = *a; /* handle [ i ] = * data_buf ; */
}
d[b] = '\0'; /* handle [ i ] = '\0' ; */
printf("\n%s: %s\n", d, a); /* printf ( "\n%s: %s\n" , handle , data_buf ) ; */
}
void InterpreteFlag5(char *a) /* void InterpreteFlag5 ( char * data_buf ) { */{
char b[255 + 1]; /* char handle [ 255 + 1 ] ; */
uint8_t c; /* uint8_t handle_len ; */
int d; /* int i ; */
c = *a; /* handle_len = * data_buf ; */
a += sizeof (c ); /* data_buf += sizeof ( handle_len ) ; */
a += c; /* data_buf += handle_len ; */
c = *a; /* handle_len = * data_buf ; */
a += sizeof (c ); /* data_buf += sizeof ( handle_len ) ; */
for (d = 0; d<c; d++) {
a++; /* data_buf ++ ; */
b[d] = *a; /* handle [ i ] = * data_buf ; */
}
b[d] = '\0'; /* handle [ i ] = '\0' ; */
printf("\n%s: %s\n", b, a); /* printf ( "\n%s: %s\n" , handle , data_buf ) ; */
}
void InterpreteFlag7(char *a) /* void InterpreteFlag7 ( char * data_buf ) { */{
int b; /* int i ; */
uint8_t c; /* uint8_t handle_len ; */
char d[255 + 1]; /* char handle [ 255 + 1 ] ; */
c = *a; /* handle_len = * data_buf ; */
a += sizeof (c ); /* data_buf += sizeof ( handle_len ) ; */
for (b = 0; b<c; b++) {
a++; /* data_buf ++ ; */
d[b] = *a; /* handle [ i ] = * data_buf ; */
}
d[b] = '\0'; /* handle [ i ] = '\0' ; */
printf("Client with handle %s does not exist\n", d); /* printf ( "Client with handle %s does not exist\n" , handle ) ; */
}
void InterpreteFlag11(char *a, uint32_t *b) /* void InterpreteFlag11 ( char * data_buf , uint32_t * handle_numb ) { */{
memcpy(b, a, sizeof (b )); /* memcpy ( handle_numb , data_buf , sizeof ( handle_numb ) ) ; */
*b = ntohs(*b); /* * handle_numb = ntohs ( * handle_numb ) ; */
printf("Number of clients: %zu\n", (*b )); /* printf ( "Number of clients: %zu\n" , ( * handle_numb ) ) ; */
}
void InterpreteFlag12(uint32_t a, char *b) /* void InterpreteFlag12 ( uint32_t handle_numb , char * data_buf ) { */{
char c[255 + 1]; /* char handle [ 255 + 1 ] ; */
uint8_t d; /* uint8_t handle_len ; */
int e, f; /* int i , j ; */
for (f = 0; f<a; f++) {
d = *b; /* handle_len = * data_buf ; */
b += sizeof (d ); /* data_buf += sizeof ( handle_len ) ; */
for (e = 0; e<d; e++) {
b++; /* data_buf ++ ; */
c[e] = *b; /* handle [ i ] = * data_buf ; */
}
c[e] = '\0'; /* handle [ i ] = '\0' ; */
printf("\t%s\n", c); /* printf ( "\t%s\n" , handle ) ; */
}
}
int InterpreteData(char *a, int b, uint32_t *c) /* int InterpreteData ( char * data_buf , int server_socket , uint32_t * handle_numb ) { */{
uint8_t d; /* uint8_t flag ; */
int e = 0; /* int isExit = 0 ; */
uint16_t f, g; /* uint16_t pack_len , pack_len_net ; */
char *h = data_buf; /* char * temp = data_buf ; */
g = *h; /* pack_len_net = * temp ; */
f = ntohs(g); /* pack_len = ntohs ( pack_len_net ) ; */
d = *h; /* flag = * temp ; */
if (d == 4) /* if (flag == 4) */ {
InterpreteFlag4(h); /* InterpreteFlag4 ( temp ) ; */
} else if (d == 5) /* if (flag == 5) */ {
InterpreteFlag5(h); /* InterpreteFlag5 ( temp ) ; */
} else if (d == 7) /* if (flag == 7) */ {
InterpreteFlag7(h); /* InterpreteFlag7 ( temp ) ; */
} else if (d == 11) /* if (flag == 11) */ {
InterpreteFlag11(h, c); /* InterpreteFlag11 ( temp , handle_numb ) ; */
} else if (d == 12) /* if (flag == 12) */ {
InterpreteFlag12(*c, h); /* InterpreteFlag12 ( * handle_numb , temp ) ; */
} else if (d == 9) /* if (flag == 9) */ {
e = 1; /* isExit = 1 ; */
}
return e; /* return isExit */
}
int MySelect(int a, char *b, char *c, uint32_t *d) /* int MySelect ( int server_socket , char * handle , char * data_buf , uint32_t * handle_numb ) { */{
fd_set e; /* fd_set fdvar ; */
int f; /* int pack_len ; */
int g = 0; /* int isExit = 0 ; */
int h; /* int message_len ; */
FD_ZERO(&e); /* FD_ZERO ( & fdvar ) ; */
FD_SET(0, &e); /* FD_SET ( 0 , & fdvar ) ; */
FD_SET(a, &e); /* FD_SET ( server_socket , & fdvar ) ; */
if (select(a + 1, &e, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) /* if (select(server_socket + 1, &fdvar, ((void *)0 ), ((void *)0 ), ((void *)0 ))<0) */ {
perror("select call"); /* perror ( "select call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (FD_ISSET(0, &e)) /* if (FD_ISSET(0, &fdvar)) */ {
f = GetInput(b, &c); /* pack_len = GetInput ( handle , & data_buf ) ; */
if (f) /* if (pack_len) */ {
if (send(a, c, f, 0)<0) /* if (send(server_socket, data_buf, pack_len, 0)<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
}
if (FD_ISSET(a, &e)) /* if (FD_ISSET(server_socket, &fdvar)) */ {
if ((h = recv(a, c, 1259, 0) )<=0) /* if ((message_len = recv(server_socket, data_buf, 1259, 0) )<=0) */ {
if (h<0) /* if (message_len<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (close(a)<0) /* if (close(server_socket)<0) */ {
perror("close call"); /* perror ( "close call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("Server Termiated\n"); /* printf ( "Server Termiated\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
} else {
g = InterpreteData(c, a, d); /* isExit = InterpreteData ( data_buf , server_socket , handle_numb ) ; */
}
}
return g; /* return isExit */
}
void cleanAll(char *a, char *b) /* void cleanAll ( char * data_buf , char * handle ) { */{
printf("\n"); /* printf ( "\n" ) ; */
free(b); /* free ( handle ) ; */
free(a); /* free ( data_buf ) ; */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
char *c = ((void *)0 ), *d = ((void *)0 ); /* char * data_buf = ( ( void * ) 0 ) , * handle = ( ( void * ) 0 ) ; */
uint32_t e = 0; /* uint32_t handle_numb = 0 ; */
int f = 0, g = 0, h = 0; /* int flag = 0 , server_socket = 0 , isExit = 0 ; */
f = ConnectionSetup(a, b, &c, &g, &d); /* flag = ConnectionSetup ( argc , argv , & data_buf , & server_socket , & handle ) ; */
if (f == 3) /* if (flag == 3) */ {
printf("Handle already in use: %s\n", d); /* printf ( "Handle already in use: %s\n" , handle ) ; */
} else {
while (!h) /* while (!isExit) */ {
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
h = MySelect(g, d, c, &e); /* isExit = MySelect ( server_socket , handle , data_buf , & handle_numb ) ; */
}
}
cleanAll(c, d); /* cleanAll ( data_buf , handle ) ; */
return 0; /* return 0 */
}
