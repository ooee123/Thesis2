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

#include "types.h"
#include "networks.h"
#include "packet.h"
#include "server.h"
#include "sys.h"

#define CLEAR_PACKET() memset(packet, 0, MAX_PKT_LEN)
#define SEND_PACKET(a,b) send_packet_((a),(b),0)
#define SEND_PACKET_L(a,b,c) send_packet_((a),(b),(c))

int8 packet[MAX_PKT_LEN];
sclient * clients;
int sizeofcs = NUM_CLIENTS;
int indexcs = 0;

void print_packet(int n) {
  int i;
  for (i = 0; i < n; i++) {
    printf("%x ", (int8)(packet[i]));
  }
  printf("\n");
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
  int server_socket = tcp_recv_setup();
  clients = calloc(sizeof(sclient), NUM_CLIENTS);
  chat(server_socket);
  return 1;
}

void chat(int socket) {
  fd_set fds;
  int i;
  int maxfd = socket + 1;
  sclient * aclient;
  //printf("starting chat on %d\n", socket);
  while (1) {
    FD_ZERO(&fds);
    FD_SET(socket, &fds);
    for (i = 0; i < indexcs; i++) {
      aclient = clients + i;
      if (aclient->state == ST_DEAD)
        continue;
      FD_SET(aclient->socket, &fds);
      maxfd = aclient->socket >= maxfd ? aclient->socket + 1 : maxfd;
    }
    myselect(maxfd, &fds, NULL, NULL, NULL);
    if (FD_ISSET(socket, &fds)) {
      init_client(socket);
    }
    for (i = 0; i < indexcs; i++) {
      aclient = clients + i;
      if (aclient->state == ST_DEAD)
        continue;
      if(FD_ISSET(aclient->socket, &fds)) {
        parse_packet(aclient);
      }
    }
  }
}

void init_client(int socket) {
  if (indexcs + 1 >= sizeofcs) {
    clients = realloc(clients, (sizeofcs + NUM_CLIENTS) * sizeof(sclient));
  }
  sclient * aclient = clients + indexcs;
  indexcs++;
  aclient->socket = myaccept(socket, NULL, NULL);
  aclient->state = ST_BORN;
}

void parse_packet(sclient * aclient) {
  char * h;
  int len, i;
  CLEAR_PACKET();
  if (myrecv(aclient->socket, packet, MAX_PKT_LEN, 0) == 0) {
    aclient->state = ST_DEAD;
    free(aclient->handle);
    return;
  }
  switch(getFlag()) {
  case PF_INIT:
    h = malloc((MAX_H_LEN + 1) * sizeof(char));
    len = *getData(0);
    memcpy(h, getData(1), len);
    *(h + len) = '\0';
    aclient->handle = h;
    aclient->hlen = len;
    for (i = 0; i < indexcs - 1; i++) {
      sclient * cmp = clients + i;
      if (cmp->state != ST_ACTIVE)
        continue;
      if (strcmp(h, cmp->handle) == 0) {
        SEND_PACKET(aclient, PF_INIT_ACK_B);
        return;
      }
    }
    SEND_PACKET(aclient, PF_INIT_ACK_G);
    break;
  case PF_C_EXIT:
    SEND_PACKET(aclient, PF_C_EXIT_ACK);
    break;
  case PF_MES:
    parse_mes(aclient);
    break;
  case PF_BCAST:
    parse_bcast(aclient);
    break;
  case PF_LIST:
    parse_list(aclient);
    break;
  default:
    break;
  }
}

void parse_list(sclient * aclient) {
  int i, count = 0;
  header2 * h2 = (void *)packet;
  for (i = 0; i < indexcs; i++) {
    sclient * bclient = clients + i;
    if (bclient->state == ST_ACTIVE) {
      count++;
    }
  }
  CLEAR_PACKET();
  setLen(HDR_LEN + 4);
  setFlag(PF_LIST_ACK);
  h2->len2 = htonl(count);
  SEND_PACKET(aclient, PF_LIST_ACK);
  CLEAR_PACKET();
  send_list(aclient);
}

