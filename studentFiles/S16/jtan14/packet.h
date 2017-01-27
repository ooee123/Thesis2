#define BUFFER_SIZE 1024

struct packetHeader { 
   uint16_t packLength;
   u_char flag;
} __attribute__((packed));

struct handle {
   char name[50];
   int fd;
   int inUse;
   int len;
} __attribute__((packed));

struct handleList {
   int curSize;
   int maxSize;
   struct handle *first;
} __attribute__((packed));