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
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <string>
#include <vector>

#include "ServerConnection.h"
#include "ServerClient.h"

using namespace std;

bool ServerConnection::setupServer()
{
   mainSocket = 0;
   struct sockaddr_in local;
   socklen_t len = sizeof(local);
   
   mainSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (mainSocket < 0) {
      cerr << "socket call";
      return false;
   }
   
   local.sin_family= AF_INET;         //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= htons(serverPort);       //let system choose the port 

   /* bind the name (address) to a port */
   if (bind(mainSocket, (struct sockaddr *) &local, sizeof(local)) < 0)
   {
      cerr << "bind call";
      return false;
   }      

   //get the port name and print it out
   if (getsockname(mainSocket, (struct sockaddr*)&local, &len) < 0)
   {
      cerr << "getsockname call";
      return false;
   }

   cout << "socket has port " << ntohs(local.sin_port) << endl;
   serverPort = ntohs(local.sin_port);
	
	if ((listen(mainSocket, 5)) < 0) {
		cerr << "listen call" << endl;
		exit(-1);
	}
   
   return true;
}


void ServerConnection::listenForClient() 
{
   fd_set fdList;
   int max;
   
   clientIndex = -1;
   isNewConnection = false;
   isNewInput = false;
   
   FD_ZERO (&fdList);
	FD_SET(mainSocket, &fdList);
   max = mainSocket;
   
   for (unsigned int i = 0; i < clients.size(); i++) {
      int childSocket = clients.at(i).socket;
      
      FD_SET(childSocket, &fdList);
      
      if (childSocket > max) {
         max = childSocket;
      }
   }
	
   select(max + 1, &fdList, NULL, NULL, NULL);
   
   if (FD_ISSET(mainSocket, &fdList)) 
   {
      isNewConnection = true;
   }
   else 
   {
      for (unsigned int i = 0; i < clients.size(); i++) {
         int childSocket = clients.at(i).socket;
         
         if (FD_ISSET(childSocket, &fdList)) {
            clientIndex = i;
            isNewInput = true;
         }
      }
   }
}
   
void ServerConnection::establishConnection()
{
   int newSocket = accept(mainSocket,(struct sockaddr*)0,(socklen_t *)0);
	
   if (newSocket < 0) {
      cerr << "accept call " << mainSocket << strerror(errno) << endl;
      exit(-1);
   }
   
   clients.push_back(*(new ServerClient(newSocket)));
}

bool ServerConnection::recieveMessage()
{
	clients.at(clientIndex).recvComplete = false;
   int length = 0;
   if (clients.at(clientIndex).packetLength == 0) 
   {
      length = recv(clients.at(clientIndex).socket, clients.at(clientIndex).recvBuffer + 
         clients.at(clientIndex).recvLength, 
         kHeaderSize - clients.at(clientIndex).recvLength, 0);
      if (length == 0) {
			if (!clients.empty()) {
				closeClient(clients.at(clientIndex));
			}
			return false;
      }
      clients.at(clientIndex).recvLength += length;
      
      if (clients.at(clientIndex).recvLength == kHeaderSize)
      {
         NormalHeader header;
         memcpy(&header, clients.at(clientIndex).recvBuffer, kHeaderSize);
         /*cout << "Pack Length:" << ntohs(header.packetLength) <<
         " FLAG RECIEVED:" << (int)header.flag << "FROM" << client.socket<< endl;*/
         clients.at(clientIndex).packetLength = ntohs(header.packetLength);
         if (clients.at(clientIndex).packetLength == kHeaderSize)
         {
            clients.at(clientIndex).recvLength = 0;
            clients.at(clientIndex).packetLength = 0;
            clients.at(clientIndex).recvComplete = true; 
				return true;
         }
      }
   }
   else if (clients.at(clientIndex).packetLength)
   {
      length = recv(clients.at(clientIndex).socket, clients.at(clientIndex).recvBuffer + 
         clients.at(clientIndex).recvLength, 
         clients.at(clientIndex).packetLength - clients.at(clientIndex).recvLength, 0);        
      if (length == 0) {
         closeClient(clients.at(clientIndex));
			return false;
      }
      clients.at(clientIndex).recvLength += length;
      if (clients.at(clientIndex).recvLength == clients.at(clientIndex).packetLength)
      {
         clients.at(clientIndex).recvLength = 0;
         clients.at(clientIndex).packetLength = 0;
			clients.at(clientIndex).recvComplete = true;
			return true;
      }
   }
   
   return false;
}

void ServerConnection::sendMessage(int length, ServerClient &client)
{
   NormalHeader header;
   memcpy(&header, sendBuffer, kHeaderSize);
   
   
   /*cout << "Pack Length:" << ntohs(header.packetLength) <<
   " FLAG Sent:" << (int)header.flag << "TO" << client.socket<< endl;*/
   
   int sent = send(client.socket,sendBuffer,length,0);
      
   if (sent < 0)
   {
      cerr << "send failed" << client.socket << "len: " << length << strerror(errno) << endl;
      exit(-1);
   }      
}

ServerClient& ServerConnection::getClient(string &handle) {
   
   int index = 0;
      
   for (unsigned int i = 0; i < clients.size(); i++) {
      if (clients.at(i).handle.compare(handle) == 0) {
         index = i;
         i = clients.size();
      }
   }
   return clients.at(index);
}

vector<ServerClient> ServerConnection::getAllClients() {

   vector<ServerClient> actives;
   for (unsigned int i = 0; i < clients.size(); i++) {
      if (!clients.at(i).handle.empty()) {
         actives.push_back(clients.at(i));
      }
   }
   return actives;
}

vector<ServerClient> ServerConnection::getOtherClients(ServerClient &client) {
   vector<ServerClient> others;
   for (unsigned int i =0; i < clients.size(); i++) {
      if (!clients.at(i).handle.empty() && clients.at(i).socket != client.socket) {
         others.push_back(clients.at(i));
      }
   }
   return others;
}

void ServerConnection::closeClient(ServerClient &client) {
   int position = -1;
   for (unsigned int i = 0; i < clients.size(); i++) {
      if (client.socket == clients.at(i).socket) {
         position = i;
         i = clients.size();
      }
   }
	if (position != -1) {
		close(clients.at(position).socket);
		clients.erase(clients.begin() + position);
	}
}

char* ServerConnection::getSendBuffer() {
   return sendBuffer;
}

ServerClient& ServerConnection::getRecvClient() {
	return clients.at(clientIndex);
}


bool ServerConnection::hasNewConnection() {
   return isNewConnection;
}

bool ServerConnection::hasNewInput() {
   return isNewInput;
}

bool ServerConnection::containsHandle(string handle) {
   bool contains = false;
   for (unsigned int i = 0; i < clients.size(); i++) {
      if (clients.at(i).handle.compare(handle) == 0) {
         contains = true;
      }
   }
   return contains;
}


ServerConnection::~ServerConnection() {
   free(sendBuffer);
}