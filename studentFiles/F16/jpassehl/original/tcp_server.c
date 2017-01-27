/******************************************************************************
* tcp_server.c
*
* CPE 464 - Program 2
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

#include "networks.h"
#include "testing.h"
#include "packet.h"

#define MAXBUF 1024
#define MAXHANDLE 255
#define CHAT_HEAD_LEN 3

int maxClients = 0;
int numClients = 0;
char *send_packet;
char *rec_packet;
int sendLen = 0, recLen = 0;
fd_set server_fds; //set of file descriptors for the server side

int PacketBuilder(int flag, struct client *theClient, char *packet);
int SetupServer(struct client *client_socket, char *portNum);
void RunServer(int server_socket, struct client *client_socket);
int CheckNewClientRequest(int server_socket, struct client *client_socket, fd_set *server_fds);
int CheckIncomingPacket(int server_socket, struct client *client_socket, fd_set *server_fds);
void ResetFDs(int server_socket, struct client *client_socket, fd_set *server_fds);
int ProcessFlag(char *packet);
int ProcessLength(char *packet);
void RespondToPacket(int flag, int clNdx, char *packet, struct client *client_socket);
 

int main(int argc, char *argv[]){

   int server_socket = 0; //socket descriptor of the server socket
   struct client client_sockets[MAXSOCKETS]; //socket descriptor for the client socket

   if(argc > 1) {   
      server_socket = SetupServer(client_sockets, argv[1]);
   }
   else {
      server_socket = SetupServer(client_sockets, NULL);
   }

   RunServer(server_socket, client_sockets);

   return 0;
}


int PacketBuilder(int flag, struct client *theClient, char *packet) {
   struct chatHeader cHead;
   char *cHeadBuf = calloc(CHAT_HEAD_LEN, sizeof(char));
   send_packet = calloc(MAXBUF, sizeof(char));
   sendLen = 0;
   cHead.length = 0;
   cHead.flag = 0;
   char *handle;
   char handleLen;
   handle = calloc(MAXHANDLE, sizeof(char));
//   int clientsToServe = 0;
//printf("packetbuilder flag: %d\n", flag);

   switch (flag) {

      //confirm good handle
      case 2:
         cHead.flag |= flag;
         cHead.length = CHAT_HEAD_LEN;
         sendLen = cHead.length;

         cHeadBuf[2] |= flag;
         memcpy(cHeadBuf, &cHead.length, sizeof(short));

         //fill out the final buffer
         memcpy(send_packet, cHeadBuf, cHead.length);
         break;

      //error: handle already exists
      case 3: 
         cHead.flag |= flag;
         cHead.length = CHAT_HEAD_LEN;

         cHeadBuf[2] |= flag;
         memcpy(cHeadBuf, &cHead.length, sizeof(short));

         //fill out the final buffer
         memcpy(send_packet, cHeadBuf, cHead.length);
         break;

      //error: handle does not exist. header + len(1) + handle(len) 
      case 7: 
         //extract the handle from the received packet
         memcpy(&handleLen, &rec_packet[3], sizeof(char)); 
//printf("FLAG 7 handleLen: %02x\n", handleLen); 
         memcpy(handle, &rec_packet[4], handleLen);
//printf("FLAG 7 handle: %s\n", handle);

         cHead.flag |= flag;
         cHead.length = CHAT_HEAD_LEN + 1 + handleLen;

         cHeadBuf[2] |= flag;
         memcpy(cHeadBuf, &cHead.length, sizeof(short));


         //chat header portion
         memcpy(send_packet, cHeadBuf, cHead.length);
         //extra portion
         memcpy(&send_packet[3], &handleLen, sizeof(char));
         memcpy(&send_packet[4], handle, handleLen);
         break;
      //ack clients exit
      case 9: 
         cHead.flag |= flag;
         cHead.length = CHAT_HEAD_LEN;

         cHeadBuf[2] |= flag;
         memcpy(cHeadBuf, &cHead.length, sizeof(short));

         //fill out the final buffer
         memcpy(send_packet, cHeadBuf, cHead.length);
         numClients--;
         break; 

      //respond to %L with number of handles
      case 11: 
         cHead.flag |= flag;
         cHead.length = CHAT_HEAD_LEN + 4;

         sendLen = cHead.length;

         cHeadBuf[2] |= flag;
         memcpy(cHeadBuf, &cHead.length, sizeof(short));

         //fill out the final buffer
         //chat header portion
         memcpy(send_packet, cHeadBuf, 3);
         //extra portion
         memcpy(&send_packet[3], &numClients, sizeof(unsigned int));
//printf("made it thru packetbuilder\n");
         break;

      //one packet per handle currently on server
      case 12: 
         cHead.flag |= flag;
//         send_packet = calloc(MAXBUF,sizeof(char));
         cHead.length = CHAT_HEAD_LEN + 1 + theClient[0].handleLen;
//printf("Packet name %s", theClient[0].handle);
//printf("Packet 12 handle Length: %d\n", theClient[0].handleLen);

         cHeadBuf[2] |= flag;

               //fill out the final buffer
               //chat header portion

          memcpy(send_packet, cHeadBuf, CHAT_HEAD_LEN);
               //extra portion
               memcpy(&send_packet[3], &theClient[0].handleLen, sizeof(char));
               memcpy(&send_packet[4], theClient[0].handle, theClient[0].handleLen);
         break;
   }
   sendLen = cHead.length;
/*
printf("New packet created: ");
int j =0;
for(j = 0; j < cHead.length; j++) {
printf("%02x", send_packet[j]);

}
printf("\n");
*/
   return cHead.length;

}

