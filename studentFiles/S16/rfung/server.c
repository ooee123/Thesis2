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
#include "server.h"

// variables for handling the list of clients
static client_info *clients = NULL;
static uint32_t max_clients = NUM_INITIAL_MAX_CLIENTS;
static uint32_t num_clients = 0;
static uint32_t max_fd = 0;
static int32_t server_sock = 0;
static fd_set fd_var;

int main(int argc, char **argv) {

    init_server(argc, argv);
    monitor_server();

    return 0;
}

void init_server(int argc, char **argv) {
    struct sockaddr_in local; // socket address for server side
    socklen_t len = sizeof(local); // length of server address
    uint16_t port_num = 0;

    // check command line arguements
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [port-number]\n", argv[0]);
        exit(1);
    }
    else if (argc == 2)
        port_num = atoi(argv[1]);

    // create the socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("server socket call");
        exit(-1);
    }

    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(port_num);

    // bind the adddress to a port
    if (bind(server_sock, (struct sockaddr *)&local, len) < 0) {
        perror("bind call");
        exit(-1);
    }

    printf("Server is using port %u\n", ntohs(local.sin_port));

    // mark the socket as one that will accept incoming connections 
    if (listen(server_sock, LISTEN_BACKLOG) < 0) {
        perror("listen call");
        exit(-1);
    }

    FD_ZERO(&fd_var);
    FD_SET(server_sock, &fd_var);
}

void monitor_server() {
    uint32_t curr_client;
    
    max_fd = server_sock;
    clients = calloc(sizeof(client_info), max_clients);

    while (1) {
        set_fds();
        select(max_fd + 1, (fd_set *) &fd_var, NULL, NULL, NULL);
        
        for (curr_client = 0; curr_client < num_clients; curr_client++)
            if (FD_ISSET(clients[curr_client].sock_num, &fd_var)) {
                service_client(clients, curr_client);
            }

        // check if new connection has been established. 
        if (FD_ISSET(server_sock, &fd_var))
            add_new_client(&fd_var);
    }
}

// set all the fds we're interested in to be monitored by select
void set_fds() {
    uint32_t curr_client;

    FD_ZERO(&fd_var);  
    FD_SET(server_sock, &fd_var);
    for (curr_client = 0; curr_client < num_clients; curr_client++)
        FD_SET(clients[curr_client].sock_num, &fd_var);
}

void service_client(client_info *clients, uint32_t client_num) {
    uint16_t recv_tot, data_len;
    uint8_t flag;

    char *buf = malloc(BUFF_SIZE);

    recv_tot = 0;

    recv_tot += retrieve_data(client_num, 2, buf, 0);
    if (!recv_tot) {
        free(buf);
        return;
    }

    data_len = ntohs(((normal_header *)buf)->len);

    recv_tot = retrieve_data(client_num, data_len, buf, recv_tot);
    if (!recv_tot) {
        free(buf);
        return;
    }

    flag = ((normal_header *)buf)->flag;

    switch (flag) {
        case 1: //handle request
            add_handle(client_num, buf);
            break;
        case 4: // broadcast
            send_broadcast(client_num, buf, data_len);
            break;
        case 5: //msg
            redirect_msg(client_num, buf, data_len);
            break;
        case 8: //client exiting
            client_req_exit(client_num);
            break;
        case 10: //user list request
            send_client_list(client_num);
            break;
        default:
            fprintf(stderr, "This shouldn't happen...\n");
            break;

    }
    free(buf);
}

// removes client from the list and reorganizes the list
void remove_client(uint32_t client_num) {
    uint32_t target_fd = clients[client_num].sock_num, curr_client,
     curr_client_fd;

    // change the max fd if needed
    if (target_fd == max_fd) {
        max_fd = server_sock;
        for (curr_client = 0; curr_client < num_clients; curr_client++) {
            curr_client_fd = clients[curr_client].sock_num;
            if (curr_client_fd > max_fd && curr_client_fd != target_fd) {
                max_fd = curr_client_fd;
            }
        }
    }

    // only need to reorganize list if target isnt at the end
    if (client_num != num_clients - 1) {
        memmove(clients + client_num, clients + client_num + 1,
         (num_clients - client_num - 1) * sizeof(client_info));
    }

    FD_CLR(target_fd, &fd_var);
    num_clients--;
    close(target_fd);

}

void client_req_exit(uint32_t client_num) {
    uint32_t sent;
    normal_header hdr;

    hdr.len = htons(sizeof(normal_header));
    hdr.flag = 9;
    sent = send(clients[client_num].sock_num, &hdr, sizeof(normal_header), 0);

    if (sent < 0) {
        perror("send_handle");
        exit(-1);
    }
}

void add_handle(uint32_t client_num, char *buf) {
    int32_t curr_client, sent;
    char req_handle[MAX_HANDLE_LEN + 1] = {0}, req_handle_len;
    normal_header hdr = {0};

    // go past the normal header to get the length
    buf += sizeof(normal_header);
    req_handle_len = *buf;
    buf++;

    memcpy(req_handle, buf, req_handle_len);

    for (curr_client = 0; curr_client < num_clients; curr_client++) {
        if (!strcmp(clients[curr_client].handle, req_handle)) {
            hdr.len = htons(sizeof(normal_header));
            hdr.flag = 3;
            sent = send(clients[client_num].sock_num, &hdr, sizeof(normal_header), 0);

            if (sent < 0) {
                perror("send_handle");
                exit(-1);
            }
            remove_client(client_num);
            return;
        }
    }

    // handle doesn't already exists so add to our list
    memcpy(clients[client_num].handle, req_handle, req_handle_len);

    hdr.len = htons(sizeof(normal_header));
    hdr.flag = 2;
    sent = send(clients[client_num].sock_num, &hdr, sizeof(normal_header), 0);

    if (sent < 0) {
        perror("send_handle");
        exit(-1);
    }
}

