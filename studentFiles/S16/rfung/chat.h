#ifndef CHAT_H
#define CHAT_H

#define MAX_HANDLE_LEN 255
#define BUFF_SIZE 1024

typedef struct normal_header {
	uint16_t len;
	uint8_t flag;
} __attribute__((packed)) normal_header;

#endif