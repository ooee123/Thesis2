/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"

/*
   typedef struct node {
   int fd;
   char handle[MAX_HANDLE];
   struct node *next;
   } node;
   */

void push(node **head, int val, char *handle) {
   node *newNode = malloc(sizeof(node));

   newNode->fd = val;
   strcpy(newNode->handle, handle);
   newNode->next = *head;
   *head = newNode;
}

int pop(node **head) {
   int retval = -1;
   node* nodeNext = NULL;

   if (*head != NULL) {
      nodeNext = (*head)->next;
      retval = (*head)->fd;
      free(*head);
      *head = nodeNext;
   }

   return retval;
}

void printNode(node *head) {
   node *nodeCur = head;

   while (nodeCur != NULL && nodeCur->fd != 0) {
      printf("fd: %d\n", nodeCur->fd);
      printf("handle: %s\n\n", nodeCur->handle);
      nodeCur = nodeCur->next;
   }
}

int findHandle(node *head, char *handle) {
   int result = 0;
   node *nodeCur = head;

   while (nodeCur != NULL && result != 1) {
      if (strcmp(nodeCur->handle, handle) == 0) {
         result = 1;
      }
      nodeCur = nodeCur->next;
   }

   return result;
}

// use only when handle exists
int getHandleFD(node *head, char *handle) {
   node *nodeCur = head;

   while (strcmp(nodeCur->handle, handle) != 0) {
      nodeCur = nodeCur->next;
   }

   return nodeCur->fd;
}

int removeNodeIdx(node ** head, int idx) {
   int i = 0;
   int retval = -1;
   node *nodeCur= *head;
   node *nodeTemp = NULL;

   if (idx == 0) {
      retval = pop(head);
   }
   else {
      for (i = 0; i < idx - 1; i++) {
         if (nodeCur->next == NULL) {
            break;
         }
         nodeCur = nodeCur->next;
      }

      if (nodeCur->next != NULL) {
         nodeTemp = nodeCur->next;
         retval = nodeTemp->fd;
         nodeCur->next = nodeTemp->next;
         free(nodeTemp);
      }
   }

   return retval;
}

int removeNode(node **head, int fd) {
   int err = 0;
   node *nodeCur = *head;

   int idx = 0;

   while ((nodeCur)->fd != fd && nodeCur != NULL) {
      idx++;
      nodeCur = (nodeCur)->next;
   }

   removeNodeIdx(head, idx);

   return err;

}
