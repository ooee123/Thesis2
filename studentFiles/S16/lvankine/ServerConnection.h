#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <vector>
#include <string>
#include <stdlib.h>
#include "ServerClient.h"
#include "PacketStructs.h"

using namespace std;

class ServerConnection
{

   private:
   
      int mainSocket;
      uint16_t serverPort;
      bool isNewConnection;
      bool isNewInput;
      vector<ServerClient> clients;
      int clientIndex;
      char *sendBuffer;
   
   public:
   
      ServerConnection (uint16_t serverPort): 
		serverPort(serverPort), isNewConnection(false), isNewInput(false),
		clientIndex(-1), sendBuffer((char*)malloc(kBufferSize)){}
      
      bool setupServer();
      void listenForClient();
      bool hasNewConnection();
      void establishConnection();
      bool hasNewInput();
      bool recieveMessage(); 
      void sendMessage(int length, ServerClient &client);
      char* getSendBuffer();
      ServerClient& getClient(string &handle);
		ServerClient& getRecvClient();
      vector<ServerClient> getAllClients();
      vector<ServerClient> getOtherClients(ServerClient &client);
      bool containsHandle(string handle);
      void closeClient(ServerClient &client);
      ~ServerConnection();
};

#endif
