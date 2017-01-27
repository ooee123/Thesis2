#ifndef PACKET_H
#define PACKET_H

  #include <stdlib.h>
  #include <stdio.h>
  #include <sys/types.h>
  #include <stdint.h>
  #include <arpa/inet.h>

  #pragma pack(1)

  typedef struct packet_header {
    uint16_t packet_length;
    uint8_t flags;
  } packet_header;

  typedef struct packet {
    packet_header header;
    uint8_t data[1000];
  } packet;

  uint16_t extract_packet_length(uint8_t *packet);
  uint8_t extract_packet_flags(uint8_t *packet);

#endif
