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

#include "testing.h"
#include "cclient.h"

static int32_t client_sock = 0;
static char handle[MAX_HANDLE_LEN] = {0};
static uint8_t handle_len = 0;
static fd_set fd_var;

int main(int argc, char **argv) {
    init_client(argc, argv);

    run_client();

    //run_chat_client(sock_num);
    close(client_sock);
    return 0;
}

void init_client(int argc, char **argv) {
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host

    // Check command line arguements
    if (argc != NUM_CMD_ARGS) {
        fprintf(stderr, "Usage: %s handle server-name server-port\n", argv[0]);
        exit(1);
    }

    // get the client handle
    if (strlen(argv[1]) > MAX_HANDLE_LEN) {
        fprintf(stderr, "Error: Handle name too long\n");
        exit(-1);
    }
    else {
        handle_len = strlen(argv[1]);
        memcpy(handle, argv[1], handle_len);
    }

    // create the socket
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client socket call");
        exit(-1);
    }

    // designate the addressing family
    remote.sin_family= AF_INET;

    // get the address of the remote host and store
    if ((hp = gethostbyname(argv[2])) == NULL) {
        fprintf(stderr, "Error getting hostname: %s\n", argv[2]);
        exit(-1);
    }
    
    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

    // get the port used on the remote side and store
    remote.sin_port= htons(atoi(argv[3]));

    if (connect(client_sock, (struct sockaddr*)&remote,
     sizeof(struct sockaddr_in)) < 0) {
        perror("connect call");
        exit(-1);
    }

    FD_ZERO(&fd_var);
    FD_SET(client_sock, &fd_var);

    send_handle();
}

void send_handle() {
    int32_t sent;
    uint16_t pack_len = sizeof(normal_header) + 1 + handle_len;
    char *send_buffer = malloc(pack_len), *tmp_buf_ptr, recv_buffer[3],
     flag = 0, tmp_handle[MAX_HANDLE_LEN + 1] = {0};

    ((normal_header *)send_buffer)->len = htons(pack_len);
    ((normal_header *)send_buffer)->flag = 1;

    //offset to handle len
    tmp_buf_ptr = send_buffer + sizeof(normal_header);
    *tmp_buf_ptr = handle_len;

    tmp_buf_ptr++;
    memcpy(tmp_buf_ptr, handle, handle_len);

    sent = send(client_sock, send_buffer, pack_len, 0);

    if (sent < 0) {
        perror("send_handle");
        exit(-1);
    }

    while (flag != 2 && flag != 3) {
        retrieve_data(3, recv_buffer, 0);
        flag = recv_buffer[2];
    }

    if (flag == 3) {
        memcpy(tmp_handle, handle, handle_len);
        printf("Handle already in use: %s\n", handle);
        exit(-1);
    }
    free(send_buffer);
}

uint16_t retrieve_data(uint16_t buff_size, char *buffer, uint16_t recv_tot) {
    int16_t recv_len;

    while (recv_tot != buff_size)  {
        recv_len = recv(client_sock, buffer + recv_tot,
         buff_size - recv_tot, 0);

        if (recv_len < 0) {
            perror("recv call");
            exit(-1);
        }

        if (recv_len == 0) { // server closed
            printf("\nServer Terminated\n");
            exit(-1);
        }
        recv_tot += recv_len;
    }
    return recv_tot;
}

void run_client () {
    while(1) {
        printf("$: ");
        fflush(stdout);
        set_fds();
        select(client_sock + 1, (fd_set *) &fd_var, NULL, NULL, NULL);

        if (FD_ISSET(0 , &fd_var)) {
            read_stdin();
        }
        else if (FD_ISSET(client_sock, &fd_var)) {
            read_server_msg();
        }
    }
}

void set_fds() {
    // monitor stdin and the client socket
    FD_ZERO(&fd_var);  
    FD_SET(client_sock, &fd_var);
    FD_SET(0, &fd_var);
}

