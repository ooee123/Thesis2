
#include "testing.h"
#include "socketWrapper.h"
#include "packet.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//Helper functions to send packets
static void sendHeader(int sockfd, uint16_t lenHost, FlagType_t type)
{
    PacketHeader_t header;

    header.packetLen = htons(lenHost);
    header.flags = type;

    wrapSend(sockfd, &header, sizeof(PacketHeader_t), 0);
}

static void sendHandle(int sockfd, char* handle)
{
    uint8_t handleLen;

    handleLen = strlen(handle);
    wrapSend(sockfd, &handleLen, 1, 0);
    wrapSend(sockfd, handle, handleLen, 0);
}

void sendRawPacket(int sockfd, char* packet)
{
    PacketHeader_t* header = (PacketHeader_t*)packet;
    uint16_t len;

    len = ntohs(header->packetLen);

    wrapSend(sockfd, packet, len, 0);
}

//Helper functions to send each type of packet
void sendPacketInit(int sockfd, char* handle)
{
    int packetLen = 0;
    uint8_t handleLen;

    packetLen += sizeof(PacketHeader_t);
    packetLen += 1 + strlen(handle);

    sendHeader(sockfd, packetLen, PFLAG_INIT);

    handleLen = strlen(handle);

    wrapSend(sockfd, &handleLen, 1, 0);
    wrapSend(sockfd, handle, handleLen, 0);
}

void sendPacketInitResp(int sockfd)
{
    sendHeader(sockfd, sizeof(PacketHeader_t), PFLAG_INIT_RESP);
}

void sendPacketHandleExists(int sockfd)
{
    sendHeader(sockfd, sizeof(PacketHeader_t), PFLAG_HANDLE_EXISTS);
}

void sendPacketBroadcast(int sockfd, char* handle, char* msg)
{
    int packetLen = 0;

    packetLen += sizeof(PacketHeader_t);
    packetLen += 1 + strlen(handle);
    packetLen += 1 + strlen(msg);

    sendHeader(sockfd, packetLen, PFLAG_BROADCAST);

    sendHandle(sockfd, handle);

    wrapSend(sockfd, msg, strlen(msg) + 1, 0);
}

void sendPacketMessage(int sockfd, char* srcHandle, char* dstHandle, char* msg)
{
    int packetLen = 0;

    packetLen += sizeof(PacketHeader_t);
    packetLen += 1 + strlen(srcHandle);
    packetLen += 1 + strlen(dstHandle);
    packetLen += 1 + strlen(msg);

    sendHeader(sockfd, packetLen, PFLAG_MESSAGE);

    sendHandle(sockfd, dstHandle);

    sendHandle(sockfd, srcHandle);

    wrapSend(sockfd, msg, strlen(msg) + 1, 0);
}

void sendPacketBadHandle(int sockfd, char* handle)
{
    int packetLen = 0;

    packetLen += sizeof(PacketHeader_t);
    packetLen += 1 + strlen(handle);

    sendHeader(sockfd, packetLen , PFLAG_BAD_HANDLE);

    sendHandle(sockfd, handle);
}

void sendPacketExiting(int sockfd)
{
    sendHeader(sockfd, sizeof(PacketHeader_t), PFLAG_EXITING);  
}

void sendPacketExitingAck(int sockfd)
{
    sendHeader(sockfd, sizeof(PacketHeader_t), PFLAG_EXITING_ACK);
}

void sendPacketHandleListReq(int sockfd)
{
    sendHeader(sockfd, sizeof(PacketHeader_t), PFLAG_HANDLE_LIST_REQ);
}

void sendPacketHandleListNum(int sockfd, uint32_t num)
{
    sendHeader(sockfd, sizeof(PacketHeader_t) + 4, PFLAG_HANDLE_LIST_NUM);

    num = htonl(num);
    wrapSend(sockfd, &num, 4, 0);
}

void sendPacketHandleList(int sockfd, char** handleList)
{
    uint32_t i;

    sendHeader(sockfd, 0, PFLAG_HANDLE_LIST);

    i = 0;
    while(handleList[i])
    {
        i++;
    }
    i = htonl(i);

    wrapSend(sockfd, &i, 4, 0);

    i = 0;
    while(handleList[i])
    {
        sendHandle(sockfd, handleList[i]);
        i++;
    }
}


//Functions used to read in an entire packet with recv
int getNumHandlesInBuffer(char* buffer, int len)
{
    int count = 0;

    while (len > 0)
    {
        if (*buffer == 0)
        {
            return 0;
        }
        len -= *buffer + 1;
        buffer += *buffer + 1;
        count++;
    }

    if (len < 0)
    {
        count--;
    }

    return count;
}

