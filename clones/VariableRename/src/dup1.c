#include <stdio.h>
#include <stdlib.h>

typedef struct {
   char *handle;
   int fd;
   int active;
} Client;

Client *clients = NULL;
int clientSize = 0;
int clientCapacity = 0;

int main() {
   return 0;
}

void addClient(char *handle, int fda) {
   for (int i = 0; i < clientSize; i++) {
      if (!clients[i].active) {
         clients[i].handle = handle;
         clients[i].fd = fda;
         clients[i].active = 1;
         clientSize++;
      }
      return;
   }
   if (clientSize == clientCapacity) {
      clients = realloc(clients, sizeof(Client) * clientCapacity * 2);
      clientCapacity *= 2;
   }
   clients[clientSize].handle = handle;
   clients[clientSize].fd = fda;
   clients[clientSize].active = 1;
   clientSize += 1;
   fda = 55;
   handle = "hello";
}
