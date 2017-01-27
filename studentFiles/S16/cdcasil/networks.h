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
#include <ctype.h>

#define MAX_LEN_MSG 999
#define MAX_HANDLE 255
#define FLAG_1 1
#define FLAG_2 2
#define FLAG_3 3
#define FLAG_B 4
#define FLAG_M 5
#define FLAG_6 6
#define FLAG_7 7
#define FLAG_E 8
#define FLAG_ACK 9
#define FLAG_L 10
#define FLAG_11 11
#define FLAG_12 12
#define FLAG_13 13

typedef struct __attribute__((packed)) {
   u_int seqNum;
   u_short packetLength;
   u_char flag;
} NormalHeader;

typedef struct __attribute__((packed)) {
   NormalHeader header;
   u_char text_message[MAX_LEN_MSG];
} Message;

typedef struct __attribute__((packed)) {
   int socketCount;
   u_char handle_len;
   u_char handle[255];
} Client;