
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "testing.h"
#include "networks.h"

#define BUFF_SIZE 1024
#define SIZE_HEADER 3
#define MAX_TEXT_SIZE 999
#define LENGTH_BYTES 2
#define MAX_USER_INPUT 2000

struct Message {
   uint8_t destLen;
   char *dest;
   uint8_t srcLen;
   char *src;
}__attribute__((__packed__));
typedef struct Message Message;

struct Broadcast{
   uint8_t length;
   char *handle;
}__attribute__((__packed__));
typedef struct Broadcast Broadcast;

struct Header {
   uint16_t length;
   uint8_t flag;
}__attribute__((__packed__));
typedef struct Header Header;

struct Packet {
   Header header;
   char *data;
}__attribute__((__packed__));
typedef struct Packet Packet;

struct Socket {
   char *handle;
   uint32_t socketNum;
}__attribute__((__packed__));
typedef struct Socket Socket;

struct SocketBin {
   uint32_t size;
   uint32_t numSockets;
   Socket *sockets;
}__attribute__((__packed__));
typedef struct SocketBin SocketBin;

char *readDataFromSocket(int socket_num, int numBytesLen);
char *genPacket(char *message, uint8_t flag, int length, int clear);
int sendPacket(char *packet, int socketNum, int freeFlag);
void increaseSocketBin(SocketBin *socketBin);
void addSocket(SocketBin *socketBin, char *handle, uint32_t socketNum);
Socket *listenSockets(fd_set *readFds,  int max, SocketBin *socketBin);
void removeSocket(SocketBin *socketBin, int clientSocket);
char *genMessage(uint8_t flag, int length, char *srcHandle, char *buff, int more);






