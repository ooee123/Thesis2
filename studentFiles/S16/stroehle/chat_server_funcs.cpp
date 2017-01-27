
#include "chat_server_funcs.h"

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
#include <vector>
#include "testing.h"

std::vector<client_info> client_data;
char *buffer;
int server_socket;
fd_set used_skt_fds;
/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

char src_handle_buffer[255];
char dst_handle_buffer[255];

int tcp_recv_setup(int specified_port) {
    sockaddr_in local;      /* socket address for local side  */
    socklen_t len= sizeof(local);  /* length of local address        */

    server_socket= socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0) {
      perror("socket call");
      exit(1);
    }

    local.sin_family= AF_INET;         //internet family
    local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
    local.sin_port= htons(specified_port); 

    if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
      exit(-1);
    }
    
    if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
      perror("getsockname call");
      exit(-1);
    }

    printf("socket has port %d \n", ntohs(local.sin_port));
           
    return server_socket;
}

// Returns the index of the handle if found
// if not returns -1

int does_handle_exist(char *handle) {

  for(int i = 0; i < (int) client_data.size(); i++) {
    if(strcmp(client_data.at(i).handle, handle) == 0) {
      return i;
    }
  }

  return -1;
}

// Returns the port number of the given handle if it 
// exists, if it does not then it returns -1

int get_handle_port(char *handle) {

  for(int i = 0; i < (int) client_data.size(); i++) {
    if(strcmp(client_data.at(i).handle, handle) == 0) {
      return client_data.at(i).port_number;
    }
  }

  return -1;
}

void send_bad_handle_packet(int c_socket) {

  uint16_t net_ord_pl = htons(3);
  memcpy(buffer, &net_ord_pl, 2);
  buffer[2] = 3;

  send_packet(c_socket, buffer, 3);
}
void send_good_handle_packet(int c_socket) {

  uint16_t net_ord_pl = htons(3);
  memcpy(buffer, &net_ord_pl, 2);
  buffer[2] = 2;

  send_packet(c_socket, buffer, 3);
}

void create_client_info(uint8_t handle_len, int c_socket, char *handle_strt) {

  client_info out;

  out.port_number = c_socket;
  out.handle = (char *) malloc(handle_len + 1);
  memcpy(out.handle, handle_strt, handle_len);
  out.handle[handle_len] = 0;

  if(does_handle_exist(out.handle) >= 0) {
    send_bad_handle_packet(c_socket);
  }
  else {
    send_good_handle_packet(c_socket);
    client_data.push_back(out);
  }

}

