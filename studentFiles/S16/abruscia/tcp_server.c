/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
 *****************************************************************************/

#include "tcp.h"

static int numClientsConnected = 0;
static int maxNumClients;
static int largestSockNum = 3;  //starts with server having largest sockNum
static client* clients;

void terminateClient(int sockNum) {
      int i = 0;

      //Remove client from list
      while (i < numClientsConnected && clients[i].sockNum != sockNum) {   
          i++;
      }
    
    //slide clients down to delete client at clients[i] if i isn't last in the list
    if (i != numClientsConnected - 1) {
       /*memmove((void *) clients + i, (void *) clients + i + 1, sizeof(client) * 
        (numClientsConnected - i));*/
        memcpy(clients + i, clients + i + 1, sizeof(client) * 
        (numClientsConnected - i));
    }

     numClientsConnected--;

     //Readjust the largestSockNum
     i = 0;
      largestSockNum = 3;
      while (i < numClientsConnected) {   
         if (clients[i].sockNum > largestSockNum) {
            largestSockNum = clients[i].sockNum;
         }
         i++;
      }

      close(sockNum);
}

// Function for server and client to recv one packet 
// Pass buf malloced as NORMAL_HEAD_LEN
// Returns length of buffer in host order
uint16_t recvFuncServer(int socketNum, char *buf) {
   int bytesReceived = 0;
   int index = 0;
   uint16_t mesgLen = 0;
   
   // put header into buffer
   while (index < NORMAL_HEAD_LEN) {
      bytesReceived = recv(socketNum, buf + index, NORMAL_HEAD_LEN - 
       bytesReceived, 0); //get 3 bytes of header
      index += bytesReceived;

      if (bytesReceived < 0) {
         perror("Error in bytes Received");
         //exit(-1);
      }

      if (bytesReceived == 0) {
         terminateClient(socketNum);
         return 0;
      }
   }

   memcpy(&mesgLen, buf, FLAG_OFFSET);
   mesgLen = ntohs(mesgLen);

   buf = realloc(buf, mesgLen);
   
   // Retrieve message len worth
   while (index < mesgLen) {
      bytesReceived = recv(socketNum, buf + index, mesgLen - bytesReceived, 0); //get 3 bytes of header
      index += bytesReceived;

      if (bytesReceived < 0) {
         perror("Error in bytes Received");
         exit(-1);
      }

      if (bytesReceived == 0) {
         terminateClient(socketNum);
         return 0;
      }
   }


   return mesgLen;  
}


