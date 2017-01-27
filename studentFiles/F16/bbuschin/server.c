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
#include "common.h"

#define BACKLOG 5
#define MAXBUF 1024

typedef struct Client {
  int socket;
  char *handle;
  struct Client *next;
} Client;

/* Global vars */
static Client *client_list = NULL;
static fd_set fds;
static int num_fds;
static int server_socket;

/* This function sets the server socket.  It lets the system
determine the port number.  The function returns the server
socket number and prints the port number to the screen.  */

int tcpServerSetup(uint16_t port) {
  int server_socket= 0;
  struct sockaddr_in local;      /* socket address for local side  */
  socklen_t len= sizeof(local);  /* length of local address        */

  /* create the socket  */
  server_socket= socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket < 0) {
    perror("socket call");
    exit(1);
  }

  local.sin_family= AF_INET;         //internet family
  local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
  local.sin_port= htons(port);       //let system choose the port

  /* bind the name (address) to a port */
  if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
    perror("bind call");
    exit(-1);
  }
  
  //get the port name and print it out
  if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
    perror("getsockname call");
    exit(-1);
  }

  if (listen(server_socket, BACKLOG) < 0) {
    perror("listen call");
    exit(-1);
  }
  
  printf("Server is using port %d \n", ntohs(local.sin_port));
  
  return server_socket;
}

/* This function waits for a client to ask for services.  It returns
the socket number to service the client on.    */
int tcpAccept(int server_socket) {
  int client_socket= 0;

  if ((client_socket = accept(server_socket, NULL, NULL)) < 0) {
    perror("accept call");
    exit(-1);
  }

  Client *client = malloc(sizeof(client));
  client->socket = client_socket;

  if (client_list == NULL) {
    client_list = client;
  } else {
    Client *cur = client_list;
    while (cur->next != NULL) {
      cur = cur->next;
    }
    cur ->next = client;
  }

  return(client_socket);
}

void removeClient(int socket) {
  if (client_list == NULL) return;
  Client *tmp;

  if (client_list->socket == socket) {
    tmp = client_list;
    client_list = client_list->next;
  } else {
    Client *cur = client_list;
    while (cur->next != NULL && cur->next->socket != socket) {
      cur = cur->next;
    }

    if (cur->next != NULL) {
      tmp = cur->next;
      cur->next = cur->next->next;
    }
  }

  free(tmp->handle);
  free(tmp);
  FD_CLR(socket, &fds);
  close(socket);
}

int isHandleUsed(int socket, char *handle) {
  Client *cur = client_list;

  while (cur != NULL) {
    if (cur->socket != socket && strcmp(cur->handle, handle) == 0) {
      return 1;
    }
    cur = cur->next;
  }
  
  return 0;
}

void addClient(int socket, int len) {
  uint8_t buffer[len];

  Client *cur = client_list;
  while (cur->socket != socket) {
    cur = cur->next;
  }

  if (recv(socket, buffer, len, 0) < 0) {
    perror("recv addClient");
    exit(-1);
  }

  uint8_t handle_len = *buffer;
  char *handle = calloc(1, handle_len + 1);
  memmove(handle, buffer + 1, handle_len);

  Header header;
  header.len = htons(HEADER_LEN);

  if (isHandleUsed(socket, handle))  {
    header.flag = DENY_HANDLE;
  } else {
    header.flag = CONF_HANDLE;
    cur->handle = handle;
  }

  if (send(socket, &header, HEADER_LEN, 0) < 0) {
    perror("send addClient");
    exit(-1);
  }

  if (header.flag == DENY_HANDLE) {
    removeClient(socket);
  }
}
  
void broadcast(int socket, int len) {
  char buffer[HEADER_LEN + len];
  *((uint16_t *) buffer) = htons(HEADER_LEN + len);
  *((uint8_t *) buffer + 2) = BROADCAST;

  if (recv(socket, buffer + HEADER_LEN, len, 0) < 0) {
    perror("recv broadcast");
    exit(-1);
  }

  Client *cur = client_list;
  while (cur != NULL) {
    if (cur->socket != socket) {
      if (send(cur->socket, buffer, HEADER_LEN + len, 0) < 0) {
        perror("send broadcast");
        exit(-1);
      }
    }
    cur = cur->next;
  }
}

