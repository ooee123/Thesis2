/******************************************************************************
 * tcp_client.c
 *
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

#include "networks.h"
#include "testing.h"

int main(int argc, char * argv[])
{
    int socket_num;         //socket descriptor
    char *send_buf;         //data buffer
    int bufsize= 0;         //data buffer size
    int send_len= 0;        //amount of data to send
    int sent= 0;            //actual amount of data sent

    /* check command line arguments  */
    if(argc!= 3)
    {
        printf("usage: %s host-name port-number \n", argv[0]);
	exit(1);
    }

    /* set up the socket for TCP transmission  */
    socket_num= tcp_send_setup(argv[1], argv[2]);

    /* initialize data buffer for the packet */
    bufsize= 1024;
    send_buf= (char *) malloc(bufsize);

    /* get the data and send it   */
    printf("Enter the data to send: ");
    
    send_len = 0;
    while ((send_buf[send_len] = getchar()) != '\n' && send_len < 80)
	   send_len++;

    send_buf[send_len] = '\0';
            
    /* now send the data */
    sent =  send(socket_num, send_buf, send_len, 0);
    if(sent < 0)
    {
        perror("send call");
	exit(-1);
    }

    printf("String sent: %s \n", send_buf);
    printf("Amount of data sent is: %d\n", sent);
        
    close(socket_num);
    return 0;
    
}



int tcp_send_setup(char *host_name, char *port)
{
    int socket_num;
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host

    // create the socket
    if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    perror("socket call");
	    exit(-1);
	}
    

    // designate the addressing family
    remote.sin_family= AF_INET;

    // get the address of the remote host and store
    if ((hp = gethostbyname(host_name)) == NULL)
	{
	  printf("Error getting hostname: %s\n", host_name);
	  exit(-1);
	}
    
    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

    // get the port used on the remote side and store
    remote.sin_port= htons(atoi(port));

    if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
    {
	perror("connect call");
	exit(-1);
    }

    return socket_num;
}

