

unsigned int __bswap_32(unsigned int __bsx) /*  */{
return __builtin_bswap32(__bsx); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t __bsx) /*  */{
return __builtin_bswap64(__bsx); /* return __builtin_bswap64(__bsx) */
}
int tcpServerSetup() /* int tcpServerSetup ( ) { */{
int server_socket = 0; /* int server_socket = 0 ; */
struct sockaddr_in local; /* struct sockaddr_in local ; */
socklen_t len = sizeof (local ); /* socklen_t len = sizeof ( local ) ; */
server_socket = socket(2, SOCK_STREAM, 0); /* server_socket = socket ( 2 , SOCK_STREAM , 0 ) ; */
if (server_socket<0) /* if (server_socket<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(1); /* exit ( 1 ) ; */
}
local.sin_family = 2; /* local . sin_family = 2 ; */
local.sin_addr.s_addr = ((in_addr_t)0 ); /* local . sin_addr . s_addr = ( ( in_addr_t ) 0x00000000 ) ; */
local.sin_port = htons(0); /* local . sin_port = htons ( 0 ) ; */
if (bind(server_socket, (struct sockaddr *)&local, sizeof (local ))<0) /* if (bind(server_socket, (struct sockaddr *)&local, sizeof (local ))<0) */ {
perror("bind call"); /* perror ( "bind call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (getsockname(server_socket, (struct sockaddr *)&local, &len)<0) /* if (getsockname(server_socket, (struct sockaddr *)&local, &len)<0) */ {
perror("getsockname call"); /* perror ( "getsockname call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
if (listen(server_socket, 5)<0) /* if (listen(server_socket, 5)<0) */ {
perror("listen call"); /* perror ( "listen call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
printf("socket has port %d \n", ntohs(local.sin_port)); /* printf ( "socket has port %d \n" , ntohs ( local . sin_port ) ) ; */
return server_socket; /* return server_socket */
}
int tcpAccept(int server_socket) /* int tcpAccept ( int server_socket ) { */{
int client_socket = 0; /* int client_socket = 0 ; */
if ((client_socket = accept(server_socket, (struct sockaddr *)0, (socklen_t *)0) )<0) /* if ((client_socket = accept(server_socket, (struct sockaddr *)0, (socklen_t *)0) )<0) */ {
perror("accept call"); /* perror ( "accept call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return (client_socket ); /* return (client_socket ) */
}
int tcpClientSetup(char *host_name, char *port) /* int tcpClientSetup ( char * host_name , char * port ) { */{
int socket_num; /* int socket_num ; */
struct sockaddr_in remote; /* struct sockaddr_in remote ; */
struct hostent *hp; /* struct hostent * hp ; */
if ((socket_num = socket(2, SOCK_STREAM, 0) )<0) /* if ((socket_num = socket(2, SOCK_STREAM, 0) )<0) */ {
perror("socket call"); /* perror ( "socket call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
remote.sin_family = 2; /* remote . sin_family = 2 ; */
if ((hp = gethostbyname(host_name) ) == ((void *)0 )) /* if ((hp = gethostbyname(host_name) ) == ((void *)0 )) */ {
printf("Error getting hostname: %s\n", host_name); /* printf ( "Error getting hostname: %s\n" , host_name ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
memcpy((char *)&remote.sin_addr, (char *)hp->h_addr_list[0], hp->h_length); /* memcpy ( ( char * ) & remote . sin_addr , ( char * ) hp -> h_addr_list [ 0 ] , hp -> h_length ) ; */
remote.sin_port = htons(atoi(port)); /* remote . sin_port = htons ( atoi ( port ) ) ; */
if (connect(socket_num, (struct sockaddr *)&remote, sizeof(struct sockaddr_in))<0) /* if (connect(socket_num, (struct sockaddr *)&remote, sizeof(struct sockaddr_in))<0) */ {
perror("connect call"); /* perror ( "connect call" ) ; */
exit(-1); /* exit ( - 1 ) ; */
}
return socket_num; /* return socket_num */
}
