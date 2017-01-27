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

#include "testing.h"

#include "networks.h"

#define ARR_START_SIZE 20
#define ADD_HANDLE 1
#define REMOVE_HANDLE 2
#define FIND_HANDLE 4
#define HANDLE_MSG_BUF 1000

uint server_socket;
char **handle_arr;
uint num_handles = 3;
int handle_arr_size = ARR_START_SIZE;

void my_send(uint dest_socket, char *buf, uint buf_size)
{
    int bytes_sent;

    while (buf_size > 0)
    {
        bytes_sent = send(dest_socket, buf, buf_size, 0);

        if (!bytes_sent)
            break;
        else if (bytes_sent < 0)
        {
            perror("send call");
            exit(-1);
        }

        buf += bytes_sent;
        buf_size -= bytes_sent;
    }

}

// Appends the normal header to packets
// Sends the packet to the specified socket
void send_packet_to_client(char *data, uint8_t flag, uint data_buf_size, uint client_socket)
{
    static uint32_t seq_num = 0;
    uint buf_size = HEADER_LEN + data_buf_size;
    char send_buf[buf_size];
    struct packet_header header;

    if (flag == SERVER_HANDLE_MSG)
        header.pkt_len = 0;
    else
        header.pkt_len = htons(buf_size);

    header.seq_num = htonl(seq_num++);
    header.flag = flag;

    memcpy(send_buf, &header, sizeof(struct packet_header));

    if (data_buf_size > 0)
        memcpy(send_buf + sizeof(struct packet_header), data, data_buf_size);

    my_send(client_socket, send_buf, buf_size);
}

// Set the handle list to nulls
void empty_handle_array(int start)
{
    int i = start;
    for (i = start; i < handle_arr_size; i++)
    {
        handle_arr[i] = NULL;
    }
}

// Return the socket number for given handle str
int find_handle(char *handle)
{
    int i;

    for (i = 3; i < handle_arr_size; i++)
    {
        if (handle_arr[i] != NULL && strcmp(handle_arr[i], handle) == 0)
            return i;
    }
    return 0;
}

// Tell client that the handle is already taken
void send_duplicate_handle_msg(char *handle, uint client_socket)
{
    uint8_t flag = HANDLE_DENY;
    size_t handle_len = strlen(handle);
    uint buf_size = 1 + handle_len + DUPLICATE_HANDLE_MSG_LEN;
    char buf[buf_size + 1];
    char *msg = "Handle already in use: ";

    memcpy(buf, msg, DUPLICATE_HANDLE_MSG_LEN);
    memcpy(buf + DUPLICATE_HANDLE_MSG_LEN, handle, handle_len + 1);

    send_packet_to_client(buf, flag, buf_size, client_socket);
}

void add_handle(char *handle, uint client_socket, int last_socket)
{
    handle_arr[client_socket] = malloc(strlen(handle) + 1);
    // Add handle to list
    strcpy(handle_arr[client_socket], handle);

    last_socket = client_socket;
    num_handles++;
  
    // Realloc array if size is exceeding limit
    if (num_handles + 1 == handle_arr_size)
    {
        handle_arr_size *= 2;
        handle_arr = realloc(handle_arr, handle_arr_size * sizeof(char *));
        if (handle_arr == NULL)
        {
            perror("realloc, likely out of memory");
            exit(-1);
        }   

        empty_handle_array(handle_arr_size/2);
    }
           
    // Send ack to client for good handle
    send_packet_to_client(NULL, HANDLE_CONF, 0, client_socket);
} 

//manages handle list operations
int manage_handle(int op, char *handle, uint client_socket)
{
    //Set default number of fds in array, stdio is 3 at start
    static int first_time = 1;
    static int last_socket = -1;

    // Populate array with nulls on first launch
    if(first_time == 1)
    {
        handle_arr = malloc(sizeof(char *) * ARR_START_SIZE);
        empty_handle_array(0);
        first_time = 0;
    }

    if (op == ADD_HANDLE)
    {
        if (find_handle(handle))
        {
            // If handle is already taken on server, notify client
            send_duplicate_handle_msg(handle, client_socket);
        }
        else 
        {
            add_handle(handle, client_socket, last_socket);
        }
    }
    else if (op == REMOVE_HANDLE)
    {
        // Remove handle
        free(handle_arr[client_socket]);
        handle_arr[client_socket] = NULL;
        num_handles--;

        if (num_handles < 3)
            num_handles = 3;

        return -2;
    }
    else if (op == FIND_HANDLE)
    {
        // retrieve socket from handle
        return find_handle(handle);
    }

    return -1;
}

