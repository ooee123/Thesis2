/******************************************************************************
 * server.c
 *
 * CPE 464 - Program 2
 *****************************************************************************/

#include <ctype.h>
#include "networks.h"

static int max_fd = 0;

int main(int argc, char *argv[])
{
    server_data sd;
    init_server(argc, argv, &sd);

    while (select_input(&sd) != -1);

    /* close the sockets */
    cleanup_sockets(&sd);
    return 0;
}

void cleanup_sockets(server_data *sd) {
    int i;

    close(sd->srv_fd);
    for (i = next_entry(-1, sd); i >= 0; i = next_entry(i, sd)) {
        close(sd->dir[i].socket);
    }
}

// Look for activity
int select_input(server_data *sd) {
    struct timeval tv = { 0, 25000 };
    int current_socket;
    int num_ready = select(max_fd+1, sd->set, NULL, NULL, &tv);

    int i;

    if (num_ready < 0) {
        perror("Select failed");
        return -1;
    }
    else if (num_ready == 0) {
        refresh_set(sd);
    }
    else if (FD_ISSET(sd->srv_fd, sd->set)) {
        add_client(sd);
    }
    else {
        for (i = next_entry(-1, sd); i >= 0 && num_ready > 0; i = next_entry(i, sd)) {
            current_socket = sd->dir[i].socket;
            if (FD_ISSET(current_socket, sd->set)) {
                // printf("Activity on client %d\n", i);
                traffic_control(current_socket, sd);
                num_ready--;
            }
        }
    }

    refresh_set(sd);
    return 0;
}

// Figure out what a packet is supposed to do
void traffic_control(int src_socket, server_data *sd) {
    char buf[PKT_BUF_SIZE+3];
    uint16_t pkt_len = recv_data(src_socket, buf, 3);

    if (pkt_len < 3) {
        // printf("Bad exiting on socket=%d\n", src_socket);
        allow_exit(src_socket, sd);
        return;
    }
    memcpy(&pkt_len, buf, 2); // Get new packet len
    pkt_len = ntohs(pkt_len);

    switch (buf[2]) {
        case FLAG_C_CONNECT:
            // printf("Connect on socket=%d\n", src_socket);
            update_entry(src_socket, sd, buf, pkt_len);
            break;
        case FLAG_BROADCAST:
            // printf("Broadcast on socket=%d\n", src_socket);
            broadcast(src_socket, sd, buf, pkt_len);
            break;
        case FLAG_C_MESSAGE:
            // printf("Message on socket=%d\n", src_socket);
            message(src_socket, sd, buf, pkt_len);
            break;
        case FLAG_C_EXITING:
            // printf("Exiting on socket=%d\n", src_socket);
            allow_exit(src_socket, sd);
            break;
        case FLAG_WANT_HNDL:
            // printf("Listing on socket=%d\n", src_socket);
            handle_length_packet(src_socket, sd);
            handle_stream(src_socket, sd);
            break;
        default:
            fprintf(stderr, "%s\n", "Not a valid flag");
            exit(1);
            break;
    }

    // server_print_packets(buf, pkt_len);
}

// Flag 11
void handle_length_packet(int src_socket, server_data *sd) {
    int offset;
    uint32_t num_handles = htonl(sd->count);
    char* reply = prep_base_packet(7, &offset, FLAG_AMNT_HNDL);

    memcpy(reply + offset, &num_handles, 4);
    send_data(src_socket, reply, 7);

    free(reply);
}

// Flag 12
void handle_stream(int src_socket, server_data *sd) {
    char* header = prep_base_packet(0, NULL, FLAG_LIST_HNDL);
    char stream[257];

    int i;

    send_data(src_socket, header, 3);
    free(header);

    for (i = next_entry(-1, sd); i >= 0; i = next_entry(i, sd)) {
        stream[0] = sd->dir[i].handle_len;
        memcpy(stream + 1, sd->dir[i].handle, stream[0]);
        send_data(src_socket, stream, stream[0] + 1);
    }
}

// Initial connect w/ new client
void add_client(server_data *sd) {
    int new_fd = 0;
    int new_index = 0;

    new_index = get_empty_entry(sd);
    // Grow directory if necessary
    if (new_index < 0) {
        new_index = expand_directory(sd);
    }
    sd->count++;

    // Accept connection, add to directory
    new_fd = tcp_accept(sd->srv_fd);
    max_fd = MAX(max_fd, new_fd);
    mark_entry(new_index, sd);

    sd->dir[new_index].socket = new_fd;
    sd->dir[new_index].handle = NULL;
    sd->dir[new_index].handle_len = 0;
}

// Get new client's handle
void update_entry(int src_socket, server_data *sd, char* buf, uint16_t pkt_len) {
    char* reply;
    int dup_index = -1;

    int i;

    recv_data(src_socket, buf + 3, 1);
    recv_data(src_socket, buf + 4, buf[3]);

    // Check for duplicate
    find_socket(sd, buf[3], buf+4, NULL, &dup_index);

    for (i = next_entry(-1, sd); i >= 0; i = next_entry(i, sd)) {
        if (src_socket == sd->dir[i].socket) {
            if (dup_index < 0) { // Login allowed
                sd->dir[i].handle = strndup(buf + 4, buf[3]);
                sd->dir[i].handle_len = buf[3];

                reply = prep_base_packet(3, NULL, FLAG_S_CONN_OK);
                send_data(src_socket, reply, 3);
            }
            else {  // Login denied
                unmark_entry(i, sd);
                sd->dir[i].socket = 0;
                sd->count--;

                reply = prep_base_packet(3, NULL, FLAG_S_CONN_NO);
                send_data(src_socket, reply, 3);
                close(src_socket);
            }
        }
    }
    free(reply);
    return;
}