void add_new_client () {
    int32_t client_sock;

    client_sock = accept(server_sock, NULL, NULL); 
    if (client_sock < 0) {
        perror("accept call");
        exit(-1);
    }

    // add the client to the list and clear out the handle field
    clients[num_clients].sock_num = client_sock;
    memset(clients[num_clients].handle, 0, MAX_HANDLE_LEN + 1);
    num_clients++;

    // resize clients list 
    if (num_clients >= max_clients) {
        max_clients += NUM_INITIAL_MAX_CLIENTS;
        clients = realloc(clients, max_clients * sizeof(client_info));
    }

    // update the largest number fd
    if(client_sock > max_fd) 
        max_fd = client_sock;

 }

uint16_t retrieve_data(uint32_t client_num, uint16_t buff_size, char *buffer,
 uint16_t recv_tot) {
    int16_t recv_len;

    while (recv_tot != buff_size)  {
        recv_len = recv(clients[client_num].sock_num, buffer + recv_tot,
         buff_size - recv_tot, 0);

        if (recv_len < 0) {
            perror("recv call");
            exit(-1);
        }

        if (recv_len == 0) { // client didnt cleanly exit
            remove_client(client_num);
            return 0;
        }
        recv_tot += recv_len;
    }
    return recv_tot;
}

void redirect_msg(uint32_t client_num, char *buf, uint32_t data_len) {
    uint32_t dest_len, curr_client;
    char dest[MAX_HANDLE_LEN + 1] = {0}, *tmp_ptr = buf;

    char *reject_buf;
    uint32_t reject_len;

    tmp_ptr += sizeof(normal_header);
    dest_len = *tmp_ptr++;

    memcpy(dest, tmp_ptr, dest_len);

    for (curr_client = 0; curr_client < num_clients; curr_client++) {
        if (!strcmp(clients[curr_client].handle, dest)) { // found the right dest
            if (send(clients[curr_client].sock_num, buf, data_len, 0) < 0) {
                perror("send call");
                exit(-1);
            }
            break;
        }
    }
    // dest missing
    if (curr_client == num_clients) {
        reject_len = sizeof(normal_header) + 1 + dest_len;
        reject_buf = malloc(reject_len);
        ((normal_header *)reject_buf)->len = htons(reject_len);
        ((normal_header *)reject_buf)->flag = 7;

        *(reject_buf + sizeof(normal_header)) = dest_len;

        memcpy(reject_buf + 1 + sizeof(normal_header), dest, dest_len);

        if (send(clients[client_num].sock_num, reject_buf,
         reject_len, 0) < 0) {
            perror("send call");
            exit(-1);
        }
        free(reject_buf);
    }
}

void send_broadcast(uint32_t client_num, char *buf, uint32_t data_len) {
    uint32_t curr_client;

    for (curr_client = 0; curr_client < num_clients; curr_client++) {
        if (curr_client != client_num) {
            if (send(clients[curr_client].sock_num, buf, data_len, 0) < 0) {
                perror("send call");
                exit(-1);
            }
        }
    }
}

void send_client_list(uint32_t client_num) {
    uint32_t curr_client;
    int32_t send_len;
    uint16_t buff_size = sizeof(normal_header) + sizeof(uint32_t);
    char *buf = malloc(buff_size), *tmp_ptr, handle_len;

    ((normal_header *)buf)->len = htons(buff_size); 
    ((normal_header *)buf)->flag = 11;

    *(uint32_t *)(buf + sizeof(normal_header)) = htonl(num_clients);

    if (send(clients[client_num].sock_num, buf, buff_size, 0) < 0) {
            perror("send call");
            exit(-1);
    }
    free(buf);

    send_len = 0;
    tmp_ptr = buf = malloc(BUFF_SIZE);
    buff_size = BUFF_SIZE;

    ((normal_header *)buf)->len = htons(0); 
    ((normal_header *)buf)->flag = 12;

    tmp_ptr += sizeof(normal_header);
    *(uint32_t *)tmp_ptr = htonl(num_clients);
    tmp_ptr +=  sizeof(uint32_t);

    send_len += sizeof(normal_header) + sizeof(uint32_t);

    for (curr_client = 0; curr_client < num_clients; curr_client++) {
        handle_len = strlen(clients[curr_client].handle);
        
        // increase our buffer size if needed 
        if (send_len + handle_len + 1 > buff_size) {
            buff_size += BUFF_SIZE;
            buf = realloc(buf, buff_size);
            tmp_ptr = buf + send_len;
        }

        *tmp_ptr++ = handle_len;
        memcpy(tmp_ptr, clients[curr_client].handle, handle_len);
        tmp_ptr += handle_len;

        send_len += handle_len + 1;
    }

    if (send(clients[client_num].sock_num, buf, send_len, 0) < 0) {
            perror("send call");
            exit(-1);
    }
    free(buf);
}