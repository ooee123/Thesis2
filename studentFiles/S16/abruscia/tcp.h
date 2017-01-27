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
#include <time.h>

#include "networks.h"
#include "testing.h"
#include "recvFunction.h"

#define NORMAL_HEAD_LEN 3
#define FLAG_OFFSET 2
#define NULL_CHAR_LEN 1
#define NULL_CHAR '\0'
#define STD_IN_FD 0

enum {FROM_SERVER, FROM_STD_IN};

#define TYPE_M_1 'M'
#define TYPE_M_2 'm'
#define TYPE_B_1 'B'
#define TYPE_B_2 'b'
#define TYPE_L_1 'L'
#define TYPE_L_2 'l'
#define TYPE_E_1 'E'
#define TYPE_E_2 'e'

#define FLAG_ONE (uint8_t) 1
#define FLAG_TWO (uint8_t) 2
#define FLAG_THREE (uint8_t) 3
#define FLAG_FOUR (uint8_t) 4
#define FLAG_FIVE (uint8_t) 5
#define FLAG_SIX (uint8_t) 6
#define FLAG_SEVEN (uint8_t) 7
#define FLAG_EIGHT (uint8_t) 8
#define FLAG_NINE (uint8_t) 9
#define FLAG_TEN (uint8_t) 10
#define FLAG_ELEVEN (uint8_t) 11
#define FLAG_TWELVE (uint8_t) 12

typedef struct client {
   int sockNum;
   uint8_t handleLen;
   char handle[256];
} client;
