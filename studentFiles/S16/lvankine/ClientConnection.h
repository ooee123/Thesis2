#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <string>
#include <stdlib.h>
#include "PacketStructs.h"

using namespace std;

class ClientConnection
{
   private:	
		string &username;
		char* host_name;
      uint16_t port;
      bool readUserInput;
      bool readServerInput;
		bool clientExitStatus;
      uint32_t handleLength;      
      char *recvBuffer;
      char *sendBuffer;
		int socketNumber;
      int sendLength;
      int recvLength;
      int packetLength;
		
      void recieveHandleList();
   
   public:
   
      ClientConnection(string &username, char* host_name, uint16_t port):
         username(username), host_name(host_name), port(port), readUserInput(false),
			readServerInput(false), clientExitStatus(false), handleLength(-1), 
			recvBuffer((char*)malloc(kBufferSize)),sendBuffer((char*)malloc(kBufferSize)),
			socketNumber(0), sendLength(0), recvLength(0), packetLength(0){}
         
      void setupConnection();
      bool verifyHandle();
      string getUsername();
      void listen();
      bool isUserInput();
      bool isServerInput();
      void sendMessage(char *buffer);
      bool receiveMessage();
      char *getRecvBuffer();
      char *getSendBuffer();
      void enableClientExit();
      bool getExitStatus();
      void setHandleLength(uint32_t handleLength);
      uint32_t getHandleLength();
      void setSendLength(int sendLength);
      int getSendLength();
      void closeConnection();
      int getSocket(); 
      ~ClientConnection();
};

#endif