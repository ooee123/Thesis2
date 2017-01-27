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

#include "testing.h"

#include "networks.h"

uint max_read = MAX_MSG_LEN + MAX_HANDLE_LEN + 3;
char *handle;
int num_handles = 0;

void receive_from_server(uint, char *);
void parse_server_message(char *, uint);
void parse_handles_msg(char *buf);

uint16_t client_socket;

void my_send(uint dest_socket, char *buf, uint buf_size)
{
    int bytes_sent;

    while (buf_size > 0)
    {
        bytes_sent = send(dest_socket, buf, buf_size, 0);

        if (!bytes_sent)
        {
            break;
        }
        else if (bytes_sent < 0)
        {
            perror("send call");
            exit(-1);
        }

        buf += bytes_sent;
        buf_size -= bytes_sent;
    }

}

int my_recv(uint sender_socket, char *buf)
{
    int bytes_read, total_bytes_read = 0;
    int good_read = 0;
    int buf_size = HEADER_LEN;
    struct packet_header *header;

    while (buf_size > 0)
    {

        bytes_read = recv(sender_socket, buf, buf_size, 0);
        total_bytes_read += bytes_read;
    
        if (bytes_read < 0)
        {
            perror("recv call");
            exit(-1);
        }
        else if (bytes_read == 0)
        {
            printf("\nServer Terminated.\n");
            exit(0);
        }
        else if (!good_read)
        {
            header = (struct packet_header *)buf;
            buf_size = ntohs(header->pkt_len);
            good_read = 1;

            if (header->flag == SERVER_HANDLE_MSG)
            {
                parse_handles_msg(buf);
                return 0;
            }
        }
        
        buf += bytes_read;
        buf_size -= bytes_read;
    }

    return total_bytes_read;
}

void receive_from_server(uint server_socket, char *buf)
{
    uint16_t message_len;

    message_len = my_recv(server_socket, buf); 

    parse_server_message(buf, message_len);
}

/* Adds header to specified data block and sends to given
 * sont tcp_send_setup(char *host_name, char *port)
 * cket number */
void send_packet(char *data, uint8_t flag, uint data_buf_size)
{
    static uint32_t seq_num = 0;
    uint buf_size = HEADER_LEN + data_buf_size; 
    char send_buf[buf_size];
    struct packet_header *header = malloc(sizeof(struct packet_header));

    header->seq_num = htonl(seq_num++);
    header->pkt_len = htons(buf_size);
    header->flag = flag;
 
    memcpy(send_buf, header, HEADER_LEN);
    
    if (data_buf_size > 0)
        memcpy(send_buf + HEADER_LEN, data, data_buf_size);

    my_send(client_socket, send_buf, buf_size);
}

/* Sends the intial configuration packet to the server 
 * containing the handle name and length */
void send_config_packet(char *handle) {
    uint8_t flag = 1;
    size_t handle_len = strlen(handle);
    uint send_buf_size = 1 + handle_len;
    uint recv_buf_size = HEADER_LEN + DUPLICATE_HANDLE_MSG_LEN + MAX_HANDLE_LEN;
    char send_buf[send_buf_size];
    char recv_buf[recv_buf_size];

    send_buf[0] = handle_len;
    memcpy(send_buf + 1, handle, handle_len); 

    send_packet(send_buf, flag, send_buf_size);

    receive_from_server(client_socket, recv_buf); 
}

// Finds the next space in string
int find_next_space(char *buf, uint buf_size)
{
    int i;
    for (i = 0; i < buf_size; i++)
    {
        if (buf[i] == ' ' || buf[i] == '\n')
            return i;
    }
    return i;
}

// Receives the handle not found message from server
void parse_handle_not_found_msg(char *buf)
{
    uint8_t handle_len = *(buf + 7);
    char handle_str[handle_len + 1];

    memcpy(handle_str, buf + HEADER_LEN + 1, handle_len);
    handle_str[handle_len] = '\0';

    printf("Client with handle %s does not exist.\n", handle_str);
    printf("$");
    fflush(stdout);
}

