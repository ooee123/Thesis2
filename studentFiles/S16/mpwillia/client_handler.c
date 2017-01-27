
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "client_handler.h"

// adds a new client connection with the given socket
// returns -1 on error
int addClient(int socket, struct connections* cons) {
   int i, addidx = -1;

   // find the next open slot in the clients array
   for(i = 0; i < cons->max; i++) {
      // set addidx to the first open spot
      if(cons->clients[i] == NULL && addidx < 0) {
         addidx = i;
      }
      // check we already have a client at that socket (for whatever reason)
      else if(cons->clients[i] != NULL && cons->clients[i]->socket == socket) {
         return -1;
      } 
   } 
   
   // if we didn't find an empty spot then make the array bigger
   if(addidx < 0) {
      addidx = cons->max;
      cons->max *= 2;
      cons->clients = realloc(cons->clients, cons->max * sizeof(struct client*));
   } 

   // add the new client
   cons->clients[addidx] = malloc(sizeof(struct client));
   memset(cons->clients[addidx]->handle, 0, MAX_HANDLE_LEN);
   cons->clients[addidx]->socket = socket;
   cons->num++;

   return addidx;
} 

void setClientHandle(char* handle, int socket, struct connections* cons) {
   struct client* c = getClient(socket, cons);
   strcpy(c->handle, handle);
} 

// removes the client at the given socket
// returns -1 if there is no client with that socket; 0 on success
int removeClient(int socket, struct connections* cons) {
   int i;
   for(i = 0; i < cons->max; i++) {
      if(cons->clients[i] != NULL && cons->clients[i]->socket == socket) {
         free(cons->clients[i]);
         cons->clients[i] = NULL;
         cons->num--;
         return 0;
      } 
   } 
   return -1;
} 


// gets the client who has the given socket
// returns a pointer to the client if found or NULL if not found
struct client* getClient(int socket, const struct connections* cons) {
   int i;
   for(i = 0; i < cons->max; i++) {
      if(cons->clients[i] != NULL && cons->clients[i]->socket == socket)
         return cons->clients[i]; 
   } 
   return NULL;
} 


// gets the client who has the given handle
// returns a pointer to the client if found or NULL if not found
struct client* getClientByHandle(const char* handle, const struct connections* cons) {
   int i;
   for(i = 0; i < cons->max; i++) {
      if(cons->clients[i] != NULL && strcmp(cons->clients[i]->handle, handle) == 0)
         return cons->clients[i];
   } 
   return NULL;
} 

int getClientSocket(const char* handle, const struct connections* cons) {
   struct client* c = getClientByHandle(handle, cons);
   if(c == NULL)
      return -1;
   return c->socket;
} 


int getFDMax(const struct connections* cons) {
   int i, max = 0;
   for(i = 0; i < cons->max; i++) {
      if(cons->clients[i] != NULL && cons->clients[i]->socket > max) 
         max = cons->clients[i]->socket;
   } 
   return max;
} 


