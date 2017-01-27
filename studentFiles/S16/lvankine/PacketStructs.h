#ifndef PACKETSTRUCTS_H
#define PACKETSTRUCTS_H

#pragma pack(1)

#include <stdint.h>


const int kHeaderSize = 3;
const int kHandleFlag = 1;
const int kGoodHandle = 2;
const int kBadHandle = 3;
const int kBroadcastFlag = 4;
const int kMessageFlag = 5;
const int kErrorFlag = 7;
const int kExitRequest = 8;
const int kExitFlag = 9;
const int kRequestHandle = 10;
const int kHandleLength = 11;
const int kHandleList = 12;
const int kMaxMessageLength = 1000;
const int kBufferSize = 1513;

struct NormalHeader 
{
   uint16_t packetLength;
   unsigned char flag;
};

#endif