void read_handle_packet(int c_socket) {

  uint16_t pkt_len;
  int message_len;

  // read in two bytes
  if ((message_len = recv(c_socket, &pkt_len, 2, 0)) < 0) {
    perror("read_handle_packet: recv call");
    exit(-1);
  }
  int rest_of_pkt_len = ntohs(pkt_len) - 2;

  if ((message_len = recv(c_socket, buffer, rest_of_pkt_len, 0)) < 0) {
    perror("read_handle_packet: recv call");
    exit(-1);
  }

  create_client_info((uint8_t) buffer[1], c_socket, &(buffer[2]));
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

void setup_new_client( int *max_fd) {

  int new_client_fd;
  if ((new_client_fd= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
    perror("setup_new_client: accept call");
    exit(-1);
  }
  FD_SET(new_client_fd, &used_skt_fds);
  if (new_client_fd > *max_fd) {  
      *max_fd = new_client_fd;
  }

  read_handle_packet(new_client_fd);

}

void send_broadcast_packet(char *data, char *sender_handle, int pkt_len) {

  for(int i = 0; i < (int) client_data.size(); i++) {
    if(strcmp(sender_handle, client_data.at(i).handle) != 0)
      send_packet(client_data.at(i).port_number, data, pkt_len);
  }
}

void process_broadcast_packet(char *data, int pkt_len) {

  char *sender_handle;
  uint8_t handle_len = data[3];
  sender_handle = src_handle_buffer;

  memcpy(sender_handle, data + 4, handle_len);
  sender_handle[handle_len] = 0;

  send_broadcast_packet(data, sender_handle, pkt_len);  
}

void send_bad_dest_handle_packet(int c_socket, char *bad_handle) {

  uint8_t bad_handle_len = strlen(bad_handle);
  uint16_t net_ord_pl = htons(3 + bad_handle_len);
  memcpy(buffer, &net_ord_pl, 2);
  buffer[2] = 7;
  buffer[3] = bad_handle_len;
  memcpy(buffer+4, bad_handle, bad_handle_len);


  send_packet(c_socket, buffer, 3);
}

void process_msg_packet(char *data, int pkt_len) {

  uint8_t dst_handle_len = data[3];
  uint8_t src_handle_len = data[3+dst_handle_len+1];

  memcpy(dst_handle_buffer, data +4, dst_handle_len);
  memcpy(src_handle_buffer, data + 4 + dst_handle_len + 1, src_handle_len);

  src_handle_buffer[src_handle_len] = 0;
  dst_handle_buffer[dst_handle_len] = 0;

  int dst_port = get_handle_port(dst_handle_buffer);

  if(dst_port == -1) {
    send_bad_dest_handle_packet(get_handle_port(src_handle_buffer), dst_handle_buffer);
  }
  else {
    send_packet(dst_port, data, pkt_len);
  }

}

void send_handle_packets(int c_socket) {

  int pkt_len = 0;

  memcpy(buffer, &pkt_len, 2);
  buffer[2] = ALL_HANDLES;

  send_packet(c_socket, buffer, 3);

  for(int i = 0; i < (int)client_data.size(); i++) {
    int cur_str_len =  strlen(client_data.at(i).handle);
    buffer[0] = cur_str_len;
    memcpy(&(buffer[1]), client_data.at(i).handle, cur_str_len);
    send_packet(c_socket, buffer, cur_str_len+1);
  }
}

void send_num_handles_packet(int c_socket) {
  uint16_t pkt_len = 2 + 1 + 4;
  uint16_t no_pkt_len = htons(pkt_len);
  uint32_t num_handles = htonl(client_data.size());

  memcpy(buffer, &no_pkt_len, 2);
  buffer[2] = NUM_HANDLES;
  memcpy(buffer+3, &num_handles, 4);

  send_packet(c_socket, buffer, pkt_len);
  send_handle_packets(c_socket);
}

void send_exit_ack_packet(int c_socket) {
  uint16_t net_ord_pl = htons(3);
  memcpy(buffer, &net_ord_pl, 2);
  buffer[2] = EXIT_ACK;

  send_packet(c_socket, buffer, 3);

}

void read_packet_data(int c_socket, char *data, int pkt_len) {
  // Call appropriate processing function
  switch(data[2]) {
    case BROADCAST:      
      process_broadcast_packet(data, pkt_len);
      break;
    case PRIVATE_MESSAGE:
      process_msg_packet(data, pkt_len);
      break;
    case CLIENT_EXITING:
      send_exit_ack_packet(c_socket);
      break;
    case PRINT_HANDLES:
      send_num_handles_packet(c_socket);
      break;
  }
}

void clean_client(int client_socket) {

  close(client_socket);

  FD_CLR(client_socket, &used_skt_fds);
  for(int i = 0; i < (int) client_data.size(); i++) {
    if(client_socket == client_data.at(i).port_number) {
      free(client_data.at(i).handle);
      client_data.erase(client_data.begin() + i);
    }
  }
}

void read_data_from_client(int fd_to_read) {

  int message_len;
  uint16_t pkt_len;
  if ((message_len = recv(fd_to_read, buffer, 2, 0)) == 0) {
    clean_client(fd_to_read);
  }
  else if (message_len < 0) {
    perror("read_data_from_client: recv call");
    exit(-1);
  }
  else {
    memcpy(&pkt_len, buffer, 2);
    pkt_len = ntohs(pkt_len);

    if ((message_len = recv(fd_to_read, buffer + 2, pkt_len - 2, 0)) < 0) {
      perror("read_handle_packet: recv call");
      exit(-1);
    }
    read_packet_data(fd_to_read, buffer, pkt_len);
  } 
}

void server_select() {
  
  buffer = (char *) malloc(1100);
  fd_set tmp_set;
  int max_fd = server_socket;
  // Clear the fd lists
  FD_ZERO(&used_skt_fds); 
  FD_ZERO(&tmp_set);
  
  // Add the starting connection to the list of used fds
  FD_SET(server_socket, &used_skt_fds);

  while(1) {
    tmp_set = used_skt_fds; // copy it
    if (select(max_fd+1, &tmp_set, NULL, NULL, NULL) == -1) {
        perror("select");
        exit(0);
    }
    for(int i = 0; i <= max_fd; i++) {
      if(FD_ISSET(i, &tmp_set)) {
        // Check if there is a new client
        if(i == server_socket) {
          setup_new_client(&max_fd);
        }
        else {
          read_data_from_client(i);
        }
      }
    }
  }
}

void send_packet(int socket, char *packet, int length) {
    
    int sent = send(socket, packet, length, 0);

    if(sent < 0) {
        perror("send call");
        exit(-1);
    }
}