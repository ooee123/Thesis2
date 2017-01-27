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
#include "common.h"

#define IS_MESSAGE(input) (strncmp("%M", (input), 2) == 0 || strncmp("%m", (input), 2) == 0)
#define IS_BROADCAST(input) (strncmp("%B", (input), 2) == 0 || strncmp("%b", (input), 2) == 0)
#define IS_HANDLES(input) (strncmp("%L", (input), 2) == 0 || strncmp("%l", (input), 2) == 0)
#define IS_EXIT(input) (strncmp("%E", (input), 2) == 0 || strncmp("%e", (input), 2) == 0)
#define MAXBUF 1024

static int server_socket;
static uint8_t handle_len;
static char *handle;
static int num_handles = 0;

void checkArgs(int argc, char **argv) {
  /* check command line arguments  */
  if (argc != 4) {
    printf("usage: %s handle host-name port-number \n", argv[0]);
    exit(1);
  } else if (strlen(argv[1]) > 255) {
    perror("Handle must be <= 255 characters\n");
    exit(1);
  }

  handle = argv[1];
  handle_len = strlen(handle);
}

int tcpClientSetup(char *host_name, char *port)
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

void initClient() {
  int packet_len = HEADER_LEN + handle_len + 1;
  char *buffer = malloc(packet_len);

  *((uint16_t *) buffer) = packet_len;
  *((uint8_t *) buffer + 2) = INIT_CLIENT;

  *((uint8_t *) buffer + 3) = handle_len;
  memmove(buffer + 4, handle, handle_len);

  if (send(server_socket, buffer, packet_len, 0) < 0) {
    perror("send initClient");
    exit(-1);
  }

  Header header;

  if (recv(server_socket, &header, HEADER_LEN, 0) < 0) {
    perror("recv initClient");
    exit(-1);
  }

  if (header.flag == DENY_HANDLE) {
    printf("Handle already in use: %s\n", handle);
    exit(1);
  }

  free(buffer);
}

void recvMessage(int len) {
  char buffer[len];
  uint8_t src_handle_len;
  uint8_t dst_handle_len;
  char *src_handle;
  char *msg;

  if (recv(server_socket, buffer, len, 0) < 0) {
    perror("recv recvMessage");
    exit(-1);
  }

  dst_handle_len = *((uint8_t *) buffer);
  src_handle_len = *((uint8_t *) buffer + 1 + dst_handle_len);
  src_handle = calloc(1, src_handle_len + 1);
  memmove(src_handle, buffer + 2 + dst_handle_len, src_handle_len);
  msg = malloc(len - src_handle_len - dst_handle_len - 2);
  memmove(msg, buffer + 2 + src_handle_len + dst_handle_len,
          len - src_handle_len - dst_handle_len - 2);

  printf("\n%s: %s\n", src_handle, msg);
  free(src_handle);
  free(msg);
}

void recvBroadcast(int len) {
  char buffer[len];
  uint8_t handle_len;
  char *handle;
  char *msg;

  if (recv(server_socket, buffer, len, 0) < 0) {
    perror("recv recvMessage");
    exit(-1);
  }

  handle_len = *((uint8_t *) buffer);
  handle = calloc(1, handle_len + 1);
  memmove(handle, buffer + 1, handle_len);
  msg = malloc(len - handle_len - 1);
  memmove(msg, buffer + 1 + handle_len, len - handle_len - 1);

  printf("\n%s: %s\n", handle, msg);
  free(handle);
  free(msg);
}

void recvHandles(int len) {
  char buffer[len];

  if (recv(server_socket, buffer, len, 0) < 0) {
    perror("recv recvHandles");
    exit(-1);
  }

  num_handles = ntohl(*((uint32_t *) buffer));
  printf("\nNumber of clients: %d\n", num_handles);
}

void recvHandle(int len) {
  char buffer[len];

  if (recv(server_socket, buffer, len, 0) < 0) {
    perror("recv recvHandle");
    exit(-1);
  }

  int handle_len = *((uint8_t *) buffer);
  char h_buf[handle_len + 1];
  h_buf[handle_len] = '\0';
  memmove(h_buf, buffer + 1, handle_len);

  printf("\t%s\n", h_buf);
  num_handles--;
}

void recvHandleDNE(int len) {
  char buffer[len];

  if (recv(server_socket, buffer, len, 0) < 0) {
    perror("recv recvHandle");
    exit(-1);
  }

  int handle_len = *((uint8_t *) buffer);
  char h_buf[handle_len + 1];
  h_buf[handle_len] = '\0';
  memmove(h_buf, buffer + 1, handle_len);

  printf("Client with handle %s does not exist.\n", h_buf);
}

