#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "testing.h"

#define MAXBUF 1024
#define MAXSOCKETS 10
#define MAXMESSAGE 1000
#define MAXHANDLE 255
#define CHAT_HEAD_LEN 3

int numHandles = 0;

struct client {
   int socketNum;
   char* handle;
   unsigned short handleLen;
   int isAlive;
};

struct chatHeader {
   unsigned short length;
   char flag;
};
