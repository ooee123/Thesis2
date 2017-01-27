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
#include "sys.h"
#include "cclient.h"
#include "packet.h"

#define CLEAR_PACKET() memset(packet, 0, MAX_PKT_LEN)

int8 packet[MAX_PKT_LEN];
int count = 0, newc = 0;


void print_packet(int n) {
  int i;
  for (i = 0; i < n; i++) {
    printf("%x ", (int8)(packet[i]));
  }
  printf("\n");
}

void checkArgs(int argc, char * argv[]) {
    if (argc != 4) {
        printf("usage: cclient <handle> <host-name> <port-number> \n");
        exit(-1);
    }
    if (strlen(argv[1]) > MAX_H_LEN) {
        printf("handle too long");
        exit(-1);
    }
}

void setFlag(int flag) {
    header *pkt = (header *) packet;
    pkt->flag = flag;
}

void setLen(int len) {
    header *pkt = (header *) packet;
    pkt->len = htons(len);
}

int getFlag() {
    header *pkt = (header *) packet;
    return pkt->flag;
}

int getLen() {
    header *pkt = (header *) packet;
    return ntohs(pkt->len);
}

void setData(int offset, int8 * data, int n) {
    memcpy(packet + HDR_LEN + offset, data, n);
}

int8 * getData(int offset) {
    return packet + HDR_LEN + offset;
}

int main(int argc, char * argv[]) {
    cclient * myclient;
    
    checkArgs(argc, argv);
    myclient = init(argv);
    init_ack(myclient);
    chat(myclient);
    close(myclient->socket);
    return 0;
}

void init_ack(cclient *myclient) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(myclient->socket, &fds);
    myselect(myclient->socket + 1, &fds, NULL, NULL, NULL);
    if (FD_ISSET(myclient->socket, &fds)) {
        parse_packet(myclient);
    }
}

void parse_packet(cclient * myclient) {
    CLEAR_PACKET();
    if (myrecv(myclient->socket, packet, HDR_LEN, 0) == 0) {
        printf("Server Terminated\n");
        exit(-1);
    }
    //print_packet(3);
    switch(getFlag()) {
    case PF_INIT_ACK_G:
        myclient->state = ST_ACTIVE;
        break;
    case PF_INIT_ACK_B:
        printf("Handle already in use: %s\n", myclient->handle);
        exit(-1);
        break;
    case PF_MES:
        parse_message(myclient);
        break;
    case PF_BCAST:
        parse_broadcast(myclient);
        break;
    case PF_LIST_ACK:
        newc = count = parse_list(myclient);
        myclient->state = ST_LIST_ACK_WAIT;
        break;
    case PF_LIST_ACK2:
        parse_list2(myclient);
        if (newc <= 0) {
            myclient->state = ST_ACTIVE;
        }
        break;
    case PF_ERROR:
        parse_error(myclient);
        break;
    case PF_C_EXIT_ACK:
        close(myclient->socket);
        myclient->state = ST_DISCONNECT;
        break;
    default:
        break;
    }
}

int parse_list(cclient * myclient) {
    myrecv(myclient->socket, packet+HDR_LEN, getLen()-HDR_LEN, 0);
    header2 * h2 = (void *)packet;
    return ntohl(h2->len2);
}

void parse_list2(cclient * myclient) {
    if (newc == count)
        printf("Number of clients: %d", count);
    do {
        int hlen;
        char h[MAX_H_LEN + 1];
        myrecv(myclient->socket, packet+HDR_LEN, 1, 0);
        hlen = *getData(0);
        if (hlen == 0) break;
        myrecv(myclient->socket, packet+HDR_LEN+ 1, hlen, 0);
        memcpy(h, getData(1), hlen);
        *(h + hlen) = '\0';
        printf("\n\t%s", h);
    } while (--newc);
}

void parse_error(cclient * myclient) {
    char dst[MAX_H_LEN + 1];
    int dlen;
    myrecv(myclient->socket, packet+HDR_LEN, getLen(), 0);
    dlen = *getData(0);
    memcpy(dst, getData(1), dlen);
    *(dst + dlen) = '\0';
    printf("\nClient with handle %s does not exist.", dst);
    return;
}

void parse_message(cclient * myclient) {
    char dst[MAX_H_LEN + 1], src[MAX_H_LEN + 1], c[MAX_MSG];
    int dlen, slen, off;
    myrecv(myclient->socket, packet+HDR_LEN, getLen() - HDR_LEN, 0);
    dlen = *getData(0);
    memcpy(dst, getData(1), dlen);
    *(dst + dlen) = '\0';
    slen = *getData(dlen + 1);
    memcpy(src, getData(dlen+2), slen);
    *(src + slen) = '\0';
    off = dlen+slen+2;
    memcpy(c, getData(off), getLen() - off - 3);
    printf("\n%s: %s", src, c);
}

cclient * init(char * argv[]) {
    int socket;
    cclient * myclient;
    socket = tcp_send_setup(argv[2], argv[3]);
    myclient = malloc(sizeof(cclient));
    myclient->socket = socket;
    myclient->handle = argv[1];
    myclient->hlen = strlen(myclient->handle);
    myclient->state = ST_DISCONNECT;
    setFlag(PF_INIT);
    setLen(HDR_LEN + myclient->hlen + sizeof(int8));
    setData(0, &(myclient->hlen), sizeof(int8));
    setData(1, (int8 *)myclient->handle, (int)myclient->hlen);
    mysend(myclient->socket, packet, getLen(), 0);
    return myclient;
}