void read_stdin() {
    char buff[3] = {0}, cmd;
    scanf("%3s", buff);

    if (buff[0] != '%' || buff[2] != 0)
        printf("Invalid command\n");
    else {
        cmd = toupper(buff[1]);

        switch (cmd) {
            case 'M':
                message_client();
                break;
            case 'B':
                broadcast_msg();
                break;
            case 'L':
                req_client_list();
                break;
            case 'E': 
                req_exit_chat();
                break;
            default: 
                printf("Invalid command\n");
        }
    }
}

void message_client() {
    int32_t dest_len;
    char buff[BUFF_SIZE], dest[MAX_HANDLE_LEN + 1] = {0}, *tmp;

    scanf("%255s", dest); 
    dest_len = strlen(dest);

    // set up the header 
    tmp = buff;
    ((normal_header *)tmp)->flag = 5;
    tmp += sizeof(normal_header);

    *tmp++ = dest_len;
    memcpy(tmp, dest, dest_len);
    tmp += dest_len;

    *tmp++ = handle_len;
    memcpy(tmp, handle, handle_len);
    tmp += handle_len;

    append_and_send_msg(buff, tmp, dest_len);
}

void broadcast_msg() {
    char buff[BUFF_SIZE], *tmp;

    // set up the header 
    tmp = buff;
    ((normal_header *)tmp)->flag = 4;
    tmp += sizeof(normal_header);

    *tmp++ = handle_len;
    memcpy(tmp, handle, handle_len);
    tmp += handle_len;

    append_and_send_msg(buff, tmp, 0);

}

void append_and_send_msg(char *buff, char *msg_offset, int32_t dest_len) {
    int32_t sent, send_len, msg_len, hdr_offset;
    char next_char;

    hdr_offset = sizeof(normal_header) + 1 + handle_len;
    if (dest_len)
        hdr_offset += 1 + dest_len;

    next_char = getchar();
    msg_len = 0;
    //skips the white space
    if (next_char != '\n') {
        while ((next_char = getchar())!= '\n') {
            msg_offset[msg_len] = next_char;
            msg_len++;

            if (msg_len == MAX_MESSAGE_LEN) {
                msg_offset[msg_len] = 0;
                send_len = hdr_offset + msg_len + 1;
                ((normal_header *)buff)->len = htons(send_len);

                sent = send(client_sock, buff, send_len, 0);
                if (sent < 0) {
                    perror("send call");
                    exit(-1);
                }
                msg_len = 0;
            }
        }
    }
    msg_offset[msg_len] = 0;
    send_len = hdr_offset + msg_len + 1;
    ((normal_header *)buff)->len = htons(send_len);

    sent = send(client_sock, buff, send_len, 0);
    if (sent < 0) {
        perror("send call");
        exit(-1);
    } 
}

void req_client_list() {
    int32_t sent; 
    normal_header hdr;

    hdr.len = htons(sizeof(normal_header));
    hdr.flag = 10;
    sent = send(client_sock, &hdr, sizeof(normal_header), 0);

    if (sent < 0) {
        perror("send_handle");
        exit(-1);
    }
}

void req_exit_chat() {
    int32_t sent; 
    normal_header hdr;

    hdr.len = htons(sizeof(normal_header));
    hdr.flag = 8;
    sent = send(client_sock, &hdr, sizeof(normal_header), 0);

    if (sent < 0) {
        perror("send_handle");
        exit(-1);
    }
}

