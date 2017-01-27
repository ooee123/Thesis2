#ifndef HELPERS_H
#define HELPERS_H

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

  void print_bytes(uint8_t *data, short size);
  void send_packet(int socket_num, uint8_t *packet, uint16_t packet_length);

#endif
