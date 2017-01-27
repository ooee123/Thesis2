/******************************************************************************
 * tcp_client.c
 *
 *****************************************************************************/


#include "shared.h"



static int current_state = START_STATE;
int server_socket= 0;   //socket descriptor for the server socket
int client_socket= 0;   //socket descriptor for the client socket
char *receive_buf;              //buffer for receiving from client
char *receive_buf_position;
int receive_buffer_size= 1024;  //packet size variable
int message_len= 0;     //length of the received message

char *send_buf;              //buffer for sending to server
int send_buffer_size= 1024;

char *stdin_buf;
int stdin_buffer_size = 4096;

char *prompt = "$: ";

char *client_name = NULL;

fd_set set;

int requested_num_handles = 0;

char *receive_data()
{
    static int bytes_received = 0;
    static int expected_length = 0;

    switch(current_state)
    {
        case START_STATE:
            message_len = 0;
            
            if ((message_len = recv(server_socket, receive_buf_position, 2, 0)) < 0)
            {
                perror("recv call");
                exit(-1);
            }
            if (message_len == 0)
            {
                printf("Server terminated\n");
                close(server_socket);
                exit(-1);
            }         
            else 
            {
                receive_buf_position += message_len;
                bytes_received += message_len;

                if (bytes_received == 1)
                {
                    current_state = PROCESS_LEN_STATE;
                    bytes_received = 0;
                    return 0;
                }
                else
                {
                    current_state = GET_DATA;
                    bytes_received = 0;
                    return 0; 
                }
            }
            break;
        case PROCESS_LEN_STATE:
            if ((message_len = recv(server_socket, receive_buf_position, 1, 0)) < 0)
            {
                perror("recv call");
                exit(-1);
            }
            if (message_len == 0)
            {
                printf("Server terminated\n");
                close(server_socket);
                exit(-1);
            }
            else 
            {
                receive_buf_position += message_len;
                bytes_received += message_len;

                if (bytes_received == 1)
                {
                    current_state = PROCESS_LEN_STATE;
                    bytes_received = 0;
                    return 0;
                }
                else
                {
                    current_state = GET_DATA;
                    bytes_received = 0;
                    return 0; 
                }
            }
            break;
        case GET_DATA:
            expected_length = *(short *)receive_buf;

            if ((message_len = recv(server_socket, receive_buf_position, expected_length - bytes_received, 0)) < 0)
            {
                perror("recv call");
                exit(-1);
            }
            receive_buf_position += message_len;
            bytes_received += message_len;

            if (bytes_received == expected_length)
            {
                receive_buf_position = receive_buf;
                bytes_received = 0;
                current_state = START_STATE;
                return receive_buf;
            }
            else
            {
                return 0;
            }

            break;
        default:
            exit(-1);
    }
}

