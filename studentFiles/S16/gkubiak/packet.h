#ifndef _PACKET_H_
#define _PACKET_H_

#include <stdint.h>

#define MAX_HANDLE_LEN (256)
#define MAX_MSG_LEN (1000)
#define MAX_PACKET_LEN (2048) //Longer the needed

typedef enum {
    PFLAG_INIT = 1,
    PFLAG_INIT_RESP,
    PFLAG_HANDLE_EXISTS,
    PFLAG_BROADCAST,
    PFLAG_MESSAGE,
    PFLAG_RESERVED,
    PFLAG_BAD_HANDLE,
    PFLAG_EXITING,
    PFLAG_EXITING_ACK,
    PFLAG_HANDLE_LIST_REQ,
    PFLAG_HANDLE_LIST_NUM,
    PFLAG_HANDLE_LIST
} FlagType_t;

typedef struct {
    uint16_t packetLen;
    uint8_t flags;
}__attribute__((packed)) PacketHeader_t;


void sendRawPacket(int sockfd, char* packet);
void sendPacketInit(int sockfd, char* handle);
void sendPacketInitResp(int sockfd);
void sendPacketHandleExists(int sockfd);
void sendPacketBroadcast(int sockfd, char* handle, char* msg);
void sendPacketMessage(int sockfd, char* srcHandle, char* dstHandle, char* msg);
void sendPacketBadHandle(int sockfd, char* handle);
void sendPacketExiting(int sockfd);
void sendPacketExitingAck(int sockfd);
void sendPacketHandleListReq(int sockfd);
void sendPacketHandleListNum(int sockfd, uint32_t num);
void sendPacketHandleList(int sockfd, char** handleList);

int getNumHandlesInBuffer(char* buffer, int len);

int readPacketHandleList(int sockfd, char* dataBuf, int* dataBufIdx);
int readPacket(int sockfd, char* dataBuf, int* dataBufIdx);

void decodeInitPacket(char* data, char* handle, int* handleLen);
void decodeBroadcastPacket(char* data, char* handle, int* handleLen, char* msg, int* msgLen);
void decodeMessagePacket(char* data,
                         char* srcHandle, int* srcHandleLen,
                         char* dstHandle, int* dstHandleLen,
                         char* msg, int* msgLen);
void decodeBadHandlePacket(char* data, char* handle, int* handleLen);
void decodeHandleListNumPacket(char* data, int* numHandles);
void printHandleListPacket(char* data);


#endif