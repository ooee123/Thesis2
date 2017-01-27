struct client{
   char handle[MAX_STR];
   int socket;
   u_char buf[MAX_BUF];
   struct client* next;
};

struct {
   int count;
   struct client* head;
   struct client* tail;
   int maxFd;
   char* hanldeList[MAX_STR];
} typedef clientTable;

int addClient(fd_set* rfds, clientTable* table, int server_socket) {
   struct client* client = calloc(1, sizeof(struct client));
   client->socket = tcp_accept(server_socket, 5);
   table->count += 1;
   table->maxFd = table->maxFd > client->socket  
      ? table->maxFd : client->socket + 1;
   FD_SET(client->socket, rfds);
   if (table->count == 1) {
      table->head = client;
      table->tail = client;
   } else {
      table->tail->next = client;
      table->tail = client;
   }
   return 0;
}
void checkServerSocket(fd_set* rfds, clientTable* table, int server_socket) {
   if (FD_ISSET(server_socket, rfds)) {
      addClient(rfds, table, server_socket);
   }
}
int removeNewClient(clientTable* table) {
   if (table->count == 1) {
      close(table->head->socket);
      free(table->head);
      table->head == NULL;
      table->tail == NULL;
      table->count = 0;
      return 1;
   } else {
      int i = 0;
      struct client* tmp = table->head;
      for (i = 1; i < table->count; i++) {
         tmp = tmp->next;
      }
      tmp->next = NULL;
      close(table->tail->socket);
      free(table->tail);
      table->tail = tmp;
      table->count -= 1;
   }
}       

void dropHead(clientTable* table) {
   if (table->count == 1) {
      free(table->head);
      table->head = NULL;
      table->tail = NULL;
   } else {
      struct client* tmp = table->head;
      table->head = table->head->next;
      free(tmp);
   }
}
void dropMiddle(clientTable* table, int pos) {
   int i = 0;
   struct client* cur = table->head;
   struct client* tmp;
   for (i = 0; i < pos - 2; i++, cur = cur->next);
   tmp = cur->next;
   cur->next = cur->next->next;
   free(tmp);
}

void dropTail(clientTable* table) {
   int i = 0;
   struct client* tmp = table->head;
   for (i = 0; i < table->count - 2; i++, tmp = tmp->next);
   if (table->count == 2) {
      free(table->tail);
      table->tail = table->head;
   } else {
      free(tmp->next);
      tmp->next = NULL;
      table->tail = tmp;
   }
}
      
int removeClient(clientTable* table, int socket) {
   struct client* tmp = table->head;
   int i;
   for (i = 1; i <= table->count; i++, tmp = tmp->next) {
      if (tmp->socket == socket) {
         if (i == 1) {
            dropHead(table);
         } else if (i == table->count) {
            dropTail(table);
         } else {
            dropMiddle(table, i);
         }
         break;
      }
   } 
   table->count -= 1;
   close(socket);
   return i;
}
         
void printHexMessage(u_char* packet, int packetLen) {
   int i = 0; 
   for (i = 0; i < packetLen; i++) {
      printf("%02X ", packet[i]);
   } 
   printf("\n");
}


/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

int tcp_accept(int server_socket, int back_log) {
   int client_socket = 0;
   if ((client_socket= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
   {
      perror("accept call");
      exit(-1);
   }

   return client_socket;
}

int tcp_listen(int server_socket, int back_log)
{
   int status = 1;
   status = listen(server_socket, back_log);
   if (status < 0)
   {
      perror("listen call");
      exit(-1);
   }
   return status;
}

