/******************************************************************************
 * tcp_client.c
 *
 *****************************************************************************/



 #include "shared.h"

static int current_state = START_STATE;
int server_socket= 0;   //socket descriptor for the server socket
char *receive_buf;              //buffer for receiving from client
char *receive_buf_position;
int receive_buffer_size= 1024;  //packet size variable
int message_len= 0;     //length of the received message

char *send_buf;              //buffer for sending to server
int send_buffer_size= 1024;


struct handle *clients;
int client_list_size = 20;
int num_clients = 0;
fd_set set;

void clean_up(int socket_num)
{
    int k;
    int found = 0;

    for (k = 0; k < client_list_size; ++k)
    {
        //printf("loop\n");
        if (clients[k].socket == socket_num)
        {
            found = 1;
            clients[k].active = 0;
            FD_CLR(socket_num, &set);
            close(socket_num);
            //printf("removed client\n");
            num_clients--;
        }
    }
    if (!found) //not found
    {
        printf("client not found for deletion\n");
    }
}
char *receive_data(struct handle client)
{
    static int bytes_received = 0;
    static int expected_length = 0;

    int client_socket = client.socket;
    
    switch(current_state)
    {
        case START_STATE:
            message_len = 0;
            if ((message_len = recv(client_socket, receive_buf_position, 2, 0)) < 0)
            {
                perror("recv call");
                exit(-1);
            }
            if (message_len == 0)
            {
                clean_up(client.socket);
                return 0;  
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
            if ((message_len = recv(client_socket, receive_buf_position, 1, 0)) < 0)
            {
                perror("recv call");
                exit(-1);
            }
            if (message_len == 0)
            {
                clean_up(client.socket);
                return 0;
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

            if ((message_len = recv(client_socket, receive_buf_position, expected_length - bytes_received, 0)) < 0)
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
                //printf("received all data\n");
                return receive_buf;
            }
            else
            {
                //printf("not yet\n");
                return 0;
            }

            break;
        default:
            //printf("Error\n");
            exit(-1);
    }
}

void place_handle(struct handle item)
{
    int i = 0;

    while(i < client_list_size && clients[i].active != 0)
    {
        ++i;
    }

    if (i == client_list_size) //reached end of list, need more space
    {
        if(realloc(clients, (client_list_size + 20) * sizeof(struct handle) == 0))
        {
            perror("select error");
            exit(EXIT_FAILURE);
        }
        client_list_size += 20;
    }
    
    clients[i] = item;
    num_clients++;
    //printf("client %d added at %d\n", item.socket, i);
}

void send_generic_flag(int flag_num, int socket_num)
{
    int tot_len = 1;
    //uint8_t current_flag = FLAG2;
    //placing 2 byte length
    memcpy(send_buf, &tot_len, 2);
    //placing flag
    memcpy(send_buf + 2, &flag_num, sizeof(uint8_t));
    // now send the data 
    if(send(socket_num, send_buf, tot_len + 2, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}
void send_handles(int socket_num)
{


    int tot_len = 1;
    int i;
    char *position = send_buf + 3;
    int name_len = 0;
    int current_flag = FLAG12;
    for (i = 0; i < client_list_size; ++i)
    {
        if (clients[i].active == 2)
        {
            //printf("a client in list\n");
            name_len = strlen(clients[i].name);
            memcpy(position, &name_len, 1);
            position += 1;
            tot_len += 1;
            memcpy(position, clients[i].name, name_len);
            position += name_len;
            tot_len += name_len;
        }
    }

    position = send_buf;

    memcpy(position, &tot_len, 2);
    position += 2;
    //placing flag
    memcpy(position, &current_flag, sizeof(uint8_t));
    position += 1;
    
    // now send the data 
    if(send(socket_num, send_buf, tot_len + 2, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}
void handle_connections()
{
    int i;
    int new_client = 0;
    char *final = 0;

    FD_ZERO(&set);

    if (listen(server_socket, 5) < 0)
    {
        perror("listen call");
        exit(-1);
    }
    while(1)
    {
        FD_SET(server_socket, &set);
        int set_count;
        for (set_count = 0; set_count < client_list_size; ++set_count)
        {
            if (clients[set_count].active == 2)
            {
                FD_SET(clients[set_count].socket, &set);
            }
        }

        if(select(FD_SETSIZE, &set, NULL, NULL, NULL) < 0)
        {
            perror("select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_socket, &set))
        {
            new_client = 0;
            
            //printf("new client\n");
            new_client= tcp_listen(server_socket, 5);

            struct handle new;
            new.active = 1;
            new.socket = new_client;

            place_handle(new);

            FD_SET(new_client, &set);

            //printf("added client\n");
        }

        for (i = 0; i < client_list_size; ++i)
        {
            if (FD_ISSET(clients[i].socket, &set))
            {
                final = receive_data(clients[i]);
                if (final)
                {
                    if (final[2] == FLAG1)
                    {
                        uint8_t handle_len = 0;

                        memcpy(&handle_len, final + 3, sizeof(uint8_t));
                        char namebuf[255];
                        memcpy(namebuf, final + 4, handle_len);
                        namebuf[handle_len] = '\0';

                        //printf("NEW HANDLE: %s\n", namebuf);
                        
                        int j = 0;
                        int found = 0;

                        //seraching for handles in the array
                        while(j < client_list_size)
                        {     
                            if(strcmp(clients[j].name, namebuf) == 0)
                            {
                                found = 1;
                            }
                            ++j;
                        }
                        j = 0;

                        if (found == 0)
                        {
                            //send back flag 2
                            memcpy(&clients[i].name, namebuf, handle_len + 1);
                            clients[i].active = 2;
                            send_generic_flag(FLAG2, clients[i].socket);
                        }
                        else
                        {
                            ///////send back flag 3
                            //printf("handle already there\n");
                            //////sending FLAG 3, fail
                            send_generic_flag(FLAG3, clients[i].socket);

                            ///remove client from list
                            clients[i].active = 0;


                        }

                    }
                    else if (final[2] == FLAG4)
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
                        name[name_len] = '\0';
                        int k;

                        for (k = 0; k < client_list_size; ++k)
                        {
                            //printf("loop\n");
                            if (clients[k].active == 2 && (clients[k].socket != clients[i].socket))
                            {
                                if(send(clients[k].socket, final, tot_len + 2, 0) < 0)
                                {
                                    perror("send call");
                                    exit(-1);
                                }
                            }
                        }

                        //final[]
                    }
                    else if (final[2] == FLAG5)
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
                        name[name_len] = '\0';

                        int k;
                        int found = 0;

                        for (k = 0; k < client_list_size; ++k)
                        {
                            //printf("loop\n");
                            if (strcmp(clients[k].name, name) == 0)
                            {
                                found = 1;
                                if(send(clients[k].socket, final, tot_len + 2, 0) < 0)
                                {
                                    perror("send call");
                                    exit(-1);
                                }
                            }
                        }
                        if (!found) //not found
                        {
                            final[2] = FLAG7;
                            if(send(clients[i].socket, final, tot_len + 2, 0) < 0)
                            {
                                perror("send call");
                                exit(-1);
                            }

                            printf("clliet not found\n");
                        }

                        //final[]
                    }
                    else if (final[2] == FLAG8)
                    {
                        send_generic_flag(FLAG9, clients[i].socket);
                        clean_up(clients[i].socket);

                        //final[]
                    }
                    else if (final[2] == FLAG10)
                    {
                        int tot_len = 1 + 4;
                        int current_flag = FLAG11;
                        //uint8_t current_flag = FLAG2;
                        //placing 2 byte length
                        memcpy(send_buf, &tot_len, 2);
                        //placing flag
                        memcpy(send_buf + 2, &current_flag, sizeof(uint8_t));
                        memcpy(send_buf + 3, &num_clients, sizeof(uint32_t));
                        // now send the data 
                        if(send(clients[i].socket, send_buf, tot_len + 2, 0) < 0)
                        {
                            perror("send call");
                            exit(-1);
                        }

                       
                        send_handles(clients[i].socket);
                        
                    }

                }  
                //FD_SET(clients[i].socket, &set);
            }

 
        } 
    }
}

int main(int argc, char * argv[])
{
    //create packet receive_buffer
    receive_buf =  (char *) malloc(receive_buffer_size);
    receive_buf_position = receive_buf;

    clients = calloc(sizeof(struct handle), client_list_size);
    send_buf = malloc(send_buffer_size);
    
    //create the server socket
    server_socket= tcp_recv_setup();
    
    handle_connections();

    /* close the sockets */
    close(server_socket);
    

    
    return 0;
    
}



/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup()
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

    local.sin_family= AF_INET;         //internet family
    local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
    local.sin_port= htons(0);                 //let system choose the port

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

    printf("socket has port %d \n", ntohs(local.sin_port));
            
    return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

int tcp_listen(int server_socket, int back_log)
{
  int client_socket= 0; 
  if ((client_socket= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
    {
      perror("accept call");
      exit(-1);
    }
  
  return(client_socket);
}






