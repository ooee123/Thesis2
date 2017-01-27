/******************************************************************************
 * tcp_client.cpp
 *
 * Chris Clark
 * CPE 464
 * Program 2
 *****************************************************************************/

// c libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

// c++ libraries
#include <string>
#include <iostream>
#include <sstream>
#include <locale>

// local libraries
#include "tcp_client.h"
#include "chat_util.h"
#include "testing.h"

using namespace std;

int main(int argc, char **argv) {
  struct input input = read_input(argc, argv);

  int socket = setup_server_socket(input.server_name, input.server_port);

  send_init_message(socket, input.handle);

  handle_init_response(socket, input.handle);

  write_input_prompt();

  client_listen(socket, input.handle);

  return 0;
}

struct input read_input(int argc, char **argv) {
  string prog_name(argv[0]);
  struct input input;

  if (argc != 4) {
    cerr << "Usage: " << prog_name << " handle server-name server-port" << endl;
    exit(-1);
  }

  strcpy(input.handle, argv[1]);
  input.server_name = argv[2];
  input.server_port = atoi(argv[3]);

  if (strlen(input.handle) > MAX_HANDLE_SIZE) {
    cerr << "Handle length cannot exceed " << MAX_HANDLE_SIZE << " characters" << endl;
    exit(-1);
  }

  if (input.server_port <= 0) {
    cerr << "Invalid port number" << endl;
    exit(-1);
  }

  return input;
}

int setup_server_socket(char *host_name, int port) {
  int socket_num;
  struct sockaddr_in remote;     // socket address for remote side
  struct hostent *hp;        // address of remote host

  // create the socket
  socket_num = safe_socket(AF_INET, SOCK_STREAM, 0);

  // designate the addressing family
  remote.sin_family= AF_INET;

  // get the address of the remote host and store
  hp = safe_gethostbyname(host_name);
  
  memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

  // get the port used on the remote side and store
  remote.sin_port= htons(port);

  safe_connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in));

  return socket_num;
}

void send_init_message(int socket, char *handle) {
  static char buffer[sizeof(struct message_header) + 1 + MAX_HANDLE_SIZE];
  int msg_len = sizeof(struct message_header) + 1 + strlen(handle);

  write_header(buffer, msg_len, FLAG_CLIENT_INIT);
  write_handle(buffer + sizeof(struct message_header), handle);

  safe_send(socket, buffer, msg_len, 0);
}

// Could do this in the main loop, but decided to get this response separately
// since it should occur before all other server messages.
void handle_init_response(int socket, char *handle) {
  struct packet_read_state server_state;
  init_packet_read_state(&server_state);

  fd_set sockets;
  list<int> socket_fds;
  socket_fds.push_back(socket);
  struct timeval timer;

  int data_read = 0;

  do {
    reset_timer(&timer);
    reset_sockets(socket_fds, &sockets);

    safe_select(socket + 1, &sockets, NULL, NULL, &timer);

    // unecessary since select() is only watching one socket, but just in case
    if (FD_ISSET(socket, &sockets)) {
      data_read = read_packet(socket, &server_state);
    }

  } while (data_read == 0);

  if (data_read < 0) {
    cerr << "Server Terminated" << endl;
    exit(-1);
  }

  int flag = ((struct message_header *)server_state.data)->flag;

  if (flag == FLAG_SRVR_BAD_INIT) {
    printf("Handle already in use: %s\n", handle);
    exit(-1);
  }
  if (flag != FLAG_SRVR_GOOD_INIT) {
    cerr << "Don't trust the server! The machines have taken over!" << endl;
    exit(-1);
  }
}

void client_listen(int socket, char *handle) {
  struct packet_read_state server_state;
  uint32_t num_handles = 0;

  static struct direct_message dm;

  fd_set sockets;
  struct timeval timer;
  list<int> socket_fds;

  int client_want_exit = 0;
  int server_want_exit = 0;
  
  init_packet_read_state(&server_state);

  socket_fds.push_back(socket);
  socket_fds.push_back(STDIN_FILENO);

  while (!client_want_exit || !server_want_exit) {
    reset_sockets(socket_fds, &sockets);
    reset_timer(&timer);

    safe_select(get_highest_fd(socket_fds) + 1, &sockets, NULL, NULL, &timer);

    // message from server
    if (FD_ISSET(socket, &sockets)) {
      int data_read = read_packet(socket, &server_state);

      if (data_read < 0) {
        cerr << "Server Terminated" << endl;
        exit(-1);
      }

      if (data_read > 0) {
        int flag = read_header(server_state.data).flag;
        switch(flag) {
          case FLAG_CLIENT_MESSAGE:
            parse_direct_message(&dm, server_state.data);
            output_direct_message(&dm);
            break;
          case FLAG_SRVR_BAD_MESSAGE:
            output_bad_message(server_state.data);
            break;
          case FLAG_SRVR_ACK_EXIT:
            server_want_exit = 1;
          case FLAG_SRVR_ACK_HANDLES:
            num_handles = parse_num_handles(server_state.data);
            break;
          case FLAG_SRVR_SEND_HANDLES:
            process_handles(socket, num_handles);
            break;
          default:
            cerr << "Bad server flag, I'm scared, shutting down" << endl;
            exit(-1);
        }
      }
    }

    // user input
    if (FD_ISSET(STDIN_FILENO, &sockets)) {
      client_want_exit = client_want_exit ? client_want_exit : handle_user_input(socket, handle);
      write_input_prompt();
    }
  }

  cout << endl;
  exit(0);
}

