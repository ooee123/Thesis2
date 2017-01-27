

unsigned int __bswap_32(unsigned int __bsx) /*  */{
return __builtin_bswap32(__bsx); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t __bsx) /*  */{
return __builtin_bswap64(__bsx); /* return __builtin_bswap64(__bsx) */
}
int test() /* int test ( ) { */{
char *h2 = "xyz"; /* char * h2 = "xyz" ; */
char *mssg = "Hello world"; /* char * mssg = "Hello world" ; */
char *handle1; /* char * handle1 ; */
char *myMssg; /* char * myMssg ; */
char *handle2; /* char * handle2 ; */
char *h1 = "abc"; /* char * h1 = "abc" ; */
packetHandle *hPack = makePacketMssg(1, getHLen(h1), h1, getHLen(h2), h2, mssg); /* packetHandle * hPack = makePacketMssg ( 1 , getHLen ( h1 ) , h1 , getHLen ( h2 ) , h2 , mssg ) ; */
printf("%d\n", headerGetLen(&(hPack->head ))); /* printf ( "%d\n" , headerGetLen ( & ( hPack -> head ) ) ) ; */
getDestHandle(hPack, &handle1); /* getDestHandle ( hPack , & handle1 ) ; */
getSrcHandle(hPack, &handle2); /* getSrcHandle ( hPack , & handle2 ) ; */
getMssg(hPack, &mssg); /* getMssg ( hPack , & mssg ) ; */
printf("%s\n", handle1); /* printf ( "%s\n" , handle1 ) ; */
printf("%s\n", handle2); /* printf ( "%s\n" , handle2 ) ; */
printf("%s\n", mssg); /* printf ( "%s\n" , mssg ) ; */
return 0; /* return 0 */
}
void headerSetLen(header *head, uint16_t len) /* void headerSetLen ( header * head , uint16_t len ) { */{
head->packetLen = htons(len + sizeof(header)); /* head -> packetLen = htons ( len + sizeof ( header ) ) ; */
}
uint16_t headerGetLen(header *head) /* uint16_t headerGetLen ( header * head ) { */{
return ntohs(head->packetLen) - sizeof(header); /* return ntohs(head->packetLen) - sizeof(header) */
}
void packetHandleAckSetNum(packetHandleAck *packet, uint32_t num) /* void packetHandleAckSetNum ( packetHandleAck * packet , uint32_t num ) { */{
packet->numHandles = htonl(num); /* packet -> numHandles = htonl ( num ) ; */
}
uint32_t packetHandleAckGetNum(packetHandleAck *packet) /* uint32_t packetHandleAckGetNum ( packetHandleAck * packet ) { */{
return ntohl(packet->numHandles); /* return ntohl(packet->numHandles) */
}
void * makePacketHandle(uint8_t flag, uint8_t hLen, char *handle) /* void * makePacketHandle ( uint8_t flag , uint8_t hLen , char * handle ) { */{
uint16_t len = hLen + 1; /* uint16_t len = hLen + 1 ; */
void *packet = malloc(len + sizeof(header)); /* void * packet = malloc ( len + sizeof ( header ) ) ; */
packetHandle *hPack = (packetHandle *)packet; /* packetHandle * hPack = ( packetHandle * ) packet ; */
hPack->head.flag = flag; /* hPack -> head . flag = flag ; */
headerSetLen(&(hPack->head ), len); /* headerSetLen ( & ( hPack -> head ) , len ) ; */
hPack->handleLen = hLen; /* hPack -> handleLen = hLen ; */
memcpy(&(hPack->handleStart ), handle, hLen); /* memcpy ( & ( hPack -> handleStart ) , handle , hLen ) ; */
return packet; /* return packet */
}
void * makePacketBroadcast(uint8_t flag, uint8_t hLen, char *handle, char *mssg) /* void * makePacketBroadcast ( uint8_t flag , uint8_t hLen , char * handle , char * mssg ) { */{
unsigned mssgLen = strlen(mssg) + 1; /* unsigned mssgLen = strlen ( mssg ) + 1 ; */
uint16_t len = hLen + 1 + mssgLen; /* uint16_t len = hLen + 1 + mssgLen ; */
void *packet = malloc(len + sizeof(header)); /* void * packet = malloc ( len + sizeof ( header ) ) ; */
packetHandle *hPack = (packetHandle *)packet; /* packetHandle * hPack = ( packetHandle * ) packet ; */
hPack->head.flag = flag; /* hPack -> head . flag = flag ; */
headerSetLen((header *)hPack, len); /* headerSetLen ( ( header * ) hPack , len ) ; */
hPack->handleLen = hLen; /* hPack -> handleLen = hLen ; */
memcpy(&(hPack->handleStart ), handle, hLen); /* memcpy ( & ( hPack -> handleStart ) , handle , hLen ) ; */
memcpy(&(hPack->handleStart ) + hLen, mssg, mssgLen); /* memcpy ( & ( hPack -> handleStart ) + hLen , mssg , mssgLen ) ; */
return packet; /* return packet */
}
void * makePacketMssg(uint8_t flag, uint8_t destLen, char *destH, uint8_t srcLen, char *srcH, char *mssg) /* void * makePacketMssg ( uint8_t flag , uint8_t destLen , char * destH , uint8_t srcLen , char * srcH , char * mssg ) { */{
unsigned mssgLen = strlen(mssg) + 1; /* unsigned mssgLen = strlen ( mssg ) + 1 ; */
uint16_t len = destLen + 1 + srcLen + 1 + mssgLen; /* uint16_t len = destLen + 1 + srcLen + 1 + mssgLen ; */
void *packet = malloc(len + sizeof(header)); /* void * packet = malloc ( len + sizeof ( header ) ) ; */
packetHandle *hPack = (packetHandle *)packet; /* packetHandle * hPack = ( packetHandle * ) packet ; */
hPack->head.flag = flag; /* hPack -> head . flag = flag ; */
headerSetLen(&(hPack->head ), len); /* headerSetLen ( & ( hPack -> head ) , len ) ; */
hPack->handleLen = destLen; /* hPack -> handleLen = destLen ; */
memcpy(&(hPack->handleStart ), destH, destLen); /* memcpy ( & ( hPack -> handleStart ) , destH , destLen ) ; */
*((uint8_t *)&(hPack->handleStart ) + destLen ) = srcLen; /* * ( ( uint8_t * ) & ( hPack -> handleStart ) + destLen ) = srcLen ; */
memcpy(&(hPack->handleStart ) + destLen + 1, srcH, srcLen); /* memcpy ( & ( hPack -> handleStart ) + destLen + 1 , srcH , srcLen ) ; */
memcpy(&(hPack->handleStart ) + destLen + 1 + srcLen, mssg, mssgLen); /* memcpy ( & ( hPack -> handleStart ) + destLen + 1 + srcLen , mssg , mssgLen ) ; */
return packet; /* return packet */
}
void getFirstHandle(void *packet, char **handle) /* void getFirstHandle ( void * packet , char * * handle ) { */{
packetHandle *hPack = (packetHandle *)packet; /* packetHandle * hPack = ( packetHandle * ) packet ; */
*handle = malloc(hPack->handleLen + 1); /* * handle = malloc ( hPack -> handleLen + 1 ) ; */
memcpy(*handle, &(hPack->handleStart ), hPack->handleLen); /* memcpy ( * handle , & ( hPack -> handleStart ) , hPack -> handleLen ) ; */
(*handle )[hPack->handleLen] = 0; /* ( * handle ) [ hPack -> handleLen ] = 0 ; */
}
void getDestHandle(void *packet, char **handle) /* void getDestHandle ( void * packet , char * * handle ) { */{
getFirstHandle(packet, handle); /* getFirstHandle ( packet , handle ) ; */
}
void getSrcHandle(void *packet, char **handle) /* void getSrcHandle ( void * packet , char * * handle ) { */{
getSecondHandle(packet, handle); /* getSecondHandle ( packet , handle ) ; */
}
void getSecondHandle(void *packet, char **handle) /* void getSecondHandle ( void * packet , char * * handle ) { */{
packetHandle *hPack = (packetHandle *)packet; /* packetHandle * hPack = ( packetHandle * ) packet ; */
hPack = ((void *)hPack ) + hPack->handleLen + sizeof(packetHandle) - 1 - sizeof(header); /* hPack = ( ( void * ) hPack ) + hPack -> handleLen + sizeof ( packetHandle ) - 1 - sizeof ( header ) ; */
*handle = malloc(hPack->handleLen + 1); /* * handle = malloc ( hPack -> handleLen + 1 ) ; */
memcpy(*handle, &(hPack->handleStart ), hPack->handleLen); /* memcpy ( * handle , & ( hPack -> handleStart ) , hPack -> handleLen ) ; */
(*handle )[hPack->handleLen] = 0; /* ( * handle ) [ hPack -> handleLen ] = 0 ; */
}
void getMssg(void *packet, char **mssg) /* void getMssg ( void * packet , char * * mssg ) { */{
unsigned srcLen; /* unsigned srcLen ; */
packetHandle *hPack = (packetHandle *)packet; /* packetHandle * hPack = ( packetHandle * ) packet ; */
unsigned destLen = hPack->handleLen; /* unsigned destLen = hPack -> handleLen ; */
unsigned packetLen = headerGetLen(packet); /* unsigned packetLen = headerGetLen ( packet ) ; */
hPack = ((void *)hPack ) + hPack->handleLen + sizeof(packetHandle) - 1 - sizeof(header); /* hPack = ( ( void * ) hPack ) + hPack -> handleLen + sizeof ( packetHandle ) - 1 - sizeof ( header ) ; */
srcLen = hPack->handleLen; /* srcLen = hPack -> handleLen ; */
unsigned mssgLen = packetLen - (destLen + 1 ) - (srcLen + 1 ); /* unsigned mssgLen = packetLen - ( destLen + 1 ) - ( srcLen + 1 ) ; */
*mssg = malloc(mssgLen); /* * mssg = malloc ( mssgLen ) ; */
memcpy(*mssg, &(hPack->handleStart ) + hPack->handleLen, mssgLen); /* memcpy ( * mssg , & ( hPack -> handleStart ) + hPack -> handleLen , mssgLen ) ; */
}
void getBroadCastMssg(void *packet, char **mssg) /* void getBroadCastMssg ( void * packet , char * * mssg ) { */{
packetHandle *hPack = (packetHandle *)packet; /* packetHandle * hPack = ( packetHandle * ) packet ; */
unsigned mssgLen = headerGetLen(packet) - (hPack->handleLen + 1 ); /* unsigned mssgLen = headerGetLen ( packet ) - ( hPack -> handleLen + 1 ) ; */
*mssg = malloc(mssgLen); /* * mssg = malloc ( mssgLen ) ; */
memcpy(*mssg, &(hPack->handleStart ) + hPack->handleLen, mssgLen); /* memcpy ( * mssg , & ( hPack -> handleStart ) + hPack -> handleLen , mssgLen ) ; */
}
uint8_t getHLen(char *handle) /* uint8_t getHLen ( char * handle ) { */{
unsigned len = strlen((char *)handle); /* unsigned len = strlen ( ( char * ) handle ) ; */
if (len>getMaxHLen()) /* if (len>getMaxHLen()) */ {
printf("The handle you entered is too long\n"); /* printf ( "The handle you entered is too long\n" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return len; /* return len */
}
uint8_t getMaxHLen() /* uint8_t getMaxHLen ( ) { */{
uint8_t max = 0; /* uint8_t max = 0 ; */
return max - 1; /* return max - 1 */
}
void sendPacketFlag(uint8_t flag, int socket) /* void sendPacketFlag ( uint8_t flag , int socket ) { */{
header head; /* header head ; */
headerSetLen(&head, 0); /* headerSetLen ( & head , 0 ) ; */
head.flag = flag; /* head . flag = flag ; */
if (mySend(socket, &head, sizeof(header), 0)<0) /* if (mySend(socket, &head, sizeof(header), 0)<0) */ {
perror("Flag Packet Send"); /* perror ( "Flag Packet Send" ) ; */
exit(1); /* exit ( 1 ) ; */
}
}
int sendPacket(void *packet, int socket) /* int sendPacket ( void * packet , int socket ) { */{
return mySend(socket, packet, headerGetLen((header *)packet) + sizeof(header), 0); /* return mySend(socket, packet, headerGetLen((header *)packet) + sizeof(header), 0) */
}
