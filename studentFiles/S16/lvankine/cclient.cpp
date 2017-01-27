#include <iostream>
#include <string>
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
#include "ClientApp.h"

using namespace std;

int main(int argc, char * argv[])
{
   /* check command line arguments  */
   if(argc!= 4)
   {
      cerr << "usage: " << argv[0] << " client-handle host-name port-number";
      exit(1);
   }
   
	string handle (argv[1]);
   ClientConnection connection(handle, argv[2], (uint16_t)atoi(argv[3]));
   connection.setupConnection();
   
   
   /* check if handler is good */
   if (!connection.verifyHandle())
   {
      cerr << "Handle already in use: " << connection.getUsername() << endl;
      exit(1);
   }
   
   ClientApp app(connection);
   app.run();
   
   connection.closeConnection();
   
   return 0;
}