void handleNotExistPacketSend(int sockNum, char *data, char *handle, client *clients) {
   int sent = -1, index = 0;
   uint16_t packetLen, packetLenNet;
   uint8_t flag = FLAG_SEVEN;
   uint8_t handleLen = strlen(handle);
   char *sendBuf;

   packetLen = NORMAL_HEAD_LEN + 1 + handleLen;
   packetLenNet = htons(packetLen);

   sendBuf = malloc(packetLen);

   memcpy(sendBuf, &packetLenNet, index += 2);
   memcpy(sendBuf + index, &flag, 1);
   index += 1;
   memcpy(sendBuf + index, &handleLen, 1);
   index += 1;
   memcpy(sendBuf + index, handle, handleLen);

   sent =  send(sockNum, sendBuf, packetLen, 0);

   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

//add 1 to hanleLen because handle passed in has terminating char added
void sendToThisHandle(int sockNum, char *data, char *handle, client *clients) {
   int i = 0, sent = -1;
   uint16_t packetLenNet; 

   memcpy(&packetLenNet, data, FLAG_OFFSET);

   while (i < numClientsConnected && strcmp(clients[i].handle, handle)) {   
      i++;
   }

   if (i == numClientsConnected) {
      handleNotExistPacketSend(sockNum, data, handle, clients);
   }
   else {
      sent =  send(clients[i].sockNum, data, ntohs(packetLenNet), 0);  //***

      if(sent < 0)
      {
         perror("send call");
         exit(-1);
      }
   }
}

//get dest handle and add terminating char
void sendMesg(int sockNum, char *data, client *clients) {
   char handle[256];
   uint8_t handleLen = data[NORMAL_HEAD_LEN];

   memcpy(handle, data + NORMAL_HEAD_LEN + 1, handleLen);
   handle[handleLen] = NULL_CHAR;

   sendToThisHandle(sockNum, data, handle, clients);
}

void sendClientExitAck(int sockNum, char *data) {
   char sendBuf[NORMAL_HEAD_LEN];
   uint16_t packetLen = NORMAL_HEAD_LEN;
   uint16_t packetLenNet = htons(packetLen);
   uint8_t flag = FLAG_NINE;  //Ack to exit
   int sent = 0;

   //Packet telling client to exit
   memcpy(sendBuf, &packetLenNet, FLAG_OFFSET);
   memcpy(sendBuf + FLAG_OFFSET, &flag, 1); 

   sent =  send(sockNum, sendBuf, packetLen, 0);  //***
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

//sockNum is the client who the message is coming from
void sendBroadcast(int sockNum, char *data) {
   int sent = 0, i = 0;
   uint16_t packetLen, packetLenNet;
      
   memcpy(&packetLenNet, data, FLAG_OFFSET);
   packetLen = ntohs(packetLenNet);
      
   while (i < numClientsConnected) {
      if(clients[i].sockNum != sockNum) {
         sent = send(clients[i].sockNum, data, packetLen, 0);  //***
         if(sent < 0) {
            perror("send call");
            exit(-1);
         }
      }
      i++;
   }
}

//send list of handles to sockNum in two packets
void sendHandlesPart1(int sockNum) {
   char sendBuf[NORMAL_HEAD_LEN + sizeof(int)];  //normal header + int
   uint16_t packetLen = NORMAL_HEAD_LEN + sizeof(int);
   uint16_t packetLenNet = htons(packetLen);
   uint8_t flag = FLAG_ELEVEN;  
   int sent = 0;  
   int numOfHandlesNet = htons(numClientsConnected);


   memcpy(sendBuf, &packetLenNet, FLAG_OFFSET);
   memcpy(sendBuf + FLAG_OFFSET, &flag, 1); 
   memcpy(sendBuf + FLAG_OFFSET + 1, &numOfHandlesNet, sizeof(int));

   sent =  send(sockNum, sendBuf, packetLen, 0);  //***
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

void sendHandlesPart2(int sockNum) {
   //char sendBuf[NORMAL_HEAD_LEN + numClientsConnected * 256]; 
   char sendBuf[255];
   uint8_t flag = FLAG_TWELVE, handleLen;  
   int sent = 0, index = NORMAL_HEAD_LEN, i, packetLenNet; 
   //int zero = 0;

   //memcpy(sendBuf, &zero, FLAG_OFFSET);
   memcpy(sendBuf + FLAG_OFFSET, &flag, 1); 

   for (i = 0; i < numClientsConnected; i++) {
      handleLen = clients[i].handleLen;
      memset(sendBuf + index, handleLen, 1);
      index += 1;
      memcpy(sendBuf + index, clients[i].handle, handleLen);
      index += handleLen;
   }

   packetLenNet = htons(index);
   memcpy(sendBuf, &packetLenNet, FLAG_OFFSET);

   sent =  send(sockNum, sendBuf, index, 0);  //***
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}


//select has returned and ISSET determined that client with sockNum is 
// ready to be serviced
// returns -1 if client terminated
int serviceClient(int sockNum, client *clients) {
   char *data = malloc(NORMAL_HEAD_LEN);
   int responseFlag = -1;

   if (recvFuncServer(sockNum, data) == 0) {  //no data because client terminated
      return -1;
   }

   responseFlag = data[FLAG_OFFSET]; 

   switch(responseFlag) {
      case FLAG_FOUR:  //broadcast
         sendBroadcast(sockNum, data);
         break;
      case FLAG_FIVE:  //message
         sendMesg(sockNum, data, clients);
         break;
      case FLAG_EIGHT:  //client exiting
         sendClientExitAck(sockNum, data);
         break;
      case FLAG_TEN:   //list of handles
         sendHandlesPart1(sockNum);
         sendHandlesPart2(sockNum);
         break;
      default:
         break;
   }

   return 1;
}

fd_set setAndSelect(int serverSock, client *clients) {
   fd_set fdvar; 
   int i;

   // Block and wait for response
   FD_ZERO(&fdvar);  //reset variables
   FD_SET(serverSock, &fdvar);  //set to monitor server's socket

   for (i = 0; i < numClientsConnected; i++) {   //change this**
      FD_SET(clients[i].sockNum, &fdvar);  //sockNum is field in struct
   }

   // SELECT  
   if(select(largestSockNum + 1, &fdvar, NULL, NULL, NULL) < 0) {
     perror("select setAndSelect");
     exit(-1);
   } //*****
   return fdvar;
}


void newClientSendPacket(int sockNum, uint8_t flag) {
   char sendBuf[NORMAL_HEAD_LEN];
   uint16_t packetLen = NORMAL_HEAD_LEN;
   uint16_t packetLenNet = htons(packetLen);
   int sent = 0;
   
   memcpy(sendBuf, &packetLenNet, FLAG_OFFSET);
   memcpy(sendBuf + FLAG_OFFSET, &flag, 1); 

   sent =  send(sockNum, sendBuf, packetLen, 0);  //***
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

//Returns socketNum of newly connected client
int newClientConnect(int serverSock, client *clients) {
   int clientSock = 0, i;
   uint16_t mesgLen = 0;
   uint8_t handleLen = 0;
   char handle[256];
   char *data = malloc(NORMAL_HEAD_LEN);

   //accept the new client and give them a socket
   if ((clientSock = accept(serverSock, (struct sockaddr*)0, (socklen_t *)0)) < 0)
   {
      perror("accept call");
      exit(-1);
   }

   if (numClientsConnected == maxNumClients) {
      maxNumClients += 50;
      clients = realloc(clients, sizeof(client) * maxNumClients);
   }

   mesgLen = recvFuncServer(clientSock, data);
   handleLen = data[NORMAL_HEAD_LEN];
   memcpy(handle, data + NORMAL_HEAD_LEN + 1, handleLen);
   handle[handleLen++] = NULL_CHAR;

      for(i = 0; i < numClientsConnected; i++) {
         if (strcmp(clients[i].handle, handle) == 0) {  //handle already in use
            newClientSendPacket(clientSock, FLAG_THREE);
            return -1;
         } 
      }
      //good new handle, add to table
      clients[numClientsConnected].sockNum = clientSock;
      clients[numClientsConnected].handleLen = handleLen;
      memcpy(&(clients[numClientsConnected].handle), handle, handleLen);

      newClientSendPacket(clientSock, FLAG_TWO);
      numClientsConnected++;
      if (clientSock > largestSockNum) {
         largestSockNum = clientSock;
      }

   return clientSock;
}

void serve(int serverSock, client *clients) {
   fd_set fdvar;
   int i;

   // Block and wait for response
   fdvar = setAndSelect(serverSock, clients);  //returned from select, data ready to read

   for (i = 0; i < numClientsConnected; i++) {
      if (FD_ISSET(clients[i].sockNum, &fdvar)) {
         serviceClient(clients[i].sockNum, clients); 
      }
   }

   if (FD_ISSET(serverSock, &fdvar)) {   //new client connecting
      newClientConnect(serverSock, clients);
   }
}


int main(int argc, char *argv[])
{
    int serverSock = 0;   //socket descriptor for the server socket
    int portNum = 0;      //port number that user is requesting, if they don't specify, 0 assigns
    
    clients = calloc(sizeof(client), 50);  //start with list holding 50 clients
    maxNumClients = 50; 
    
    // init server
    if (argc > 1) {
       portNum = atoi(argv[1]);
       serverSock = tcp_recv_setup(portNum);
    }
    else {
       serverSock = tcp_recv_setup(portNum);
    }

    //look for a client to serve
    if (listen(serverSock, 5) < 0) { 
       perror("listen call");
       exit(-1);
    }

    while (1) {
       serve(serverSock, clients); 
    }

   return 0; 
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */
int tcp_recv_setup(int portNum)
{
    int server_socket= 0;
    struct sockaddr_in local;      /* socket address for local side  */
    socklen_t len = sizeof(local);  /* length of local address        */

    /* create the socket  */
    server_socket= socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
    {
       perror("socket call");
       exit(1);
    }

    local.sin_family= AF_INET;         //internet family
    local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
    local.sin_port= htons(portNum);                 //let system choose the port

    /* bind the name (address) to a port */
    if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
    {
	     perror("bind call");
	     exit(-1);
    }
    
    //get the port name and print it out
    if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0)
    {
	     perror("getsockname call");
	     exit(-1);
    }

    printf("Server is using port %d \n", ntohs(local.sin_port));
	        
    return server_socket;
}

