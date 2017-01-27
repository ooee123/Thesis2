#ifndef SERVERAPP_H
#define SERVERAPP_H

#include "ServerConnection.h"
#include "ServerClient.h"

class ServerApp
{
   private:
      ServerConnection connection;
		void ProcessClientMessage (ServerClient &client);
      void ConstructHandle(ServerClient &client);
		void ConstructBroadcast(ServerClient &client);
		void ConstructMessage(ServerClient &client);
      void ConstructExit(ServerClient &client);
      void ConstructHandleList(ServerClient &client);
      
   public:
      ServerApp(ServerConnection &connection): 
		connection(connection){};
      void run();
};

#endif