void send_message(cclient * myclient) {
    char c [MAX_MSG];
    char h [MAX_H_LEN+1];
    char n;
    int msgoff, pktlen = 0, i;
    for (i = 0; i < MAX_H_LEN+1; i++) {
        n = getchar();
        if (n == ' ') break;
        h[i] = n;
    }
    if (n != ' ') {
        perror ("recepient handle too long");
        return;
    }
    setData(0, (int8 *)&i, 1);
    setData(1, (int8 *)h, i);
    setData(1 + i, (int8 *)&(myclient->hlen), 1);
    setData(2 + i, (int8 *)myclient->handle, myclient->hlen);
    setFlag(PF_MES);
    msgoff = 2 + i + myclient->hlen;
    i = 0;
    while ((n = getchar()) != '\n' && n != EOF) {
        c[i] = n;
        i++;
        if (i == MAX_MSG - 1) {
            c[i] = '\0';
            setData(msgoff, (int8 *)c, i + 1);
            pktlen = HDR_LEN + msgoff + i;
            setLen(pktlen);
            mysend(myclient->socket, packet, pktlen, 0);
            i = 0;
        }
    }
    if (i) {
        c[i] = '\0';
        setData(msgoff, (int8 *)c, i + 1);
        pktlen = HDR_LEN + msgoff + i + 1;
        setLen(pktlen);
        mysend(myclient->socket, packet, pktlen, 0);
    }
}

void print_prompt() {
    printf("$: ");
    fflush(stdout);
}

void parse_input(cclient * myclient) {
    char input[MAX_CMD_LEN + 1];
    char c;
    int i;
    for (i = 0; i < MAX_CMD_LEN + 1; i++) {
        c = getchar();
        if (c == ' ' || c == '\n') break;
        input[i] = c;
    }
    if (input[0] != '%') {
        printf("Invalid command\n");
        while ((c = getchar()) != '\n' && c != EOF); 
        return;
    }
    switch (input[1]) {
    case 'm': case 'M':
        //message to myclient
        send_message(myclient);
        break;
    case 'b': case 'B':
        //broadcast
        broadcast(myclient);
        break;
    case 'l': case 'L':
        //list handles
        send_ls(myclient);
        break;
    case 'e': case 'E':
        send_dc(myclient);
        break;
    default:
        printf("Invalid command\n");
        while (c != '\n' && c != EOF && 
            (c = getchar()) != '\n' && c != EOF);
        break;
    }
}

void parse_broadcast(cclient * myclient) {
    char src[MAX_H_LEN + 1], c[MAX_MSG];
    int slen, off;
    myrecv(myclient->socket, packet+HDR_LEN, getLen()-HDR_LEN, 0);
    slen = *getData(0);
    memcpy(src, getData(1), slen);
    *(src + slen) = '\0';
    off = slen+1;
    memcpy(c, getData(off), getLen() - off - 3);
    printf("\n%s: %s", src, c);
}

void broadcast(cclient * myclient) {
    char c [MAX_MSG];
    char n;
    int msgoff, pktlen = 0, i;
    setData(0, (int8 *)&(myclient->hlen), 1);
    setData(1, (int8 *)myclient->handle, myclient->hlen);
    setFlag(PF_BCAST);
    msgoff = 1 + myclient->hlen;
    i = 0;
    while ((n = getchar()) != '\n' && n != EOF) {
        c[i] = n;
        i++;
        if (i == MAX_MSG - 1) {
            c[i] = '\0';
            setData(msgoff, (int8 *)c, i + 1);
            pktlen = HDR_LEN + msgoff + i;
            setLen(pktlen);
            mysend(myclient->socket, packet, pktlen, 0);
            i = 0;
        }
    }
    if (i) {
        c[i] = '\0';
        setData(msgoff, (int8 *)c, i + 1);
        pktlen = HDR_LEN + msgoff + i + 1;
        setLen(pktlen);
        mysend(myclient->socket, packet, pktlen, 0);
    }
    CLEAR_PACKET();
}

void send_dc(cclient * myclient) {
    CLEAR_PACKET();
    setLen(HDR_LEN);
    setFlag(PF_C_EXIT);
    mysend(myclient->socket, packet, HDR_LEN, 0);
    myclient->state = ST_EXIT_ACK_WAIT;
    
}

void send_ls(cclient * myclient) {
    CLEAR_PACKET();
    setLen(HDR_LEN);
    setFlag(PF_LIST);
    mysend(myclient->socket, packet, HDR_LEN, 0);
}

void chat(cclient * myclient) {
    fd_set fds;
    FD_ZERO(&fds);
    while (myclient->state != ST_DISCONNECT) {
        if (myclient->state == ST_ACTIVE) { 
            print_prompt();
            FD_SET(STDIN_FILENO, &fds);
        }
        FD_SET(myclient->socket, &fds);
        myselect(myclient->socket + 1, &fds, NULL, NULL, NULL);
        if (FD_ISSET(myclient->socket, &fds)) {
            parse_packet(myclient);
            printf("\n");
        }
        if (myclient->state == ST_ACTIVE && 
            FD_ISSET(STDIN_FILENO, &fds)) {
            parse_input(myclient);
        } 
    }
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

