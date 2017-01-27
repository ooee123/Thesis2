#ifndef SERVER_H
#define SERVER_H

#define MAXBUF 2048
#define INITIAL_HANDLES 5

struct my_socket {
   char *handle;
   uint32_t socketNum;
}__attribute__((__packed__));
typedef struct my_socket my_socket;

#endif