void RunServer(int server_socket, struct client *client_socket) {

   int receivedClNdx = 0, inFlag = 0;

   while(1) {
      send_packet = calloc(MAXBUF, sizeof(char));
      rec_packet = calloc(MAXBUF, sizeof(char));

      //NEED TO CLEAR MESSAGE BUFFER EVERY TIME "PrepareForInput()
      ResetFDs(server_socket, client_socket, &server_fds);
//printf("\nselect() is waiting...\n");
      select(FD_SETSIZE, &server_fds, NULL, NULL, NULL);
//printf("DONE WAITING!\n");

      if (FD_ISSET(server_socket, &server_fds)) { 
         CheckNewClientRequest(server_socket, client_socket, &server_fds);
      }
      else {
         //sets the recLen and fills the rec_packet
         receivedClNdx = CheckIncomingPacket(server_socket, client_socket, &server_fds);

/*	 
printf("Received Data:");

int j =0;
for(j = 0; j < recLen; j++) {
printf("%02x", rec_packet[j]);
}
printf("\n");
*/

      inFlag = 0;
      inFlag = ProcessFlag(rec_packet);
//printf("inFlag: %d", inFlag);
      recLen = 0;
      recLen = ProcessLength(rec_packet);//might not be necessary due to CheckIncomingPacket()
      RespondToPacket(inFlag, receivedClNdx, rec_packet, client_socket);
        }
//   free(send_packet);
//   free(rec_packet);

   }
 
}

