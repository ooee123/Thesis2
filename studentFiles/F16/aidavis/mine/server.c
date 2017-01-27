

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void parseMessage(char *a, int b, char ***c, int *d, uint16_t e) /* void parseMessage ( char * buf , int client_socket , char * * * handles , int * count , uint16_t packetLen ) { */{
uint8_t f; /* uint8_t sendlen ; */
char *g = malloc(1000); /* char * text = malloc ( 1000 ) ; */
int h = 0; /* int valid = 0 ; */
int i; /* int i ; */
uint8_t j; /* uint8_t destlen ; */
int k; /* int socket_num ; */
memcpy(&j, a + 3, 1); /* memcpy ( & destlen , buf + 3 , 1 ) ; */
char *l = malloc(j); /* char * desthandle = malloc ( destlen ) ; */
memcpy(l, a + 4, j); /* memcpy ( desthandle , buf + 4 , destlen ) ; */
memcpy(&f, a + 4 + j, 1); /* memcpy ( & sendlen , buf + 4 + destlen , 1 ) ; */
char *m = malloc(f); /* char * sendhandle = malloc ( sendlen ) ; */
memcpy(m, a + 5 + j, f); /* memcpy ( sendhandle , buf + 5 + destlen , sendlen ) ; */
strcpy(g, a + 5 + j + f); /* strcpy ( text , buf + 5 + destlen + sendlen ) ; */
for (i = 0; i<*d; i++) {
if ((*c )[i] != ((void *)0 )&&(strcmp((*c )[i], l) == 0 )) /* if ((*handles )[i] != ((void *)0 )&&(strcmp((*handles )[i], desthandle) == 0 )) */ {
h = 1; /* valid = 1 ; */
}
}
if (h) /* if (valid) */ {
int n; /* int dest_socket ; */
for (i = 0; i<*d; i++) {
if ((*c )[i] != ((void *)0 )&&strcmp((*c )[i], l) == 0) /* if ((*handles )[i] != ((void *)0 )&&strcmp((*handles )[i], desthandle) == 0) */ {
n = i; /* dest_socket = i ; */
}
}
mySend(n, a, e, 0); /* mySend ( dest_socket , buf , packetLen , 0 ) ; */
} else {
uint8_t o = 7; /* uint8_t flag = 7 ; */
uint16_t p = 4 + j; /* uint16_t invalidRes = 4 + destlen ; */
char *q = malloc(4 + j); /* char * invalid = malloc ( 4 + destlen ) ; */
char *r = invalid; /* char * mover = invalid ; */
uint16_t s = htons(p); /* uint16_t ninvalidRes = htons ( invalidRes ) ; */
memcpy(r, &s, 2); /* memcpy ( mover , & ninvalidRes , 2 ) ; */
r += 2; /* mover += 2 ; */
memcpy(r, &o, 1); /* memcpy ( mover , & flag , 1 ) ; */
r += 1; /* mover += 1 ; */
memcpy(r, &j, 1); /* memcpy ( mover , & destlen , 1 ) ; */
r += 1; /* mover += 1 ; */
memcpy(r, l, j); /* memcpy ( mover , desthandle , destlen ) ; */
mySend(b, q, p, 0); /* mySend ( client_socket , invalid , invalidRes , 0 ) ; */
}
}
void parseBroadcast(char *a, int b, char ***c, int *d, uint16_t e) /* void parseBroadcast ( char * buf , int client_socket , char * * * handles , int * count , uint16_t packetLen ) { */{
int f; /* int sent ; */
int g; /* int i ; */
for (g = 0; g<*d; g++) {
if ((*c )[g] != ((void *)0 )&&g != b) /* if ((*handles )[i] != ((void *)0 )&&i != client_socket) */ {
f = mySend(g, a, e, 0); /* sent = mySend ( i , buf , packetLen , 0 ) ; */
}
}
}
void parseExit(char *a, int b, char ***c, int *d, uint16_t e) /* void parseExit ( char * buf , int client_socket , char * * * handles , int * count , uint16_t packetLen ) { */{
uint8_t f = 9; /* uint8_t flag = 9 ; */
char *g = malloc(3); /* char * res = malloc ( 3 ) ; */
uint16_t h = htons(e); /* uint16_t len = htons ( packetLen ) ; */
memcpy(g, &h, 2); /* memcpy ( res , & len , 2 ) ; */
memcpy(g + 2, &f, 1); /* memcpy ( res + 2 , & flag , 1 ) ; */
mySend(b, g, e, 0); /* mySend ( client_socket , res , packetLen , 0 ) ; */
(*c )[b] = ((void *)0 ); /* ( * handles ) [ client_socket ] = ( ( void * ) 0 ) ; */
close(b); /* close ( client_socket ) ; */
}
void parseList(int a, char ***b, int *c) /* void parseList ( int client_socket , char * * * handles , int * count ) { */{
uint8_t d = 11; /* uint8_t flag = 11 ; */
uint8_t e; /* uint8_t handlelen ; */
int f; /* int i ; */
uint16_t g = htons(7); /* uint16_t len = htons ( 7 ) ; */
int h; /* int sent ; */
uint32_t i = 0; /* uint32_t num = 0 ; */
int j = 0; /* int total = 0 ; */
char *k, *l; /* char * list , * mover ; */
char *m = malloc(7); /* char * buf = malloc ( 7 ) ; */
memcpy(m, &g, 2); /* memcpy ( buf , & len , 2 ) ; */
memcpy(m + 2, &d, 1); /* memcpy ( buf + 2 , & flag , 1 ) ; */
for (f = 0; f<*c; f++) {
if ((*b )[f] != ((void *)0 )) /* if ((*handles )[i] != ((void *)0 )) */ {
i++; /* num ++ ; */
}
}
uint32_t n = htonl(i); /* uint32_t numHandles = htonl ( num ) ; */
memcpy(m + 3, &n, 4); /* memcpy ( buf + 3 , & numHandles , 4 ) ; */
h = mySend(a, m, 7, 0); /* sent = mySend ( client_socket , buf , 7 , 0 ) ; */
g = htons(0); /* len = htons ( 0 ) ; */
d = 12; /* flag = 12 ; */
for (f = 0; f<*c; f++) {
if ((*b )[f] != ((void *)0 )) /* if ((*handles )[i] != ((void *)0 )) */ {
j += strlen((*b )[f]); /* total += strlen ( ( * handles ) [ i ] ) ; */
}
}
k = malloc(3 + i + j); /* list = malloc ( 3 + num + total ) ; */
l = k; /* mover = list ; */
memcpy(l, &g, 2); /* memcpy ( mover , & len , 2 ) ; */
l += 2; /* mover += 2 ; */
memcpy(l, &d, 1); /* memcpy ( mover , & flag , 1 ) ; */
l += 1; /* mover += 1 ; */
for (f = 0; f<*c; f++) {
if ((*b )[f] != ((void *)0 )) /* if ((*handles )[i] != ((void *)0 )) */ {
l += 1; /* mover += 1 ; */
e = strlen((*b )[f]); /* handlelen = strlen ( ( * handles ) [ i ] ) ; */
memcpy(l, &e, 1); /* memcpy ( mover , & handlelen , 1 ) ; */
memcpy(l, (*b )[f], e); /* memcpy ( mover , ( * handles ) [ i ] , handlelen ) ; */
}
}
h = mySend(a, k, 3 + i + j, 0); /* sent = mySend ( client_socket , list , 3 + num + total , 0 ) ; */
}
void parsePacket(char *a, int b, char ***c, int *d) /* void parsePacket ( char * buf , int client_socket , char * * * handles , int * count ) { */{
uint16_t e; /* uint16_t len ; */
char *f = malloc(3); /* char * stoc = malloc ( 3 ) ; */
int g; /* int i ; */
uint8_t h; /* uint8_t flag ; */
char *i = stoc; /* char * mover = stoc ; */
memcpy(&e, a, 2); /* memcpy ( & len , buf , 2 ) ; */
uint16_t j = ntohs(e); /* uint16_t packetLen = ntohs ( len ) ; */
memcpy(&h, a + 2, 1); /* memcpy ( & flag , buf + 2 , 1 ) ; */
if (h == (uint8_t)1) /* if (flag == (uint8_t)1) */ {
uint8_t k; /* uint8_t handleLen ; */
int l; /* int sent ; */
int m = 0; /* int check = 0 ; */
uint16_t n = 3; /* uint16_t pLen = 3 ; */
memcpy(&k, a + 3, 1); /* memcpy ( & handleLen , buf + 3 , 1 ) ; */
char *o = malloc(k); /* char * handle = malloc ( handleLen ) ; */
memcpy(o, a + 4, k); /* memcpy ( handle , buf + 4 , handleLen ) ; */
i += 2; /* mover += 2 ; */
for (g = 0; g<*d; g++) {
if ((*c )[g] != ((void *)0 )&&strcmp((*c )[g], o) == 0) /* if ((*handles )[i] != ((void *)0 )&&strcmp((*handles )[i], handle) == 0) */ {
m = 1; /* check = 1 ; */
}
}
uint16_t p = htons(n); /* uint16_t npLen = htons ( pLen ) ; */
memcpy(i, &p, 2); /* memcpy ( mover , & npLen , 2 ) ; */
if (m) /* if (check) */ {
uint8_t q = 3; /* uint8_t pFlag = 3 ; */
memcpy(i, &q, 1); /* memcpy ( mover , & pFlag , 1 ) ; */
} else {
uint8_t r; /* uint8_t handleLen ; */
uint8_t s = 2; /* uint8_t pFlag = 2 ; */
memcpy(i, &s, 1); /* memcpy ( mover , & pFlag , 1 ) ; */
memcpy(&r, a + 3, 1); /* memcpy ( & handleLen , buf + 3 , 1 ) ; */
(*c )[b] = malloc(r); /* ( * handles ) [ client_socket ] = malloc ( handleLen ) ; */
memcpy((*c )[b], a + 4, r); /* memcpy ( ( * handles ) [ client_socket ] , buf + 4 , handleLen ) ; */
}
l = mySend(b, f, n, 0); /* sent = mySend ( client_socket , stoc , pLen , 0 ) ; */
if (m) /* if (check) */ {
close(b); /* close ( client_socket ) ; */
}
if (l<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
} else if (h == (uint8_t)5) /* if (flag == (uint8_t)5) */ {
parseMessage(a, b, c, d, j); /* parseMessage ( buf , client_socket , handles , count , packetLen ) ; */
} else if (h == 4) /* if (flag == 4) */ {
parseBroadcast(a, b, c, d, j); /* parseBroadcast ( buf , client_socket , handles , count , packetLen ) ; */
} else if (h == 8) /* if (flag == 8) */ {
parseExit(a, b, c, d, j); /* parseExit ( buf , client_socket , handles , count , packetLen ) ; */
} else if (h == 10) /* if (flag == 10) */ {
parseList(b, c, d); /* parseList ( client_socket , handles , count ) ; */
}
}
int findLargest(fd_set *a, int b, char ***c) /* int findLargest ( fd_set * fdvar , int count , char * * * handles ) { */{
int d = 3; /* int maxfd = 3 ; */
int e; /* int i ; */
for (e = 3; e<b; e++) {
if ((*c )[e] != ((void *)0 )&&e>d) /* if ((*handles )[i] != ((void *)0 )&&i>maxfd) */ {
d = e; /* maxfd = i ; */
}
}
return d; /* return maxfd */
}
void testClear(char ***a, int b, fd_set *c) /* void testClear ( char * * * handles , int count , fd_set * fdvar ) { */{
int d; /* int i ; */
FD_ZERO(c); /* FD_ZERO ( fdvar ) ; */
FD_SET(3, c); /* FD_SET ( 3 , fdvar ) ; */
for (d = 0; d<b; d++) {
if ((*a )[d] != ((void *)0 )) /* if ((*handles )[i] != ((void *)0 )) */ {
FD_SET(d, c); /* FD_SET ( i , fdvar ) ; */
}
}
}
void initHandles(char ***a, int b) /* void initHandles ( char * * * handles , int count ) { */{
int c; /* int i ; */
*a = malloc(sizeof(char *) * b); /* * handles = malloc ( sizeof ( char * ) * count ) ; */
for (c = 0; c<b; c++) {
(*a )[c] = ((void *)0 ); /* ( * handles ) [ i ] = ( ( void * ) 0 ) ; */
}
}
void init(char *a) /* void init ( char * argv ) { */{
fd_set b; /* fd_set fdvar ; */
char **c; /* char * * handles ; */
int d, e, f, g = 0; /* int server_socket , client_socket , i , message_len = 0 ; */
char *h = ((void *)0 ); /* char * buf = ( ( void * ) 0 ) ; */
int i = 5; /* int count = 5 ; */
initHandles(&c, i); /* initHandles ( & handles , count ) ; */
h = (char *)malloc(1024); /* buf = ( char * ) malloc ( 1024 ) ; */
d = tcpServerSetup(a); /* server_socket = tcpServerSetup ( argv ) ; */
FD_ZERO(&b); /* FD_ZERO ( & fdvar ) ; */
FD_SET(d, &b); /* FD_SET ( server_socket , & fdvar ) ; */
while (1) /* while (1) */ {
int j = findLargest(&b, i, &c); /* int largest = findLargest ( & fdvar , count , & handles ) ; */
if (j == i - 1) /* if (largest == count - 1) */ {
c = (char **)realloc(c, sizeof(char *) * i * 2); /* handles = ( char * * ) realloc ( handles , sizeof ( char * ) * count * 2 ) ; */
i = i * 2; /* count = count * 2 ; */
}
mySelect(j + 1, &b, ((void *)0 ), ((void *)0 ), ((void *)0 )); /* mySelect ( largest + 1 , & fdvar , ( ( void * ) 0 ) , ( ( void * ) 0 ) , ( ( void * ) 0 ) ) ; */
if (FD_ISSET(d, &b)) /* if (FD_ISSET(server_socket, &fdvar)) */ {
e = tcpAccept(d); /* client_socket = tcpAccept ( server_socket ) ; */
FD_SET(e, &b); /* FD_SET ( client_socket , & fdvar ) ; */
}
for (f = 4; f<=j + 1; f++) {
if (FD_ISSET(f, &b)) /* if (FD_ISSET(i, &fdvar)) */ {
if ((g = myRecv(f, h, 1024, 0) )<0) /* if ((message_len = myRecv(i, buf, 1024, 0) )<0) */ {
perror("recv call"); /* perror ( "recv call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
parsePacket(h, f, &c, &i); /* parsePacket ( buf , i , & handles , & count ) ; */
}
}
testClear(&c, i, &b); /* testClear ( & handles , count , & fdvar ) ; */
}
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
init(b[1]); /* init ( argv [ 1 ] ) ; */
return 0; /* return 0 */
}
