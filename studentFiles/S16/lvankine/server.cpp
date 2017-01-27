#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#include "testing.h"
#include "ServerConnection.h"
#include "ServerApp.h"

using namespace std;

int main(int argc, char *argv[])
{

   uint16_t port = 0;
   
   if (argc == 2)
   {
      port = atoi(argv[1]);
   }
   
   ServerConnection connection(port);
   
   if (!connection.setupServer())
   {
      cout << "Error setting up server" << endl;
      exit(1);
   }

   ServerApp app (connection);
	app.run();
   
}

