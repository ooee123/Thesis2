#include <string.h>
#include <string>
#include <netinet/in.h>
#include "ConstructMessage.h"
#include <iostream>


using namespace std;



bool ConstructMessage::constructMessage()
{
   if (input.at(0) != '%')
   {
   	return false;
   }
   char type = input.at(1);
	
   switch (type)
   {
		case 'm':
      case 'M':
      	return constructDirect();
      	break;
      	
      case 'b':
      case 'B':
      	return constructBroadcast();
      	break;
      	
      case 'l':
      case 'L':
      	return constructHandles();
      	break;
      	
      case 'e':
      case 'E':
      	return constructExit();
      	break;
      	
      default:
      	return false;
   }
	return false;
}

bool ConstructMessage::constructDirect()
{
	string message(input,3);
   string::size_type pos = message.find(' ', 0);
   string destName = message.substr(0,pos);
   message = message.substr(pos + 1);
   string::size_type pos1 = message.find(' ', 0);   	
   string srcName = message.substr(0,pos1);
   string text = message.substr(pos1 + 1);
   unsigned destLength = destName.length();
   unsigned char srcLength = srcName.length();
   
   if (text.empty() || (srcName.compare(text) == 0)) {
   	text = " ";
   }
   	
   int textLength = text.length() + 1;
   if (textLength > kMaxMessageLength)
   {
      int messageRemaining = textLength;
      int messageSent = 0;
      while (messageRemaining > kMaxMessageLength)
      {
         
         header.packetLength = htons(kHeaderSize + 1 + destLength + 1 + srcLength + kMaxMessageLength);
         header.flag = kMessageFlag;
         string subText (messageSent,kMaxMessageLength-1);
         memcpy(buffer, &header, kHeaderSize);
         memcpy(buffer + kHeaderSize, &destLength, 1);
         memcpy(buffer + kHeaderSize + 1, destName.c_str(),destLength);
         memcpy(buffer + kHeaderSize + 1 + destLength, &srcLength, 1);
         memcpy(buffer + kHeaderSize + 1 + destLength + 1, srcName.c_str(),srcLength);
         memcpy(buffer + kHeaderSize + 1 + destLength + 1 + srcLength, subText.c_str(), kMaxMessageLength);
         connection.setSendLength(ntohs(header.packetLength));
         connection.sendMessage(buffer);
         messageSent += kMaxMessageLength;
         messageRemaining -= kMaxMessageLength - 1;
      }
      textLength = messageRemaining;
   }
   header.packetLength = htons(kHeaderSize + 1 + destLength + 1 + srcLength + textLength);
   header.flag = kMessageFlag;
	memcpy(buffer, &header, kHeaderSize);
   memcpy(buffer + kHeaderSize, &destLength, 1);
   memcpy(buffer + kHeaderSize + 1, destName.c_str(),destLength);
   memcpy(buffer + kHeaderSize + 1 + destLength, &srcLength, 1);
   memcpy(buffer + kHeaderSize + 1 + destLength + 1, srcName.c_str(),srcLength);
   memcpy(buffer + kHeaderSize + 1 + destLength + 1 + srcLength, text.c_str(), textLength);
   connection.setSendLength(ntohs(header.packetLength));
   return true;
}

bool ConstructMessage::constructBroadcast()
{
	string message(input,3);
   string srcName = connection.getUsername();
   unsigned char srcLength = srcName.length();
   
   if (message.empty()) {
   	message = " ";
   }
   int messageLength = message.length() + 1;
   header.packetLength = htons(kHeaderSize + 1 + srcLength + messageLength);
   header.flag = kBroadcastFlag;
   memcpy(buffer, &header, kHeaderSize);
   memcpy(buffer + kHeaderSize, &srcLength, 1);
   memcpy(buffer + kHeaderSize + 1, srcName.c_str(), srcLength);
   memcpy(buffer + kHeaderSize + 1 + srcLength, message.c_str(), messageLength);
   connection.setSendLength(ntohs(header.packetLength));
   return true;   
}

bool ConstructMessage::constructHandles()
{
   header.packetLength = htons(kHeaderSize);
   header.flag = kRequestHandle;
   memcpy(buffer, &header, kHeaderSize);
   connection.setSendLength(ntohs(header.packetLength));
   return true;
}

bool ConstructMessage::constructExit()
{
   header.packetLength = htons(kHeaderSize);
   header.flag = kExitRequest;
   memcpy(buffer, &header, kHeaderSize);
   connection.setSendLength(ntohs(header.packetLength));
   return true;   
   
}