void processServer() {
  int bytesRecv;
  Header header;

  if ((bytesRecv = recv(server_socket, &header, HEADER_LEN, 0)) < 0) {
    perror("recv processServer");
    exit(-1);
  }

  if (bytesRecv == 0) {
    printf("\nServer Terminated\n");
    exit(0);
  }

  int len = ntohs(header.len);
  if (header.flag == BROADCAST) {
    recvBroadcast(len - HEADER_LEN);
  } else if (header.flag == MESSAGE) {
    recvMessage(len - HEADER_LEN);
  } else if (header.flag == ACK_EXIT) {
    close(server_socket);
    exit(0); 
  } else if (header.flag == NUM_HANDLES) {
    recvHandles(len - HEADER_LEN);
  } else if (header.flag == SINGLE_HANDLE) {
    recvHandle(len - HEADER_LEN);
  } else if (header.flag == HANDLE_DNE) {
    recvHandleDNE(len - HEADER_LEN);
  } else {
    printf("Unknown flag: %d\n", header.flag);
  }
}

void sendMessage(int len, char *message) {
  char dest[256];
  sscanf(message, "%s ", dest);
  int dst_handle_len = strlen(dest);
  len -= dst_handle_len + 1;
  message += dst_handle_len +1;

  char buffer[HEADER_LEN + 2 + handle_len + dst_handle_len + 1001];
  *((uint8_t *) buffer + 2) = MESSAGE;
  *((uint8_t *) buffer + HEADER_LEN) = dst_handle_len;
  memmove(buffer + HEADER_LEN + 1, dest, dst_handle_len);
  *((uint8_t *) buffer + HEADER_LEN + dst_handle_len + 1) = handle_len;
  memmove(buffer + HEADER_LEN + dst_handle_len + 2, handle, handle_len);

  while (len > 0) {
    int msg_len = len > 1000 ? 1001 : len + 1;
    *((uint16_t *) buffer) = 
        htons(HEADER_LEN + 2 + handle_len + dst_handle_len + msg_len);
    memmove(buffer + HEADER_LEN + 2 + handle_len + dst_handle_len,
            message, msg_len);
    buffer[HEADER_LEN + 2 + handle_len + dst_handle_len + msg_len + 1] = '\0';

    if (send(server_socket,
             buffer,
             HEADER_LEN + 2 + handle_len + dst_handle_len + msg_len + 1,
             0) < 0) {
      perror("send sendMessage\n");
      exit(-1);
    }

    len -= 1000;
    message += 1000;
  }
}

void sendBroadcast(int len, char *message) {
  int packet_len = HEADER_LEN + 1 + handle_len + len;
  char buffer[packet_len];
  
  *((uint16_t *) buffer) = htons(packet_len);
  *((uint8_t *) buffer + 2) = BROADCAST;
  *((uint8_t *) buffer + HEADER_LEN) = handle_len;
  memmove(buffer + HEADER_LEN + 1, handle, handle_len);

  while (len > 0) {
    int msg_len = len > 1000 ? 1000 : len;
    memmove(buffer + HEADER_LEN + 1 + handle_len, message, msg_len);

    if (send(server_socket, buffer, HEADER_LEN + 1 + handle_len + msg_len, 0) < 0) {
      perror("send broadcast");
      exit(-1);
    }

    len -= 1000;
    message += 1000;
  }
}

void sendHandles() {
  Header header;
  header.len = htons(HEADER_LEN);
  header.flag = GET_HANDLES;

  if (send(server_socket, &header, HEADER_LEN, 0) < 0) {
    perror("send sendHandles");
    exit(-1);
  }
}

void sendExit() {
  Header header;
  header.len = htons(HEADER_LEN);
  header.flag = CLIENT_EXIT;

  if (send(server_socket, &header, HEADER_LEN, 0) < 0) {
    perror("send sendExit");
    exit(-1);
  }
}

void processUser() {
  char *line = NULL;
  size_t size;

  if (getline(&line, &size, stdin) < 0) {
    perror("getline");
    exit(-1);
  }
  int len = strlen(line) + 1; //include null char

  if (IS_MESSAGE(line)) {
    sendMessage(len - 3, line + 3);
  } else if (IS_BROADCAST(line)) {
    sendBroadcast(len - 3, line + 3);
  } else if (IS_HANDLES(line)) {
    sendHandles();
  } else if (IS_EXIT(line)) {
    sendExit();
  } else {
    printf("Invalid command\n");
  }

  free(line);
}

void processInput() {
  fd_set fds;

  while (1) {
    if (num_handles == 0) {
      printf("$");
      fflush(stdout);
    }

    FD_ZERO(&fds);
    FD_SET(0, &fds);
    FD_SET(server_socket, &fds);

    if (select(server_socket + 1, &fds, NULL, NULL, NULL) < 0) {
      perror("select call");
      exit(-1);
    }

    if (FD_ISSET(server_socket, &fds)) {
      processServer();
    }
    if (FD_ISSET(0, &fds)) {
      processUser();
    }
  }
}
  

int main(int argc, char * argv[]) {
  checkArgs(argc, argv);

  /* set up the TCP Client socket  */
  server_socket = tcpClientSetup(argv[2], argv[3]);
  initClient();
  processInput();

  return 0;
}

