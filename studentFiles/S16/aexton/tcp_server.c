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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "testing.h"
#include "networks.h"
#include "client_linked_list.h"
#include "helpers.h"
#include "packet.h"

void start_server();

int tcp_recv_setup();
int tcp_listen(int server_socket, int back_log);
int receive_data_from_client(int client_socket, uint8_t *buffer, int buffer_size);

int parse_recieved_packet(int socket_num, uint8_t *packet);
int parse_client_initial_packet(int socket_num, uint8_t *packet);
void parse_broadcast_packet(uint8_t *packet);
void parse_message_packet(uint8_t *packet);
void parse_client_exiting_packet(uint8_t *packet);
void parse_handle_list_request_packet(uint8_t *packet);

void send_handle_authorization_packet(uint8_t *client_handle);
void send_handle_declined_packet(int socket_num, uint8_t *declined_handle);
void forward_message_packet(uint8_t *packet, uint8_t *sender_handle, uint8_t *receiver_handle);
void broadcast_packet(uint8_t *packet);
void send_ACK_client_exit_packet(int socket_num);
void send_receiver_DNE_packet(int socket_num, uint8_t *receiver_handle);
void send_active_handle_count_packet(int socket_num);
void send_active_handles_list_packet(int socket_num);


int main(int argc, char *argv[]) {
  start_server();
  return 0;
}

void start_server() {

  int server_socket = 0;   //socket descriptor for the server socket
  int client_socket = 0;   //socket descriptor for the client socket

  uint8_t *buffer;          //buffer for receiving from client
  int buffer_size = 1024;  //packet size variable

  printf("sockaddr: %lu sockaddr_in %lu\n", sizeof(struct sockaddr), sizeof(struct sockaddr_in));

  //create packet buffer
  buffer = (uint8_t *) malloc(buffer_size);

  //create the server socket
  server_socket = tcp_recv_setup();

  if (listen(server_socket, 5) < 0) {
      perror("listen call");
      exit(-1);
  }

  fd_set active_fd_set, read_fd_set;
  int i;
  struct sockaddr_in client;
  socklen_t size;

  /* Initialize the set of active sockets. */
  FD_ZERO (&active_fd_set);
  FD_SET (server_socket, &active_fd_set);

  while (1) {

    read_fd_set = active_fd_set;

    if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
        perror("SELECT FAILURE");
        exit(EXIT_FAILURE);
    }

    // Receive Data From Sockets With Input
    for (i = 0; i < FD_SETSIZE; ++i) {

      if (FD_ISSET(i, &read_fd_set)) {

          if (i == server_socket) {

              /* Connection request on original socket. */
              int new_client;
              size = sizeof(client);
              new_client = accept(server_socket, (struct sockaddr *) &client, &size);

              if (new_client < 0) {
                  perror("ACCEPT FAILURE");
                  exit(EXIT_FAILURE);
              }

              printf("Server: connection from host %s, using port %u.\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

              FD_SET (new_client, &active_fd_set);

          } else {

            if (receive_data_from_client(i, buffer, buffer_size) == -1) {
              struct client_type *exited_client = search_clients_by_socket_num(i);
              //printf("CLIENT CLOSED CONNECTION: %s\n", exited_client->handle);
              if (exited_client) {
                remove_client(exited_client->handle);
              }
              close (i);
              FD_CLR (i, &active_fd_set);
            }

          }
      }

    }

  }

  /* close the sockets */
  close(server_socket);
  close(client_socket);

}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup() {

    int server_socket= 0;
    struct sockaddr_in local;      /* socket address for local side  */
    socklen_t len = sizeof(local);  /* length of local address        */

    /* create the socket  */
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
      perror("socket call");
      exit(1);
    }

    local.sin_family = AF_INET;         //internet family
    local.sin_addr.s_addr = INADDR_ANY; //wild card machine address
    local.sin_port = htons(0);          //let system choose the port

    /* bind the name (address) to a port */
    if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
    	perror("bind call");
    	exit(-1);
    }

    //get the port name and print it out
    if (getsockname(server_socket, (struct sockaddr*) &local, &len) < 0) {
    	perror("getsockname call");
    	exit(-1);
    }

    printf("socket has port %d \n", ntohs(local.sin_port));

    return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

