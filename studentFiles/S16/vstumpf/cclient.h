#ifndef CCLIENT_H
#define CCLIENT_H

#include "types.h"

#define MAX_CMD_LEN 2

#define MAX_MSG 1000
enum state {
   ST_ACTIVE = 0,
   ST_H_ACK_WAIT,
   ST_EXIT_ACK_WAIT,
   ST_LIST_ACK_WAIT,
   ST_DISCONNECT,
};

typedef struct cclient {
   char * handle;
   int8 hlen;
   int socket;
   int state;
}__attribute__((packed)) cclient;

void checkArgs(int, char **);
void setFlag(int);
void setLen(int);
void setData(int offset, int8 * data, int len);
cclient * init(char **);
void init_ack(cclient *);
void sendMessage(int, char *);
void chat(cclient *);
void parse_packet(cclient *);
void print_prompt();
void parse_input(cclient *);
void send_message(cclient *);
void send_dc(cclient *);
void disconnect(cclient *);
void parse_message(cclient *);
void parse_error(cclient *);
void broadcast(cclient *);
void parse_broadcast(cclient *);
void send_ls(cclient *);
int parse_list(cclient *);
void parse_list2(cclient *);
#endif