// Sends a direct message to the server
void send_msg_packet(char *buf, uint buf_size)
{
    uint8_t flag = PRIVATE_MSG;
    uint8_t dest_handle_len = find_next_space(buf + 2, buf_size);
    uint8_t sender_handle_len = strlen(handle);
    int text_size = buf_size - 3 - dest_handle_len;
    int send_buf_size = text_size + dest_handle_len + strlen(handle) + 2;

    if (text_size <= 0)
        send_buf_size++;

    if (dest_handle_len < 2)
    {
        //printf("Error, no handle given.\n");
        //return;
    }

    char send_buf[send_buf_size];

    // set size of dest handle, copy in dest handle after
    send_buf[0] = dest_handle_len;
    memcpy(send_buf + 1, buf + 2, dest_handle_len);

    // write sender handle length
    send_buf[1 + dest_handle_len] = sender_handle_len;
    // write sender handle
    memcpy(send_buf + 2 + dest_handle_len, handle, strlen(handle));

    // write text data into buffer
    if (text_size > 0)
    {
        memcpy(send_buf + 2 + dest_handle_len + strlen(handle),
          buf + 2 + dest_handle_len + 1, text_size);
    }
    send_buf[send_buf_size - 1] = '\0';

    send_packet(send_buf, flag, send_buf_size); 
}

// Sends a broadcast message to all users on server
void send_broadcast_msg(char *buf, uint nbytes)
{
    uint8_t flag = BROADCAST_MSG;
    uint buf_size = nbytes + strlen(handle) - 1;

    if (nbytes == 2)
        buf_size++;

    char msg_buf[buf_size];

    msg_buf[0] = strlen(handle);
    memcpy(msg_buf + 1, handle, strlen(handle));
    memcpy(msg_buf + 1 + strlen(handle), buf + 2, nbytes - 2);
    msg_buf[buf_size - 1] = '\0';

    send_packet(msg_buf, flag, buf_size);
}

// Receives a broadcast message from server, and prints
void parse_broadcast_msg(char *buf, uint message_len)
{
    uint sender_handle_len = *(buf + HEADER_LEN);

    char sender_handle[sender_handle_len + 1];

    memcpy(sender_handle, buf+ HEADER_LEN + 1, sender_handle_len);
    sender_handle[sender_handle_len] = '\0';

    printf("\n%s: %s\n", sender_handle, buf + HEADER_LEN + 1 
      + sender_handle_len);

    printf("$");
    fflush(stdout);
}

// Receives a direct message from server and prints
void parse_direct_msg(char *buf, uint message_len)
{
    uint8_t my_handle_len = *(buf + HEADER_LEN);

    uint8_t sender_handle_len = *(buf + 1 + my_handle_len + HEADER_LEN);
    char sender_handle[sender_handle_len + 1];
    
    memcpy(sender_handle, buf + 2 + my_handle_len + HEADER_LEN, sender_handle_len);
    sender_handle[sender_handle_len] = '\0';

    printf("\n%s: %s\n", sender_handle, buf + HEADER_LEN + 2 + 
      my_handle_len + sender_handle_len);

    printf("$");
    fflush(stdout);
}

// Prints the number of handles when requested
void parse_num_handles_msg(char *buf)
{
    uint *lnum_handles = (uint *)(buf + HEADER_LEN);

    num_handles = ntohl(*lnum_handles);

    printf("\nThere are %d handles on the server.\n", ntohl(*lnum_handles));
}

// Prints the list of handles one at a time on server
void parse_handles_msg(char *buf)
{
    char *handle_buf = malloc(MAX_HANDLE_LEN);
    int i = 0;
    uint8_t *len = malloc(sizeof(char));

    while (i < num_handles)
    {
        read(client_socket, len, 1);
        read(client_socket, handle_buf, *len);
        handle_buf[*len] = '\0';
    
        printf("%s\n", handle_buf);
        
        i++;
    }

    printf("$");
    fflush(stdout);
}