// Send packet to client that handle exists on server
void send_handle_is_found(char *handle_str, uint8_t flag, uint socket)
{
    uint8_t handle_len = strlen(handle_str);
    char send_buf[handle_len + 1];

    send_buf[0] = handle_len;
    memcpy(send_buf + 1, handle_str, handle_len);

    send_packet_to_client(send_buf, flag, handle_len + 1, socket);
}

// Parses a direct message from a client
void parse_direct_msg(char *buf, uint message_len, uint source_socket)
{
    uint8_t dest_handle_len = *(buf + HEADER_LEN);
    uint8_t flag;
    char handle_str[dest_handle_len + 1];

    memcpy(handle_str, buf + HEADER_LEN + 1, dest_handle_len);
    handle_str[dest_handle_len] = '\0';

    int ret = manage_handle(FIND_HANDLE, handle_str, -1);

    // If the handle exists, send ack and then data
    if (!ret)
    {
        flag = HANDLE_NOT_FOUND;
        send_handle_is_found(handle_str, flag, source_socket);
    }
    else
    {
        flag = HANDLE_FOUND;
        send_handle_is_found(handle_str, flag, source_socket);

        my_send(ret, buf, message_len);
    }
}

// Pull the handle out of a direct message
void get_handle_from_msg(char * handle, char *buf)
{
    uint8_t handle_len = *(uint8_t *) (buf + HEADER_LEN);

    memcpy(handle, buf + HEADER_LEN + 1, handle_len);
    handle[handle_len] = '\0';
}

// Sends broadcast message to the specified handle
void send_broadcast_msg(char *buf, uint message_len, char *dest_handle)
{
    uint dest_socket = find_handle(dest_handle);

    my_send(dest_socket, buf, message_len);
}

// Receives broadcast msg and sends to all clients except sender
void parse_broadcast_msg(char *buf, uint message_len, uint client_socket)
{
    char *sender_handle = malloc(MAX_HANDLE_LEN);
    get_handle_from_msg(sender_handle, buf);

    int i;
    for (i = 4; i < handle_arr_size; i++)
    {
        if (handle_arr[i] != NULL && strcmp(handle_arr[i], sender_handle) != 0)
            //then send to handle
            send_broadcast_msg(buf, message_len, handle_arr[i]);
    }

    free(sender_handle);
    //get all sockets here except for main socket
}

//adds handle if avail, sends reply
void parse_client_setup_msg(char *buf, uint message_len, uint client_socket)
{
    char *handle = malloc(MAX_HANDLE_LEN);

    get_handle_from_msg(handle, buf);
    manage_handle(ADD_HANDLE, handle, client_socket); 
    
    free(handle);
}

// Sends handles to client in one contiguous stream
void send_handles_to_client(uint client_socket)
{
    //send with normal header size 0

    int buf_size = HANDLE_MSG_BUF; 
    char *buf = malloc(buf_size);
    int nbytes = 0, len, cursor = 0;
    int i;

    for (i = 4; i < handle_arr_size; i++)
    {
        if (handle_arr[i] != NULL)
        {
            len = strlen(handle_arr[i]);
            nbytes += len + 1;
            buf[cursor++] = len;
            strcpy(buf + cursor, handle_arr[i]);
            cursor += len;
        }

        if (nbytes + MAX_HANDLE_LEN + 1 > buf_size)
        {
            buf_size *= 2;
            buf = realloc(buf, buf_size);
        }
    }
    send_packet_to_client(buf, SERVER_HANDLE_MSG, nbytes, client_socket);
}

// Sends number of handles as net ordered 4 byte int
void send_num_handles(uint client_socket)
{
    char buf[sizeof(uint)];
    uint total_handles = htonl(num_handles - 3);
    
    memcpy(buf, &total_handles, sizeof(uint));
    send_packet_to_client(buf, SERVER_NUM_HANDLE, sizeof(uint), client_socket);
}

int my_recv(uint sender_socket, char *buf)
{
    int bytes_read = 0, total_bytes_read = 0;
    int good_read = 0;
    int buf_size = HEADER_LEN;
    struct packet_header *header;

    while (buf_size > 0)
    {
        bytes_read = recv(sender_socket, buf, buf_size, 0);
        total_bytes_read += bytes_read;

        if (!bytes_read)
        {
            break;
        }
        else if (bytes_read < 0)
        {
            perror("recv call");
            exit(-1);
        }
        else if (!good_read)
        {
            header = (struct packet_header *)buf;
            buf_size = ntohs(header->pkt_len);
            good_read = 1;
    
        }
        buf += bytes_read;
        buf_size -= bytes_read;
    }

    return total_bytes_read;
}

