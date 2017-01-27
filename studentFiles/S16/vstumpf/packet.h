#ifndef PACKET_H
#define PACKET_H

#include "types.h"

typedef struct header {
   int16 len;
   int8 flag;
}__attribute__((packed)) header;

typedef struct header2 {
   int16 len;
   int8 flag;
   int32 len2;
}__attribute__((packed)) header2;

#define HDR_LEN 3
#define MAX_PKT_LEN 2000
#define MAX_H_LEN 255

#define PF_INIT 1
#define PF_INIT_ACK_G 2
#define PF_INIT_ACK_B 3
#define PF_BCAST 4
#define PF_MES 5
#define PF_ERROR 7
#define PF_C_EXIT 8
#define PF_C_EXIT_ACK 9
#define PF_LIST 10
#define PF_LIST_ACK 11
#define PF_LIST_ACK2 12 


#endif
