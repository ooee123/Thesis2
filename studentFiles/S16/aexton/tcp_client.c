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
#include <ctype.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "testing.h"
#include "networks.h"
#include "helpers.h"
#include "packet.h"

#define MAX_BUFFER_SIZE 1024

void start_client(char *server_name, char *server_port, char *handle);

void command_loop(int socket_num, char *client_handle);
void send_message(int socket_num, char *sending_client, char *receiving_client, char *message);
void send_broadcast(int socket_num, char *sending_client, char *message);
void initialize_client(int socket_num, char *requesting_client_handle);
void send_exiting_packet(int socket_num, char *client_handle);
void request_active_handles(int socket_num, char *client_handle);

uint8_t * receive_data_from_server(int socket_num);

uint8_t parse_recieved_packet(uint8_t *packet);
void parse_handle_already_exists_packet(uint8_t *packet);
void parse_message_packet(uint8_t *packet);
void parse_broadcast_packet(uint8_t *packet);
void parse_receiver_DNE_packet(uint8_t *packet);
void parse_active_handle_count_packet(uint8_t *packet);
void parse_active_handle_list_packet(uint8_t *packet);


int main(int argc, char * argv[]) {

  // Check for 3 arguments
  if (argc != 4) {
    printf("usage: %s handle server-name server-port\n", argv[0]);
    exit(1);
  }

  char *handle = calloc(strlen(argv[1]), sizeof(char));
  memcpy(handle, argv[1], strlen(argv[1]));

  char *server_name = argv[2];
  char *server_port = argv[3];

  start_client(server_name, server_port, handle);

  free(handle);

  return 0;
}

void start_client(char *server_name, char *server_port, char *handle) {

  // Set up the Clients socket for TCP transmission
  int socket_num = tcp_send_setup(server_name, server_port);

  initialize_client(socket_num, handle);
  command_loop(socket_num, handle);

  close(socket_num);

  return;
}

void command_loop(int socket_num, char *client_handle) {

  char *user_input = calloc(1024, sizeof(char));
  char *receiver_handle, *message;

  fd_set active_fd_set;
  struct timeval timeout;
  timeout.tv_usec = 250000;

  int prompt_printed = 0;

  while (1) {

    if (!prompt_printed) {
      printf("$: ");
      fflush(stdout);
      prompt_printed = 1;
    }

    // Set Client<->Server Socket
    FD_ZERO (&active_fd_set);
    FD_SET(0, &active_fd_set);
    FD_SET(socket_num, &active_fd_set);

    if (select(socket_num + 1, &active_fd_set, NULL, NULL, &timeout) < 0) {
      perror("select failure.");
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(0, &active_fd_set)) {

      //printf("HIT\n");

      fgets(user_input, 1024, stdin);
      prompt_printed = 0;

      if (*user_input == '%') {

        user_input++;
        char command = *user_input;
        user_input++;

        switch (tolower(command)) {

            // %M(essage) [Reciever Handle] [Message]
            case 'm':

              receiver_handle = strtok(user_input, " ");
              message = strtok(NULL, "\n");
              send_message(socket_num, client_handle, receiver_handle, message);

              break;

            // %B(roadcast) [Message]
            case 'b':

              message = strtok(++user_input, "\n");
              send_broadcast(socket_num, client_handle, message);

              break;

            // %L(ist Active Handles)
            case 'l':

              request_active_handles(socket_num, client_handle);

              break;

            // %E(xit)
            case 'e':

              send_exiting_packet(socket_num, client_handle);
              //ongoing = 0;
              break;

            default:
              break;
        }

      } else {
        printf("Command must begin with %%\n");
      }
    }

    if (FD_ISSET(socket_num, &active_fd_set)) {
      //printf("HIT\n");
      uint8_t *packet = receive_data_from_server(socket_num);
      parse_recieved_packet(packet);
      prompt_printed = 0;
    }

  }

  return;
}

void initialize_client(int socket_num, char *requesting_client_handle) {

  packet initial_packet;

  initial_packet.header.flags = 0x01;

  uint8_t requesting_client_handle_length = strlen(requesting_client_handle);
  memcpy(initial_packet.data, &requesting_client_handle_length, 1);
  memcpy(initial_packet.data + 1, requesting_client_handle, requesting_client_handle_length);

  // Header Bytes + Handle Length Bytes + Handle Bytes + Message Bytes
  int packet_length = 3 + 1 + requesting_client_handle_length;
  initial_packet.header.packet_length = htons(packet_length);

  send_packet(socket_num, (uint8_t *)&initial_packet, packet_length);

  uint8_t *data = receive_data_from_server(socket_num);
  uint8_t packet_flag = parse_recieved_packet(data);

  if (packet_flag == 3) {
    printf("Handle already in use: %s\n", requesting_client_handle);
    exit(1);
  }

  return;
}