int tcp_listen(int server_socket, int back_log) {
  int client_socket = 0;

  if (listen(server_socket, back_log) < 0) {
      perror("listen call");
      exit(-1);
  }

  if ((client_socket = accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
      perror("accept call");
      exit(-1);
  }

  return client_socket;
}

int receive_data_from_client(int client_socket, uint8_t *buffer, int buffer_size) {
  int connection_closed = 0;
  int received = recv(client_socket, buffer, buffer_size, 0);

  //now get the data on the client_socket
  if (received < 0) {
    perror("RECV FROM CLIENT");
    exit(-1);
  } else if (received == 0) {
    connection_closed = -1;
  } else {
    // printf("PACKET BYTES (%d): ", received);
    // print_bytes((uint8_t *) buffer, received);
    connection_closed = parse_recieved_packet(client_socket, buffer);
  }

  return connection_closed;
}

int parse_recieved_packet(int socket_num, uint8_t *packet) {

  uint8_t packet_flag = *(packet + 2);
  //printf("FLAG BYTE: %u\n", packet_flag);
  int client_exiting = 0;

  switch (packet_flag) {

    case 1:     // Client Initial Packet
      client_exiting = parse_client_initial_packet(socket_num, packet);
      break;

    case 4:    // Broadcast Packet
      parse_broadcast_packet(packet);
      break;

    case 5:    // Message Packet
      parse_message_packet(packet);
      break;

    case 8:    // Client Exiting Packet
      parse_client_exiting_packet(packet);
      client_exiting = -1;
      break;

    case 10:    // Client Requesting List of Handles

      parse_handle_list_request_packet(packet);

      break;

    default:

      break;

  }


  return client_exiting;
}

int parse_client_initial_packet(int socket_num, uint8_t *packet) {

  uint8_t handle_length = *(packet + 3);
  int client_exiting = 0;

  uint8_t *requesting_handle = (uint8_t *) calloc(handle_length + 1, sizeof(uint8_t));
  memcpy(requesting_handle, packet + 4, handle_length);

  //printf("CLIENT: %s, Requesting Handle Authorization\n", requesting_handle);

  client * found_client = search_clients((char *)requesting_handle);

  if (found_client == NULL) {

    // Handle does not exist, Assign Client Handle
    add_client((char *)requesting_handle, socket_num);

    send_handle_authorization_packet(requesting_handle);
    //printf("CLIENT: %s, Handle Authorized\n", requesting_handle);

    print_clients();


  } else {

    // Handle already exists, Report back to Client
    send_handle_declined_packet(socket_num, requesting_handle);
    //printf("CLIENT: %s, Handle Declined\n", requesting_handle);
    client_exiting = -1;
    //print_clients();

  }

  return client_exiting;
}

void parse_client_exiting_packet(uint8_t *packet) {

  uint8_t handle_length = *(packet + 3);

  uint8_t *requesting_handle = (uint8_t *) calloc(handle_length + 1, sizeof(uint8_t));
  memcpy(requesting_handle, packet + 4, handle_length);

  //printf("CLIENT: %s, Exiting Chat Room\n", requesting_handle);

  client *exiting_client = search_clients((char *)requesting_handle);
  send_ACK_client_exit_packet(exiting_client->socket_num);

  return;
}

void parse_broadcast_packet(uint8_t *packet) {

  uint8_t sender_handle_length = *(packet + 3);

  uint8_t *sender_handle = (uint8_t *) calloc(sender_handle_length + 1, sizeof(uint8_t));
  memcpy(sender_handle, packet + 4, sender_handle_length);

  //printf("CLIENT: %s, Broadcasting Message\n", sender_handle);

  broadcast_packet(packet);

  return;
}

void parse_message_packet(uint8_t *packet) {

  uint8_t receiver_handle_length = *(packet + 3);

  uint8_t *receiver_handle = (uint8_t *) calloc(receiver_handle_length + 1, sizeof(uint8_t));
  memcpy(receiver_handle, packet + 4, receiver_handle_length);

  uint8_t sender_handle_length = *(packet + 3 + 1 + receiver_handle_length);

  uint8_t *sender_handle = (uint8_t *) calloc(sender_handle_length + 1, sizeof(uint8_t));
  memcpy(sender_handle, packet + 3 + 1 + 1 + receiver_handle_length, sender_handle_length);

  //printf("CLIENT: %s, Sending Message TO: %s\n", sender_handle, receiver_handle);

  forward_message_packet(packet, sender_handle, receiver_handle);

  return;
}

void parse_handle_list_request_packet(uint8_t *packet) {

  uint8_t requester_handle_length = *(packet + 3);

  uint8_t *requester_handle = (uint8_t *) calloc(requester_handle_length + 1, sizeof(uint8_t));
  memcpy(requester_handle, packet + 4, requester_handle_length);

  //printf("CLIENT: %s, Requesting Active Handles List\n", requester_handle);

  client *requesting_client = search_clients((char *)requester_handle);

  send_active_handle_count_packet(requesting_client->socket_num);
  send_active_handles_list_packet(requesting_client->socket_num);

  return;
}

void send_handle_authorization_packet(uint8_t *client_handle) {

  client *authorized_client = search_clients((char *)client_handle);

  packet authorized_handle_packet;

  uint16_t packet_length = 3;
  authorized_handle_packet.header.packet_length = htons(packet_length);
  authorized_handle_packet.header.flags = 0x02;

  //printf("SERVER: Sending Handle Authorization TO: %s\n", client_handle);

  send_packet(authorized_client->socket_num, (uint8_t *) &authorized_handle_packet, packet_length);

  return;
}

void send_handle_declined_packet(int socket_num, uint8_t *declined_handle) {

  int declined_handle_length = strlen((char *)declined_handle);

  packet declined_handle_packet;

  uint16_t packet_length = 3 + 1 + declined_handle_length + 1;
  declined_handle_packet.header.packet_length = htons(packet_length);
  declined_handle_packet.header.flags = 0x03;
  memcpy(declined_handle_packet.data, &declined_handle_length, 1);
  memcpy(declined_handle_packet.data + 1, declined_handle, declined_handle_length);

  //printf("SERVER: Sending Handle Declined Packet TO: %s\n", declined_handle);

  send_packet(socket_num, (uint8_t *) &declined_handle_packet, packet_length);

  return;
}

void forward_message_packet(uint8_t *packet, uint8_t *sender_handle, uint8_t *receiver_handle) {

  uint16_t packet_length = extract_packet_length(packet);
  client *receiving_client = search_clients((char *)receiver_handle);

  if (receiving_client != NULL) {

    // Receiving Client Found on Server

    //printf("SERVER: Forwarding Message Packet TO: %s\n", receiver_handle);

    send_packet(receiving_client->socket_num, packet, packet_length);

  } else {

    // Receiving Client Does Not Exist on Server

    //printf("SERVER: Receiving Client (%s) Does Not Exist on Server\n", receiver_handle);
    client *sender = search_clients((char *)sender_handle);
    send_receiver_DNE_packet(sender->socket_num, receiver_handle);

  }

  return;
}

void broadcast_packet(uint8_t *packet) {

  uint16_t packet_length = extract_packet_length(packet);

  uint8_t sender_handle_length = *(packet + 3);
  uint8_t *sender_handle = (uint8_t *) calloc(sender_handle_length + 1, sizeof(uint8_t));
  memcpy(sender_handle, packet + 4, sender_handle_length);

  client *current = get_root_client();

  while (current) {

    // If Not Broadcasting Client, Forward Broadcast to Client
    if (strcmp((char *)sender_handle, (char *)current->handle) != 0) {
      send_packet(current->socket_num, packet, packet_length);
    }

    current = current->next_client;
  }

  return;

}

void send_ACK_client_exit_packet(int socket_num) {

  packet ACK_exit_packet;

  uint16_t packet_length = 3;
  ACK_exit_packet.header.packet_length = htons(packet_length);
  ACK_exit_packet.header.flags = 0x09;

  //printf("SERVER: Sending ACK Client is Exiting\n");

  send_packet(socket_num, (uint8_t *) &ACK_exit_packet, packet_length);

  return;
}
void send_receiver_DNE_packet(int socket_num, uint8_t *receiver_handle) {

  int receiver_handle_length = strlen((char *)receiver_handle);

  packet receiver_DNE_packet;

  uint16_t packet_length = 3 + 1 + receiver_handle_length;
  receiver_DNE_packet.header.packet_length = htons(packet_length);
  receiver_DNE_packet.header.flags = 0x07;
  memcpy(receiver_DNE_packet.data, &receiver_handle_length, 1);
  memcpy(receiver_DNE_packet.data + 1, receiver_handle, receiver_handle_length);

  //printf("SERVER: Sending Handle Does Not Exist Packet\n");

  send_packet(socket_num, (uint8_t *) &receiver_DNE_packet, packet_length);

  return;
}

void send_active_handle_count_packet(int socket_num) {

  uint32_t active_client_count = get_active_client_count();

  packet active_handle_count_packet;

  uint16_t packet_length = 3 + 4;
  active_handle_count_packet.header.packet_length = htons(packet_length);
  active_handle_count_packet.header.flags = 0x0B;

  uint32_t active_client_count_network_order = htonl(active_client_count);
  memcpy(active_handle_count_packet.data, &active_client_count_network_order, 4);

  //printf("SERVER: Sending Active Handle Count Packet\n");

  send_packet(socket_num, (uint8_t *) &active_handle_count_packet, packet_length);

  return;
}

void send_active_handles_list_packet(int socket_num) {

  packet active_handle_list_packet;
  active_handle_list_packet.header.flags = 0x0C;
  uint8_t *packet_data_position = active_handle_list_packet.data;

  uint8_t last_packet_sent = 0;
  uint16_t packet_length = 3;

  client *current = get_root_client();

  while (current != NULL) {

    uint8_t current_handle_length = strlen(current->handle);
    memcpy(packet_data_position, &current_handle_length, 1);
    memcpy(packet_data_position + 1, current->handle, current_handle_length);

    packet_data_position  += (1 + current_handle_length);
    packet_length += (1 + current_handle_length);

    if (packet_length > 700) {

      active_handle_list_packet.header.packet_length = htons(packet_length);
      send_packet(socket_num, (uint8_t *) &active_handle_list_packet, packet_length);

      //memset(active_handle_list_packet.data, 0, 1000);
      packet_data_position = active_handle_list_packet.data;
      packet_length = 3;
      last_packet_sent = 1;

    }


    current = current->next_client;
  }

  active_handle_list_packet.header.packet_length = htons(packet_length);
  send_packet(socket_num, (uint8_t *) &active_handle_list_packet, packet_length);

  return;
}
