
#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "packet.h"

struct client {
   char handle[MAX_HANDLE_LEN];
   int socket;
};


#define INITIAL_CAPACITY 1
struct connections {
   unsigned int num;
   unsigned int max;

   struct client** clients;
};

void printClient(const struct client* c);
void listClients(const struct connections* cons);


int addClient(int socket, struct connections* cons);
void setClientHandle(char* handle, int socket, struct connections* cons);
int removeClient(int socket, struct connections* cons);
struct client* getClient(int socket, const struct connections* cons);
struct client* getClientByHandle(const char* handle, const struct connections* cons);
int getClientSocket(const char* handle, const struct connections* cons);
int getFDMax(const struct connections* cons);

#endif

