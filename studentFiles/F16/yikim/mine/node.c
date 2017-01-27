

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void push(node **a, int b, char *c) /* void push ( node * * head , int val , char * handle ) { */{
node *d = malloc(sizeof(node)); /* node * newNode = malloc ( sizeof ( node ) ) ; */
d->fd = b; /* newNode -> fd = val ; */
strcpy(d->handle, c); /* strcpy ( newNode -> handle , handle ) ; */
d->next = *a; /* newNode -> next = * head ; */
*a = d; /* * head = newNode ; */
}
int pop(node **a) /* int pop ( node * * head ) { */{
node *b = ((void *)0 ); /* node * nodeNext = ( ( void * ) 0 ) ; */
int c = -1; /* int retval = - 1 ; */
if (*a != ((void *)0 )) /* if (*head != ((void *)0 )) */ {
b = (*a )->next; /* nodeNext = ( * head ) -> next ; */
c = (*a )->fd; /* retval = ( * head ) -> fd ; */
free(*a); /* free ( * head ) ; */
*a = b; /* * head = nodeNext ; */
}
return c; /* return retval */
}
void printNode(node *a) /* void printNode ( node * head ) { */{
node *b = head; /* node * nodeCur = head ; */
while (b != ((void *)0 )&&b->fd != 0) /* while (nodeCur != ((void *)0 )&&nodeCur->fd != 0) */ {
printf("fd: %d\n", b->fd); /* printf ( "fd: %d\n" , nodeCur -> fd ) ; */
printf("handle: %s\n\n", b->handle); /* printf ( "handle: %s\n\n" , nodeCur -> handle ) ; */
b = b->next; /* nodeCur = nodeCur -> next ; */
}
}
int findHandle(node *a, char *b) /* int findHandle ( node * head , char * handle ) { */{
int c = 0; /* int result = 0 ; */
node *d = head; /* node * nodeCur = head ; */
while (d != ((void *)0 )&&c != 1) /* while (nodeCur != ((void *)0 )&&result != 1) */ {
if (strcmp(d->handle, b) == 0) /* if (strcmp(nodeCur->handle, handle) == 0) */ {
c = 1; /* result = 1 ; */
}
d = d->next; /* nodeCur = nodeCur -> next ; */
}
return c; /* return result */
}
int getHandleFD(node *a, char *b) /* int getHandleFD ( node * head , char * handle ) { */{
node *c = head; /* node * nodeCur = head ; */
while (strcmp(c->handle, b) != 0) /* while (strcmp(nodeCur->handle, handle) != 0) */ {

}
return c->fd; /* return nodeCur->fd */
}
int removeNodeIdx(node **a, int b) /* int removeNodeIdx ( node * * head , int idx ) { */{
int c = -1; /* int retval = - 1 ; */
node *d = ((void *)0 ); /* node * nodeTemp = ( ( void * ) 0 ) ; */
int e = 0; /* int i = 0 ; */
node *f = *a; /* node * nodeCur = * head ; */
if (b == 0) /* if (idx == 0) */ {
c = pop(a); /* retval = pop ( head ) ; */
} else {
for (e = 0; e<b - 1; e++) {
if (f->next == ((void *)0 )) /* if (nodeCur->next == ((void *)0 )) */ {
break;
}
}
if (f->next != ((void *)0 )) /* if (nodeCur->next != ((void *)0 )) */ {
d = f->next; /* nodeTemp = nodeCur -> next ; */
c = d->fd; /* retval = nodeTemp -> fd ; */
f->next = d->next; /* nodeCur -> next = nodeTemp -> next ; */
free(d); /* free ( nodeTemp ) ; */
}
}
return c; /* return retval */
}
int removeNode(node **a, int b) /* int removeNode ( node * * head , int fd ) { */{
int c = 0; /* int idx = 0 ; */
int d = 0; /* int err = 0 ; */
node *e = *a; /* node * nodeCur = * head ; */
while ((e )->fd != b&&e != ((void *)0 )) /* while ((nodeCur )->fd != fd&&nodeCur != ((void *)0 )) */ {
c++; /* idx ++ ; */
e = (e )->next; /* nodeCur = ( nodeCur ) -> next ; */
}
removeNodeIdx(a, c); /* removeNodeIdx ( head , idx ) ; */
return d; /* return err */
}
