#ifndef __LIB_H
#define __LIB_H

#define CMD_OFFSET_IP 2
#define CMD_OFFSET_PORT 3
#define CMD_OFFSET_NAME 1
#define MAX_NAME_SIZE 255
#define MAX_INPUT_SIZE 1000
#define MAX_PACKET_LEN 1515

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define ALREADY_EXISTS -3
#define NO_RECIPIENT -7
#define JOIN_ACK -2
#define EXIT_ACK -9
#define NUM_CLIENTS -11
#define LIST_CLIENTS -12

typedef enum {FALSE, TRUE} BOOLEAN;

#endif
