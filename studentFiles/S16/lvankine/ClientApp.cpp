#include <iostream>
#include <string>
#include <stdlib.h>  
#include "ClientApp.h"

using namespace std;

void ClientApp::run()
{
   cout << "$:" << flush;
   while (!connection.getExitStatus())
   {     
      connection.listen();
      if (connection.isUserInput())
      {
         string input;
         getline(cin, input);
         ConstructMessage sMessage(input, connection);
         if(!sMessage.constructMessage())
         {
            cout << "Invalid command" << endl;
				cout << "$:" << flush;
         }
         else {
            connection.sendMessage(connection.getSendBuffer());
				cout << "$:" << flush;
         }
      }
      else if (connection.isServerInput())
      {
         if (connection.receiveMessage())
         {
            ProcessMessage rMessage (connection);
            string message = rMessage.analyzeBuffer();
            if (!message.empty())
            {
               cout << endl << message << endl;
               cout << "$:" << flush;
            }
         }
      }
      else 
      {
         cerr << "Select Statement returned with no input ready";
         exit(1);
      }
   }
}

