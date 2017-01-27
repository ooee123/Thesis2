typedef enum {MT_M,MT_B,MT_E,MT_J} MESSAGE_TYPE;
int construct_message(MESSAGE_TYPE mt, int serv_addr); /* int construct_message(MESSAGE_TYPE mt, int serv_addr) */
void handle_input(int serv_addr, char *handle); /* void handle_input(int serv_addr, char *handle) */
void parse_message(char *data); /* void parse_message(char *data) */
void list_clients(int serv_addr); /* void list_clients(int serv_addr) */

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
int c, d, e; /* int serv_addr , port_addr , handle_len ; */
char *f; /* char * handle ; */
if (a != 4) /* if (argc != 4) */ {
printf("Usage:  cclient handle server-name server-port\n"); /* printf ( "Usage:  cclient handle server-name server-port\n" ) ; */
return -1; /* return -1 */
}
d = atoi(b[3]); /* port_addr = atoi ( argv [ 3 ] ) ; */
c = tcpClientSetup(b[2], b[3]); /* serv_addr = tcpClientSetup ( argv [ 2 ] , argv [ 3 ] ) ; */
f = b[1]; /* handle = argv [ 1 ] ; */
e = strlen(f); /* handle_len = strlen ( handle ) ; */
if (construct_message(MT_J, c, f)>=0) /* if (construct_message(MT_J, serv_addr, handle)>=0) */ {
handle_input(c, f); /* handle_input ( serv_addr , handle ) ; */
}
close(d); /* close ( port_addr ) ; */
close(c); /* close ( serv_addr ) ; */
return 0; /* return 0 */
}
void handle_input(int a, char *b) /* void handle_input ( int serv_addr , char * handle ) { */{
fd_set c, d; /* fd_set active_fds , read_fds ; */
char e[256]; /* char dest_handle [ 256 ] ; */
char f[1024]; /* char message [ 1024 ] ; */
char g[1024]; /* char recv_buf [ 1024 ] ; */
char h[1024]; /* char screen_buf [ 1024 ] ; */
int i = 0; /* int i = 0 ; */
FD_ZERO(&c); /* FD_ZERO ( & active_fds ) ; */
FD_SET(a, &c); /* FD_SET ( serv_addr , & active_fds ) ; */
FD_SET(0, &c); /* FD_SET ( 0 , & active_fds ) ; */
while (1) /* while (1) */ {
d = c; /* read_fds = active_fds ; */
printf("$: "); /* printf ( "$: " ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
if (select(1024, &d, ((void *)0 ), ((void *)0 ), ((void *)0 ))>0) /* if (select(1024, &read_fds, ((void *)0 ), ((void *)0 ), ((void *)0 ))>0) */ {
if (FD_ISSET(0, &d)) /* if (FD_ISSET(0, &read_fds)) */ {
scanf("%s", h); /* scanf ( "%s" , screen_buf ) ; */
if (memcmp(h, "%E", 2) == 0) /* if (memcmp(screen_buf, "%E", 2) == 0) */ {
construct_message(MT_E, a); /* construct_message ( MT_E , serv_addr ) ; */
return; /* return */
} else if (memcmp(h, "%e", 2) == 0) /* if (memcmp(screen_buf, "%e", 2) == 0) */ {
construct_message(MT_E, a); /* construct_message ( MT_E , serv_addr ) ; */
return; /* return */
} else if (memcmp(h, "%M", 2) == 0) /* if (memcmp(screen_buf, "%M", 2) == 0) */ {
scanf("%s", e); /* scanf ( "%s" , dest_handle ) ; */
i = 0; /* i = 0 ; */
while ((f[i] = getchar() ) != '\n'&&i<1024 - 1) /* while ((message[i] = getchar() ) != '\n'&&i<1024 - 1) */ {
i++; /* i ++ ; */
}
f[i] = '\0'; /* message [ i ] = '\0' ; */
memmove(f, f + 1, strlen(f)); /* memmove ( message , message + 1 , strlen ( message ) ) ; */
construct_message(MT_M, a, b, e, f); /* construct_message ( MT_M , serv_addr , handle , dest_handle , message ) ; */
} else if (memcmp(h, "%m", 2) == 0) /* if (memcmp(screen_buf, "%m", 2) == 0) */ {
scanf("%s", e); /* scanf ( "%s" , dest_handle ) ; */
i = 0; /* i = 0 ; */
while ((f[i] = getchar() ) != '\n'&&i<1024 - 1) /* while ((message[i] = getchar() ) != '\n'&&i<1024 - 1) */ {
i++; /* i ++ ; */
}
f[i] = '\0'; /* message [ i ] = '\0' ; */
memmove(f, f + 1, strlen(f)); /* memmove ( message , message + 1 , strlen ( message ) ) ; */
construct_message(MT_M, a, b, e, f); /* construct_message ( MT_M , serv_addr , handle , dest_handle , message ) ; */
} else if (memcmp(h, "%L", 2) == 0) /* if (memcmp(screen_buf, "%L", 2) == 0) */ {
printf("This command is not supported at this time\n"); /* printf ( "This command is not supported at this time\n" ) ; */
} else if (memcmp(h, "%l", 2) == 0) /* if (memcmp(screen_buf, "%l", 2) == 0) */ {
printf("This command is not supported at this time\n"); /* printf ( "This command is not supported at this time\n" ) ; */
} else if (memcmp(h, "%B", 2) == 0) /* if (memcmp(screen_buf, "%B", 2) == 0) */ {
i = 0; /* i = 0 ; */
while ((f[i] = getchar() ) != '\n'&&i<1024 - 1) /* while ((message[i] = getchar() ) != '\n'&&i<1024 - 1) */ {
i++; /* i ++ ; */
}
f[i] = '\0'; /* message [ i ] = '\0' ; */
memmove(f, f + 1, strlen(f)); /* memmove ( message , message + 1 , strlen ( message ) ) ; */
construct_message(MT_B, a, b, f); /* construct_message ( MT_B , serv_addr , handle , message ) ; */
} else if (memcmp(h, "%b", 2) == 0) /* if (memcmp(screen_buf, "%b", 2) == 0) */ {
i = 0; /* i = 0 ; */
while ((f[i] = getchar() ) != '\n'&&i<1024 - 1) /* while ((message[i] = getchar() ) != '\n'&&i<1024 - 1) */ {
i++; /* i ++ ; */
}
f[i] = '\0'; /* message [ i ] = '\0' ; */
memmove(f, f + 1, strlen(f)); /* memmove ( message , message + 1 , strlen ( message ) ) ; */
construct_message(MT_B, a, b, f); /* construct_message ( MT_B , serv_addr , handle , message ) ; */
}
} else {
if (recv(a, g, 1024, 0)<=0) /* if (recv(serv_addr, recv_buf, 1024, 0)<=0) */ {
return; /* return */
}
parse_message(g); /* parse_message ( recv_buf ) ; */
fflush(stdout); /* fflush ( stdout ) ; */
}
}
}
return; /* return */
}
void parse_message(char *a) /* void parse_message ( char * data ) { */{
char b[1024]; /* char message [ 1024 ] ; */
short c; /* short data_len ; */
char d[256]; /* char dest_handle [ 256 ] ; */
char e[256]; /* char src_handle [ 256 ] ; */
int f, g, h = a[2]; /* int dest_len , src_len , flag = data [ 2 ] ; */
switch (h) /* switch (flag) */ {
case 4: memcpy(&c, a, 2); /* memcpy ( & data_len , data , 2 ) ; */
printf("%s: %s\n", e, b); /* printf ( "%s: %s\n" , src_handle , message ) ; */
memcpy(e, a + 4, g); /* memcpy ( src_handle , data + 4 , src_len ) ; */
strncpy(b, a + 4 + g, 1024 - 1); /* strncpy ( message , data + 4 + src_len , 1024 - 1 ) ; */
g = a[3]; /* src_len = data [ 3 ] ; */
c = ntohs(c); /* data_len = ntohs ( data_len ) ; */
break;
case 5: f = a[3]; /* dest_len = data [ 3 ] ; */
printf("%s: %s\n", e, b); /* printf ( "%s: %s\n" , src_handle , message ) ; */
memcpy(d, a + 4, f); /* memcpy ( dest_handle , data + 4 , dest_len ) ; */
memcpy(e, a + 5 + f, g); /* memcpy ( src_handle , data + 5 + dest_len , src_len ) ; */
strncpy(b, a + 5 + f + g, 1024 - 1); /* strncpy ( message , data + 5 + dest_len + src_len , 1024 - 1 ) ; */
g = a[4 + f]; /* src_len = data [ 4 + dest_len ] ; */
d[f] = '\0'; /* dest_handle [ dest_len ] = '\0' ; */
break;
case 7: f = a[3]; /* dest_len = data [ 3 ] ; */
printf("Client with handle %s does not exist\n", d); /* printf ( "Client with handle %s does not exist\n" , dest_handle ) ; */
memcpy(d, a + 4, f); /* memcpy ( dest_handle , data + 4 , dest_len ) ; */
d[f] = '\0'; /* dest_handle [ dest_len ] = '\0' ; */
break;
}
return; /* return */
}
void list_handles(int a, uint32_t b, char *c, int d) /* void list_handles ( int serv_addr , uint32_t num_handles , char * recv_buf , int recv_len ) { */{
return; /* return */
}
int construct_message(MESSAGE_TYPE a, int b) /* int construct_message ( MESSAGE_TYPE mt , int serv_addr , ... ) { */{
char c[256]; /* char src_handle [ 256 ] ; */
char d[256]; /* char dest_handle [ 256 ] ; */
char e[1024]; /* char recv_buf [ 1024 ] ; */
char f[1024]; /* char mesg_buf [ 1024 ] ; */
uint16_t g, h; /* uint16_t send_len , send_len_no ; */
va_list i; /* va_list params ; */
char j[1024]; /* char send_buf [ 1024 ] ; */
switch (a) /* switch (mt) */ {
case MT_B: __builtin_va_start(i, b); /* __builtin_va_start ( params , serv_addr ) ; */
sprintf(f, "%s", __buildin_va_arg(i, char *)); /* sprintf ( mesg_buf , "%s" , __builtin_va_arg ( params , char * ) ) ; */
sprintf(c, "%s", __buildin_va_arg(i, char *)); /* sprintf ( src_handle , "%s" , __builtin_va_arg ( params , char * ) ) ; */
send(b, j, g, 0); /* send ( serv_addr , send_buf , send_len , 0 ) ; */
memcpy(j, &h, 2); /* memcpy ( send_buf , & send_len_no , 2 ) ; */
__builtin_va_end(i); /* __builtin_va_end ( params ) ; */
memcpy(j + 4, c, j[3]); /* memcpy ( send_buf + 4 , src_handle , send_buf [ 3 ] ) ; */
strcpy(j + 4 + strlen(c), f); /* strcpy ( send_buf + 4 + strlen ( src_handle ) , mesg_buf ) ; */
h = htons(g); /* send_len_no = htons ( send_len ) ; */
g = 5 + strlen(c) + strlen(f); /* send_len = 5 + strlen ( src_handle ) + strlen ( mesg_buf ) ; */
j[2] = 4; /* send_buf [ 2 ] = 0x04 ; */
j[3] = strlen(c); /* send_buf [ 3 ] = strlen ( src_handle ) ; */
return 0; /* return 0 */
break;
case MT_J: __builtin_va_start(i, b); /* __builtin_va_start ( params , serv_addr ) ; */
__builtin_va_end(i); /* __builtin_va_end ( params ) ; */
sprintf(c, "%s", __buildin_va_arg(i, char *)); /* sprintf ( src_handle , "%s" , __builtin_va_arg ( params , char * ) ) ; */
recv(b, e, 1024, 0); /* recv ( serv_addr , recv_buf , 1024 , 0 ) ; */
send(b, j, strlen(c) + 4, 0); /* send ( serv_addr , send_buf , strlen ( src_handle ) + 4 , 0 ) ; */
memcpy(j, &g, 2); /* memcpy ( send_buf , & send_len , 2 ) ; */
memcpy(j + 4, c, strlen(c)); /* memcpy ( send_buf + 4 , src_handle , strlen ( src_handle ) ) ; */
g = strlen(c) + 4; /* send_len = strlen ( src_handle ) + 4 ; */
j[2] = 1; /* send_buf [ 2 ] = 1 ; */
j[3] = strlen(c); /* send_buf [ 3 ] = strlen ( src_handle ) ; */
if (e[2] == 2) /* if (recv_buf[2] == 2) */ {
return 0; /* return 0 */
} else if (e[2] == 3) /* if (recv_buf[2] == 3) */ {
printf("ERROR: handle already exists\n"); /* printf ( "ERROR: handle already exists\n" ) ; */
return -1; /* return -1 */
} else {
printf("Unexpected error\n"); /* printf ( "Unexpected error\n" ) ; */
return -1; /* return -1 */
}
break;
case MT_E: j[0] = 3; /* send_buf [ 0 ] = 3 ; */
send(b, j, 3, 0); /* send ( serv_addr , send_buf , 3 , 0 ) ; */
recv(b, e, 1024, 0); /* recv ( serv_addr , recv_buf , 1024 , 0 ) ; */
j[1] = 0; /* send_buf [ 1 ] = 0 ; */
j[2] = 8; /* send_buf [ 2 ] = 8 ; */
return 1; /* return 1 */
case MT_M: __builtin_va_start(i, b); /* __builtin_va_start ( params , serv_addr ) ; */
__builtin_va_end(i); /* __builtin_va_end ( params ) ; */
sprintf(f, "%s", __buildin_va_arg(i, char *)); /* sprintf ( mesg_buf , "%s" , __builtin_va_arg ( params , char * ) ) ; */
sprintf(d, "%s", __buildin_va_arg(i, char *)); /* sprintf ( dest_handle , "%s" , __builtin_va_arg ( params , char * ) ) ; */
sprintf(c, "%s", __buildin_va_arg(i, char *)); /* sprintf ( src_handle , "%s" , __builtin_va_arg ( params , char * ) ) ; */
send(b, j, g, 0); /* send ( serv_addr , send_buf , send_len , 0 ) ; */
memcpy(j, &h, 2); /* memcpy ( send_buf , & send_len_no , 2 ) ; */
sprintf(j + 4, "%s %s%s", d, c, f); /* sprintf ( send_buf + 4 , "%s %s%s" , dest_handle , src_handle , mesg_buf ) ; */
h = htons(g); /* send_len_no = htons ( send_len ) ; */
g = strlen(j + 3) + 4; /* send_len = strlen ( send_buf + 3 ) + 4 ; */
j[2] = 5; /* send_buf [ 2 ] = 5 ; */
j[3] = strlen(d); /* send_buf [ 3 ] = strlen ( dest_handle ) ; */
j[4 + strlen(d)] = strlen(c); /* send_buf [ 4 + strlen ( dest_handle ) ] = strlen ( src_handle ) ; */
return 0; /* return 0 */
}
return -1; /* return -1 */
}