// Client wants to leave
void allow_exit(int src_socket, server_data *sd) {
    char* reply = prep_base_packet(3, NULL, FLAG_S_EXIT_OK);

    int i;

    for (i = next_entry(-1, sd); i >= 0; i = next_entry(i, sd)) {
        if (sd->dir[i].socket == src_socket) {
            unmark_entry(i, sd);
            sd->dir[i].socket = 0;
            free(sd->dir[i].handle);
            sd->dir[i].handle = NULL;
            sd->dir[i].handle_len = 0;
            break;
        }
    }

    send_data(src_socket, reply, 3);
    sd->count--;
    // printf("New number of clients logged in: %d\n", sd->count);

    free(reply);
    close(src_socket);
}

void broadcast(int src_socket, server_data *sd, char* buf, uint16_t pkt_len) {
    recv_data(src_socket, buf + 3, pkt_len - 3);

    int i;

    for (i = next_entry(-1, sd); i >= 0; i = next_entry(i, sd)) {
        if (sd->dir[i].socket != src_socket) {
            // Don't broadcast back to sender
            send_data(sd->dir[i].socket, buf, pkt_len);
        }
    }
}

void message(int src_socket, server_data *sd, char* buf, uint16_t pkt_len) {
    char* dst_handle;
    char* reply;
    char handle_len;
    int pkt_off = 0;
    int send_fd = 0;
    int recv_len = recv_data(src_socket, buf + 3, pkt_len - 3);

    parse_handle(buf, 3, &handle_len, &dst_handle);
    find_socket(sd, handle_len, dst_handle, &send_fd, NULL);

    if (send_fd)
        send_data(send_fd, buf, recv_len + 3);
    else {
        reply = prep_base_packet(4 + handle_len, &pkt_off, FLAG_S_DST_DNE);
        memcpy(reply + 3, buf + 3, handle_len + 1);
        send_data(src_socket, reply, 4 + handle_len);
        free(reply);
    }

    free(dst_handle);
}

void parse_handle(char* buf, int offset, char* handle_len, char** handle) {
    *handle_len = buf[offset++];
    *handle = strndup(buf + offset, *handle_len);
}

void find_socket(server_data *sd, char handle_len, char* handle, int* socket, int* index) {
    int i;

    if (socket)
        *socket = 0;
    if (index)
        *index = -1;

    for (i = next_entry(-1, sd); i >= 0; i = next_entry(i, sd)) {
        if (sd->dir[i].handle_len == handle_len) {
            if (!memcmp(sd->dir[i].handle, handle, handle_len)) {
                if (socket)
                    *socket = sd->dir[i].socket;
                if (index)
                    *index = i;
            }
        }
    }
}

void init_server(int argc, char** argv, server_data *sd) {
    sd->dir = malloc(sizeof(entry) * 8);
    sd->bit_array = calloc(1, 1);
    sd->capacity = 8;
    sd->count = 0;
    sd->set = malloc(sizeof(fd_set));

    //set up network stuff
    tcp_recv_setup(sd);
    if (argc == 2)
        tcp_port_setup(sd, atoi(argv[1]));
    else
        tcp_port_setup(sd, 0);
    tcp_listen(sd->srv_fd, 5);
}

// Helps select
void refresh_set(server_data *sd) {
    FD_ZERO(sd->set);
    FD_SET(sd->srv_fd, sd->set);

    int i;

    for (i = next_entry(-1, sd); i >= 0; i = next_entry(i, sd)) {
        FD_SET(sd->dir[i].socket, sd->set);
    }
}

// Create socket and add to select set
void tcp_recv_setup(server_data* sd) {
    sd->srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd->srv_fd < 0) {
        perror("socket call");
        exit(1);
    }

    max_fd = sd->srv_fd;
    FD_SET(sd->srv_fd, sd->set);
}

// Get port
void tcp_port_setup(server_data* sd, uint16_t port) {
    struct sockaddr_in local;      /* socket address for local side  */
    socklen_t len = sizeof(local);  /* length of local address        */

    local.sin_family = AF_INET;         //internet family
    local.sin_addr.s_addr = INADDR_ANY; //wild card machine address
    local.sin_port = htons(port);          //let system choose the port

    /* bind the name (address) to a port */
    if (bind(sd->srv_fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
        perror("bind call");
        exit(-1);
    }

    //get the port name and print it out
    if (getsockname(sd->srv_fd, (struct sockaddr*)&local, &len) < 0) {
        perror("getsockname call");
        exit(-1);
    }

    printf("Server is using port %d \n", ntohs(local.sin_port));

    // For rapid testing - remove in final build
    // FILE * portf = fopen("portnumber", "w");
    // fprintf(portf, "%d\n", ntohs(local.sin_port));
    // fflush(portf);
    // fclose(portf);
}

//look for a client to serve
void tcp_listen(int srv_fd, int back_log) {
    if (listen(srv_fd, back_log) < 0) {
        perror("listen call");
        exit(-1);
    }
}

int tcp_accept(int srv_fd) {
    int client_socket = 0;

    if ((client_socket = accept(srv_fd, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
        perror("accept call");
        exit(-1);
    }

    return client_socket;
}