void send_init_data(char *handle_name)
{
    uint8_t name_len = strlen(handle_name);
 
    //flag + handle length + handle
    int tot_len = 1 + 1 + name_len;

    uint8_t current_flag = FLAG1;
    //placing 2 byte length
    memcpy(send_buf, &tot_len, 2);
    //placing flag
    memcpy(send_buf + 2, &current_flag, sizeof(uint8_t));
    //placing handle length
    memcpy(send_buf + 3, &name_len, sizeof(uint8_t));
    //
    memcpy(send_buf + 4, handle_name, name_len);

    // now send the data 
    if(send(server_socket, send_buf, tot_len + 2, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }

    char *final = 0;

    int ready = 0;

    while(ready == 0)    /////may be dangerous, is done bc 1st runthorugh is 0
    {
        final = receive_data();

        if (final)
        {   
            if (final[2] == FLAG2)
            {
                ready = 1;
            }
            else if (final[2] == FLAG3)
            {
                printf("Handle already in use: %s\n", client_name);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
        }

    }
    
}
void request_exit()
{
    char *temp_send_buf = send_buf;
    uint8_t current_flag = FLAG8;
    
    temp_send_buf = send_buf;
    
    //flag + handle length + handle
    uint16_t tot_len = 1;

    memcpy(temp_send_buf, &tot_len, 2);    //placing 2 byte length 
    temp_send_buf += 2;
    memcpy(temp_send_buf, &current_flag, sizeof(uint8_t));    //placing flag
    temp_send_buf += 1;

    // now send the data 
    if(send(server_socket, send_buf, tot_len + 2, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }

}
void request_handles()
{
    char *temp_send_buf = send_buf;
    uint8_t current_flag = FLAG10;
    
    temp_send_buf = send_buf;
    
    //flag + handle length + handle
    uint16_t tot_len = 1;

    memcpy(temp_send_buf, &tot_len, 2);    //placing 2 byte length 
    temp_send_buf += 2;
    memcpy(temp_send_buf, &current_flag, sizeof(uint8_t));    //placing flag
    temp_send_buf += 1;

    // now send the data 
    if(send(server_socket, send_buf, tot_len + 2, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}
void send_message(char *text, char *dest_handle)
{
    uint32_t textlen = strlen(text);
    uint8_t name_len = strlen(client_name);
    uint8_t dest_len = strlen(dest_handle);

    uint8_t current_flag = FLAG5;

    char *temp_text = text;
    char *temp_send_buf = send_buf;

    int i;

    int iterations = (textlen / 1000); //# of 1000 byte iterations
    for (i = 0; i < iterations; ++i)
    {
        //printf("for loop?\n");
        temp_send_buf = send_buf;
        temp_text = text + (i *1000);
        //flag + handle length + handle
        uint16_t tot_len = 1 + 1 + dest_len + 1 + name_len + 1000 + 1;////////////// 1000 + null?
 
        memcpy(temp_send_buf, &tot_len, 2);    //placing 2 byte length 
        temp_send_buf += 2;
        memcpy(temp_send_buf, &current_flag, sizeof(uint8_t));    //placing flag
        temp_send_buf += 1;
        memcpy(temp_send_buf, &dest_len, sizeof(uint8_t));      //placing dest handle length
        temp_send_buf += 1;
        memcpy(temp_send_buf, dest_handle, dest_len);           //placing dest handle
        temp_send_buf += dest_len;
        memcpy(temp_send_buf, &name_len, sizeof(uint8_t));     //placing handle length
        temp_send_buf += 1;
        memcpy(temp_send_buf, client_name, name_len);    //placing handle
        temp_send_buf += name_len;
        memcpy(temp_send_buf, temp_text, 1000);    //placing text

        // now send the data 
        if(send(server_socket, send_buf, tot_len + 2, 0) < 0)
        {
            perror("send call");
            exit(-1);
        }
    }

    //temp_text = text + (i *1000);
    int remainder = textlen % 1000; /////remainder after 1000 byte iterations
    //flag + handle length + handle
    uint16_t tot_len = 1 + 1 + dest_len + 1 + name_len + remainder + 1;////////////// 1000 + null?
 
    memcpy(temp_send_buf, &tot_len, 2);    //placing 2 byte length 
    temp_send_buf += 2;
    memcpy(temp_send_buf, &current_flag, sizeof(uint8_t));    //placing flag
    temp_send_buf += 1;
    memcpy(temp_send_buf, &dest_len, sizeof(uint8_t));      //placing dest handle length
    temp_send_buf += 1;
    memcpy(temp_send_buf, dest_handle, dest_len);           //placing dest handle
    temp_send_buf += dest_len;
    memcpy(temp_send_buf, &name_len, sizeof(uint8_t));     //placing handle length
    temp_send_buf += 1;
    memcpy(temp_send_buf, client_name, name_len);    //placing handle
    temp_send_buf += name_len;
    memcpy(temp_send_buf, temp_text, remainder);    //placing text

    // now send the data 
    if(send(server_socket, send_buf, tot_len + 2, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}
void send_broadcast(char *text)
{

    int textlen = strlen(text);
    int name_len = strlen(client_name);
    uint8_t current_flag = FLAG4;

    char *temp_text = text;

    int i;

    int iterations = (textlen / 1000); //# of 1000 byte iterations
    for (i = 0; i < iterations; ++i)
    {
        temp_text = text + (i *1000);
        //flag + handle length + handle
        int tot_len = 1 + 1 + name_len + 1000 + 1;////////////// 1000 + null?
 
        memcpy(send_buf, &tot_len, 2);    //placing 2 byte length 
        memcpy(send_buf + 2, &current_flag, sizeof(uint8_t));    //placing flag
        memcpy(send_buf + 3, &name_len, sizeof(uint8_t));     //placing handle length
        memcpy(send_buf + 4, client_name, name_len);    //placing handle
        memcpy(send_buf + 4 + name_len, temp_text, 1001);    //placing text

        // now send the data 
        if(send(server_socket, send_buf, tot_len + 2, 0) < 0)
        {
            perror("send call");
            exit(-1);
        }
    }

    //temp_text = text + (i *1000);
    int remainder = textlen % 1000; /////remainder after 1000 byte iterations
    //flag + handle length + handle
    int tot_len = 1 + 1 + name_len + remainder + 1;////////////// 1000 + null?
 
    memcpy(send_buf, &tot_len, 2);    //placing 2 byte length 
    memcpy(send_buf + 2, &current_flag, sizeof(uint8_t));    //placing flag
    memcpy(send_buf + 3, &name_len, sizeof(uint8_t));     //placing handle length
    memcpy(send_buf + 4, client_name, name_len);    //placing handle
    memcpy(send_buf + 4 + name_len, temp_text, remainder);    //placing text

    // now send the data 
    if(send(server_socket, send_buf, tot_len + 2, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}

void parse_args()
{
    char *pch;

    int position = 0;
    char *dest_name = 0;
    char *text = 0;
  
    pch = strtok(stdin_buf," ");
    position += strlen(pch);


    char *newline = strchr(pch, '\n');
    if (newline)
    {
        *newline = 0;
    }

    if (strcmp(pch, "%M") == 0 || strcmp(pch, "%m") == 0)
    {
        if (pch != NULL)
        {
            pch = strtok (NULL, " ");
            position += strlen(pch);
            dest_name = pch;

            //printf("dest_name: %s\n", dest_name);

            text = stdin_buf + position + 2;

            send_message(text, dest_name);
        }
        else
        {
            printf("missing arguments\n");
        }
    }
    else if (strcmp(pch, "%B") == 0 || strcmp(pch, "%b") == 0)
    {
        text = stdin_buf + position + 1;
  
        send_broadcast(text);
    }
    else if (strcmp(pch, "%L") == 0 || strcmp(pch, "%l") == 0)
    {
        request_handles();
    }
    else if (strcmp(pch, "%E") == 0 || strcmp(pch, "%e") == 0)
    {
        //printf("E selected\n");
        request_exit();
    }
    else
    {
        printf("Invalid command\n");
    }
}
void print_handles(char *data)
{
    char *position = data;
    char name_buf[256];
    int name_len = 0;
   
    printf("Number of clients: %d\n", requested_num_handles);

    position += 3;

    int i;
    for (i = 0; i < requested_num_handles; ++i)
    {
        memcpy(&name_len, position, 1);
        position += 1;
        memcpy(name_buf, position, name_len);  
        position += name_len;

        name_buf[name_len] = '\0';

        printf("\t%s\n", name_buf);
    }
}
void handle_connection(char **argv)
{
    //int i;
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int read_len = 0;
    char *final = 0;

    send_init_data(argv[1]);

    printf("%s", prompt);
    fflush(stdout);

    while(1)
    {
        
        //init FD set
        FD_ZERO(&set);


    
        FD_SET(STDIN_FILENO, &set);
        FD_SET(server_socket, &set);

        if(select(FD_SETSIZE, &set, NULL, NULL, NULL) < 0)
        {
            perror("select error");
            exit(EXIT_FAILURE);
        }
        
        if (FD_ISSET(server_socket, &set))
        {
            
            final = receive_data();

             if (final)
                {
                    if (final[2] == FLAG4)
                    {
                        uint16_t tot_len;
                        uint8_t name_len;

                        char *position = final;
                        char name[256];

                        memcpy(&tot_len, position, sizeof(uint16_t));
                        position += 2;
                        position += 1; ///skip flag
                        memcpy(&name_len, position, sizeof(uint8_t));
                        position += 1;
                        memcpy(name, position, name_len);
                        position += name_len;
                        name[name_len] = '\0';
                        printf("%s: %s", name, position);
                    }
                    else if (final[2] == FLAG5)
                    {
                        uint16_t tot_len;
                        uint8_t name_len;
                        uint8_t dest_len;

                        char *position = final;
                        char name[256];

                        memcpy(&tot_len, position, sizeof(uint16_t));
                        position += 2;
                        position += 1; ///skip flag
                        memcpy(&dest_len, position, sizeof(uint8_t));
                        position += 1;
                        position += dest_len;
                        memcpy(&name_len, position, sizeof(uint8_t));
                        position += 1;
                        memcpy(name, position, name_len);
                        position += name_len;
                        name[name_len] = '\0';
                        printf("%s: %s", name, position);
                    }
                    else if (final[2] == FLAG7)
                    {
                        uint16_t tot_len;
                        uint8_t name_len;

                        char *position = final;
                        char name[256];

                        memcpy(&tot_len, position, sizeof(uint16_t));
                        position += 2;
                        position += 1; ///skip flag
                        memcpy(&name_len, position, sizeof(uint8_t));
                        position += 1;
                        memcpy(name, position, name_len);
                        position += name_len;
                        name[name_len] = '\0';
                        printf("Client with handle %s does not exist\n", name);
                    }
                    else if (final[2] == FLAG9)
                    {
                        exit(0);
                    }
                    else if (final[2] == FLAG11)
                    {
                        requested_num_handles = 0;
                        memcpy(&requested_num_handles, final + 3, sizeof(int));
                        //printf("requested_num_handles:%d\n", requested_num_handles);
                    }
                    else if (final[2] == FLAG12)
                    {
                        print_handles(final);
                    }
                    
                }
        }
        if (FD_ISSET(STDIN_FILENO, &set))
        {
          
            if((read_len = read(STDIN_FILENO, stdin_buf, stdin_buffer_size)) < 0)
            {
                perror("read from STDIN error");
                exit(-1);
            }

            stdin_buf[read_len] = '\0';
            parse_args();

            printf("%s", prompt);
            fflush(stdout);
        }
        
    }
}

int main(int argc, char * argv[])
{
    int handle_len = 0;

    //create packet buffer
    receive_buf =  (char *) malloc(receive_buffer_size);
    receive_buf_position = receive_buf;

    stdin_buf = malloc(stdin_buffer_size);
    send_buf = malloc(send_buffer_size);

    /* check command line arguments  */
    if(argc!= 4)
    {
        printf("usage: %s handle-name host-name port-number \n", argv[0]);
	    exit(1);
    }

    handle_len = strlen(argv[1]);
    if (handle_len > 255)
    {
        printf("handle name too long\n");
        exit(1);
    }

    client_name = argv[1];

    /* set up the socket for TCP transmission  */
    server_socket = tcp_send_setup(argv[2], argv[3]);
  
    handle_connection(argv);
        
    close(server_socket);
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


