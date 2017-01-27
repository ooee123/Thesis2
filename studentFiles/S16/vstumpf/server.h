#ifndef SERVER_H
#define SERVER_H

#include "types.h"

#define NUM_CLIENTS 5
enum state {
   ST_DEAD = 0,
   ST_BORN,
   ST_ACTIVE
};

typedef struct sclient {
   char * handle;
   int8 hlen;
   int socket;
   int state;
}__attribute__((packed)) sclient;

void chat(int);
void init_client(int);
void parse_packet(sclient *);
void send_packet_(sclient *, int, int);
void parse_mes(sclient *);
void parse_bcast(sclient *);
void parse_list(sclient *);
void send_list(sclient *);
#endif