void send_exiting_packet(int socket_num, char *client_handle) {

  packet exiting_packet;

  exiting_packet.header.flags = 0x08;

  uint8_t client_handle_length = strlen(client_handle);
  memcpy(exiting_packet.data, &client_handle_length, 1);
  memcpy(exiting_packet.data + 1, client_handle, client_handle_length);

  // Header Bytes + Handle Length Bytes + Handle Bytes + Message Bytes
  int packet_length = 3 + 1 + client_handle_length;
  exiting_packet.header.packet_length = htons(packet_length);

  send_packet(socket_num, (uint8_t *)&exiting_packet, packet_length);

};


void send_message(int socket_num, char *sending_client, char *receiving_client, char *message) {

  packet message_packet;

  message_packet.header.flags = 0x05;

  uint8_t receiving_client_handle_length = strlen(receiving_client);
  memcpy(message_packet.data, &receiving_client_handle_length, 1);
  memcpy(message_packet.data + 1, receiving_client, receiving_client_handle_length);

  uint8_t sending_client_handle_length = strlen(sending_client);
  memcpy(message_packet.data + 1 + receiving_client_handle_length, &sending_client_handle_length, 1);
  memcpy(message_packet.data + 2 + receiving_client_handle_length, sending_client, sending_client_handle_length);

  // Message length with ending NULL
  int message_length = strlen(message) + 1;

  // printf("MESSAGE BYTES: ");
  // print_bytes((uint8_t *) message, message_length);

  memcpy(message_packet.data + 2 + receiving_client_handle_length + sending_client_handle_length, message, message_length);

  // Header Bytes + Handle Length Bytes + Handle Bytes + Message Bytes
  int packet_length = 3 + 2 + sending_client_handle_length + receiving_client_handle_length + message_length;
  message_packet.header.packet_length = htons(packet_length);

  send_packet(socket_num, (uint8_t *)&message_packet, packet_length);

  return;
}

void send_broadcast(int socket_num, char *sending_client, char *message) {

  packet message_packet;

  message_packet.header.flags = 0x04;

  uint8_t sending_client_handle_length = strlen(sending_client);
  memcpy(message_packet.data, &sending_client_handle_length, 1);
  memcpy(message_packet.data + 1, sending_client, sending_client_handle_length);

  // Message length with ending NULL
  int message_length = strlen(message) + 1;

  // printf("MESSAGE BYTES: ");
  // print_bytes((uint8_t *) message, message_length);

  memcpy(message_packet.data + 1 + sending_client_handle_length, message, message_length);

  // Header Bytes + Handle Length Bytes + Handle Bytes + Message Bytes
  int packet_length = 3 + 1 + sending_client_handle_length + message_length;
  message_packet.header.packet_length = htons(packet_length);

  send_packet(socket_num, (uint8_t *)&message_packet, packet_length);

  return;
}

void request_active_handles(int socket_num, char *client_handle) {

  packet request_active_handles_packet;

  request_active_handles_packet.header.flags = 0x0A;

  uint8_t client_handle_length = strlen(client_handle);
  memcpy(request_active_handles_packet.data, &client_handle_length, 1);
  memcpy(request_active_handles_packet.data + 1, client_handle, client_handle_length);

  // Header Bytes + Handle Length Byte + Handle Bytes
  int packet_length = 3 + 1 + client_handle_length;
  request_active_handles_packet.header.packet_length = htons(packet_length);

  send_packet(socket_num, (uint8_t *)&request_active_handles_packet, packet_length);

  return;
}



int tcp_send_setup(char *host_name, char *port) {
    int socket_num;
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host

    // create the socket
    if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket call");
      exit(-1);
	  }

    // designate the addressing family
    remote.sin_family= AF_INET;

    // get the address of the remote host and store
    if ((hp = gethostbyname(host_name)) == NULL) {
  	  printf("Error getting hostname: %s\n", host_name);
  	  exit(-1);
	   }

    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

    // get the port used on the remote side and store
    remote.sin_port= htons(atoi(port));

    if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0) {
    	perror("connect call");
    	exit(-1);
    }

    return socket_num;
}

uint8_t * receive_data_from_server(int socket_num) {

  int bytesRecieved = 0;

  uint8_t *buffer = (uint8_t *) calloc(MAX_BUFFER_SIZE, sizeof(uint8_t));

  while (bytesRecieved < 2) {
    bytesRecieved += recv(socket_num, buffer + bytesRecieved, 2 - bytesRecieved, 0);
  }

  uint16_t packetBytes = ntohs(*((uint16_t *)buffer));

  while (bytesRecieved < packetBytes) {
    bytesRecieved += recv(socket_num, buffer + bytesRecieved, packetBytes - bytesRecieved, 0);
  }

  // printf("RECIEVED BYTES (%d): ", bytesRecieved);
  // print_bytes((uint8_t *) buffer, packetBytes);

  uint8_t *packet = (uint8_t *) calloc(packetBytes, sizeof(uint8_t));
  memcpy(packet, buffer, packetBytes);

  free(buffer);

  return packet;
}

