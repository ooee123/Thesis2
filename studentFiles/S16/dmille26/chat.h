#define NORMAL_HDR_LEN 3
#define MIN_PORT 1
#define MAX_PORT 65535
#define MAX_BUF 4096
#define MAX_STR 256
#define MAX_MSG_LEN 1000

#define C_S_INIT 1
#define S_C_GOOD_HNDL 2
#define S_C_ERR_HNDL_EXISTS 3 // response to C_S_INIT
#define C_A_BROADCAST 4
#define C_S_C_DIR_MSG 5
#define RESERVED 6
#define S_C_ERR_DST_NEXISTS 7
#define C_S_REQ_EXT 8
#define S_C_ACK_EXT 9 // response to 8
#define C_S_REQ_HNDLS 10
#define S_C_NUM_HNDLS 11 // response to 10
#define S_C_VALID_HNDL 12 // follows 11
struct {
   char handle[MAX_STR];
   char serverName[MAX_STR];
   char port[6];
} typedef cliArgs;

struct msgPkt {
   u_char flags;
   // stored with out a null 
   int lenDstHandle;
   char dstHandle[MAX_STR - 1];
   int lenSrcHandle;
   char srcHandle[MAX_STR - 1];
   char msgData[MAX_MSG_LEN]; // 999 chars and a null
   uint16_t pktLenHost; // in bytes, conver to network before sending.
   struct msgPkt* next;
};

void cpyData(char* buf, u_char* packet, int len, int offset) {
   memcpy(packet + offset, buf, len);
   packet[offset + len] = '\0';
}
int cpyHdr(struct msgPkt* msg, u_char* packet) {
   uint16_t len = htons(msg->pktLenHost);
   memcpy(packet, &len, sizeof(uint16_t));
   packet[2] = msg->flags;
   return NORMAL_HDR_LEN; // 3
}

int setClientHandle(struct msgPkt* msg, cliArgs* meta) {
   u_char len = strlen(meta->handle);
   strncpy(msg->srcHandle, meta->handle, len);
   msg->lenSrcHandle = len;
   return len + 1; 
}

void setMsgLen(struct msgPkt* msg, int len) {
   msg->pktLenHost = len;
}

int setDestHandle(struct msgPkt* msg, char* handle) {
   u_char len = strlen(handle); 
   strncpy(msg->dstHandle, handle, len);
   msg->lenDstHandle = len;
   return len + 1; // return bytes for total packet size, +1 for len field 
}

int cpyCli(struct msgPkt* msg, u_char* packet, int offset) {
   packet[offset] = msg->lenSrcHandle;
   offset++;
   memcpy(packet + offset, msg->srcHandle, msg->lenSrcHandle);
   offset += msg->lenSrcHandle;
   return offset; 
}


int cpyDst(struct msgPkt* msg, u_char* packet, int offset) {
   packet[offset] = msg->lenDstHandle;
   offset++;
   memcpy(packet + offset, msg->dstHandle, msg->lenDstHandle);
   offset += msg->lenDstHandle;
   return offset;
}

int parseHdr(u_char* buf, int* flag) {
   uint16_t size;
   memcpy(&size, buf, sizeof(uint16_t));
   size= ntohs(size);
   *flag = buf[2];
   int dataLeft = size - NORMAL_HDR_LEN;
   return dataLeft;
}

void printPacket(u_char* packet, int len) {
   int i = 0;
   printf("\nPacket len: %d \n", len);
   for(i = 0; i < len; i++) {
      printf("%02X ", packet[i]);
   }
   printf("\n");
}
