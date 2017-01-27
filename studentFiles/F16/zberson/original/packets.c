#include <arpa/inet.h>
#include <string.h>

#include "packets.h"

static void clear_raw_packet(raw_packet* packet) {
   packet->type = NULL0;
   packet->packet_len = 0;
   packet->src_handle_len = 0;
   packet->src_handle_name = NULL;
   packet->dest_handle_len = 0;
   packet->dest_handle_name = NULL;
   packet->message = NULL;
   packet->to_be_sent = 0;
}

raw_packet* create_raw_packet() {
   raw_packet* raw = calloc(1, sizeof(raw_packet));
   clear_raw_packet(raw);
   return raw;
}

rdy_packet* create_rdy_packet(uint16_t size) {
   rdy_packet* rdy = calloc(size, sizeof(char));
   return rdy;
}

void destroy_raw_packet(raw_packet* packet) {
   if(packet->src_handle_name) {
      free(packet->src_handle_name);
      packet->src_handle_name = NULL;
   }
   if(packet->dest_handle_name) {
      free(packet->dest_handle_name);
      packet->dest_handle_name = NULL;
   }
   if(packet->message) {
      free(packet->message);
      packet->message = NULL;
   }
   free(packet);
}

void destroy_rdy_packet(rdy_packet* packet) {
   if(packet) {
      free(packet);
   }
}

raw_packet* recycle_raw_packet(raw_packet* packet) {
   destroy_raw_packet(packet);
   return create_raw_packet();
}

rdy_packet* recycle_rdy_packet(rdy_packet* packet, uint16_t size) {
   destroy_rdy_packet(packet);
   return create_rdy_packet(size);
}

static void construct_packet_header(rdy_packet* rdy, raw_packet* raw) {
   uint8_t* pos;

   pos = rdy + OFFSET_PACKET_LEN;
   if(raw->type == FLAG12) {
      *(pos) = 0;
      *(pos+1) = 0;
   }
   else {
      *((uint16_t*)pos) = htons(raw->packet_len);
   }
   pos = rdy + OFFSET_FLAG;
   *(pos) = raw->type;
}

int construct_packet(rdy_packet* rdy, raw_packet* raw) {
   uint8_t* pos;
   uint16_t message_len;

   if(raw->type < FLAG1 || raw->type == NULL6 || raw->type > FLAG12) {
      fprintf(stderr, "Invalid flag type '%u'\n", raw->type);
      return -1;
   }
   
   construct_packet_header(rdy, raw);

   pos = rdy + OFFSET_HANDLE_LEN;
   if(raw->dest_handle_len) {
      *(pos++) = raw->dest_handle_len;
      memcpy(pos, raw->dest_handle_name, raw->dest_handle_len);
      pos += raw->dest_handle_len;
   }
   if(raw->src_handle_len) {
      *(pos++) = raw->src_handle_len;
      memcpy(pos, raw->src_handle_name, raw->src_handle_len);
      pos += raw->src_handle_len;
   }
   else if(raw->to_be_sent) {
      *((uint32_t*)pos) = htonl(raw->to_be_sent);
   }
   if(raw->type == FLAG4 || raw->type == FLAG5) {
      message_len = raw->packet_len - raw->src_handle_len -
                     raw->dest_handle_len - raw->type;
      memcpy(pos, raw->message, message_len);
   }
   return 0;
}

int deconstruct_packet(raw_packet* raw, rdy_packet* rdy) {
   uint8_t* pos;
   uint16_t message_len;
   
   pos = rdy + OFFSET_FLAG;
   raw->type = *(pos);
   if(raw->type < FLAG1 || raw->type == NULL6 || raw->type > FLAG12) {
      fprintf(stderr, "Invalid flag type '%u'\n", raw->type);
      return -1;
   }
   if(raw->type == FLAG12) {
      pos = rdy + OFFSET_HANDLE_LEN;
      raw->packet_len = *(pos) + OFFSET_HANDLE_NAME;
   }
   else {
      pos = rdy + OFFSET_PACKET_LEN;
      raw->packet_len = ntohs(*((uint16_t*)pos));
   }

   pos = rdy + OFFSET_HANDLE_LEN;
   switch (raw->type) {
      case FLAG5:
         raw->dest_handle_len = *(pos++);
         raw->dest_handle_name = calloc(raw->dest_handle_len + 1, sizeof(uint8_t));
         memcpy(raw->dest_handle_name, pos, raw->dest_handle_len);
         pos += raw->dest_handle_len;
      case FLAG1:
      case FLAG4:
      case FLAG7:
      case FLAG12:
         raw->src_handle_len = *(pos++);
         raw->src_handle_name = calloc(raw->src_handle_len + 1, sizeof(uint8_t));
         memcpy(raw->src_handle_name, pos, raw->src_handle_len);
         pos += raw->src_handle_len;
         break;
      default:
         break;
   }

   if(raw->type == FLAG4 || raw->type == FLAG5) {
      message_len = raw->packet_len - raw->src_handle_len -
                     raw->dest_handle_len - raw->type;
      raw->message = calloc(message_len + 1, sizeof(uint8_t));
      memcpy(raw->message, pos, message_len);
   }
   else if(raw->type == FLAG11) {
      raw->to_be_sent = ntohl(*((uint32_t*)pos));
   }
   return 0;
}