void RespondToPacket(int flag, int clNdx, char *packet, struct client *client_socket) {
   int i = 0;
   char *srcHandle;
   char srcHandleLen;
   char *dstHandle;
   char dstHandleLen;
   srcHandle = calloc(MAXHANDLE, sizeof(char));
   dstHandle = calloc(MAXHANDLE, sizeof(char));
   int uniqueHandle = 1;

//printf("clNdx in RespondToPacket: %d\n", clNdx);

   switch(flag){

      //client wants to add
      case 1:

         client_socket[maxClients - 1].handle = calloc(MAXHANDLE, sizeof(char));
         memcpy(&srcHandleLen, &rec_packet[3], sizeof(char)); 
//printf("handleLen: %02x\n", srcHandleLen); 
         memcpy(srcHandle, &rec_packet[4], srcHandleLen); 

//printf("client_socket[%d].handle added: %s\n",clNdx, client_socket[maxClients - 1].handle);
//printf("maxClients: %d\n", maxClients);
         send_packet = calloc(MAXBUF, sizeof(char));
         
		 //if its a good handle
		 for(i = 0; i < maxClients; i++){
		    if(!strcmp(srcHandle, client_socket[i].handle)) {
                       uniqueHandle = 0;     
		    }
         	    else{
//printf("senderHandle: %s\n", srcHandle);
//printf("client_socket[%d].handle: %s\n", i, client_socket[i].handle);
		    }
		 }

                 if(uniqueHandle) {
			PacketBuilder(2, &client_socket[maxClients - 1], NULL);//clientPtr shouldnet really matter?
			send(client_socket[maxClients - 1].socketNum, send_packet, sendLen, 0);

                        client_socket[clNdx].isAlive = 1;
                        client_socket[clNdx].handleLen = srcHandleLen; 
                        memcpy(client_socket[maxClients - 1].handle, srcHandle, srcHandleLen); 
	          }
                  else {
	        	PacketBuilder(3, &client_socket[maxClients - 1], NULL);//clientPtr shouldnet really matter?
			send(client_socket[maxClients - 1].socketNum, send_packet, sendLen, 0); 
			numClients--;
			maxClients--;
                  }

          //work is taken care of in the NewClientRequest() funciton
         break;

      //broadcast
      case 4:
         //just need to forward the message to everybody except the sender
         memcpy(&srcHandleLen, &rec_packet[3], sizeof(char));
         memcpy(srcHandle, &rec_packet[4], srcHandleLen);
//printf("\nsrcHandleLen %d\n", srcHandleLen);
//printf("srcHandle: %s\n", srcHandle);

/*
j =0;
for(j = 0; j < sendLen; j++) {
printf("%02x", send_packet[j]);
}
printf("\n");
*/
//printf("maxClients %d\n", maxClients);
//printf("srcHandle: %s\n", srcHandle);
//printf("client_socket[%d].handle: %s\n",clNdx, client_socket[clNdx].handle);

         for(i = 0; i < maxClients; i++) {
            if (clNdx == i){
               //do nothing
//printf("did not send to socket # %d\n", client_socket[i].socketNum);

            }
            else {
               //make sure client is alive
               if(client_socket[i].isAlive){
//printf("client_socket[%d].isAlive: %d", i, client_socket[i].isAlive);
                  send(client_socket[i].socketNum, rec_packet, recLen, 0);
               }
          }
       }
       break;

      //message
      case 5:
         //just need to forward the destination handle
         memcpy(&dstHandleLen, &rec_packet[3], sizeof(char));
         memcpy(dstHandle, &rec_packet[4], dstHandleLen);
//printf("\ndstHandleLen %d\n", dstHandleLen);
//printf("dstHandle: %s\n", dstHandle);
/*
j =0;
for(j = 0; j < sendLen; j++) {
printf("%02x", send_packet[j]);
}
printf("\n");
*/
//printf("client_socket[%d].handle: %s\n",clNdx, client_socket[clNdx].handle);
         int matched = 0;
         for(i = 0; i < maxClients; i++) {
            //if the handle matches
//printf("dstHandle: %s\n", dstHandle);
//printf("client_socket[%d].handle: %s", i, client_socket[i].handle);
            if (!strcmp(dstHandle, client_socket[i].handle)){
               //and the handle is alive
               if (client_socket[i].isAlive){
                  send(client_socket[i].socketNum, rec_packet, recLen, 0);
                  matched = 1;
               }
            }
         }
         if(matched == 0) {
            PacketBuilder(7, client_socket, packet);
            send(client_socket[clNdx].socketNum, send_packet, sendLen, 0);
//printf("handle did not match a destination\n");
         }
         break;

      //client wants to exit
      case 8:
         send_packet = calloc(MAXBUF, sizeof(char));

         //if it's allowed to exit
         PacketBuilder(9, &client_socket[clNdx], NULL);
         numClients--;
         client_socket[clNdx].isAlive = 0;
         send(client_socket[clNdx].socketNum, send_packet, sendLen, 0);
         close(client_socket[clNdx].socketNum);
         break;

      //client wants a list
      case 10:
         send_packet = calloc(MAXBUF, sizeof(char));

         //first respond with how many handles are on the server
         PacketBuilder(11, client_socket, packet);
         send(client_socket[clNdx].socketNum, send_packet, sendLen, 0);
//sleep(100);
//printf("sent\n");
         //iterate through each client and p
         int i = 0;
//         send(client_socket[clNdx].socketNum, "HEYEYEY", 7, 0);
         for(i = 0; i < maxClients; i++){
            if(client_socket[i].isAlive){  
               send_packet = calloc(MAXBUF, sizeof(char));
               PacketBuilder(12, &client_socket[i], packet);
               send(client_socket[clNdx].socketNum, send_packet, sendLen, 0);
            }
         }
         break;
   }

}


