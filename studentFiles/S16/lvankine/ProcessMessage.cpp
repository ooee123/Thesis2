#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <vector>
#include <string>
#include "ProcessMessage.h"

using namespace std;


string ProcessMessage::analyzeBuffer()
{
	memcpy(&header, recvBuffer, kHeaderSize);
   packetLength = ntohs(header.packetLength);
   switch (header.flag)
   {
      case kBroadcastFlag:
         return ProcessBroadcast();
         break;
      
      case kMessageFlag:
         return ProcessDirect();
         break;
      
      case kErrorFlag:
         return ProcessError();
         break;
      
      case kExitFlag:
         connection.enableClientExit();
         return "";
         break;
      
      case kHandleLength:
         uint32_t length;
         memcpy(&length, recvBuffer + kHeaderSize, 4);
         connection.setHandleLength(ntohl(length));
         return "";
         break;
      
      default:
         return "";
   }
}

string ProcessMessage::ProcessBroadcast()
{
   unsigned char handleLength;
   memcpy(&handleLength,recvBuffer + kHeaderSize, 1);
   string handle(recvBuffer + kHeaderSize + 1, handleLength);
   string message(recvBuffer + kHeaderSize + 1 + handleLength);
   
   return handle + ": " + message;
}

string ProcessMessage::ProcessDirect()
{
   unsigned char destLength;
   unsigned char srcLength;
   memcpy(&destLength,recvBuffer + kHeaderSize, 1);
   memcpy(&srcLength, recvBuffer + kHeaderSize + 1 + destLength, 1);
   string srcHandle(recvBuffer + kHeaderSize + 1 + destLength + 1, srcLength);
   string message(recvBuffer + kHeaderSize + 1 + destLength + 1 + srcLength);
   
   return srcHandle + ": " + message;
}

string ProcessMessage::ProcessError()
{
   unsigned char handleLength;
   memcpy(&handleLength,recvBuffer + kHeaderSize, 1);
   string handle(recvBuffer + kHeaderSize + 1, handleLength);
   return "Client with handle " + handle + " does not exist";
}
