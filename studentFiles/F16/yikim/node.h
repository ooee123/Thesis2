/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define MAX_HANDLE 255

typedef struct node {
   int fd;
   char handle[MAX_HANDLE];
   struct node *next;
} node;

void push(node **head, int val, char *handle);

int pop(node **head); 

void printNode(node *head); 

int findHandle(node *head, char *handle);

int getHandleFD(node *head, char *handle);

int removeNodeIdx(node **head, int idx); 

int removeNode(node **head, int fd); 
