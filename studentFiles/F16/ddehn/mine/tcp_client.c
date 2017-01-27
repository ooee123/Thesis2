

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
int c = 0; /* int sent = 0 ; */
int d = 0; /* int bufsize = 0 ; */
int e; /* int socket_num ; */
int f = 0; /* int send_len = 0 ; */
char *g; /* char * send_buf ; */
if (a != 3) /* if (argc != 3) */ {
printf("usage: %s host-name port-number \n", b[0]); /* printf ( "usage: %s host-name port-number \n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
}
e = tcp_send_setup(b[1], b[2]); /* socket_num = tcp_send_setup ( argv [ 1 ] , argv [ 2 ] ) ; */
d = 1024; /* bufsize = 1024 ; */
g = (char *)malloc(d); /* send_buf = ( char * ) malloc ( bufsize ) ; */
printf("Enter the data to send: "); /* printf ( "Enter the data to send: " ) ; */
f = 0; /* send_len = 0 ; */
while ((g[f] = getchar() ) != '\n'&&f<80) /* while ((send_buf[send_len] = getchar() ) != '\n'&&send_len<80) */ {
f++; /* send_len ++ ; */
}
g[f] = '\0'; /* send_buf [ send_len ] = '\0' ; */
c = mySend(e, g, f, 0); /* sent = mySend ( socket_num , send_buf , send_len , 0 ) ; */
if (c<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("String sent: %s \n", g); /* printf ( "String sent: %s \n" , send_buf ) ; */
printf("Amount of data sent is: %d\n", c); /* printf ( "Amount of data sent is: %d\n" , sent ) ; */
close(e); /* close ( socket_num ) ; */
return 0; /* return 0 */
}
int tcp_send_setup(char *a, char *b) /* int tcp_send_setup ( char * host_name , char * port ) { */{
int c; /* int socket_num ; */
struct sockaddr_in d; /* struct sockaddr_in remote ; */
struct hostent *e; /* struct hostent * hp ; */
d.sin_family = 2; /* remote . sin_family = 2 ; */
if ((c = socket(2, SOCK_STREAM, 0) )<0) /* if ((socket_num = socket(2, SOCK_STREAM, 0) )<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if ((e = gethostbyname(a) ) == ((void *)0 )) /* if ((hp = gethostbyname(host_name) ) == ((void *)0 )) */ {
printf("Error getting hostname: %s\n", a); /* printf ( "Error getting hostname: %s\n" , host_name ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy((char *)&d.sin_addr, (char *)e->h_addr_list[0], e->h_length); /* memcpy ( ( char * ) & remote . sin_addr , ( char * ) hp -> h_addr_list [ 0 ] , hp -> h_length ) ; */
d.sin_port = htons(atoi(b)); /* remote . sin_port = htons ( atoi ( port ) ) ; */
if (connect(c, (struct sockaddr *)&d, sizeof(struct sockaddr_in))<0) /* if (connect(socket_num, (struct sockaddr *)&remote, sizeof(struct sockaddr_in))<0) */ {
perror("connect call"); /* perror ( "connect call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return c; /* return socket_num */
}
