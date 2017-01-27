/******************************************************************************
 * tcp_server.c
 * Alexander DeMello (ademello)
 * CPE 464 - Program 1
 *****************************************************************************/

#include <stdio.h>
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
#include "networks.h"

user * users;
int server_socket = 0;
int numUsers;
int maxUsers;
char * buf;
fd_set tempList, socketList;

int handleDuplicateChecker(char* inHandle) {
   int i;
   for(i = 0; i <maxUsers; i++) {
      if(users[i].handle && strcmp(users[i].handle, inHandle) == 0) {
         return 1;
      }  
   }
   return 0;
}

void sendPacket(int client_socket, char *toSend, int sendSize) {
   if(send(client_socket, toSend, sendSize, 0) < 0) {
      perror("sending packet");
      exit(-1);
   }
}

void sendGoodInitial(int client_socket) {
   int len = 3;
   char * packet = calloc(1, len);
  
   memset(packet, htons(3), 2);
   memset(packet + 2, FLAG_GOOD, 1);
  
   sendPacket(client_socket, packet, 3);

}

void sendBadInitial(int client_socket) {
   int len = 3;
   char * packet = calloc(1, len);

   memset(packet, htons(3), 2);
   memset(packet + 2, FLAG_BAD, 1);

   sendPacket(client_socket, packet, 3);

}

void readHandle(int client_socket, char* buf) {
   char* userHandle;
   char handleLen;
   int i ,message_len;
   uint16_t buffLen=0;

   if ((message_len = recv(client_socket, &buffLen, 2, 0)) == 0) {
      FD_CLR(client_socket, &socketList);
      close(client_socket);
   }
   buffLen = ntohs(buffLen);

   if((recv(client_socket, buf + 2, buffLen - 2, 0)) < 1) {
      perror("recv form client");
      exit(-1);
   } 

   handleLen = buf[3];
   userHandle = (char *) calloc(1, handleLen +1);

   memcpy(userHandle, buf + 4, handleLen);
   userHandle[(int)handleLen] = '\0';

   if(handleDuplicateChecker(userHandle) == 1) {
      sendBadInitial(client_socket);
   }
   else {
      for(i =0; i < numUsers; i++) {
         if(users[i].socket == client_socket) {
            break;
         }
      }
      users[i].handle = userHandle;
      strcpy(users[i].handle, userHandle);

      sendGoodInitial(client_socket);
   }
}

int newUser() {
   int i, client_socket = 0;
   if ((client_socket = accept(server_socket, 
               (struct sockaddr*)0, (socklen_t *)0)) < 0) {
      perror("accept call");
      exit(-1);
   }
   users[numUsers++].socket = client_socket;

   if(numUsers == maxUsers) {
      maxUsers = maxUsers *2;
      user *temp = users;
      users = realloc(users, sizeof(user) * maxUsers);
      
      for(i = 0; i < numUsers; i ++) {
         users[i] = temp[i];
      }
   }

   return client_socket;
}

int findHandle(char * tempName) {
   int i;

   for(i=0; i < numUsers; i++) {
      if(strcmp(tempName, users[i].handle) == 0) {
         return i;
      }
   }

   return -5;
}

void recieveMessage(int sourceSocket, int packetLen) {
   uint8_t destLen, handleIndex = 0;
   char * tempName, *errorPacket;
   uint16_t netPacketLen;
   
   destLen = buf[3];
   tempName = calloc(1, destLen + 1);
   memcpy(tempName, buf + 4, destLen);
   tempName[destLen] = '\0';
   handleIndex = findHandle(tempName);
   
   if(handleIndex < 0) {
      errorPacket = calloc(1, 3 + 1 + destLen);
      netPacketLen = htons(4 + destLen);

      memcpy(errorPacket, &netPacketLen, 2);
      memset(errorPacket + 2, FLAG_MSG_ERROR, 1);
      memset(errorPacket + 3, destLen, 1);
      memcpy(errorPacket + 4, tempName, destLen);

      sendPacket(sourceSocket, errorPacket, 3);
   }
   else {
      netPacketLen = htons(packetLen);
      memcpy(buf, &netPacketLen, 2);

      sendPacket(users[handleIndex].socket, buf, packetLen);  
   }
}

void recieveBroadCast(int sourceSocket, int packetLen) {
   uint16_t netPacketLen;
   int i;

   netPacketLen = htons(packetLen);
   memcpy(buf, &netPacketLen, 2);

   for(i =0; (users + i) != NULL; i++) {
      if(users[i].socket != sourceSocket) {
         sendPacket(users[i].socket, buf, packetLen);
      }
   }
}

void recievePacket(int index) {
   int flag,  message_len;
   uint16_t buffLen;

   if ((message_len = recv(index, &buffLen, 2, 0)) == 0) {
      FD_CLR(index, &socketList);
      close(index);
   }
   buffLen = ntohs(buffLen);

   if((message_len = recv(index, buf + 2, buffLen -2, 0)) < 1) {
      perror("recv form client");
      exit(-1);
   }
   flag = buf[2];

   switch(flag) {

      case FLAG_BROADCAST:
         break;

      case FLAG_MSG:
         recieveMessage(index, message_len + 2);
         break;

      case FLAG_SERVER_EXIT:
         break;

      case FLAG_REQ_CLIENT_LIST:
         break;
   }
}

void mainServerLoop() {
   int i;
   int client_socket= 0;   //socket descriptor for the client socket
   int maxClient = 0;
   FD_ZERO(&socketList);

   //create packet buffer
   buf = (char *) malloc(1100);

   //create the server socket
   server_socket= tcp_recv_setup();
   if(listen(server_socket, 5) < 0) {
      perror("-1");
   }

   FD_SET(server_socket, &socketList);
   maxClient = server_socket;

   while(1) {
      tempList = socketList;
      if(select(maxClient + 1, &tempList, NULL, NULL, NULL) == -1) {
         perror("select");
         exit(2);
      }

      for(i=0; i <= maxClient; i++) {
         if(FD_ISSET(i, &tempList)) { 
            if(i == server_socket) {
               client_socket = newUser(); 
               if(client_socket == -1) {
                  perror("first packet recv");
               }
               else {
                  FD_SET(client_socket, &socketList);
                  if(client_socket > maxClient) {
                     maxClient = client_socket;   
                  }
               }
               readHandle(client_socket, buf);
            }
            else {
               recievePacket(i);
            }
         }
      }
   }
}


int main(int argc, char *argv[])
{
   users = (user *) calloc(maxUsers,sizeof(user));
   mainServerLoop();
   
   return 0;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup()
{
    struct sockaddr_in local;      /* socket address for local side  */
    socklen_t len= sizeof(local);  /* length of local address        */

    /* create the socket  */
    server_socket= socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
    {
      perror("socket call");
      exit(1);
    }

    local.sin_family= AF_INET;         //internet family
    local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
    local.sin_port= htons(0);                 //let system choose the port

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

    printf("Server has port %d \n", ntohs(local.sin_port));
	        
    return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

int tcp_listen(int back_log)
{
   int client_socket= 0;
   if (listen(server_socket, back_log) < 0) {
      perror("listen call");
      exit(-1);
   }
  
   if ((client_socket= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
      perror("accept call");
      exit(-1);
   }
  
   return(client_socket);
}


