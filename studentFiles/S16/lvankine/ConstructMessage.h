#ifndef CONSTRUCTMESSAGE_H
#define CONSTRUCTMESSAGE_H

#include <string>
#include "ClientConnection.h"

using namespace std;


class ConstructMessage 
{
   private:
		string &input;
      ClientConnection &connection;
		char *buffer;
      NormalHeader header;
      bool constructBroadcast();
      bool constructHandles();
      bool constructExit();
      bool constructDirect();
      
   public:
      ConstructMessage(string &input, ClientConnection &connection):
		input(input), connection(connection), buffer(connection.getSendBuffer()){}
      bool constructMessage();
};

#endif