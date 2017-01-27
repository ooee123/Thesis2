#ifndef __PACKETS_H
#define __PACKETS_H

#define MASK_BYTE 0xFF
#define MAX_PACKET_LEN 1515
#define OFFSET_PACKET_LEN 0x00
#define OFFSET_FLAG 0x02
#define OFFSET_HANDLE_LEN 0x03
#define OFFSET_HANDLE_NAME 0x04

#include "lib.h"

typedef uint8_t rdy_packet;
typedef enum {NULL0, FLAG1, FLAG2, FLAG3, FLAG4, FLAG5, NULL6, FLAG7, FLAG8,
               FLAG9, FLAG10, FLAG11, FLAG12} PACKET_TYPE;

typedef struct raw_packet {
   PACKET_TYPE type;
   uint16_t packet_len;
   uint8_t src_handle_len;
   char *src_handle_name;
   uint8_t dest_handle_len;
   char *dest_handle_name;
   char *message;
   uint32_t to_be_sent;
} raw_packet;

raw_packet* create_raw_packet();
rdy_packet* create_rdy_packet(uint16_t size);

raw_packet* recycle_raw_packet(raw_packet* packet);
rdy_packet* recycle_rdy_packet(rdy_packet* packet, uint16_t size);

/* Fills buffer rdy with information
 * given by raw packet
 */
int construct_packet(rdy_packet* rdy, raw_packet* raw);


/* Fills packet raw with information
 * given by rdy buffer
 */
int deconstruct_packet(raw_packet* raw, rdy_packet* rdy);

/* Frees all allocated memory associated with packet pointed to by packet
 */
void destroy_raw_packet(raw_packet* packet);


/* Frees all allocated memory associated with buffer pointed to by packet
 */
void destroy_rdy_packet(rdy_packet* packet);

#endif
