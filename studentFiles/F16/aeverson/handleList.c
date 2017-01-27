#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "handleList.h"

#define FOUND 1
#define NOT_FOUND 0 
#define FAIL -1
#define PASS 0

Handle *head = NULL;
int32_t size = 0;

int findHandle(char *name) {
	int found = NOT_FOUND;
	Handle *temp = head;

	while(!found && temp != NULL) {
		found = !(strcmp(name, temp->name));
		temp = temp->next;
	}
	return found;
}

int findSocket(int socket) {
	int found = NOT_FOUND;
	Handle *temp = head;
	
	while(!found && temp != NULL) {
		found = socket == temp->socket;
		temp = temp->next;
	}
	return found;
}

void addHandle(char *name, int hSize, int socket) {
	Handle *add = malloc(sizeof(Handle));

	add->name = malloc(hSize + 1);
	memcpy(add->name, name, hSize);
	add->name[hSize] = '\0';

	add->length = hSize;
	add->socket = socket;
	add->next = head;
	
	head = add;
	size++;
}

void removeHandle(char *name) {
	int found = NOT_FOUND;
	Handle *prev, *curr = head;

	prev = curr;
	while(!found && curr!= NULL) {
		found = !(strcmp(name, curr->name));
		prev = curr;
		curr = curr->next;
	}

	prev->next = curr->next;

	free(curr->name);
	free(curr);
	size--;
}

void removeSocket(int socket) {
	Handle *prev, *curr = head;
	
	/* Do this for the case when haven't added
    * socket/handle due to duplicate handle. */ 	
	if(findSocket(socket) == NOT_FOUND)
		return;

	prev = NULL;
	while(socket != curr->socket && curr->next != NULL) {
		prev = curr;
		curr = curr->next;
	}
	
	if(prev != NULL)
		prev->next = curr->next;
	else 
		head = curr->next;

	free(curr->name);
	free(curr);
	size--;
}		

int getSocket(char *name) {
	int found = NOT_FOUND;
	Handle *curr = head;

	while(!found && curr != NULL) {
		if(strcmp(name, curr->name) == 0)
			return curr->socket;
		curr = curr->next;
	}
	return FAIL;
}

Handle *getClientData(int ndx) {
	int count = 0;
	Handle *curr = head;	

	while(count < ndx) {
		curr = curr->next;
		count++;
	}	
	return curr;
}
			
int32_t getNumHandles() {
	return size;
}
