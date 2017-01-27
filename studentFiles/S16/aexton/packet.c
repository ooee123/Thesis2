#include "packet.h"

uint16_t extract_packet_length(uint8_t *packet) {
  uint16_t packet_length = ntohs(*((uint16_t *)packet));
  return packet_length;
}

uint8_t extract_packet_flags(uint8_t *packet) {
  return *(packet + 2);
}
