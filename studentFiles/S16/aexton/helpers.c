#include "helpers.h"

void print_bytes(uint8_t *data, short size) {

  printf("\n");

  while (size > 0) {
    printf("%02X ", *data);
    data++;
    size--;
  }

  printf("\n");

  return;
}

void send_packet(int socket_num, uint8_t *packet, uint16_t packet_length) {

  int sent = send(socket_num, packet, packet_length, 0);

  if (sent < 0) {
    perror("FAILED SENDING PACKET");
    exit(-1);
  } else {
    // printf("SENT PACKET BYTES: ");
    // print_bytes(packet, packet_length);
  }

  return;

}
