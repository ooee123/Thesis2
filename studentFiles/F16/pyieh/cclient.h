#ifndef CCLIENT_H
#define CCLIENT_H

#define HANDLE_MAX_LEN 255
#define CHAT_HEADER_LEN 3
#define PACKET_LEN_BYTES 2
#define MAX_MSG_LEN 1000

#define C_HANDLE_FLAG 1
#define S_GOOD_HANDLE_FLAG 2
#define S_BAD_HANDLE_FLAG 3
#define C_BROADCAST_FLAG 4
#define C_MESSAGE_FLAG 5
#define S_BAD_MSG_DEST_FLAG 7
#define C_EXIT_FLAG 8
#define S_ACK_EXIT_FLAG 9
#define C_LIST_HANDLES_FLAG 10
#define S_NUM_HANDLES_FLAG 11
#define S_HANDLE_FLAG 12

struct chat_header {
   uint16_t len;
   uint8_t flag;
}__attribute__((__packed__));
typedef struct chat_header chat_header;

struct packet {
   chat_header header;
   u_char *data;
}__attribute__((__packed__));
typedef struct packet packet;

#endif