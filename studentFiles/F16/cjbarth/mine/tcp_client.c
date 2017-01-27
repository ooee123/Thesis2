

unsigned int __bswap_32(unsigned int a) /*  */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /*  */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
int main(int a, char *b) /* int main ( int argc , char * argv [ ] ) { */{
int c; /* int socket_num ; */
int d = 0; /* int send_len = 0 ; */
char *e; /* char * send_buf ; */
int f = 0; /* int bufsize = 0 ; */
int g = 0; /* int sent = 0 ; */
if (a != 3) /* if (argc != 3) */ {
printf("usage: %s host-name port-number \n", b[0]); /* printf ( "usage: %s host-name port-number \n" , argv [ 0 ] ) ; */
exit(1); /* exit ( 1 ) ; */
}
c = tcp_send_setup(b[1], b[2]); /* socket_num = tcp_send_setup ( argv [ 1 ] , argv [ 2 ] ) ; */
f = 1024; /* bufsize = 1024 ; */
e = (char *)malloc(f); /* send_buf = ( char * ) malloc ( bufsize ) ; */
printf("Enter the data to send: "); /* printf ( "Enter the data to send: " ) ; */
d = 0; /* send_len = 0 ; */
while ((e[d] = getchar() ) != '\n'&&d<80) /* while ((send_buf[send_len] = getchar() ) != '\n'&&send_len<80) */ {
d++; /* send_len ++ ; */
}
e[d] = '\0'; /* send_buf [ send_len ] = '\0' ; */
g = send(c, e, d, 0); /* sent = send ( socket_num , send_buf , send_len , 0 ) ; */
if (g<0) /* if (sent<0) */ {
perror("send call"); /* perror ( "send call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("String sent: %s \n", e); /* printf ( "String sent: %s \n" , send_buf ) ; */
printf("Amount of data sent is: %d\n", g); /* printf ( "Amount of data sent is: %d\n" , sent ) ; */
close(c); /* close ( socket_num ) ; */
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
