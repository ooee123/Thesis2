
#include <stdio.h> 
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#include "ClientConnection.h"
#include "PacketStructs.h"

using namespace std;

void ClientConnection::setupConnection() 
{
   struct sockaddr_in remote;       // socket address for remote side
   struct hostent *hp;              // address of remote host
   
   // create the socket
   if ((socketNumber = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
	   cerr << "socket call";
	   exit(-1);
	}
   
   // designate the addressing family
   remote.sin_family= AF_INET;
   
   // get the address of the remote host and store
   if ((hp = gethostbyname(host_name)) == NULL)
	{
      cout << "Error getting hostname: " << host_name << endl;
      exit(-1);
	}
   
   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   // get the port used on the remote side and store
   remote.sin_port= htons(port);

   if(connect(socketNumber, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
   {
      cerr << "connect call FAILED " << strerror(errno) << endl;
      exit(-1);
   }
}

void ClientConnection::listen() {

   fd_set fdList;
   FD_ZERO(&fdList);
   FD_SET(socketNumber, &fdList);
   FD_SET(fileno(stdin), &fdList);
   readServerInput = false;
   readUserInput = false;
   select(socketNumber + 1, &fdList, NULL, NULL, NULL);
   if (FD_ISSET(socketNumber, &fdList))
   {
      readServerInput = true;
   }
   else if (FD_ISSET(fileno(stdin), &fdList))
   {
      readUserInput = true;
   }
}

void ClientConnection::closeConnection() {
   close(socketNumber);
}

void ClientConnection::sendMessage(char *buffer) {
   
   NormalHeader header;
   memcpy(&header, buffer, kHeaderSize);   
   
   /*cout << "Pack Length:" << ntohs(header.packetLength) <<
   " FLAG Sent:" << (int)header.flag << endl;*/
      
   int sent = send(socketNumber, buffer, sendLength, 0);
   
   if (sent < 0)
   {
      cerr << "send failed: " << strerror(errno) << endl;
      exit(-1);
   }   
}

bool ClientConnection::verifyHandle()
{
   NormalHeader header = {};
   
   int messageLength = kHeaderSize + 1 + username.size();
   header.packetLength = htons(messageLength);
   header.flag = 1;
   unsigned char nameLength = username.length();
	
   memcpy(sendBuffer, &header, kHeaderSize);
   memcpy(sendBuffer + kHeaderSize, &nameLength, 1);
   memcpy(sendBuffer + kHeaderSize + 1, username.c_str(), nameLength);
	
   int sent = send(socketNumber, sendBuffer, messageLength, 0);
   
   if (sent < 0)
   {
      cerr << "send handle failed";
      exit(-1);
   }
   
   recvLength = 0;
   packetLength = kHeaderSize;
   while (recvLength < packetLength)
   {
      int length = 0;
      length = recv(socketNumber, recvBuffer + recvLength, kHeaderSize - recvLength, 0);
      recvLength += length;
   }
   
   memcpy(&header, recvBuffer, kHeaderSize);
   
   recvLength = 0;
   packetLength = 0;
   
   if (header.flag == 2) {
      return true;
   }
	
   return false;
}

bool ClientConnection::receiveMessage()
{
   int length = 0;
   if (packetLength == 0)
   {
      length = recv(socketNumber, recvBuffer + recvLength, kHeaderSize - recvLength, 0);
      if (length <= 0) {
         cerr << "RECV LENGTH IS NOT POSITIVE";
         exit(-1);
      }
      recvLength += length;
      if (recvLength == kHeaderSize)
      {
         NormalHeader header;
         memcpy(&header, recvBuffer, kHeaderSize);
         /*cout << "Pack Length:" << ntohs(header.packetLength) <<
         " FLAG RECIEVED:" << (int)header.flag << endl;*/
         if (header.flag == kHandleList && handleLength > 0)
         {
            recieveHandleList();
         }
         else
         {
            packetLength = ntohs(header.packetLength);
            if (packetLength == kHeaderSize) {
               recvLength = 0;
               packetLength = 0;
               return true;
            }
         }
      }      
   }
   else if (packetLength)
   {
      length = recv(socketNumber, recvBuffer + recvLength, packetLength - recvLength, 0);
      if (length <= 0) {
         cerr << "RECV LENGTH IS NOT POSITIVE";
         exit(-1);
      }
      recvLength += length;
      if (recvLength == packetLength)
      {    
         /*cout << "recv:" << recvLength << "pack:" << packetLength << endl;*/
         recvLength = 0;
         packetLength = 0;
         return true;
      }
   }
   return false;
}

void ClientConnection::recieveHandleList()
{
   int handlesLeft = handleLength;
   int buffSize = kHeaderSize;
   unsigned char tempLength = 0;
   bool lengthReceived = false;
   
   cout << "Number of clients: " << handleLength << endl; 
   
   while (handlesLeft > 0)
   {
      if (!lengthReceived)
      {
         recv(socketNumber, recvBuffer + buffSize, 1, 0);
         memcpy(&tempLength, recvBuffer + buffSize, 1);
         buffSize++;
         lengthReceived = true;
      }
   
      if (lengthReceived)
      {
         int length = 0;
         while (length < tempLength)
         {
            int temp = 0;
            temp = recv(socketNumber, recvBuffer + buffSize + length, tempLength - length, 0);
            length += temp;
         }
         string message (recvBuffer + buffSize, tempLength);
         cout << "\t" << message << endl;
         buffSize += tempLength;
         lengthReceived = false;
         handlesLeft--;
      }
   }
   cout << "$:" << flush;
   handleLength = -1;
   packetLength = 0;
   recvLength = 0;
}

string ClientConnection::getUsername() {
   return username;
}

bool ClientConnection::isUserInput() {
   return readUserInput;
}

bool ClientConnection::isServerInput() {
   return readServerInput;
}

char* ClientConnection::getRecvBuffer() {
   return recvBuffer;
}

char* ClientConnection::getSendBuffer() {
   return sendBuffer;
}

void ClientConnection::enableClientExit() {
   clientExitStatus = true;
}

bool ClientConnection::getExitStatus() {
   return clientExitStatus;
}

void ClientConnection::setHandleLength(uint32_t handleLength) {
   this->handleLength = handleLength;
}

uint32_t ClientConnection::getHandleLength() {
   return handleLength;
}

void ClientConnection::setSendLength(int sendLength) {
   this->sendLength = sendLength;
}

int ClientConnection::getSendLength() {
   return sendLength;
}

int ClientConnection::getSocket()
{
   return socketNumber;
}

ClientConnection::~ClientConnection() {
   free(recvBuffer);
   free(sendBuffer);
}