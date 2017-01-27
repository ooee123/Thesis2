/******************************************************************************
 * cclient.c
 *
 * CPE 464 - Program 2
 *****************************************************************************/

#include "networks.h"
#include "testing.h"

int main(int argc, char *argv[])
{
    client_data cd;

    init_client(argc, argv, &cd);
    upload_info(&cd);
    login(&cd);

    while (check_inbox(&cd) != -1);

    close(cd.socket);
    return 1;
}

// Does the server let you in?
void login(client_data* cd) {
    char buf[3];

    if (recv_data(cd->socket, buf, 3) == 3) {
        if (buf[2] == FLAG_S_CONN_OK) {
            return;
        }
        else if (buf[2] == FLAG_S_CONN_NO) {
            printf("Handle already in use: %s\n", cd->handle);
        }
        else {
            printf("Not a valid flag\n");
        }
    }
    else {
        printf("Packet too small\n");
    }
    close(cd->socket);
    exit(1);
}

int tcp_send_setup(char *host_name, char *port)
{
    int socket_num;
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host

    // create the socket
    if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket call");
        exit(-1);
    }

    // designate the addressing family
    remote.sin_family = AF_INET;

    // get the address of the remote host and store
    if ((hp = gethostbyname(host_name)) == NULL) {
      printf("Error getting hostname: %s\n", host_name);
      exit(-1);
    }

    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);
    remote.sin_port = htons(atoi(port));
    if (connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0) {
        perror("connect call");
        exit(-1);
    }

    return socket_num;
}

void init_client(int argc, char** argv, client_data* cd) {
    int handle_len = 0;

    if (argc != 4) {
        printf("usage: cclient handle hostname port\n");
        exit(1);
    }

    if ((handle_len = strlen(argv[1])) > MAX_HNDL_LEN) {
        printf("use a handle with less than 255 chars\n");
        exit(1);
    }

    cd->socket = tcp_send_setup(argv[2], argv[3]);
    cd->handle_len = handle_len;
    cd->handle = strdup(argv[1]);
    cd->comm_size = MAX_MSG_LEN;
    cd->comm_buf = (char *) calloc(1, cd->comm_size);

    cd->set = malloc(sizeof(fd_set));

    // Don't buffer stdin because enter acts stupidly
    setvbuf(stdin, NULL, _IONBF, 0);
    printf("$: ");
    fflush(stdout);
}

// Lets the user type in a character or start a command with \n
void parse_command(client_data* cd) {
    static int len = 0;

    // Read stuff until you get \n
    if ((cd->comm_buf[len] = getchar()) != '\n') {
        len++;

        if (len > cd->comm_size - 2) {
            // Grow command buffer if necessary
            cd->comm_size *= 2;
            cd->comm_buf = realloc(cd->comm_buf, cd->comm_size);
        }
        return;
    }

    cd->comm_buf[len+1] = 0;
    len = 0;
    execute(cd);
}

// Take the command buffer and do stuff
void execute(client_data* cd) {
    if (cd->comm_buf[0] == '%') {
        switch (cd->comm_buf[1]) {
            case 'm':
            case 'M':
                create_m_packet(cd);
                break;
            case 'b':
            case 'B':
                create_b_packet(cd);
                break;
            case 'e':
            case 'E':
                cd->send_buf = prep_base_packet(3, NULL, FLAG_C_EXITING);
                send_data(cd->socket, cd->send_buf, 3);
                break;
            case 'l':
            case 'L':
                cd->send_buf = prep_base_packet(3, NULL, FLAG_WANT_HNDL);
                send_data(cd->socket, cd->send_buf, 3);
                break;
            default:
                break;
        }
    }

    memset(cd->comm_buf, 0, cd->comm_size);
    printf("$: ");
    fflush(stdout);
    return;
}

// Assemble a message packet
void create_m_packet(client_data* cd) {
    char *token, *comm_dup;
    char *dst_handle, *message;
    uint8_t dst_handle_len;
    int msg_bytes_left, pkt_off = 0;
    uint16_t frag_len;

    int head_len = M_MIN_LEN + cd->handle_len;

    comm_dup = strndup(cd->comm_buf + 3, MAX_HNDL_LEN);
    if ((token = strsep(&comm_dup, " \n")) != NULL) {
        dst_handle = strdup(token);
        head_len += (dst_handle_len = strlen(dst_handle));
    }
    else
        exit(1);

    message = cd->comm_buf + 4 + dst_handle_len;
    msg_bytes_left = strlen(message);

    if (msg_bytes_left == 0 || msg_bytes_left == 1) {
        cd->send_buf = prep_base_packet(head_len + 2, &pkt_off, FLAG_C_MESSAGE);
        prepend_handles(cd, &pkt_off, dst_handle, dst_handle_len);

        strncpy(cd->send_buf + pkt_off, " \0", 2);
        send_data(cd->socket, cd->send_buf, pkt_off + 2);

        free(cd->send_buf);
    }

    while (msg_bytes_left > 1) {
        // Determine message fragment length & prepare packet
        frag_len = MIN(msg_bytes_left, MAX_MSG_LEN);

        cd->send_buf = prep_base_packet(frag_len + head_len, &pkt_off, FLAG_C_MESSAGE);
        prepend_handles(cd, &pkt_off, dst_handle, dst_handle_len);

        strncpy(cd->send_buf + pkt_off, message, frag_len - 1); // -1
        send_data(cd->socket, cd->send_buf, pkt_off + frag_len);

        msg_bytes_left -= frag_len -1;
        message += frag_len -1;
        free(cd->send_buf);
    }

    free(dst_handle);
}