int ProcessLength(char *packet) {
   int length = 0;

   memcpy(&length, packet, sizeof(unsigned short));

//printf("Processed length: %d\n", length);

   return length;

}

int ProcessFlag(char *packet) {

   unsigned short flag = 0;
   char *flagChar = calloc(6, sizeof(char));

   memcpy(flagChar, &packet[2], 1);
   flag = *flagChar;

//printf("Processed flag: %02x\n", flagChar[0]);
//printf("Processed flag: %hu\n", flag);

   return flag;
}

void ResetFDs(int server_socket, struct client *client_socket, fd_set *server_fds) {
   int i, maxFd = 0;
   //clear and set the fd_set
   FD_ZERO(server_fds);
   FD_SET(server_socket, server_fds);

   for(i = 0; i < maxClients; i++) {      
      if(client_socket[i].isAlive) {
         FD_SET(client_socket[i].socketNum, server_fds);
//printf("FD_SET client_socket[%d].socketNum =  %d \n", i, client_socket[i].socketNum);
      }
   }
   //set maxFd so select knows how many things to wait for
   maxFd = client_socket[maxClients - 1].socketNum;
//printf("maxFD: %d\n", maxFd);
}

int CheckIncomingPacket(int server_socket, struct client *client_socket, fd_set *server_fds) {
   //find out which client socket made a request
   int j = 0, receivedClNdx = 0;

   for (j = 0; j < maxClients; j++) {
      if(FD_ISSET(client_socket[j].socketNum, server_fds)) { //if the client made a request then print the message
      //now get the data on the client_socket
         recLen = 0;
	 if ((recLen = recv(client_socket[j].socketNum, rec_packet, MAXBUF, 0)) < 0){
	    perror("recv call");
	    exit(-1);
         }
         if (recLen == 0) {
            close(client_socket[j].socketNum);
            client_socket[j].isAlive = 0;
         }
         receivedClNdx = j;
      }  
   }
   return receivedClNdx;
}

int CheckNewClientRequest(int server_socket, struct client *client_socket, fd_set *server_fds) {
   //check the server socket to see if a client wants to add 
   char *senderHandle;
   senderHandle = calloc(MAXHANDLE, sizeof(char));

   //look for a client to serve
   //if the number of clients isnt too high !!!!!!!!!!!!!!!!!!!
   client_socket[maxClients].socketNum = tcpAccept(server_socket); //uses accept() to accpet connection to server fd. returns a new fd for client

   maxClients++;
   numClients++;

   return maxClients;
}

int SetupServer(struct client *client_socket, char *portNum) {

   int server_socket;
   int clNdx = 0;	

   //create the server socket
   if(portNum != NULL) {
      server_socket = tcpServerSetup(atoi(portNum)); //returns socket, binds addr (default to port), gets and prints socket name
   }
   else {
      server_socket = tcpServerSetup(); //returns socket, binds addr (default to port), gets and prints socket name
   }
   //look for a client to serve
   client_socket[clNdx].socketNum = tcpAccept(server_socket); //uses accept() to accpet connection to server fd. returns a new fd for client
   maxClients++;
   numClients++;

   return server_socket;
}
