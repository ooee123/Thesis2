#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <arpa/inet.h>

#include "ServerApp.h"
#include "ServerConnection.h"
#include "PacketStructs.h"

using namespace std;


void ServerApp::run()
{
   
   while(1)
   {
      connection.listenForClient();
      if (connection.hasNewConnection())
      {
         connection.establishConnection();
      }
      else if (connection.hasNewInput())
      {
			bool value = connection.recieveMessage();
         if (value == true)
			{
				connection.getRecvClient().recvComplete = false;
				ProcessClientMessage(connection.getRecvClient());
			}
      }
      else 
      {
         cerr << "Select Statement returned with no input ready";
         exit(1);
      }
   }
   
}

void ServerApp::ProcessClientMessage(ServerClient &client)
{
   NormalHeader header;
   memcpy (&header, client.recvBuffer, kHeaderSize);
   switch ((int)header.flag)
   {
      case kHandleFlag:
         ConstructHandle(client);
         break;
      case kMessageFlag: {
         ConstructMessage(client);
		}	break;
      case kBroadcastFlag: {
         ConstructBroadcast(client);
		}	break;
      case kExitRequest:
         ConstructExit(client);
         break;
      case kRequestHandle:
         ConstructHandleList(client);
         break;
      default:
         cerr << "BAD FLAG RECIEVED FROM CLIENT :" << endl;
         exit(-1);
   }
}

void ServerApp::ConstructHandle(ServerClient &client)
{
   NormalHeader header;
   unsigned char handleLength;
   bool duplicate = false;
      
   memcpy(&handleLength, client.recvBuffer + kHeaderSize, 1);
   
   string handle (client.recvBuffer + kHeaderSize + 1, handleLength);
   
   duplicate = connection.containsHandle(handle);
   
   header.packetLength = htons(kHeaderSize);
   if (!duplicate)
   {
      header.flag = 2;
      client.handle = handle;
   }
   else 
   {
      header.flag = 3;
   }
   memcpy(connection.getSendBuffer(), &header, kHeaderSize);
   connection.sendMessage(ntohs(header.packetLength), client);
   
   if (header.flag == 3) {
      connection.closeClient(client);
   }
}

void ServerApp::ConstructBroadcast(ServerClient &client)
{
   NormalHeader header;
   memcpy (&header, client.recvBuffer, kHeaderSize);
   memcpy(connection.getSendBuffer(), client.recvBuffer, ntohs(header.packetLength));
   vector<ServerClient> otherClients = connection.getOtherClients(client);
   for (unsigned int i = 0; i < otherClients.size(); i++) {
      connection.sendMessage(ntohs(header.packetLength), otherClients.at(i));
   }      
}

void ServerApp::ConstructMessage(ServerClient &client)
{
   NormalHeader header;
   unsigned char handleLength;
   
   memcpy (&header, client.recvBuffer, kHeaderSize);
   memcpy(&handleLength, client.recvBuffer + kHeaderSize, 1);
   string handle(client.recvBuffer + kHeaderSize + 1,handleLength);
   if (connection.containsHandle(handle))
   {
      memcpy(connection.getSendBuffer(), client.recvBuffer, ntohs(header.packetLength));
      connection.sendMessage(ntohs(header.packetLength), connection.getClient(handle));
   }
   else {
      header.flag = kErrorFlag;
      header.packetLength = htons(kHeaderSize + 1 + handleLength);
      memcpy(connection.getSendBuffer(), &header, kHeaderSize);
      memcpy(connection.getSendBuffer() + kHeaderSize, &handleLength, 1);
      memcpy(connection.getSendBuffer() + kHeaderSize + 1, handle.c_str(), handleLength);
      connection.sendMessage(ntohs(header.packetLength), client);
   }
}

void ServerApp::ConstructExit(ServerClient &client) 
{
   NormalHeader header;
   header.flag = kExitFlag;
   header.packetLength = htons(kHeaderSize);
   memcpy(connection.getSendBuffer(), &header, kHeaderSize);
   connection.sendMessage(kHeaderSize, client);
	connection.closeClient(client);
}

void ServerApp::ConstructHandleList(ServerClient &client)
{
   NormalHeader header;
   header.flag = 11;
   header.packetLength = htons(kHeaderSize + 4);
   memcpy(connection.getSendBuffer(), &header, kHeaderSize);
   uint32_t temp = htonl(connection.getAllClients().size());
   memcpy(connection.getSendBuffer() + kHeaderSize, &temp, 4);
   connection.sendMessage(kHeaderSize + 4, client);
   
   uint16_t packetLength = kHeaderSize;
   vector<ServerClient> clients = connection.getAllClients();

   for (unsigned int i = 0; i < clients.size(); i++) {
      unsigned char clientLength = clients.at(i).handle.length();
      memcpy(connection.getSendBuffer() + packetLength, &clientLength, 1);
      packetLength++;
      memcpy(connection.getSendBuffer() + packetLength, clients.at(i).handle.c_str(), clientLength);
      packetLength += clientLength;
   }
   header.flag = 12;
   header.packetLength = htons(packetLength);
   memcpy(connection.getSendBuffer(), &header, kHeaderSize);
   connection.sendMessage(packetLength, client);
}