int handle_user_input(int socket, char *handle) {
  locale locale;
  string line;
  string message;
  string command;
  string dest_handle;

  // get the entire line from cin here so no blocking occurs later
  getline(cin, line);

  // use stringstream for input parsing
  stringstream line_stream(line);

  line_stream >> command;

  if (command.length() != 2  || command.at(0) != '%') {
    cout << "Please enter command in the form: %c" << endl;
    return 0;
  }

  switch (tolower(command.at(1), locale)) {
    case 'm':
      line_stream >> dest_handle;
      if (dest_handle.length() > MAX_HANDLE_SIZE) {
        cout << "Invalid destination handle; must be fewer than " << MAX_HANDLE_SIZE << " characters" << endl;
      }
      getline(line_stream >> ws, message);
      send_user_message(socket, handle, dest_handle, message);
      break;
    case 'b':
      getline(line_stream >> ws, message);
      broadcast_message(socket, handle, message);
      break;
    case 'l':
      send_flag(socket, FLAG_CLIENT_REQ_HANDLES);
      break;
    case 'e':
      send_flag(socket, FLAG_CLIENT_EXIT);
      return 1;
    default:
      cout << "Invalid command" << endl;
  }

  return 0;
}

void send_user_message(int socket, char *handle, string dest_handle, string message) {
  static struct direct_message dm;
  string rest_message;

  if (message.length() >= MAX_MESSAGE_SIZE) {
    // only send MAX_MESSAGE_SIZE - 1 characters at a time (-1 to account for null terminating character)
    rest_message = message.substr(MAX_MESSAGE_SIZE - 1);
    message = message.substr(0, MAX_MESSAGE_SIZE - 1);
  }

  strcpy(dm.src_handle, handle);

  dest_handle.copy(dm.dest_handle, dest_handle.length(), 0);
  dm.dest_handle[dest_handle.length()] = '\0';

  message.copy(dm.message, message.length(), 0);
  dm.message[message.length()] = '\0';

  send_direct_message(socket, dm);

  if (rest_message.length() > 0) {
    send_user_message(socket, handle, dest_handle, rest_message);
  }
}

void broadcast_message(int socket, char *handle, string message) {
  static struct broadcast_message bm;
  string rest_message;

  if (message.length() >= MAX_MESSAGE_SIZE) {
    // only send MAX_MESSAGE_SIZE - 1 characters at a time (-1 to account for null terminating character)
    rest_message = message.substr(MAX_MESSAGE_SIZE - 1);
    message = message.substr(0, MAX_MESSAGE_SIZE - 1);
  }

  strcpy(bm.src_handle, handle);

  message.copy(bm.message, message.length(), 0);
  bm.message[message.length()] = '\0';

  send_broadcast_message(socket, bm);

  if (rest_message.length() > 0) {
    broadcast_message(socket, handle, rest_message);
  }
}

uint32_t parse_num_handles(char *data) {
  return ntohs(*((uint32_t *)(data + sizeof(struct message_header))));
}

void process_handles(int socket, uint32_t num_handles) {
  fd_set sockets;
  struct timeval timer;
  struct handle_state state;
  static char cur_handle[MAX_HANDLE_SIZE + 1];

  state.state = STATE_HANDLE_START;

  cout << "Number of handles: " << num_handles << endl;

  while (num_handles > 0) {
    FD_ZERO(&sockets);
    FD_SET(socket, &sockets);
    reset_timer(&timer);

    safe_select(socket + 1, &sockets, NULL, NULL, &timer);

    // again, unecessary to check socket, but just to be safe
    if (FD_ISSET(socket, &sockets)) {
      int data_read = read_handle_state(socket, &state);

      if (data_read < 0) {
        cerr << "Server Terminated" << endl;
        exit(-1);
      }

      if (data_read > 0) {
        memcpy(cur_handle, state.data + 1, data_read - 1);
        cur_handle[data_read - 1] = '\0';
        output_handle(cur_handle);
        --num_handles;
      }
    }
  }
  write_input_prompt();
}

int read_handle_state(int socket, struct handle_state *state) {
  int rec_len;
  int data_wanted;

  switch (state->state) {
    case STATE_HANDLE_START:
      state->len_read = 0;

      rec_len = safe_recv(socket, state->data, 1, 0);

      if (rec_len == 0) {
        return -1;
      }

      state->len_read += rec_len;
      state->msg_len = state->data[0] + 1;
      state->state = STATE_HANDLE_GET_DATA;

      return 0;
    case STATE_HANDLE_GET_DATA:
      data_wanted = state->msg_len - state->len_read;
      rec_len = safe_recv(socket, state->data + state->len_read, data_wanted, 0);

      if (rec_len == 0) {
        return -1;
      }

      state->len_read += rec_len;

      if (state->len_read == state->msg_len) {
        state->state = STATE_HANDLE_START;
        return state->len_read;
      }

      return 0;
    default:
      cerr << "Bad state on getting handles (try initializing state before use?)" << endl;
      exit(-1);
  }
}

void output_handle(char *handle) {
  printf("%s\n", handle);
}

void output_direct_message(struct direct_message *dm) {
  printf("\n%s: %s\n", dm->src_handle, dm->message);
  write_input_prompt();
}

void output_bad_message(char *data) {
  static char dest_handle[MAX_HANDLE_SIZE + 1];

  read_handle(dest_handle, data + sizeof(struct message_header));

  printf("Client with handle %s does not exist\n", dest_handle);

  write_input_prompt();
}

void write_input_prompt() {
  cout << "$: " << flush;
}