#define INIT 1
#define FAIL_INIT 3
#define ACK_INIT 2
#define BROADCAST 4
#define MESSAGE 5
#define FAIL_MESSAGE 7
#define EXITING 8
#define ACK_EXIT 9
#define LIST 10
#define HANDLE_TOT 11
#define HANDLES 12
#define HEADER_LEN 3
#define MAX_MESSAGE 999

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/select.h>

#include "testing.h"

//for both
void Safe_Select(fd_set *read_fds, int n);
void Safe_Send(char *packet, int packet_len, int socket_num);

// for the server side
int tcp_recv_setup();
int tcp_listen(int server_socket, int back_log);

// for the client side
int tcp_send_setup(char *host_name, char *port);

