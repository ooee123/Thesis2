/******************************************************************************
 * tcp_server.cpp
 *
 * Chris Clark
 * CPE 464
 * Program 2
 *****************************************************************************/

// c libraries
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
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// c++ libraries
#include <string>
#include <iostream>
#include <list>

// local libraries
#include "tcp_server.h"
#include "chat_util.h"
#include "testing.h"

using namespace std;

int main(int argc, char **argv) {
  int port_number = read_input(argc, argv);

  int server_socket = get_server_socket(port_number);

  server_listen(server_socket);

   return 0;
}

int read_input(int argc, char **argv) {
  string prog_name(argv[0]);
  if (argc == 1) {
    return 0;
  }

  if (argc > 2) {
    cout << "Usage: " << prog_name << " port-number" << endl;
    exit(-1);
  }

  int port_number = atoi(argv[1]);

  if (port_number <= 0) {
    string port_num_string(argv[1]);
    cout << port_num_string << " is not a valid port number; using operating system default" << endl;
    return 0;
  }

  return port_number;
}

int get_server_socket(int port_number) {
  int server_socket = 0;
  struct sockaddr_in local;      // socket address for local side
  socklen_t len = sizeof(local);  // length of local address

  // create the socket
  server_socket = safe_socket(AF_INET, SOCK_STREAM, 0);

  local.sin_family = AF_INET;         //internet family
  local.sin_addr.s_addr = INADDR_ANY; //wild card machine address
  local.sin_port = htons(port_number);

  // bind the name (address) to a port
  safe_bind(server_socket, (struct sockaddr *) &local, sizeof(local));
    
  //get the port name and print it out
  safe_getsockname(server_socket, (struct sockaddr*)&local, &len);

  // output port number
  printf("Socket has port %d \n", ntohs(local.sin_port));
          
  return server_socket;
}

void server_listen(int server_socket) {
  list<Client> clients;
  list<int> socket_fds; // used to keep track of highest open socket for select() call
  fd_set sockets;
  struct timeval timer;

  // keep track of highest file descriptor for select call
  socket_fds.push_back(server_socket);

  // loop: select on all sockets
  while (1) {
    reset_sockets(socket_fds, &sockets);
    reset_timer(&timer);

    int num_sockets_waiting = safe_select(get_highest_fd(socket_fds) + 1, &sockets, NULL, NULL, &timer);

    // check server socket (for new clients)
    if (FD_ISSET(server_socket, &sockets)) {
      --num_sockets_waiting;
      int new_socket = handle_pending_client(server_socket, clients);
      if (new_socket > 0) {
        socket_fds.push_back(new_socket);
      }
    }

    // check all existing clients (checking newly added one is harmless, but unecessary)
    list<Client>::iterator iter = clients.begin();
    while (num_sockets_waiting > 0 && iter != clients.end()) {
      list<Client>::iterator client = iter;
      ++iter;

      if (FD_ISSET(client->socket, &sockets)) {
        --num_sockets_waiting;
        int amount_read = read_packet(client->socket, &client->state);

        if (amount_read < 0) {
          close(client->socket);
          socket_fds.remove(client->socket);
          iter = clients.erase(client);
        }
        else if (amount_read > 0) {
          if (!process_client_packet(*client, clients)) {
            iter = clients.erase(client);
          }
        }
      }
    }
  }
}

int handle_pending_client(int server_socket, list<Client> &clients) {
  Client potential_client;
  potential_client.accepted = 0;
  potential_client.socket = listen_for_new_client(server_socket);
  init_packet_read_state(&potential_client.state);

  // this will never read the full initial packet on the first call, so don't have to worry about adding the client now
  if (read_packet(potential_client.socket, &potential_client.state) < 0) {
    close(potential_client.socket);
    return 0;
  }

  clients.push_back(potential_client);

  return potential_client.socket;
}

int listen_for_new_client(int server_socket) {
  int new_client_socket;

  safe_listen(server_socket, CLIENT_BACKLOG);

  new_client_socket = safe_accept(server_socket, NULL, NULL);
  
  return new_client_socket;
}

