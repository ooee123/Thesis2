#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include "ClientConnection.h"
#include "ConstructMessage.h"
#include "ProcessMessage.h"

class ClientApp
{
   private:
   
      ClientConnection &connection;
      
   public:
   
      ClientApp(ClientConnection &connection): connection(connection){}
		
      void run();      
};

#endif