// Read message flag from clientin
int receive_from_client(uint client_socket, char *buf)
{
    struct packet_header *header = (struct packet_header *)buf;
    uint message_len = 0;       //length of the received message
    uint buf_size = ntohs(header->pkt_len);       //packet size variable
    uint8_t *flag ;

    //now get the data on the client_socket
    if (!(message_len = my_recv(client_socket, buf)))
    {
        manage_handle(REMOVE_HANDLE, NULL, client_socket);
        return 1;
    }

    flag = (uint8_t *)(buf + 6);

    switch(*flag) 
    {
        case SETUP:
            //handle setup msg
            //
            parse_client_setup_msg(buf, message_len, client_socket);
            break;
        case BROADCAST_MSG:
            //broadcast
            parse_broadcast_msg(buf, message_len, client_socket);
            break;
        case PRIVATE_MSG:
            //direct msg
            parse_direct_msg(buf, message_len, client_socket);
            break;
        case CLIENT_REQ_EXIT:
            //client exit
            send_packet_to_client(NULL, SERVER_ACK_EXIT, 0, client_socket);
            manage_handle(2, NULL, client_socket);
            return client_socket;
            break;
        case CLIENT_REQ_HANDLE:
            //return handles
            send_num_handles(client_socket);
            send_handles_to_client(client_socket);
            break;
        default:
            printf("Unknown message type.\n");
            break;
    }

    return 0;
}

// Main while loop, calls select and reads from stdin, main socket, or client sockets
void server_routine()
{
    uint client_socket;     //socket descriptor for the client socket
    fd_set temp_rfds, rfds;            //file descriptors for reading
    int index;
    char *buf = malloc(MAX_MSG_LEN);    //message buffer
    int ret = 0;

    // empty fd set and tell to watch accept socket
    FD_ZERO(&rfds);
    FD_SET(server_socket, &rfds);

    //look for a client to serve
    tcp_listen(server_socket, MAX_BACKLOG);

    while (1) {

        

        temp_rfds = rfds;
        // SELECT CALL
        //scans available socket fds and marks whether set or not
        //and need r/w from
        if (select(handle_arr_size + 1, &temp_rfds, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(-1);
        }

        //For every client, checks to see if ready to read
        for (index = 0; index < FD_SETSIZE; index++)
        {
            if (FD_ISSET(index, &temp_rfds))
            {
                //If connection request on main socket accept
                if (index == server_socket)
                {
                    //handle new connection
                    if ((client_socket = accept(server_socket, 
                      (struct sockaddr*)0, (socklen_t *)0)) < 0)
                    {      
                        perror("accept call");
                        exit(-1);
                    }

                    //add new socket to set
                    FD_SET(client_socket, &rfds);
                }
                //Otherwise its an existing client, read msg
                else
                {
                    //TODO check if message is totally read
                    ret = receive_from_client(index, buf);
                    if(ret)
                    {
                        close(index);
                        FD_CLR(index, &rfds);
                    }
                }
            }
        }
    } 
    free(buf);
}

int main(int argc, char *argv[])
{
    uint16_t server_port = 0;   //port number for server socket

    if (argc >= 2) {
       server_port = (uint16_t)strtol(argv[1], 0, 10);
    }

    server_socket = tcp_recv_setup(server_port);
    server_routine(server_socket);

    close(server_socket);
    return 0;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */
int tcp_recv_setup(uint16_t port)
{
    int server_socket= 0;
    struct sockaddr_in local;      /* socket address for local side  */
    socklen_t len= sizeof(local);  /* length of local address        */

    /* create the socket  */
    server_socket= socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
    {
      perror("socket call");
      exit(1);
    }

    local.sin_family = AF_INET;         //internet family
    local.sin_addr.s_addr = INADDR_ANY; //wild card machine address
    local.sin_port = htons(port);           //let system choose the port

    /* bind the name (address) to a port */
    if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
    {
	perror("bind call");
	exit(-1);
    }
    
    //get the port name and print it out
    if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0)
    {
	perror("getsockname call");
	exit(-1);
    }

    printf("Server is using port %d\n", ntohs(local.sin_port));
	        
    return server_socket;
}

/* This function waits for a client to ask for services */
void tcp_listen(int server_socket, int back_log)
{
  if (listen(server_socket, back_log) < 0)
    {
      perror("listen call");
      exit(-1);
    }
}
