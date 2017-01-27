#ifndef __HANDLE_H
#define __HANDLE_H

#include "lib.h"

typedef struct c_handle c_handle;

struct c_handle {
   int sock;
   char name[MAX_NAME_SIZE];
   c_handle* next;
};

int addHandle(c_handle** head, int sock, char* name);
int removeHandle(c_handle** head, int sock);
int getHandleByIndex(c_handle* head, uint16_t index);
char* getHandleName(c_handle* head, int sock);
int getHandleSocket(c_handle* head, char* name);

#endif