// Switch to decode the message type from server
void parse_server_message(char *buf, uint message_len)
{
    struct packet_header *header = (struct packet_header *)buf;
    
    switch(header->flag)
    {
        case HANDLE_CONF:
            break;
        case HANDLE_DENY:
            printf("%s\n", buf + HEADER_LEN);
            exit(-1);
            break;
        case BROADCAST_MSG:
            //broadcast packet
            parse_broadcast_msg(buf, message_len);
            break;
        case PRIVATE_MSG:
            // message packet
            parse_direct_msg(buf, message_len);
            break;
        case HANDLE_FOUND:
            // handle ack
            break;
        case HANDLE_NOT_FOUND:
            // error packet, handle does not exist dest
            parse_handle_not_found_msg(buf);
            break;
        case SERVER_ACK_EXIT:
            // client exit ack
            exit(0);
            break;
        case SERVER_NUM_HANDLE:
            // number of clients on server
            parse_num_handles_msg(buf);
            break;
        case SERVER_HANDLE_MSG:
            // handles currently in server
            break;
        default:
            printf("FLAG error %d\n", header->flag);
            break;
    
    }
}

// Switch to read user input on stdin
void parse_input(char *buf, uint nbytes)
{
    uint total_bytes = nbytes;
    char cmd = *(buf);
    
    if (total_bytes == max_read)
    {
        while (getchar() != '\n')
            total_bytes++;

        printf("Message is %d bytes, this is ", nbytes + 1);
        printf("too long. Message truncated to 32kbytes.\n");
    }

    if (cmd == 'm' || cmd == 'M')
    {
        send_msg_packet(buf, nbytes);        
    }
    else if (cmd == 'b' || cmd == 'B')
    {
        send_broadcast_msg(buf, nbytes);
    }
    else if (cmd == 'l' || cmd == 'L')
    {
        send_packet(NULL, CLIENT_REQ_HANDLE, 0);
    }
    else if (cmd == 'e' || cmd == 'E')
    {
        send_packet(NULL, CLIENT_REQ_EXIT, 0);
    }
    else
    {
        printf("Invalid command\n");
    }
}

// Main while loop for client that selects on 
// either stdin or the server connection
void client_routine()
{
    fd_set temp_rfds, rfds;
    int index;
    char *buf = malloc(max_read);
    uint nbytes;

    printf("$");
    fflush(stdout);

    // listens on both stdin and server
    FD_ZERO(&rfds);
    FD_SET(client_socket, &rfds);
    FD_SET(STDIN_FILENO, &rfds);

    while (1)
    {
        temp_rfds = rfds;

        // Select call
        if (select(client_socket + 1, &temp_rfds, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(-1);
        }

        for (index = 0; index < client_socket + 1; index++)
        {
            if (FD_ISSET(index, &temp_rfds))
            {
                if (index != STDIN_FILENO)
                {
                    receive_from_server(index, buf);
                } 
                else
                {
                    // read from stdin
                    nbytes = read(STDIN_FILENO, buf, max_read);

                    parse_input(buf, nbytes);
                    printf("$");
                    fflush(stdout);
                }
            }
        }
    }
}

// Sanitizes cmd line arguments
void parse_cmdline_args(int argc, char *argv[])
{
    /* check command line arguments  */
    if(argc != 4)
    {
        printf("usage: %s handle server-name server-port \n", argv[0]);
	exit(-1);
    }

    handle = argv[1];

    if (strlen(handle) > MAX_HANDLE_LEN)
    {
        printf("Invalid handle, max length is 255 characters.\n");
        exit(-1);
    }
}

int main(int argc, char * argv[])
{
    parse_cmdline_args(argc, argv);
   
    /* set up the socket for TCP transmission  */
    client_socket = tcp_send_setup(argv[2], argv[3]);

    send_config_packet(handle);
   
    client_routine();

    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
    return 0;
}

// Sets up the tcp socket
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
