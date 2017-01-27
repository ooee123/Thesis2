#ifndef SERVERCLIENT_H
#define SERVERCLIENT_H

#include <stdlib.h>
#include <string.h>
#include <string>
#include "PacketStructs.h"

using namespace std;

class ServerClient
{
   public:
      string handle;
      int socket;
      int packetLength;
      int recvLength;
		bool recvComplete;
      char *recvBuffer;
      
      ServerClient(int socket): 
		socket(socket), packetLength(0), recvLength(0), 
		recvComplete(false), recvBuffer((char*)malloc(kBufferSize)){}
		ServerClient(const ServerClient& other);
		~ServerClient();
};

inline ServerClient::ServerClient(const ServerClient& other) {
   handle = other.handle;
   socket = other.socket;
   packetLength = other.packetLength;
   recvLength = other.recvLength;
   recvComplete = other.recvComplete;
   recvBuffer = (char*)malloc(kBufferSize);
   memcpy(recvBuffer, other.recvBuffer, kBufferSize);
}

inline ServerClient::~ServerClient() {
   free(recvBuffer);
}

#endif