int process_client_packet(Client& client, list<Client> clients) {
  struct message_header hdr = read_header(client.state.data);

  switch (hdr.flag) {
    case FLAG_CLIENT_INIT:
      read_handle(client.handle, client.state.data + sizeof(struct message_header));
      if (!client.accepted && !client_name_exists(client.handle, clients)) {
        send_flag(client.socket, FLAG_SRVR_GOOD_INIT);
        client.accepted = 1;
      }
      else {
        send_flag(client.socket, FLAG_SRVR_BAD_INIT);
          return 0;
      }
      break;
    case FLAG_BROADCAST:
      handle_broadcast(client, clients);
      break;
    case FLAG_CLIENT_MESSAGE:
      handle_message(client, clients);
      break;
    case FLAG_CLIENT_EXIT:
      send_flag(client.socket, FLAG_SRVR_ACK_EXIT);
      break;
    case FLAG_CLIENT_REQ_HANDLES:
      send_num_handles(client.socket, clients.size());
      send_handles(client.socket, clients);
      break;
    default:
      cerr << "Bad flag in client packet: " << hdr.flag << endl;
      exit(-1);
  }

  return 1;
}

void handle_message(Client client, list<Client>& clients) {
  static struct direct_message dm;
  static char msg_buf[sizeof(struct message_header) + 1 + MAX_HANDLE_SIZE];

  parse_direct_message(&dm, client.state.data);

  int dest_socket = get_client_socket_by_handle(clients, dm.dest_handle);

  if (dest_socket < 0) {
    char *ptr = msg_buf;
    int msg_len = sizeof(struct message_header) + 1 + strlen(dm.dest_handle);
    ptr += write_header(ptr, msg_len, FLAG_SRVR_BAD_MESSAGE);
    ptr += write_handle(ptr, dm.dest_handle);
    safe_send(client.socket, msg_buf, msg_len, 0);

    return;
  }

  safe_send(dest_socket, client.state.data, ntohs(read_header(client.state.data).msg_len), 0);
}

void handle_broadcast(Client client, list<Client>& clients) {
  static struct broadcast_message bm;
  static struct direct_message dm;
  parse_broadcast_message(&bm, client.state.data);
  strcpy(dm.message, bm.message);
  strcpy(dm.src_handle, bm.src_handle);

  list<Client>::iterator dest_client = clients.begin();
  while (dest_client != clients.end()) {
    if (strcmp(dest_client->handle, client.handle) != 0) {
      strcpy(dm.dest_handle, dest_client->handle);
      send_direct_message(dest_client->socket, dm);
    }
    ++dest_client;
  }
}

void send_num_handles(int socket, uint32_t num_handles) {
  char data[sizeof(struct message_header) + sizeof(uint32_t)];
  int msg_len = sizeof(struct message_header) + sizeof(uint32_t);
  write_header(data, msg_len, FLAG_SRVR_ACK_HANDLES);
  *((uint32_t *)(data + sizeof(struct message_header))) = htons(num_handles);
  safe_send(socket, data, msg_len, 0);
}

void send_handles(int socket, list<Client> clients) {
  // send header with flag
  struct message_header hdr;
  hdr.msg_len = htons(0);
  hdr.flag = FLAG_SRVR_SEND_HANDLES;
  safe_send(socket, &hdr, sizeof(struct message_header), 0);

  // send handle for each client
  list<Client>::iterator client = clients.begin();
  while (client != clients.end()) {
    send_handle(socket, client->handle);
    ++client;
  }
}

void send_handle(int socket, char *handle) {
  static uint8_t data[MAX_HANDLE_SIZE + 1];

  int handle_len = strlen(handle);
  data[0] = (uint8_t)handle_len;
  memcpy(data + 1, handle, handle_len + 1);

  safe_send(socket, data, handle_len + 1, 0);
}

int client_name_exists(char *handle, list<Client>& clients) {
  list<Client>::iterator client = clients.begin();
  while (client != clients.end()) {
    if (client->accepted && strcmp(handle, client->handle) == 0) {
      return 1;
    }
    ++client;
  }

  return 0;
}

int get_client_socket_by_handle(list<Client>& clients, char *handle) {
  list<Client>::iterator client = clients.begin();
  while (client != clients.end()) {
    if (client->accepted && strcmp(client->handle, handle) == 0) {
      return client->socket;
    }
    ++client;
  }

  return -1;
}