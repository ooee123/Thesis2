#include <stdlib.h>
#include <string.h>

#include "handle.h"
#include "lib.h"

static void deleteHandle(c_handle* han) {
   free(han);
}

void destroy_list(c_handle** head) {
   c_handle* pos = *head;
   c_handle* last = pos;

   if(pos) {
      while(pos->next) {
         pos = pos->next;
         deleteHandle(last);
         last = pos;
      }
      deleteHandle(pos);
   }
   *head = NULL;
}

int getHandleByIndex(c_handle* head, uint16_t index) {
   c_handle* pos = head;
   uint16_t i;

   for(i = 0; i < index; i++) {
      pos = pos->next;
   }

   return pos->sock;
}

char* getHandleName(c_handle* head, int sock) {
   while(head) {
      if(head->sock == sock) {
         return head->name;
      }
      head = head->next;
   }

   return NULL;
}

int getHandleSocket(c_handle* head, char* name) {
   while(head) {
      if(!strcmp(head->name, name)) {
         return head->sock;
      }
      head = head->next;
   }

   return -1;
}

int removeHandle(c_handle** head, int sock) {
   c_handle* cur = *head;
   c_handle* last = *head;

   if(sock < 3) {
      return -1;
   }

   if(cur) {
      if(cur->sock == sock) {
         *head = cur->next;
         deleteHandle(cur);
         return 0;
      }
      cur = *head;
   }
   while(cur) {
      if(cur->sock == sock) {
         last->next = cur->next;
         deleteHandle(cur);
         return 0;
      }
      last = cur;
      cur = cur->next;
   }

   return -1;
} 

int addHandle(c_handle** head, int sock, char* name) {
   c_handle* pos = *head;

   if(sock < 3 || -1 != getHandleSocket(*head, name)) {
      return -1;
   }

   if(!*head) {
      *head = calloc(1, sizeof(c_handle));
      (*head)->sock = sock;
      strcpy((*head)->name, name);
   }
   else{
      while(pos->next) {
         pos = pos->next;
      }
      pos->next = calloc(1, sizeof(c_handle));
      pos = pos->next;
      pos->sock = sock;
      strcpy(pos->name, name);
   }
   return 0;
}
