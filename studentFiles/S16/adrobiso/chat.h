#include <stdint.h>

#define HANDLE_LEN 255
#define MSG_LEN 1000
#define BUFFER_SIZE (sizeof(struct packetHeader) + 1 + HANDLE_LEN + 1 + HANDLE_LEN + MSG_LEN)

struct packetHeader {
   uint16_t length;
   uint8_t flag;
} __attribute__((__packed__));

struct handleFd {
   char handle[HANDLE_LEN];
   int fd;
};