void read_server_msg() {
    uint16_t recv_tot, data_len;
    uint8_t flag;
    char *buf = malloc(BUFF_SIZE);

    recv_tot = 0;

    recv_tot = retrieve_data(sizeof(normal_header), buf, 0);
    if (!recv_tot) {
        free(buf);
        return;
    }

    flag = ((normal_header *)buf)->flag; 
    
    // case where there is 0 length, and you need to process the
    // number of clients instead
    if (flag == 12) {
        print_clients_list(buf);
    }
    else {
        data_len = ntohs(((normal_header *)buf)->len);

        recv_tot = retrieve_data(data_len, buf, recv_tot);
        if (!recv_tot) {
            free(buf);
            return;
        }

        switch (flag) {
            case 4:
                read_broadcast_msg(buf, data_len);
                break;
            case 5: // received msg
                read_chat_msg(buf, data_len);
                break;
            case 7: // bad dest
                print_bad_dest(buf, data_len);
                break;
            case 9: // terminate
                exit(0);
                break;
            case 11: // get num handles
//                printf("\nNumber of clients: %d\n", ntohl(*(uint32_t *)(buf +
  //               sizeof(normal_header)))); 
                break;
            default:
                break;
        }
    }
}

void read_chat_msg(char *buf, uint32_t data_len) {
    uint8_t src_len;
    char src[MAX_HANDLE_LEN + 1] = {0}, msg[MAX_MESSAGE_LEN + 1],
     *tmp_ptr = buf;
    uint32_t offset = sizeof(normal_header) + 1 + handle_len;

    // skip to src info
    tmp_ptr += offset;
    src_len = *tmp_ptr++;

    memcpy(src, tmp_ptr, src_len);
    tmp_ptr += src_len;

    memcpy(msg, tmp_ptr, data_len - offset - 1 - src_len);
    printf("\n%s: %s\n", src, msg);
    free(buf);
}

void read_broadcast_msg(char *buf, uint32_t data_len) {
    uint8_t src_len;
    char src[MAX_HANDLE_LEN + 1] = {0}, msg[MAX_MESSAGE_LEN + 1],
     *tmp_ptr = buf;
    uint32_t offset = sizeof(normal_header);

    // skip to src info
    tmp_ptr += offset;
    src_len = *tmp_ptr++;

    memcpy(src, tmp_ptr, src_len);
    tmp_ptr += src_len;

    memcpy(msg, tmp_ptr, data_len - offset - 1 - src_len);
    printf("\n%s: %s\n", src, msg);
    free(buf);
}

void print_bad_dest(char *buf, uint32_t data_len) {
    uint8_t dest_len;
    char dest[MAX_HANDLE_LEN + 1] = {0}, *tmp_ptr = buf;

    tmp_ptr += sizeof(normal_header);
    dest_len = *tmp_ptr++;

    memcpy(dest, tmp_ptr, dest_len);
    printf("\nClient with the handle %s does not exist.\n", dest);
    free(buf);
}

void print_clients_list(char *buf) {
    uint32_t num_clients, curr_client, buff_size = BUFF_SIZE;
    uint16_t recv_tot = sizeof(normal_header); // the 3 bytes of the header 
    uint8_t handle_len;
    char *tmp_ptr = buf, handle[MAX_HANDLE_LEN + 1];

    recv_tot = retrieve_data(sizeof(uint32_t) + sizeof(normal_header),
     buf, recv_tot);

    tmp_ptr += sizeof(normal_header);
    num_clients = ntohl(*(uint32_t *)tmp_ptr);
    tmp_ptr += sizeof(uint32_t);

    printf("\nNumber of clients: %d\n", num_clients);
    for (curr_client = 0; curr_client < num_clients; curr_client++) {
        if (recv_tot + 2 + MAX_HANDLE_LEN > buff_size) {
            buff_size += BUFF_SIZE;
            buf = realloc(buf, buff_size);
            tmp_ptr = buf + recv_tot;
        }

        recv_tot = retrieve_data(recv_tot + 1, buf, recv_tot);
        handle_len = *tmp_ptr++;

        recv_tot = retrieve_data(recv_tot + handle_len, buf, recv_tot);
        memcpy(handle, tmp_ptr, handle_len);
        handle[handle_len] = 0;

        tmp_ptr += handle_len;
        
        printf("\t%s\n",handle);
    }
    free(buf);
}