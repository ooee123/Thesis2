/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
 *****************************************************************************/

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
#include <vector>
#include "chat_server_funcs.h"
#include "testing.h"
int main(int argc, char **argv) 
{
    int server_socket= 0;   //socket descriptor for the server socket  

    if(argc == 1) {
        server_socket= tcp_recv_setup(0);
    } else {
        server_socket= tcp_recv_setup(atoi(argv[1]));
    }


    if (listen(server_socket, 5) < 0)
    {
      perror("listen call");
      exit(-1);
    }
    server_select();
    return 0;
}




