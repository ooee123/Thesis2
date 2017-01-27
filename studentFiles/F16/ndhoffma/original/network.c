#include "network.h"

int clientInit(char *serverName, char *portNum) {
   int sd;
   struct sockaddr_in svr;
   struct hostent *host = NULL;

   /* create socket, file descriptor */
   if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("Error creating socket: ");
      exit(-1);
   }

   svr.sin_family = AF_INET;  // set to IPv4 address type

   /* getting the hostent struct of server from string name */
   if ((host = gethostbyname(serverName)) == NULL) {
      printf("Error getting server address: %s\n", serverName);
      exit(-1);
   }

   /* extracting the servers IP address from hostent struct */
   memcpy((char *)&svr.sin_addr, (char *)host->h_addr, host->h_length);

   /* convert string of port number into network order short value */
   svr.sin_port = htons(atoi(portNum));

   /* connect to server */
   if (connect(sd, (struct sockaddr *)&svr, sizeof(struct sockaddr_in)) < 0) {
      perror("Error connecting client to server: ");
      exit(-1);
   }

   return sd;
}

int parseClientArgs(int argc, char *argv[], char **handle) {
   if (argc != 4) {
      printf("Error in parameters:\t\"./cclient %s [server-name] [port-number]\"\n", argv[0]);
      exit(1);
   }

   *handle = argv[1];

   return clientInit(argv[2], argv[3]);
}

int serverInit(char *portNum) {
   int sd;
   struct sockaddr_in svr;
   socklen_t addrLen;

   /* create socket, file descriptor */
   if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("Error creating socket: ");
      exit(-1);
   }
 
   svr.sin_family= AF_INET;            // set type to IPv4
   svr.sin_addr.s_addr= INADDR_ANY;    // wild card machine address
   svr.sin_port= htons(0);             // let system choose the port

   /* bind the IP addr to the port */
   if (bind(sd, (struct sockaddr *)&svr, sizeof(svr)) < 0) {
      perror("Error binding server: ");
      exit(-1);
   }

   /* get the IP address of this socket descriptor */
   addrLen = sizeof(svr);
   if (getsockname(sd, (struct sockaddr *)&svr, &addrLen) < 0) {
      perror("Error getting socket name: ");
      exit(-1);
   }

   /* designate socket as passive, wait for connect() calls from clients */
   if (listen(sd, BACKLOG) < 0) {
      perror("Error in listen call: ");
      exit(-1);
   }

   printf("Server socket has port number: %d\n", ntohs(svr.sin_port));

   return sd;
}

int serverAccept(int server_socket) {
   int client_socket;

   if ((client_socket = accept(server_socket, (struct sockaddr *)0, (socklen_t *)0)) < 0) {
      perror("Error accepting client: ");
      exit(-1);
   }

   return client_socket;
}

int parseServerArgs(int argc, char *argv[]) {

   if (argc < 1 && argc > 2) {
      printf("Error in parameters:\t\"./server [optional-port-number]\"\n");
      exit(1);
   }

   if (argc == 2) {
      return serverInit(argv[1]);
   }

   return serverInit(NULL);
}

