uint8_t parse_recieved_packet(uint8_t *packet) {

  uint8_t packet_flag = extract_packet_flags(packet);
  // printf("FLAG BYTE: %u\n", packet_flag);

  switch (packet_flag) {

    case 2:     // Sever Authorized Handle
      //printf("SEVER: Authorized Handle!\n");
      break;

    case 3:    // Server Response - Handle Already Exists
      //printf("SEVER: Declined Requested Handle!\n");
      parse_handle_already_exists_packet(packet);

      break;

    case 4:    // Broadcast Packet
      //printf("SEVER: Broadcasted Message Arrived!\n");
      parse_broadcast_packet(packet);

      break;

    case 5:    // Message Packet
      //printf("SEVER: Message Arrived!\n");
      parse_message_packet(packet);
      break;

    case 7:    // Message Destination Handle Does Not Exist
      //printf("SEVER: Destination Handle Does Not Exist!\n");
      parse_receiver_DNE_packet(packet);
      break;

    case 9:    // ACK Client Exit Packet
      //printf("SEVER: Acknowled client is exiting! BYE BYE!\n");
      exit(1);
      break;

    case 11:    // Current Active Handle Count Packet
      //printf("SEVER: Active Handle Count Packet!\n");
      parse_active_handle_count_packet(packet);
      break;

    case 12:    // Current Active Handles List Packet
      //printf("SEVER: Active Handle List Packet!\n");
      parse_active_handle_list_packet(packet);
      break;

    default:

      break;

  }

  return packet_flag;
}

void parse_handle_already_exists_packet(uint8_t *packet) {

  uint8_t handle_length = *(packet + 3);
  uint8_t *handle = calloc(handle_length + 1, sizeof(uint8_t));
  memcpy(handle, packet + 4, handle_length);

  printf("Handle already in use: %s\n", handle);

  free(handle);
  free(packet);

  exit(1);

  return;
}

void parse_message_packet(uint8_t *packet) {

  uint16_t packet_length = extract_packet_length(packet);

  uint8_t receiver_handle_length = *(packet + 3);

  uint8_t sender_handle_length = *(packet + 3 + 1 + receiver_handle_length);
  uint8_t *sender_handle = (uint8_t *) calloc(sender_handle_length + 1, sizeof(uint8_t));
  memcpy(sender_handle, packet + 3 + 1 + receiver_handle_length + 1, sender_handle_length);

  uint16_t message_len = packet_length - 3 - 1 - receiver_handle_length - 1 - sender_handle_length;
  uint8_t *message = (uint8_t *) calloc(message_len + 1, sizeof(uint8_t));
  memcpy(message, packet + 3 + 1 + receiver_handle_length + 1 + sender_handle_length, message_len);

  printf("%s: %s\n", sender_handle, message);

  free(sender_handle);
  free(message);
  free(packet);

  return;
}

void parse_broadcast_packet(uint8_t *packet) {

  uint16_t packet_length = extract_packet_length(packet);

  uint8_t sender_handle_length = *(packet + 3);
  uint8_t *sender_handle = (uint8_t *) calloc(sender_handle_length + 1, sizeof(uint8_t));
  memcpy(sender_handle, packet + 3 + 1, sender_handle_length);

  uint16_t message_len = packet_length - 3 - 1 - sender_handle_length;
  uint8_t *message = (uint8_t *) calloc(message_len + 1, sizeof(uint8_t));
  memcpy(message, packet + 3 + 1 + sender_handle_length, message_len);

  printf("%s: %s\n", sender_handle, message);

  free(sender_handle);
  free(message);
  free(packet);

  return;
}

void parse_receiver_DNE_packet(uint8_t *packet) {

  uint8_t DNE_handle_length = *(packet + 3);
  uint8_t *DNE_handle = (uint8_t *) calloc(DNE_handle_length + 1, sizeof(uint8_t));
  memcpy(DNE_handle, packet + 3 + 1, DNE_handle_length);

  printf("Client with handle %s does not exist.\n", DNE_handle);

  free(DNE_handle);
  free(packet);

  return;
}


void parse_active_handle_count_packet(uint8_t *packet) {

  uint32_t active_handle_count = ntohl(*((uint32_t *)(packet + 3)));
  printf("Number of clients: %u\n",  active_handle_count);

  free(packet);

  return;
}

void parse_active_handle_list_packet(uint8_t *packet) {

  uint16_t packet_length = extract_packet_length(packet);
  uint16_t bytes_parsed = 0;

  uint8_t *current_handle_length_byte = packet + 3;
  bytes_parsed += 3;

  while (bytes_parsed < packet_length) {

    uint8_t current_handle_length = *current_handle_length_byte;

    uint8_t *current_handle = (uint8_t *) calloc(current_handle_length + 1, sizeof(uint8_t));
    memcpy(current_handle, current_handle_length_byte + 1, current_handle_length);

    printf("  %s\n", current_handle);

    free (current_handle);

    current_handle_length_byte += (1 + current_handle_length);
    bytes_parsed += (1 + current_handle_length);

  }

  free(packet);

  return;

}
