#include <stdio.h>
#include <stdlib.h>

typedef struct {
   char *handle;
   int fd;
   int active;
} Client;

Client *clients = NULL;
int sizem = 0;
int clientCapacity = 0;

int main() {
   return 0;
}

void addClient(char *var, int num) {
   for (int i = 0; i < sizem; i++) {
      if (!clients[i].active) {
         clients[i].handle = var;
         clients[i].fd = num;
         clients[i].active = 1;
         sizem++;
      }
      return;
   }
   if (sizem == clientCapacity) {
      clients = realloc(clients, sizeof(Client) * clientCapacity * 2);
      clientCapacity *= 2;
   }
   clients[sizem].handle = var;
   clients[sizem].fd = num;
   clients[sizem].active = 1;
   sizem += 1;
}
