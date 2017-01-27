#ifndef PROCESSMESSAGE_H
#define PROCESSMESSAGE_H

#include <string>
#include "ClientConnection.h"
#include "PacketStructs.h"

using namespace std;

class ProcessMessage 
{
   private:
      NormalHeader header;
      string message;
      ClientConnection &connection;
		char *recvBuffer;
      int packetLength;
      string ProcessBroadcast();
      string ProcessDirect();
      string ProcessError();
		
   public:
      ProcessMessage(ClientConnection &connection): 
		connection(connection), recvBuffer(connection.getRecvBuffer()){}
		
      string analyzeBuffer();
};

#endif