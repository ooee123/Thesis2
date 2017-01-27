Handle *global_a = ((void *)0 ); /* Handle * head = ( ( void * ) 0 ) ; */
int32_t global_b = 0; /* int32_t size = 0 ; */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int findHandle(char *a) /* int findHandle ( char * name ) { */{
int b = 0; /* int found = 0 ; */
Handle *c = head; /* Handle * temp = head ; */
while (!b&&c != ((void *)0 )) /* while (!found&&temp != ((void *)0 )) */ {
b = !(strcmp(a, c->name) ); /* found = ! ( strcmp ( name , temp -> name ) ) ; */
c = c->next; /* temp = temp -> next ; */
}
return b; /* return found */
}
int findSocket(int a) /* int findSocket ( int socket ) { */{
int b = 0; /* int found = 0 ; */
Handle *c = head; /* Handle * temp = head ; */
while (!b&&c != ((void *)0 )) /* while (!found&&temp != ((void *)0 )) */ {
b = a == c->socket; /* found = socket == temp -> socket ; */
c = c->next; /* temp = temp -> next ; */
}
return b; /* return found */
}
void addHandle(char *a, int b, int c) /* void addHandle ( char * name , int hSize , int socket ) { */{
Handle *d = malloc(sizeof(Handle)); /* Handle * add = malloc ( sizeof ( Handle ) ) ; */
global_b++; /* size ++ ; */
d->name = malloc(b + 1); /* add -> name = malloc ( hSize + 1 ) ; */
memcpy(d->name, a, b); /* memcpy ( add -> name , name , hSize ) ; */
d->name[b] = '\0'; /* add -> name [ hSize ] = '\0' ; */
d->length = b; /* add -> length = hSize ; */
d->socket = c; /* add -> socket = socket ; */
d->next = global_a; /* add -> next = head ; */
global_a = d; /* head = add ; */
}
void removeHandle(char *a) /* void removeHandle ( char * name ) { */{
int b = 0; /* int found = 0 ; */
Handle *c, *d = head; /* Handle * prev , * curr = head ; */
global_b--; /* size -- ; */
c = d; /* prev = curr ; */
while (!b&&d != ((void *)0 )) /* while (!found&&curr != ((void *)0 )) */ {
b = !(strcmp(a, d->name) ); /* found = ! ( strcmp ( name , curr -> name ) ) ; */
d = d->next; /* curr = curr -> next ; */
}
c->next = d->next; /* prev -> next = curr -> next ; */
free(d->name); /* free ( curr -> name ) ; */
free(d); /* free ( curr ) ; */
}
void removeSocket(int a) /* void removeSocket ( int socket ) { */{
Handle *b, *c = head; /* Handle * prev , * curr = head ; */
global_b--; /* size -- ; */
b = ((void *)0 ); /* prev = ( ( void * ) 0 ) ; */
if (findSocket(a) == 0) /* if (findSocket(socket) == 0) */ {
return; /* return */
}
while (a != c->socket&&c->next != ((void *)0 )) /* while (socket != curr->socket&&curr->next != ((void *)0 )) */ {
b = c; /* prev = curr ; */
c = c->next; /* curr = curr -> next ; */
}
if (b != ((void *)0 )) /* if (prev != ((void *)0 )) */ {
b->next = c->next; /* prev -> next = curr -> next ; */
} else {
global_a = c->next; /* head = curr -> next ; */
}
free(c->name); /* free ( curr -> name ) ; */
free(c); /* free ( curr ) ; */
}
int getSocket(char *a) /* int getSocket ( char * name ) { */{
int b = 0; /* int found = 0 ; */
Handle *c = head; /* Handle * curr = head ; */
while (!b&&c != ((void *)0 )) /* while (!found&&curr != ((void *)0 )) */ {
if (strcmp(a, c->name) == 0) /* if (strcmp(name, curr->name) == 0) */ {
return c->socket; /* return curr->socket */
}
c = c->next; /* curr = curr -> next ; */
}
return -1; /* return -1 */
}
Handle * getClientData(int a) /* Handle * getClientData ( int ndx ) { */{
int b = 0; /* int count = 0 ; */
Handle *c = head; /* Handle * curr = head ; */
while (b<a) /* while (count<ndx) */ {
b++; /* count ++ ; */
c = c->next; /* curr = curr -> next ; */
}
return c; /* return curr */
}
int32_t getNumHandles() /* int32_t getNumHandles ( ) { */{
return global_b; /* return size */
}