void message(int socket, int len) {
  char buffer[HEADER_LEN + len];
  *((uint16_t *) buffer) = htons(HEADER_LEN + len);
  *((uint8_t *) buffer + 2) = MESSAGE;

  if (recv(socket, buffer + HEADER_LEN, len, 0) < 0) {
    perror("recv message");
    exit(-1);
  }

  uint8_t handle_len = *((uint8_t *) buffer + HEADER_LEN);
  char dest_handle[handle_len + 1];
  memmove(dest_handle, buffer + HEADER_LEN + 1, handle_len);
  dest_handle[handle_len] = '\0';

  Client *cur = client_list;
  while (cur != NULL) {
    if (strcmp(cur->handle, dest_handle) == 0) {
      if (send(cur->socket, buffer, HEADER_LEN + len, 0) < 0) {
        perror("send message");
        exit(-1);
      }
      return;
    }
    cur = cur->next;
  }

  char buffer2[HEADER_LEN + handle_len + 1];
  *((uint16_t *) buffer2) = htons(HEADER_LEN + handle_len + 1);
  *((uint8_t *) buffer2 + 2) = HANDLE_DNE;
  memmove(buffer2 + HEADER_LEN, buffer + HEADER_LEN, handle_len + 1);

  if (send(socket, buffer2, HEADER_LEN + handle_len + 1, 0) < 0) {
    perror("send handle does not exist");
    exit(-1);
  }
}

void exitClient(int socket) {
  Header header;
  header.len = htons(HEADER_LEN);
  header.flag = ACK_EXIT;

  if (send(socket, &header, HEADER_LEN, 0) < 0) {
    perror("send exitClient");
    exit(-1);
  }

  removeClient(socket);
}

void handles(int socket) {
  uint32_t num_handles = 0;
  Client *cur = client_list;
  while (cur != NULL) {
    cur = cur->next;
    num_handles++;
  }

  num_handles = htonl(num_handles);
  char buffer[HEADER_LEN + sizeof(uint32_t)];
  *((uint16_t *) buffer) = htons(HEADER_LEN + sizeof(uint32_t));
  *((uint8_t *) buffer + 2) = NUM_HANDLES;
  memmove(buffer + HEADER_LEN, &num_handles, sizeof(uint32_t));
  if (send(socket, buffer, HEADER_LEN + sizeof(uint32_t), 0) < 0) {
    perror("send ackHandles");
    exit(-1);
  }

  char buffer2[HEADER_LEN + 1 + 255];
  cur = client_list;
  while (cur != NULL) {
    int handle_len = strlen(cur->handle);
    *((uint16_t *) buffer2) = htons(HEADER_LEN + 1 + handle_len);
    *((uint8_t *) buffer2 + 2) = SINGLE_HANDLE;
    *((uint8_t *) buffer2 + HEADER_LEN) = handle_len;
    memmove(buffer2 + HEADER_LEN + 1, cur->handle, handle_len);

    if (send(socket, buffer2, HEADER_LEN + 1 + handle_len, 0) < 0) {
      perror("send singleHandle");
      exit(-1);
    }
    
    cur = cur->next;
  }
}

void processMessage(int socket) {
  int bytesRecv;
  Header header;

  if ((bytesRecv = recv(socket, &header, HEADER_LEN, 0)) < 0) {
    perror("recv call");
    exit(-1);
  }

  if (bytesRecv == 0) {
    removeClient(socket);
  } else if (header.flag == INIT_CLIENT) {
    addClient(socket, ntohs(header.len) - HEADER_LEN);
  } else if (header.flag == BROADCAST) {
    broadcast(socket, ntohs(header.len) - HEADER_LEN);
  } else if (header.flag == MESSAGE) {
    message(socket, header.len - HEADER_LEN);
  } else if (header.flag == CLIENT_EXIT) {
    exitClient(socket);
  } else if (header.flag == GET_HANDLES) {
    handles(socket);
  }
}

void processInput() {
  int i;
  fd_set temp_fds;

  memmove(&temp_fds, &fds, sizeof(fd_set));

  if (select(num_fds + 1, &temp_fds, NULL, NULL, NULL) < 0) {
    perror("select call");
    exit(-1);
  }

  for (i = server_socket; i <= num_fds; ++i) {
    if (FD_ISSET(i, &temp_fds)) {
      if (i == server_socket) {
        int new_client = tcpAccept(server_socket);
        FD_SET(new_client, &fds);
        num_fds++;
      } else {
        processMessage(i);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 1 && argc != 2) perror("Incorrect number of args.\n");
  server_socket = tcpServerSetup(argc == 2 ? atoi(argv[1]) : 0);
  num_fds = server_socket;

  FD_ZERO(&fds);
  FD_SET(server_socket, &fds);

  while(1) {
    processInput();
  }
  
  close(server_socket);
  return 0;
}
