#ifndef __chatStructs__
#define __chatStructs__

#include <stdint.h>

/*Flag values*/
#define CLIENT_CONNECT 		1
#define CONNECT_ACK				2
#define CONNECT_ERROR			3
#define CLIENT_BROADCAST	4
#define CLIENT_MESSAGE		5
#define MESSAGE_ERROR			7
#define CLIENT_EXIT				8
#define EXIT_ACK					9
#define CLIENT_HANDLES		10
#define HANDLES_ACK				11
#define SERVER_HANDLE			12

#define MAXBUF 1000

typedef struct __attribute__((__packed__)) {
	uint8_t flag;
	uint16_t packetLen; //net order
} header;

typedef struct __attribute__((__packed__)) {
	header head;
	uint8_t handleLen;
	unsigned char handleStart; //handleLen chars long
} packetHandle;

typedef struct __attribute__((__packed__)) {
	header head;
	uint32_t numHandles;  //net order
} packetHandleAck;

/* NOTE: It is the calers job to free pointser returned by these*/

void headerSetLen(header *head, uint16_t len);
uint16_t headerGetLen(header *head);
void *makePacketHandle(uint8_t flag, uint8_t hLen, char * handle);
uint8_t getHLen(char *handle);
uint8_t getMaxHLen();
void getFirstHandle(void *packet, char **handle);
void *makePacketBroadcast(uint8_t flag, uint8_t hLen, char * handle, char *mssg);
void getBroadCastMssg(void *packet, char **mssg);
void *makePacketMssg(uint8_t flag, uint8_t destLen, char *destH,
							 uint8_t srcLen, char *srcH, char *mssg);
void getMssg(void *packet, char **mssg);
void getSecondHandle(void *packet, char **handle);
void getDestHandle(void *packet, char **handle);
void getSrcHandle(void *packet, char **handle);
void packetHandleAckSetNum(packetHandleAck *packet, uint32_t num);
uint32_t packetHandleAckGetNum(packetHandleAck *packet);
void sendPacketFlag(uint8_t flag, int socket);
int sendPacket(void *packet, int socket);

#endif
