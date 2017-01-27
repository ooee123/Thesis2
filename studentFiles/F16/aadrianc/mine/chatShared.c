

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int test() /* int test ( ) { */{
char *a = "xyz"; /* char * h2 = "xyz" ; */
char *b = "Hello world"; /* char * mssg = "Hello world" ; */
char *c; /* char * handle1 ; */
char *d; /* char * myMssg ; */
char *e; /* char * handle2 ; */
char *f = "abc"; /* char * h1 = "abc" ; */
packetHandle *g = makePacketMssg(1, getHLen(f), f, getHLen(a), a, b); /* packetHandle * hPack = makePacketMssg ( 1 , getHLen ( h1 ) , h1 , getHLen ( h2 ) , h2 , mssg ) ; */
printf("%d\n", headerGetLen(&(g->head ))); /* printf ( "%d\n" , headerGetLen ( & ( hPack -> head ) ) ) ; */
getDestHandle(g, &c); /* getDestHandle ( hPack , & handle1 ) ; */
getSrcHandle(g, &e); /* getSrcHandle ( hPack , & handle2 ) ; */
getMssg(g, &b); /* getMssg ( hPack , & mssg ) ; */
printf("%s\n", c); /* printf ( "%s\n" , handle1 ) ; */
printf("%s\n", e); /* printf ( "%s\n" , handle2 ) ; */
printf("%s\n", b); /* printf ( "%s\n" , mssg ) ; */
return 0; /* return 0 */
}
void headerSetLen(header *a, uint16_t b) /* void headerSetLen ( header * head , uint16_t len ) { */{
a->packetLen = htons(b + sizeof(header)); /* head -> packetLen = htons ( len + sizeof ( header ) ) ; */
}
uint16_t headerGetLen(header *a) /* uint16_t headerGetLen ( header * head ) { */{
return ntohs(a->packetLen) - sizeof(header); /* return ntohs(head->packetLen) - sizeof(header) */
}
void packetHandleAckSetNum(packetHandleAck *a, uint32_t b) /* void packetHandleAckSetNum ( packetHandleAck * packet , uint32_t num ) { */{
a->numHandles = htonl(b); /* packet -> numHandles = htonl ( num ) ; */
}
uint32_t packetHandleAckGetNum(packetHandleAck *a) /* uint32_t packetHandleAckGetNum ( packetHandleAck * packet ) { */{
return ntohl(a->numHandles); /* return ntohl(packet->numHandles) */
}
void * makePacketHandle(uint8_t a, uint8_t b, char *c) /* void * makePacketHandle ( uint8_t flag , uint8_t hLen , char * handle ) { */{
uint16_t d = b + 1; /* uint16_t len = hLen + 1 ; */
void *e = malloc(d + sizeof(header)); /* void * packet = malloc ( len + sizeof ( header ) ) ; */
packetHandle *f = (packetHandle *)e; /* packetHandle * hPack = ( packetHandle * ) packet ; */
f->head.flag = a; /* hPack -> head . flag = flag ; */
headerSetLen(&(f->head ), d); /* headerSetLen ( & ( hPack -> head ) , len ) ; */
f->handleLen = b; /* hPack -> handleLen = hLen ; */
memcpy(&(f->handleStart ), c, b); /* memcpy ( & ( hPack -> handleStart ) , handle , hLen ) ; */
return e; /* return packet */
}
void * makePacketBroadcast(uint8_t a, uint8_t b, char *c, char *d) /* void * makePacketBroadcast ( uint8_t flag , uint8_t hLen , char * handle , char * mssg ) { */{
unsigned e = strlen(d) + 1; /* unsigned mssgLen = strlen ( mssg ) + 1 ; */
uint16_t f = b + 1 + e; /* uint16_t len = hLen + 1 + mssgLen ; */
void *g = malloc(f + sizeof(header)); /* void * packet = malloc ( len + sizeof ( header ) ) ; */
packetHandle *h = (packetHandle *)g; /* packetHandle * hPack = ( packetHandle * ) packet ; */
h->head.flag = a; /* hPack -> head . flag = flag ; */
headerSetLen((header *)h, f); /* headerSetLen ( ( header * ) hPack , len ) ; */
h->handleLen = b; /* hPack -> handleLen = hLen ; */
memcpy(&(h->handleStart ), c, b); /* memcpy ( & ( hPack -> handleStart ) , handle , hLen ) ; */
memcpy(&(h->handleStart ) + b, d, e); /* memcpy ( & ( hPack -> handleStart ) + hLen , mssg , mssgLen ) ; */
return g; /* return packet */
}
void * makePacketMssg(uint8_t a, uint8_t b, char *c, uint8_t d, char *e, char *f) /* void * makePacketMssg ( uint8_t flag , uint8_t destLen , char * destH , uint8_t srcLen , char * srcH , char * mssg ) { */{
unsigned g = strlen(f) + 1; /* unsigned mssgLen = strlen ( mssg ) + 1 ; */
uint16_t h = b + 1 + d + 1 + g; /* uint16_t len = destLen + 1 + srcLen + 1 + mssgLen ; */
void *i = malloc(h + sizeof(header)); /* void * packet = malloc ( len + sizeof ( header ) ) ; */
packetHandle *j = (packetHandle *)i; /* packetHandle * hPack = ( packetHandle * ) packet ; */
j->head.flag = a; /* hPack -> head . flag = flag ; */
headerSetLen(&(j->head ), h); /* headerSetLen ( & ( hPack -> head ) , len ) ; */
j->handleLen = b; /* hPack -> handleLen = destLen ; */
memcpy(&(j->handleStart ), c, b); /* memcpy ( & ( hPack -> handleStart ) , destH , destLen ) ; */
*((uint8_t *)&(j->handleStart ) + b ) = d; /* * ( ( uint8_t * ) & ( hPack -> handleStart ) + destLen ) = srcLen ; */
memcpy(&(j->handleStart ) + b + 1, e, d); /* memcpy ( & ( hPack -> handleStart ) + destLen + 1 , srcH , srcLen ) ; */
memcpy(&(j->handleStart ) + b + 1 + d, f, g); /* memcpy ( & ( hPack -> handleStart ) + destLen + 1 + srcLen , mssg , mssgLen ) ; */
return i; /* return packet */
}
void getFirstHandle(void *a, char **b) /* void getFirstHandle ( void * packet , char * * handle ) { */{
packetHandle *c = (packetHandle *)a; /* packetHandle * hPack = ( packetHandle * ) packet ; */
*b = malloc(c->handleLen + 1); /* * handle = malloc ( hPack -> handleLen + 1 ) ; */
memcpy(*b, &(c->handleStart ), c->handleLen); /* memcpy ( * handle , & ( hPack -> handleStart ) , hPack -> handleLen ) ; */
(*b )[c->handleLen] = 0; /* ( * handle ) [ hPack -> handleLen ] = 0 ; */
}
void getDestHandle(void *a, char **b) /* void getDestHandle ( void * packet , char * * handle ) { */{
getFirstHandle(a, b); /* getFirstHandle ( packet , handle ) ; */
}
void getSrcHandle(void *a, char **b) /* void getSrcHandle ( void * packet , char * * handle ) { */{
getSecondHandle(a, b); /* getSecondHandle ( packet , handle ) ; */
}
void getSecondHandle(void *a, char **b) /* void getSecondHandle ( void * packet , char * * handle ) { */{
packetHandle *c = (packetHandle *)a; /* packetHandle * hPack = ( packetHandle * ) packet ; */
c = ((void *)c ) + c->handleLen + sizeof(packetHandle) - 1 - sizeof(header); /* hPack = ( ( void * ) hPack ) + hPack -> handleLen + sizeof ( packetHandle ) - 1 - sizeof ( header ) ; */
*b = malloc(c->handleLen + 1); /* * handle = malloc ( hPack -> handleLen + 1 ) ; */
memcpy(*b, &(c->handleStart ), c->handleLen); /* memcpy ( * handle , & ( hPack -> handleStart ) , hPack -> handleLen ) ; */
(*b )[c->handleLen] = 0; /* ( * handle ) [ hPack -> handleLen ] = 0 ; */
}
void getMssg(void *a, char **b) /* void getMssg ( void * packet , char * * mssg ) { */{
unsigned c; /* unsigned srcLen ; */
packetHandle *d = (packetHandle *)a; /* packetHandle * hPack = ( packetHandle * ) packet ; */
unsigned e = d->handleLen; /* unsigned destLen = hPack -> handleLen ; */
unsigned f = headerGetLen(a); /* unsigned packetLen = headerGetLen ( packet ) ; */
d = ((void *)d ) + d->handleLen + sizeof(packetHandle) - 1 - sizeof(header); /* hPack = ( ( void * ) hPack ) + hPack -> handleLen + sizeof ( packetHandle ) - 1 - sizeof ( header ) ; */
c = d->handleLen; /* srcLen = hPack -> handleLen ; */
unsigned g = f - (e + 1 ) - (c + 1 ); /* unsigned mssgLen = packetLen - ( destLen + 1 ) - ( srcLen + 1 ) ; */
*b = malloc(g); /* * mssg = malloc ( mssgLen ) ; */
memcpy(*b, &(d->handleStart ) + d->handleLen, g); /* memcpy ( * mssg , & ( hPack -> handleStart ) + hPack -> handleLen , mssgLen ) ; */
}
void getBroadCastMssg(void *a, char **b) /* void getBroadCastMssg ( void * packet , char * * mssg ) { */{
packetHandle *c = (packetHandle *)a; /* packetHandle * hPack = ( packetHandle * ) packet ; */
unsigned d = headerGetLen(a) - (c->handleLen + 1 ); /* unsigned mssgLen = headerGetLen ( packet ) - ( hPack -> handleLen + 1 ) ; */
*b = malloc(d); /* * mssg = malloc ( mssgLen ) ; */
memcpy(*b, &(c->handleStart ) + c->handleLen, d); /* memcpy ( * mssg , & ( hPack -> handleStart ) + hPack -> handleLen , mssgLen ) ; */
}
uint8_t getHLen(char *a) /* uint8_t getHLen ( char * handle ) { */{
unsigned b = strlen((char *)a); /* unsigned len = strlen ( ( char * ) handle ) ; */
if (b>getMaxHLen()) /* if (len>getMaxHLen()) */ {
printf("The handle you entered is too long\n"); /* printf ( "The handle you entered is too long\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return b; /* return len */
}
uint8_t getMaxHLen() /* uint8_t getMaxHLen ( ) { */{
uint8_t a = 0; /* uint8_t max = 0 ; */
return a - 1; /* return max - 1 */
}
void sendPacketFlag(uint8_t a, int b) /* void sendPacketFlag ( uint8_t flag , int socket ) { */{
header c; /* header head ; */
headerSetLen(&c, 0); /* headerSetLen ( & head , 0 ) ; */
c.flag = a; /* head . flag = flag ; */
if (mySend(b, &c, sizeof(header), 0)<0) /* if (mySend(socket, &head, sizeof(header), 0)<0) */ {
perror("Flag Packet Send"); /* perror ( "Flag Packet Send" ) ; */
exit(1); /* exit ( 1 ) ; */
}
}
int sendPacket(void *a, int b) /* int sendPacket ( void * packet , int socket ) { */{
return mySend(b, a, headerGetLen((header *)a) + sizeof(header), 0); /* return mySend(socket, packet, headerGetLen((header *)packet) + sizeof(header), 0) */
}
