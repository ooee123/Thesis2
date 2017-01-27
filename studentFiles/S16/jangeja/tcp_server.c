/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 2
 *****************************************************************************/
#include "testing.h"
#include "tcp_read.h"

int genSocket(int server_socket, int backLog);

Socket *checkHandle(char *handle, SocketBin *socketBin) {
   int i;
   uint8_t length;
   Socket *socket;
   char *temp;
   length = *handle;
   temp = calloc(length + 1, sizeof(char));
   memcpy(temp, handle + 1, length);
   temp[length] = 0;
   for (i = 0; i < socketBin->numSockets; i++) {
      socket = (socketBin->sockets + i);
      if (socket->handle != NULL && strcmp(socket->handle, temp) == 0) {
         free(temp);
         return socket; //Handle Exists
      }
   }
   free(temp);
   return NULL; //Handle DNE
}

void sendToAll(char *packet, SocketBin *socketBin, int clientSocket) {
   int i;
   Socket *socket;
   for (i = 1; i < socketBin->numSockets; i++) {
      socket = (socketBin->sockets + i);
      if (socket->socketNum != clientSocket) {
         if (sendPacket(packet, socket->socketNum, 0)) {
            printf("error sending to socket #:%d\n", (socketBin->sockets + i)->socketNum);
         }
      }
   }
   free(packet);
}



void sendAllHandles(SocketBin *socketBin, int clientSocket) {
   int i, numBytes = 0, sent;
   char *packet;
   char *message = NULL;
   Socket *socket;
   packet = genPacket(NULL, 12, 0, 1);
   if (sendPacket(packet, clientSocket, 3)) {
      printf("Error in sending packet\n");
   }
   for (i = 1; i < socketBin->numSockets; i++) {
      socket = socketBin->sockets + i;
      if (message) {
         message = realloc(message, strlen(socket->handle) + 1 + numBytes);
      }
      else {
         message = calloc(strlen(socket->handle) + 1, sizeof(char));
      }
      *(message + numBytes) = strlen(socket->handle);
      memcpy(message + 1 + numBytes, socket->handle, strlen(socket->handle));
      numBytes += strlen(socket->handle) + 1;
      if (numBytes > 1000) {
         if ((sent = send(clientSocket, packet, numBytes, 0))) {
            printf("Error in sending packet\n");
         }
         free(message);
         message = NULL;
         numBytes = 0;
      }
   }
   if (numBytes) {
      if ((sent = send(clientSocket, message, numBytes, 0)) != numBytes) {
         printf("Error in sending packet\n");
      }
      free(message);
      message = NULL;

   }
}
void action(Socket *activeSocket, SocketBin *socketBin, int *max, int serverSocket) {
   char *buf;           //buffer for receiving from client
   int clientSocket = 0;
   uint32_t *numHandles;
   Header *header;
   char *packet;
   Socket *socket;
   if (activeSocket->socketNum == serverSocket) { //New client
      clientSocket = genSocket(activeSocket->socketNum, 5);
      addSocket(socketBin, NULL, clientSocket);
      if (*max < clientSocket) {
         *max = clientSocket; 
      }
   }
   else {
      clientSocket = activeSocket->socketNum;
      while(!(buf = readDataFromSocket(clientSocket, 2))){
      }
      header = (Header *)buf;
      if ((buf + SIZE_HEADER) && strcmp(buf + SIZE_HEADER, "END") == 0 && header->flag == 0) { //Socket terminated by client
         removeSocket(socketBin, clientSocket);   
         printf("term from client\n");
      }
      else if (header->flag == 1) {
         if (checkHandle((buf + SIZE_HEADER), socketBin) == NULL) { //Availale Handlee
            activeSocket->handle = calloc(strlen(buf + SIZE_HEADER) + 1, sizeof(char));
            memcpy(activeSocket->handle, buf + SIZE_HEADER + 1, *(buf + SIZE_HEADER));   
            packet = genPacket(NULL, 2, SIZE_HEADER, 0);
            if (sendPacket(packet, clientSocket, 1) == 0){ //No error
            }
         }
         else { //handle in use
            packet = genPacket(NULL, 3, SIZE_HEADER, 0);
            if (sendPacket(packet, clientSocket, 1) == 0){ //No error
               removeSocket(socketBin, clientSocket);
            }
         }
      }
      else if (header->flag == 4) { //Broadcast
         packet = genPacket(buf + SIZE_HEADER, 4, ntohs(header->length), 0);
         sendToAll(packet, socketBin, clientSocket); 
      }
      else if (header->flag == 5) { // Client to client send
         if ((socket = checkHandle((buf + SIZE_HEADER), socketBin))) { //valid handle
            packet = genPacket((buf + SIZE_HEADER), 5, ntohs(header->length), 0);
            sendPacket(packet, socket->socketNum, 1);
         }
         else { //Handle does not exist
            packet = genPacket((buf + SIZE_HEADER), 7, ntohs(header->length), 0);
            if (sendPacket(packet, clientSocket, 1)){ //error in send
               printf("Could not send\n");
            }
         } 
      }
      else if (header->flag == 8) { //Client Request to Exit
         packet = genPacket(NULL, 9, SIZE_HEADER, 0);
         sendPacket(packet, clientSocket, 1);
         removeSocket(socketBin, clientSocket);
      }
      else if (header->flag == 10) { //List of handles
         numHandles = calloc(1, sizeof(uint32_t));
         *numHandles = htonl(socketBin->numSockets - 1);
         
         packet = genPacket((char *)numHandles, 11, SIZE_HEADER + 4, 1);
         sendPacket(packet, clientSocket, 1);
         sendAllHandles(socketBin, clientSocket);
      }
      free(buf);      
   }
}