void create_b_packet(client_data* cd) {
    char *message;
    int msg_bytes_left, pkt_off = 0;
    uint16_t frag_len;

    int head_len = B_MIN_LEN + cd->handle_len;

    message = cd->comm_buf + 3;
    msg_bytes_left = strlen(message);

    if (msg_bytes_left == 0 || msg_bytes_left == 1) {
        cd->send_buf = prep_base_packet(head_len + 2, &pkt_off, FLAG_BROADCAST);
        prepend_handles(cd, &pkt_off, NULL, 0);

        strncpy(cd->send_buf + pkt_off, " \0", 2);
        send_data(cd->socket, cd->send_buf, pkt_off + 2);

        free(cd->send_buf);
    }

    while (msg_bytes_left > 1) {
        // Determine message fragment length & prepare packet
        frag_len = MIN(msg_bytes_left, MAX_MSG_LEN);

        cd->send_buf = prep_base_packet(frag_len + head_len, &pkt_off, FLAG_BROADCAST);
        prepend_handles(cd, &pkt_off, NULL, 0);

        strncpy(cd->send_buf + pkt_off, message, frag_len - 1);
        send_data(cd->socket, cd->send_buf, pkt_off + frag_len);

        msg_bytes_left -= frag_len - 1;
        message += frag_len - 1;
        free(cd->send_buf);
    }
}

void prepend_handles(client_data *cd, int* offset, char* dst_handle, int dst_handle_len) {
    int pkt_off = *offset;

    if (dst_handle) {
        cd->send_buf[pkt_off++] = dst_handle_len;
        memcpy(cd->send_buf + pkt_off, dst_handle, dst_handle_len);
        pkt_off += dst_handle_len;
    }
    cd->send_buf[pkt_off++] = cd->handle_len;
    memcpy(cd->send_buf + pkt_off, cd->handle, cd->handle_len);
    pkt_off += cd->handle_len;

    *offset = pkt_off;
}

void upload_info(client_data* cd) {
    int pkt_len = 4 + cd->handle_len;
    int pkt_off;
    char* first = prep_base_packet(pkt_len, &pkt_off, FLAG_C_CONNECT);

    first[pkt_off++] = cd->handle_len;
    memcpy(first + pkt_off, cd->handle, cd->handle_len);

    send_data(cd->socket, first, pkt_len);
    free(first);
}

int check_inbox(client_data *cd) {
    struct timeval tv = { 0, 10000 };
    int ready = 0;

    FD_SET(cd->socket, cd->set);
    FD_SET(STDIN_FILENO, cd->set);

    ready = select(cd->socket + 1, cd->set, NULL, NULL, &tv);
    if (ready < 0) {
        perror("select failed");
        return -1;
    }
    else if (FD_ISSET(STDIN_FILENO, cd->set)) {
        parse_command(cd);
        ready--;
    }
    else if (ready > 0 && FD_ISSET(cd->socket, cd->set)) {
        mail_sorter(cd);
    }
    return 0;
}

void mail_sorter(client_data *cd) {
    char buf[PKT_BUF_SIZE];
    uint16_t pkt_len = recv_data(cd->socket, buf, 3);

    if (pkt_len < 3) {
        printf("Server quit!\n");
        close(cd->socket);
        exit(0);
    }
    memcpy(&pkt_len, buf, 2);
    pkt_len = ntohs(pkt_len);

    switch (buf[2]) {
        case FLAG_BROADCAST:
        case FLAG_C_MESSAGE:
            recv_data(cd->socket, buf + 3, pkt_len - 3);
            print_message(buf);
            printf("$: ");
            fflush(stdout);
            break;
        case FLAG_S_DST_DNE:
            recv_data(cd->socket, buf + 3, pkt_len - 3);
            print_bad_handle(buf);
            break;
        case FLAG_S_EXIT_OK:
            close(cd->socket);
            exit(0);
            break;
        case FLAG_AMNT_HNDL:
            get_num_handles(cd);
            break;
        case FLAG_LIST_HNDL:
            get_list_handles(cd);
            break;
        default:
            break;
    }
}

void get_num_handles(client_data* cd) {
    recv_data(cd->socket, (char *)&cd->num_handles, 4);
    cd->num_handles = ntohl(cd->num_handles);
}

void get_list_handles(client_data* cd) {
    char handle_len;
    char handle[MAX_HNDL_LEN];

    printf("\nNumber of clients: %d\n", cd->num_handles);
    while (cd->num_handles > 0) {
        cd->num_handles--;
        recv_data(cd->socket, &handle_len, 1);
        recv_data(cd->socket, handle, handle_len);

        handle[(int)handle_len] = 0;
        printf("   %s\n", handle);
    }

    printf("$: ");
    fflush(stdout);
}

void print_bad_handle(char* buf) {
    char handle[MAX_HNDL_LEN];

    int handle_len = buf[3];
    memcpy(handle, buf + 4, handle_len + 1);
    handle[handle_len] = 0;

    printf("\b\b\bClient with handle %s does not exist\n", handle);
    printf("$: ");
    fflush(stdout);
}

void print_message(char* buf) {
    uint16_t len;
    char handle_len;
    char sender[MAX_HNDL_LEN];
    int pkt_off = 2;

    memcpy(&len, buf, pkt_off);
    len = ntohs(len);

    if (buf[pkt_off++] == FLAG_C_MESSAGE) {
        // Skip the first handle, it's yours!
        handle_len = buf[pkt_off++];
        pkt_off += handle_len;
    }
    handle_len = buf[pkt_off++];
    memcpy(sender, buf + pkt_off, handle_len + 1);
    sender[(int)handle_len] = 0;
    pkt_off += handle_len;

    printf("\n%s: %s\n", sender, buf + pkt_off);
}
