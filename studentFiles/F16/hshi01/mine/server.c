typedef struct handle {
char name;
int sock;
struct handle *next;
} handle;

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void SendPackHdr(handle *a, char b) /* void SendPackHdr ( handle * csock , char flag ) { */{
int c; /* int sent ; */
short d = 3; /* short len = 3 ; */
char e[len]; /* char buf [ len ] ; */
short f = htons(d); /* short nolen = htons ( len ) ; */
memcpy(e, (char *)&f, 2); /* memcpy ( buf , ( char * ) & nolen , 2 ) ; */
e[2] = b; /* buf [ 2 ] = flag ; */
c = mySend(a->sock, e, d, 0); /* sent = mySend ( csock -> sock , buf , len , 0 ) ; */
if (c<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
handle * AddHandle(int a, handle *b) /* handle * AddHandle ( int newsock , handle * csock ) { */{
char *c; /* char * buf ; */
unsigned char d; /* unsigned char hdllen ; */
char e = 0; /* char flag = 0 ; */
short f = 0; /* short packlen = 0 ; */
int g = 0; /* int hdlexists = 0 ; */
handle *h = calloc(1, sizeof(handle)); /* handle * temp = calloc ( 1 , sizeof ( handle ) ) ; */
handle *i; /* handle * ndx ; */
handle *j = csock; /* handle * head = csock ; */
myRecv(a, (char *)&f, 2, 0); /* myRecv ( newsock , ( char * ) & packlen , 2 , 0 ) ; */
f = ntohs(f) - 3; /* packlen = ntohs ( packlen ) - 3 ; */
c = malloc(f); /* buf = malloc ( packlen ) ; */
myRecv(a, &e, 1, 0); /* myRecv ( newsock , & flag , 1 , 0 ) ; */
myRecv(a, c, f, 0); /* myRecv ( newsock , buf , packlen , 0 ) ; */
d = c[0]; /* hdllen = buf [ 0 ] ; */
h->sock = a; /* temp -> sock = newsock ; */
memcpy(h->name, &c[1], d); /* memcpy ( temp -> name , & buf [ 1 ] , hdllen ) ; */
if (!b) /* if (!csock) */ {
j = h; /* head = temp ; */
h->next = ((void *)0 ); /* temp -> next = ( ( void * ) 0 ) ; */
SendPackHdr(h, 2); /* SendPackHdr ( temp , 2 ) ; */
} else {
i = j; /* ndx = head ; */
h->next = j; /* temp -> next = head ; */
while (i&&!g) /* while (ndx&&!hdlexists) */ {
if (!strcmp(i->name, h->name)) /* if (!strcmp(ndx->name, temp->name)) */ {
g = 1; /* hdlexists = 1 ; */
SendPackHdr(h, 3); /* SendPackHdr ( temp , 3 ) ; */
free(h); /* free ( temp ) ; */
}
i = i->next; /* ndx = ndx -> next ; */
}
if (!g) /* if (!hdlexists) */ {
j = h; /* head = temp ; */
SendPackHdr(h, 2); /* SendPackHdr ( temp , 2 ) ; */
}
}
return j; /* return head */
}
void Broadcast(short a, handle *b, handle *c) /* void Broadcast ( short packlen , handle * client , handle * head ) { */{
unsigned char d; /* unsigned char hdllen ; */
char e[packlen + 3]; /* char sendbuf [ packlen + 3 ] ; */
char *f; /* char * shdl ; */
handle *g; /* handle * temp ; */
char *h = calloc(a, 1); /* char * buf = calloc ( packlen , 1 ) ; */
short i = htons(a + 3); /* short nolen = htons ( packlen + 3 ) ; */
myRecv(b->sock, h, a, 0); /* myRecv ( client -> sock , buf , packlen , 0 ) ; */
g = c; /* temp = head ; */
d = h[0]; /* hdllen = buf [ 0 ] ; */
f = calloc(d + 1, 1); /* shdl = calloc ( hdllen + 1 , 1 ) ; */
memcpy(f, &h[1], d); /* memcpy ( shdl , & buf [ 1 ] , hdllen ) ; */
memcpy(e, (char *)&i, 2); /* memcpy ( sendbuf , ( char * ) & nolen , 2 ) ; */
e[2] = 4; /* sendbuf [ 2 ] = 4 ; */
memcpy(&e[3], h, a); /* memcpy ( & sendbuf [ 3 ] , buf , packlen ) ; */
while (g) /* while (temp) */ {
if (strcmp(g->name, f)) /* if (strcmp(temp->name, shdl)) */ {
mySend(g->sock, e, a + 3, 0); /* mySend ( temp -> sock , sendbuf , packlen + 3 , 0 ) ; */
}
g = g->next; /* temp = temp -> next ; */
}
}
void MessageError(handle *a, char b, char *c) /* void MessageError ( handle * client , char hdllen , char * dhdl ) { */{
int d; /* int sent ; */
char e[len]; /* char buf [ len ] ; */
short f = 3 + b + 1; /* short len = 3 + hdllen + 1 ; */
short g = htons(f); /* short nolen = htons ( len ) ; */
memcpy(e, (char *)&g, 2); /* memcpy ( buf , ( char * ) & nolen , 2 ) ; */
e[2] = 7; /* buf [ 2 ] = 7 ; */
e[3] = b; /* buf [ 3 ] = hdllen ; */
memcpy(&e[4], c, b); /* memcpy ( & buf [ 4 ] , dhdl , hdllen ) ; */
d = mySend(a->sock, e, f, 0); /* sent = mySend ( client -> sock , buf , len , 0 ) ; */
if (d<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void Message(short a, handle *b, handle *c) /* void Message ( short packlen , handle * client , handle * head ) { */{
handle *d; /* handle * temp ; */
char e[packlen + 3]; /* char sendbuf [ packlen + 3 ] ; */
int f = 0; /* int found = 0 ; */
char *g; /* char * dhdl ; */
unsigned char h; /* unsigned char hdllen ; */
char *i = calloc(a, 1); /* char * buf = calloc ( packlen , 1 ) ; */
short j = htons(a + 3); /* short nolen = htons ( packlen + 3 ) ; */
myRecv(b->sock, i, a, 0); /* myRecv ( client -> sock , buf , packlen , 0 ) ; */
d = c; /* temp = head ; */
h = i[0]; /* hdllen = buf [ 0 ] ; */
g = calloc(h + 1, 1); /* dhdl = calloc ( hdllen + 1 , 1 ) ; */
memcpy(g, &i[1], h); /* memcpy ( dhdl , & buf [ 1 ] , hdllen ) ; */
memcpy(e, (char *)&j, 2); /* memcpy ( sendbuf , ( char * ) & nolen , 2 ) ; */
e[2] = 5; /* sendbuf [ 2 ] = 5 ; */
memcpy(&e[3], i, a); /* memcpy ( & sendbuf [ 3 ] , buf , packlen ) ; */
while (d&&!f) /* while (temp&&!found) */ {
if (!strcmp(d->name, g)) /* if (!strcmp(temp->name, dhdl)) */ {
f = 1; /* found = 1 ; */
mySend(d->sock, e, a + 3, 0); /* mySend ( temp -> sock , sendbuf , packlen + 3 , 0 ) ; */
}
d = d->next; /* temp = temp -> next ; */
}
if (!f) /* if (!found) */ {
MessageError(b, h, g); /* MessageError ( client , hdllen , dhdl ) ; */
}
}
void Flag11(handle *a, int b) /* void Flag11 ( handle * client , int num ) { */{
short c = htons(7); /* short len = htons ( 7 ) ; */
int d; /* int sent ; */
char e[7]; /* char buf [ 7 ] ; */
int f = htonl(b); /* int nonum = htonl ( num ) ; */
memcpy(e, (char *)&c, 2); /* memcpy ( buf , ( char * ) & len , 2 ) ; */
e[2] = 11; /* buf [ 2 ] = 11 ; */
memcpy(&e[3], (char *)&f, 4); /* memcpy ( & buf [ 3 ] , ( char * ) & nonum , 4 ) ; */
d = mySend(a->sock, e, 7, 0); /* sent = mySend ( client -> sock , buf , 7 , 0 ) ; */
if (d<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
}
void List(handle *a, handle *b) /* void List ( handle * client , handle * head ) { */{
int c = 0; /* int num = 0 ; */
unsigned char d; /* unsigned char hdllen ; */
char *e; /* char * buf ; */
short f; /* short len ; */
handle *g = head; /* handle * temp = head ; */
while (g) /* while (temp) */ {
c++; /* num ++ ; */
g = g->next; /* temp = temp -> next ; */
}
g = b; /* temp = head ; */
Flag11(a, c); /* Flag11 ( client , num ) ; */
while (g) /* while (temp) */ {
d = strlen(g->name); /* hdllen = strlen ( temp -> name ) ; */
f = 2 + 1 + 1 + d; /* len = 2 + 1 + 1 + hdllen ; */
f = htons(f); /* len = htons ( len ) ; */
e = calloc(f, 1); /* buf = calloc ( len , 1 ) ; */
memcpy(e, (char *)&f, 2); /* memcpy ( buf , ( char * ) & len , 2 ) ; */
e[2] = 12; /* buf [ 2 ] = 12 ; */
e[3] = d; /* buf [ 3 ] = hdllen ; */
memcpy(&e[4], g->name, d); /* memcpy ( & buf [ 4 ] , temp -> name , hdllen ) ; */
mySend(a->sock, e, ntohs(f), 0); /* mySend ( client -> sock , buf , ntohs ( len ) , 0 ) ; */
free(e); /* free ( buf ) ; */
g = g->next; /* temp = temp -> next ; */
}
}
handle * RmvClient(handle *a, handle *b) /* handle * RmvClient ( handle * client , handle * head ) { */{
handle *c; /* handle * prev ; */
int d = 0; /* int found = 0 ; */
handle *e = head; /* handle * cur = head ; */
handle *f = head; /* handle * temphead = head ; */
close(a->sock); /* close ( client -> sock ) ; */
if (!strcmp(e->name, a->name)) /* if (!strcmp(cur->name, client->name)) */ {
f = b->next; /* temphead = head -> next ; */
free(b); /* free ( head ) ; */
return f; /* return temphead */
}
e = e->next; /* cur = cur -> next ; */
while (e&&!d) /* while (cur&&!found) */ {
if (!strcmp(e->name, a->name)) /* if (!strcmp(cur->name, client->name)) */ {
d = 1; /* found = 1 ; */
c->next = e->next; /* prev -> next = cur -> next ; */
free(e); /* free ( cur ) ; */
} else {
e = e->next; /* cur = cur -> next ; */
}
}
return f; /* return temphead */
}
handle * PacketParse(handle *a, handle *b) /* handle * PacketParse ( handle * client , handle * head ) { */{
int c; /* int done ; */
char d = 0; /* char flag = 0 ; */
short e = 0; /* short packlen = 0 ; */
handle *f = head; /* handle * temp = head ; */
c = myRecv(a->sock, (char *)&e, 2, 0); /* done = myRecv ( client -> sock , ( char * ) & packlen , 2 , 0 ) ; */
if (!c) /* if (!done) */ {
f = RmvClient(a, b); /* temp = RmvClient ( client , head ) ; */
}
e = ntohs(e) - 3; /* packlen = ntohs ( packlen ) - 3 ; */
myRecv(a->sock, &d, 1, 0); /* myRecv ( client -> sock , & flag , 1 , 0 ) ; */
if (d == 4) /* if (flag == 4) */ {
Broadcast(e, a, b); /* Broadcast ( packlen , client , head ) ; */
}
if (d == 5) /* if (flag == 5) */ {
Message(e, a, b); /* Message ( packlen , client , head ) ; */
}
if (d == 8) /* if (flag == 8) */ {
SendPackHdr(a, 9); /* SendPackHdr ( client , 9 ) ; */
f = RmvClient(a, b); /* temp = RmvClient ( client , head ) ; */
}
if (d == 10) /* if (flag == 10) */ {
List(a, b); /* List ( client , head ) ; */
}
return f; /* return temp */
}
handle * CheckCon(fd_set *a, int b, handle *c) /* handle * CheckCon ( fd_set * readfds , int ssock , handle * csock ) { */{
handle *d = csock; /* handle * head = csock ; */
handle *e = csock; /* handle * temp = csock ; */
int f = ssock, g; /* int maxfds = ssock , newsock ; */
FD_ZERO(a); /* FD_ZERO ( readfds ) ; */
FD_SET(b, a); /* FD_SET ( ssock , readfds ) ; */
while (e) /* while (temp) */ {
FD_SET(e->sock, a); /* FD_SET ( temp -> sock , readfds ) ; */
if (e->sock>f) /* if (temp->sock>maxfds) */ {
f = e->sock; /* maxfds = temp -> sock ; */
}
e = e->next; /* temp = temp -> next ; */
}
mySelect(f + 1, a, ((void *)0 ), ((void *)0 ), ((void *)0 )); /* mySelect ( maxfds + 1 , readfds , ( ( void * ) 0 ) , ( ( void * ) 0 ) , ( ( void * ) 0 ) ) ; */
if (FD_ISSET(b, a)) /* if (FD_ISSET(ssock, readfds)) */ {
g = tcpAccept(b); /* newsock = tcpAccept ( ssock ) ; */
d = AddHandle(g, c); /* head = AddHandle ( newsock , csock ) ; */
}
e = d; /* temp = head ; */
while (e) /* while (temp) */ {
if (FD_ISSET(e->sock, a)) /* if (FD_ISSET(temp->sock, readfds)) */ {
d = PacketParse(e, d); /* head = PacketParse ( temp , head ) ; */
}
e = e->next; /* temp = temp -> next ; */
}
return d; /* return head */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
int c = 0; /* int ssock = 0 ; */
fd_set d; /* fd_set readfds ; */
handle *e = ((void *)0 ); /* handle * csock = ( ( void * ) 0 ) ; */
if (a == 2) /* if (argc == 2) */ {
c = tcpServerSetup(atoi(b[1])); /* ssock = tcpServerSetup ( atoi ( argv [ 1 ] ) ) ; */
} else if (a == 1) /* if (argc == 1) */ {
c = tcpServerSetup(0); /* ssock = tcpServerSetup ( 0 ) ; */
} else {
printf("Error: Incorrect number of Input Parameters\n"); /* printf ( "Error: Incorrect number of Input Parameters\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
while (1) /* while (1) */ {
e = CheckCon(&d, c, e); /* csock = CheckCon ( & readfds , ssock , csock ) ; */
}
close(c); /* close ( ssock ) ; */
return 0; /* return 0 */
}