void startServer(int argc, char **argv) {
   int serverSocket= 0;   //socket descriptor for the server socket
   Socket *activeSocket;
   SocketBin *socketBin = calloc(sizeof(SocketBin), 1); //calloc socketBin1
   increaseSocketBin(socketBin);
   int max, i;
   fd_set readFds, writeFds;
   FD_ZERO(&readFds);
   FD_ZERO(&writeFds);
   printf("sockaddr: %d sockaddr_in %d\n", (int)sizeof(struct sockaddr), (int)sizeof(struct sockaddr_in));


   //create the server socket
   if (argc == 2) {
      serverSocket= tcp_recv_setup(argv[1]);
   }
   else {
      serverSocket= tcp_recv_setup(NULL);
   }
   max = serverSocket;
   addSocket(socketBin, "master", serverSocket);
   FD_SET(serverSocket, &readFds);

   while (1) {   
      FD_ZERO(&readFds);
      FD_SET(serverSocket, &readFds);
      max = 0;
      for (i = 0; i < socketBin->numSockets; i++) {
         FD_SET((socketBin->sockets + i)->socketNum, &readFds);
         if (((socketBin->sockets + i)->socketNum) > max) {
            max = (socketBin->sockets + i)->socketNum;
         }
      }
      activeSocket = listenSockets(&readFds, max, socketBin);
      action(activeSocket, socketBin, &max, serverSocket);
   }
   free(socketBin); //free socketBin1
   close(serverSocket);

}
int main(int argc, char *argv[]) {
   
   
   
   //Start up server and listening on port
   startServer(argc, argv);


   /* close the sockets */
   return 0;  
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup(char *port) {
   int server_socket= 0;
   uint16_t portNum= htons(0);
   if (port) {
      portNum = htons(strtol(port, NULL, 10));
   }
   
   struct sockaddr_in local;     /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address      */

   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0) {
      perror("socket call");
      exit(1);
   }

   local.sin_family= AF_INET;       //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= portNum;

   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
      exit(-1);
   }
   
   //get the port name and print it out
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
      perror("getsockname call");
      exit(-1);
   }

   printf("socket has port %d \n", ntohs(local.sin_port));
	      
   return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.   */

int genSocket(int server_socket, int backLog) {
   int client_socket= 0;
   if (listen(server_socket, backLog) < 0) {
       perror("listen call");
       exit(-1);
   }

   if ((client_socket= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
      perror("accept call");
      exit(-1);
   }
  
   return(client_socket);
}

