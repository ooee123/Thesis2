

unsigned int __bswap_32(unsigned int a) /* while ( cur ) { if ( cur -> sock == */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /* cur -> next ; deleteHandle ( cur ) ; */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void deleteHandle(c_handle *a) /* static void deleteHandle ( c_handle * han ) { */{
free(a); /* free ( han ) ; */
}
void destroy_list(c_handle **a) /* void destroy_list ( c_handle * * head ) { */{
c_handle *b = *a; /* c_handle * pos = * head ; */
c_handle *c = pos; /* c_handle * last = pos ; */
if (b) /* if (pos) */ {
while (b->next) /* while (pos->next) */ {
b = b->next; /* pos = pos -> next ; */
deleteHandle(c); /* deleteHandle ( last ) ; */
}
deleteHandle(b); /* deleteHandle ( pos ) ; */
}
*a = ((void *)0 ); /* * head = ( ( void * ) 0 ) ; */
}
int getHandleByIndex(c_handle *a, uint16_t b) /* int getHandleByIndex ( c_handle * head , uint16_t index ) { */{
uint16_t c; /* uint16_t i ; */
c_handle *d = head; /* c_handle * pos = head ; */
for (c = 0; c<b; c++) {

}
return d->sock; /* return pos->sock */
}
char * getHandleName(c_handle *a, int b) /* char * getHandleName ( c_handle * head , int sock ) { */{
while (a) /* while (head) */ {
if (a->sock == b) /* if (head->sock == sock) */ {
return a->name; /* return head->name */
}
a = a->next; /* head = head -> next ; */
}
return ((void *)0 ); /* return ((void *)0 ) */
}
int getHandleSocket(c_handle *a, char *b) /* int getHandleSocket ( c_handle * head , char * name ) { */{
while (a) /* while (head) */ {
if (!strcmp(a->name, b)) /* if (!strcmp(head->name, name)) */ {
return a->sock; /* return head->sock */
}
a = a->next; /* head = head -> next ; */
}
return -1; /* return -1 */
}
int removeHandle(c_handle **a, int b) /* int removeHandle ( c_handle * * head , int sock ) { */{
c_handle *c = *a; /* c_handle * cur = * head ; */
c_handle *d = *a; /* c_handle * last = * head ; */
if (c) /* if (cur) */ {
if (c->sock == b) /* if (cur->sock == sock) */ {
*a = c->next; /* * head = cur -> next ; */
deleteHandle(c); /* deleteHandle ( cur ) ; */
return 0; /* return 0 */
}
c = *a; /* cur = * head ; */
}
if (b<3) /* if (sock<3) */ {
return -1; /* return -1 */
}
while (c) /* while (cur) */ {
if (c->sock == b) /* if (cur->sock == sock) */ {
d->next = c->next; /* last -> next = cur -> next ; */
deleteHandle(c); /* deleteHandle ( cur ) ; */
return 0; /* return 0 */
}
c = c->next; /* cur = cur -> next ; */
}
return -1; /* return -1 */
}
int addHandle(c_handle **a, int b, char *c) /* int addHandle ( c_handle * * head , int sock , char * name ) { */{
c_handle *d = *a; /* c_handle * pos = * head ; */
if (b<3 || -1 != getHandleSocket(*a, c)) /* if (sock<3 || -1 != getHandleSocket(*head, name)) */ {
return -1; /* return -1 */
}
if (!*a) /* if (!*head) */ {
*a = calloc(1, sizeof(c_handle)); /* * head = calloc ( 1 , sizeof ( c_handle ) ) ; */
(*a )->sock = b; /* ( * head ) -> sock = sock ; */
strcpy((*a )->name, c); /* strcpy ( ( * head ) -> name , name ) ; */
} else {
while (d->next) /* while (pos->next) */ {

}
d->next = calloc(1, sizeof(c_handle)); /* pos -> next = calloc ( 1 , sizeof ( c_handle ) ) ; */
d = d->next; /* pos = pos -> next ; */
d->sock = b; /* pos -> sock = sock ; */
strcpy(d->name, c); /* strcpy ( pos -> name , name ) ; */
}
return 0; /* return 0 */
}
