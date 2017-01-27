#include "testing.h"
#include "chatShared.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int test() {
	char *h1 = "abc";
	char *h2 = "xyz";
	char *mssg = "Hello world";
	char *handle1;
	char *handle2;
	char *myMssg;
	packetHandle *hPack = makePacketMssg(1, getHLen(h1), h1, getHLen(h2), h2, mssg);
	printf("%d\n", headerGetLen(&(hPack->head)));
	getDestHandle(hPack, &handle1);
	getSrcHandle(hPack, &handle2);
	getMssg(hPack, &mssg);
	printf("%s\n", handle1);
	printf("%s\n", handle2);
	printf("%s\n", mssg);
	return 0;
}

void headerSetLen(header *head, uint16_t len) {
	head->packetLen = htons(len + sizeof(header));
}

uint16_t headerGetLen(header *head) {
	return ntohs(head->packetLen) - sizeof(header);
}

void packetHandleAckSetNum(packetHandleAck *packet, uint32_t num) {
	packet->numHandles = htonl(num);
}

uint32_t packetHandleAckGetNum(packetHandleAck *packet) {
	return ntohl(packet->numHandles);
}

void *makePacketHandle(uint8_t flag, uint8_t hLen, char * handle) {
	uint16_t len = hLen + 1; //Packet is header size + the one bye for the headerLen field
	void *packet = malloc(len + sizeof(header));
	packetHandle *hPack = (packetHandle *) packet;

	hPack->head.flag = flag;
	headerSetLen(&(hPack->head), len);
	hPack->handleLen = hLen;

	memcpy(&(hPack->handleStart), handle, hLen);

	return packet;
}

void *makePacketBroadcast(uint8_t flag, uint8_t hLen, char *handle, char *mssg) {
	unsigned mssgLen = strlen(mssg) + 1; //We are including the null
	uint16_t len = hLen + 1 + mssgLen; //Packet is header size + the one bye for the headerLen field
	void *packet = malloc(len + sizeof(header));
	packetHandle *hPack = (packetHandle *) packet;

	hPack->head.flag = flag;
	headerSetLen((header *)hPack, len);
	hPack->handleLen = hLen;

	memcpy(&(hPack->handleStart), handle, hLen);
	memcpy(&(hPack->handleStart) + hLen, mssg, mssgLen);

	return packet;
}

void *makePacketMssg(uint8_t flag, uint8_t destLen, char *destH,
										 uint8_t srcLen, char *srcH, char *mssg) {
	unsigned mssgLen = strlen(mssg) + 1; //We are including the null
	uint16_t len = destLen + 1 + srcLen + 1 + mssgLen;
	void *packet = malloc(len + sizeof(header));
	packetHandle *hPack = (packetHandle *) packet;

	hPack->head.flag = flag;
	headerSetLen(&(hPack->head), len);
	hPack->handleLen = destLen;

	memcpy(&(hPack->handleStart), destH, destLen);
	*((uint8_t  *) &(hPack->handleStart) + destLen) = srcLen;
	memcpy(&(hPack->handleStart) + destLen + 1, srcH, srcLen);
	memcpy(&(hPack->handleStart) + destLen + 1 + srcLen, mssg, mssgLen);

	return packet;
}

/*
	Takes a pointer to a packet, and memcopies the handle
	to a new null terminated string pointed to by handle
*/
void getFirstHandle(void *packet, char **handle) {
	packetHandle *hPack = (packetHandle *) packet;
	*handle = malloc(hPack->handleLen + 1);
	memcpy(*handle, &(hPack->handleStart), hPack->handleLen);
	(*handle)[hPack->handleLen] = 0;
}

//Handy renames for clarity sake
void getDestHandle(void *packet, char **handle) {
	getFirstHandle(packet, handle);
}
void getSrcHandle(void *packet, char **handle) {
	getSecondHandle(packet, handle);
}

/*
	Takes a pointer to a packet, and memcopies the handle
	to a new null terminated string pointed to by handle
*/
void getSecondHandle(void *packet, char **handle) {
	packetHandle *hPack = (packetHandle *) packet;
	hPack = ((void *)hPack) + hPack->handleLen + sizeof(packetHandle) - 1 - sizeof(header);
	*handle = malloc(hPack->handleLen + 1);
	memcpy(*handle, &(hPack->handleStart), hPack->handleLen);
	(*handle)[hPack->handleLen] = 0;
}

/*
	Takes a pointer to a packet, and memcopies the mssg
	to a new null terminated string pointed to by mssg
*/
void getMssg(void *packet, char **mssg) {
	packetHandle *hPack = (packetHandle *) packet;
	unsigned destLen = hPack->handleLen;
	unsigned packetLen = headerGetLen(packet);
	unsigned srcLen;

	hPack = ((void *)hPack) + hPack->handleLen + sizeof(packetHandle) - 1 - sizeof(header);
	srcLen = hPack->handleLen;

	unsigned mssgLen = packetLen - (destLen + 1) - (srcLen + 1);
	*mssg = malloc(mssgLen);
	memcpy(*mssg, &(hPack->handleStart) + hPack->handleLen, mssgLen);
}

/*
	Takes a pointer to a packet, and memcopies the mssg
	to a new null terminated string pointed to by mssg
*/
void getBroadCastMssg(void *packet, char **mssg) {
	packetHandle *hPack = (packetHandle *) packet;
	unsigned mssgLen = headerGetLen(packet) - (hPack->handleLen + 1);
	*mssg = malloc(mssgLen);
	memcpy(*mssg, &(hPack->handleStart) + hPack->handleLen, mssgLen);
}

uint8_t getHLen(char *handle) {
	unsigned len = strlen((char *)handle);
	if(len > getMaxHLen()) {
		printf("The handle you entered is too long\n");
		exit(-1);
	}
	return len;
}

uint8_t getMaxHLen() {
	uint8_t max = 0;
	return max-1;
}

void sendPacketFlag(uint8_t flag, int socket) {
  header head;
  headerSetLen(&head, 0);
  head.flag = flag;
  if(send(socket, &head, sizeof(header), 0) < 0) {
    perror("Flag Packet Send");
    exit(1);
  }
}

int sendPacket(void *packet, int socket) {
	return send(socket, packet, headerGetLen((header *)packet) + sizeof(header), 0);
}