//Special case for HandleList because the length is not in the packet header
int readPacketHandleList(int sockfd, char* dataBuf, int* dataBufIdx)
{
    uint32_t numHandles;
    uint32_t handlesRead;
    int recvLen;

    recvLen = wrapRecv(sockfd, &dataBuf[*dataBufIdx], 1, 0);
    if (recvLen == 0)
    {
        return -1;
    }
    *dataBufIdx += recvLen;

    if (*dataBufIdx < 7)
    {
        return 0;
    } else {
        numHandles = ntohl(*(uint32_t*)&dataBuf[3]);
        handlesRead = getNumHandlesInBuffer(&dataBuf[7], *dataBufIdx - 7);

        if (numHandles == handlesRead)
        {
            return 1;
        } else {
            return 0;
        }
    }
}

int readPacket(int sockfd, char* dataBuf, int* dataBufIdx)
{
    uint16_t packetLen;
    int recvLen;

    if (*dataBufIdx < 3)
    {
        recvLen = wrapRecv(sockfd, &dataBuf[*dataBufIdx], 1, 0);
        if (recvLen == 0)
        {
            return -1;
        }

        *dataBufIdx += recvLen;
    } else {
        packetLen = ntohs(*(uint16_t*)dataBuf);

        if (packetLen != 0)
        {
            recvLen = wrapRecv(sockfd, &dataBuf[*dataBufIdx], packetLen - *dataBufIdx, 0);
            if (recvLen == 0)
            {
                return -1;
            }
            *dataBufIdx += recvLen;
        } else {
            return readPacketHandleList(sockfd, dataBuf, dataBufIdx);
        }
    }

    if (*dataBufIdx < 3)
    {
        return 0;
    } else {
        packetLen = ntohs(*(uint16_t*)dataBuf);

        if (*dataBufIdx == packetLen)
        {
            return 1;
        }

        return 0;
    }
}

//Helper functions for parsing packets
void decodeInitPacket(char* data, char* handle, int* handleLen)
{
    *handleLen = data[3];
    memcpy(handle, &data[4], *handleLen);
}

void decodeBroadcastPacket(char* data, char* handle, int* handleLen, char* msg, int* msgLen)
{
    uint16_t packetLen;
    int msgOffset;

    *handleLen = data[3];
    memcpy(handle, &data[4], *handleLen);

    packetLen = ntohs(*(uint16_t*)data);

    *msgLen = packetLen;
    *msgLen -= sizeof(PacketHeader_t);
    *msgLen -= *handleLen + 1;

    msgOffset = 4 + *handleLen;

    memcpy(msg, &data[msgOffset], *msgLen);
}

void decodeMessagePacket(char* data,
                         char* srcHandle, int* srcHandleLen,
                         char* dstHandle, int* dstHandleLen,
                         char* msg, int* msgLen)
{
    uint16_t packetLen;
    int index;

    *dstHandleLen = data[3];
    memcpy(dstHandle, &data[4], *dstHandleLen);

    index = 4 + *dstHandleLen;

    *srcHandleLen = data[index];
    memcpy(srcHandle, &data[index + 1], *srcHandleLen);

    packetLen = ntohs(*(uint16_t*)data);

    *msgLen = packetLen;
    *msgLen -= sizeof(PacketHeader_t);
    *msgLen -= *dstHandleLen + 1;
    *msgLen -= *srcHandleLen + 1;

    index += *srcHandleLen + 1;
    memcpy(msg, &data[index], *msgLen);
}

void decodeBadHandlePacket(char* data, char* handle, int* handleLen)
{
    decodeInitPacket(data, handle, handleLen);
}

void decodeHandleListNumPacket(char* data, int* numHandles)
{
    *numHandles = ntohl(*(uint32_t*)(&data[3]));
}

void printHandleListPacket(char* data)
{
    uint32_t numHandles;
    char* handleBuf;

    decodeHandleListNumPacket(data, (int*)&numHandles);

    printf("Number of clients: %d\n", numHandles);

    data += sizeof(PacketHeader_t) + 4;

    handleBuf = malloc(256 * sizeof(char));

    if (handleBuf == NULL)
    {
        perror("malloc");
        exit(-1);
    }

    while (numHandles--)
    {
        memset(handleBuf, 0 ,256);
        memcpy(handleBuf, &data[1], *data);
        printf("  %s\n", handleBuf);
        data += *data + 1;
    }

    free(handleBuf);
}