void send_list(sclient * aclient) {
  int i, off = 0;
  for (i = 0; i < indexcs; i++) {
    sclient * bc = clients + i;
    if (bc->state != ST_ACTIVE)
      continue;
    setData(off, &bc->hlen, 1);
    off = off + 1;
    setData(off, (int8 *)bc->handle, bc->hlen);
    off += bc->hlen;
    if (off > MAX_PKT_LEN - (MAX_H_LEN + 1)) {
      setFlag(PF_LIST_ACK2);
      SEND_PACKET_L(aclient, PF_LIST_ACK2, HDR_LEN + off);
      CLEAR_PACKET();
      off = 0;
    }
  }
  if (off) {
    setFlag(PF_LIST_ACK2);
    SEND_PACKET_L(aclient, PF_LIST_ACK2, HDR_LEN + off);
  }
}

void parse_bcast(sclient * aclient) {
  char src[MAX_H_LEN + 1];
  int slen, i;
  slen = *getData(0);
  memcpy(src, getData(1), slen);
  *(src + slen) = '\0';
  for (i = 0; i < indexcs; i++) {
    sclient * bclient = clients + i;
    if (bclient->state != ST_ACTIVE)
      continue;
    if (strcmp(src, bclient->handle)) {
      SEND_PACKET(bclient, PF_BCAST);
    }
  }
}

void parse_mes(sclient * aclient) {
  char dst[MAX_H_LEN + 1], src[MAX_H_LEN + 1];
  int dlen, slen, i;
  dlen = *getData(0);
  memcpy(dst, getData(1), dlen);
  *(dst + dlen) = '\0';
  slen = *getData(dlen + 1);
  memcpy(src, getData(dlen+2), slen);
  *(src + slen) = '\0';
  for (i = 0; i < indexcs; i++) {
    sclient * bclient = clients + i;
    if (bclient->state != ST_ACTIVE)
      continue;
    if (strcmp(dst, bclient->handle) == 0) {
      SEND_PACKET(bclient, PF_MES);
      return;
    }
  }
  setLen(HDR_LEN + dlen + 1);
  SEND_PACKET_L(aclient, PF_ERROR, HDR_LEN + dlen + 1);
}

void send_packet_(sclient * aclient, int flag, int len) {
  switch (flag) {
  case PF_INIT_ACK_B:
    CLEAR_PACKET();
    setLen(HDR_LEN);
    setFlag(PF_INIT_ACK_B);
    mysend(aclient->socket, packet, HDR_LEN, 0);
    aclient->state = ST_DEAD;
    break;
  case PF_INIT_ACK_G:
    CLEAR_PACKET();
    setLen(HDR_LEN);
    setFlag(PF_INIT_ACK_G);
    aclient->state = ST_ACTIVE;
    mysend(aclient->socket, packet, HDR_LEN, 0);
    break;
  case PF_C_EXIT_ACK:
    CLEAR_PACKET();
    setLen(HDR_LEN);
    setFlag(PF_C_EXIT_ACK);
    mysend(aclient->socket, packet, HDR_LEN, 0);
    aclient->state = ST_DEAD;
    break;
  case PF_MES:
    mysend(aclient->socket, packet, getLen(), 0);
    break;
  case PF_BCAST:
    mysend(aclient->socket, packet, getLen(), 0);
    break;
  case PF_LIST_ACK:
    mysend(aclient->socket, packet, getLen(), 0);
    break;
  case PF_LIST_ACK2:
    mysend(aclient->socket, packet, len, 0);
    break;
  case PF_ERROR:
    len = HDR_LEN + *(getData(0)) + 1;
    setLen(len);
    setFlag(PF_ERROR);
    mysend(aclient->socket, packet, len, 0);
    break;
  default:
    break;
  }
}

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
    mylisten(server_socket, 5);
    printf("socket has port %d \n", ntohs(local.sin_port));
	        
    return server_socket;
}
