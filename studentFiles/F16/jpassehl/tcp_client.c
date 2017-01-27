/******************************************************************************
* tcp_client.c
*
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


int PacketBuilder(int flag, struct client *dstClient, struct client *srcClient, char *message, int messageLen);
void SetupClient(int argc, char *argv[], struct client *thisClient); 
void ProcessInput(struct client *thisClient);
int ParseInput(char *in_buf, int in_len, struct client *thisClient);
int ProcessFlag(char *packet);
int ProcessLength(char *packet);
void RespondToPacket(int flag, int inLen, char *packet, int received_socket, struct client *client_socket);

char *send_packet = NULL;
char *rec_packet = NULL;
int sendLen = 0;
int recLen = 0;
fd_set openFDs;
int numClients = 0;


int main(int argc, char * argv[]){
   struct client thisClient;

   thisClient.handleLen = 0;
   thisClient.isAlive = 0;

   //send and receive the first packet
   SetupClient(argc, argv, &thisClient);
   //process user input and send outgoing pacekts the receive response
   ProcessInput(&thisClient);

   return 0;
}

void ProcessInput(struct client *thisClient) {

   char *in_buf = NULL;
   int in_len = 0, flag = 0, sent = 0;  //actual amount of data sent
   sendLen = 0;        //amount of data to send
   printf("\n$: ");

   while(thisClient->isAlive) {
      printf("\n$: ");
      send_packet = calloc(MAXBUF, sizeof(char));
      rec_packet = calloc(MAXBUF, sizeof(char));

      FD_ZERO(&openFDs);
      FD_SET(thisClient->socketNum, &openFDs);
      FD_SET(0, &openFDs);

      if (select(FD_SETSIZE, &openFDs, NULL, NULL, NULL) < 0) {
         perror("Error with client select\n");
         exit(-1);
      }

      //SERVER TRIGGERED      
      if (FD_ISSET(thisClient->socketNum, &openFDs)) {
         recLen = 0;
         rec_packet = calloc(MAXBUF, sizeof(char));

         //get the receive packet
         if((recLen = recv(thisClient->socketNum, rec_packet, MAXBUF, 0)) < 0) {
            printf("error receiving reply");
            exit(-1); 
         }
         if (recLen == 0){
            close(thisClient->socketNum);
            thisClient->isAlive = 0;
            printf("Server Terminated.\n");
         }
 
//printf("received length: %d\n", recLen);
/*
printf("Received Data: ");
int j =0; 
for(j = 0; j < recLen; j++) { 
printf("%02x", rec_packet[j]); 
} 
printf("\n"); 
 */
         flag = ProcessFlag(rec_packet);
         RespondToPacket(flag, recLen, rec_packet, thisClient->socketNum, thisClient);
      }

      //STDIN TRIGGERED
      else if (FD_ISSET(0, &openFDs)) {
         //allocate space for the text input buffer
         in_buf = calloc(MAXBUF, sizeof(char));
         //reset the text input length
         in_len = 0;
         //fill the in buffer with user input
         while ((in_buf[in_len] = getchar()) != '\n'){
            in_len++;
         }
         in_buf[in_len] = '\0';


         //MAKE THE PACKET TO SEND TO THE SERVER
         //get the flag and parse the input into packet form
         flag = ParseInput(in_buf, in_len, thisClient);

		 /*
printf("Sending Packet: ");
int j = 0;
for(j = 0; j < sendLen; j++) { 
printf("%02x", send_packet[j]); 
} 
printf("\n");
 	*/
         //send the packet
         sent = send(thisClient->socketNum, send_packet, sendLen, 0);//1 = fd of socket, 2 = data to send, 3 = #of bytes to send
         if(sent < 0){
            perror("send call");
            exit(-1);
         }
      }
      free(send_packet);
      free(rec_packet);
   }	
   //if the client is not alive
   close(thisClient->socketNum);
}

void RespondToPacket(int flag, int inLen, char *packet, int received_socket, struct client *client_socket) {
   char handleLen;
   int messageLen = 0;
   char *senderHandle;
   char *destHandle;
   char senderHandleLen;
   char *msgBuffer;
   int recFlag;
 
//printf("got in function");
   switch(flag){
      //good handle
      case 2:
         //essentially do nothing, it was a good handle and client was added 
         break;

      //error on initial packet
      case 3:
         senderHandle = calloc(MAXHANDLE, sizeof(char));		

         memcpy(&handleLen, &rec_packet[3], sizeof(char));
//printf("handleLen: %d\n", handleLen);
         memcpy(senderHandle, &rec_packet[4], handleLen);
//printf("senderHandle: %s\n", senderHandle);
         printf("\nHandle already in use: %s\n", client_socket->handle );
	 //then the client "terminates"
         close(client_socket->socketNum);
	 client_socket->isAlive = 0;
	 break;
		 
      //broadcast
      case 4:
         //break up the broadcast message into its components
         senderHandle = calloc(MAXHANDLE, sizeof(char));		
         msgBuffer = calloc(1000, sizeof(char));

         handleLen = 0;
         memcpy(&handleLen, &rec_packet[3], sizeof(char));
//printf("handleLen: %02x\n", handleLen);
         memcpy(senderHandle, &rec_packet[4], handleLen);
//printf("senderHandle %s\n", senderHandle);
         messageLen = recLen - (4 + handleLen);
         memcpy(msgBuffer, &rec_packet[4+handleLen], messageLen); 
//printf("msgBuffer: %s\n", msgBuffer);

         //message is printed in the format: <handle>: <message>
         printf("%s: %s\n", senderHandle, msgBuffer);
//         printf("$: ");	
         break;
 
      //message
      case 5:
         //break up the message into its components
	 //message is printed in the format: <handle>: <message>
         senderHandle = calloc(MAXHANDLE, sizeof(char));		
         msgBuffer = calloc(1000, sizeof(char));

         handleLen = 0;

         memcpy(&handleLen, &rec_packet[3], sizeof(char));
//printf("handleLen: %02x\n", handleLen);
         memcpy(&senderHandleLen, &rec_packet[4 + handleLen], sizeof(char));
//printf("senderHandleLen: %02x\n", senderHandleLen);

         memcpy(senderHandle, &rec_packet[5 + handleLen], senderHandleLen);
//printf("senderHandle %s\n", senderHandle);

//printf("recLen: %d\n", recLen);
         messageLen = recLen - (5 + handleLen + senderHandleLen);
         memcpy(msgBuffer, &rec_packet[5+handleLen+senderHandleLen], messageLen); 
//printf("msgBuffer: %s\n", msgBuffer);
 
        //message is printed in the format: <handle>: <message>
         printf("\n%s: %s\n", senderHandle, msgBuffer);
//         printf("$: ");
         break;

      //error for message packet
      case 7:
         //error in response to a message packet
         handleLen = 0;

         destHandle = calloc(MAXHANDLE, sizeof(char));
//printf("trying to process flag 7\n");
         memcpy(&handleLen, &rec_packet[3], sizeof(char));
//printf("handleLen: %02x\n", handleLen);
         memcpy(destHandle, &rec_packet[4], handleLen);
//printf("senderHandle %s\n", senderHandle);
         printf("\nClient with handle %s does not exist.\n", destHandle);		         
         break;

      //client wants to terminate
      case 9:
         //ACK client's termination
         close(client_socket->socketNum);
         client_socket->isAlive = 0;
         //terminate the client
         break;
		 
      case 11:
      //Tells the client how many servers are on the handle
         numClients = 0;
         memcpy(&numClients, &rec_packet[3], sizeof(unsigned int));
         printf("\nNumber of clients: %d\n", numClients);      
         
 /*
  		        FD_ZERO(&openFDs);
                           FD_SET(received_socket, &openFDs);
				FD_SET(0, &openFDs);

				if (select(FD_SETSIZE, &openFDs, NULL, NULL, NULL) < 0) {
					 perror("Error with client select\n");
					 exit(-1);
				}

				 //SERVER TRIGGERED      
				 if (FD_ISSET(received_socket, &openFDs)) {
*/
         int i = 0;
         for(i =0; i < numClients; i++) {       
            recLen = 0;

	    rec_packet = calloc(MAXBUF, sizeof(char));

	    //get the receive packet
	    if((recLen = recv(received_socket, rec_packet, MAXBUF, 0)) < 0) {
		printf("error receiving reply");
		exit(-1); 
	    } 
//printf("RECEIVED A PACKET!!\n");

            recFlag = ProcessFlag(rec_packet);
            RespondToPacket(recFlag, recLen, rec_packet, client_socket->socketNum, client_socket);
 
         }
//printf("Received packet: %s\n", rec_packet);
//				 }

		 break;

      case 12:
         //One packet per client name on the server
         //keep track of the handle names in an array
         //when the number of received flag=12 packets reaches the number 
         //specified, print all the packets in the form \n\t<handle>
		
//printf("GETTING IN CASE 12:\n");
         senderHandle = calloc(MAXHANDLE, sizeof(char));		

         handleLen = 0;
         memcpy(&handleLen, &rec_packet[3], sizeof(char));
//printf("handleLen: %d\n", handleLen);
         memcpy(senderHandle, &rec_packet[4], handleLen);

         printf("\t %s\n", senderHandle);
         
		 break;

      default:
         printf("\nUnknown incoming packet type:");
		 
      }
      printf("$: ");
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

int ParseInput(char *in_buf, int in_len, struct client *thisClient) {
   int flag = 0, inNdx = 0, msgNdx = 0;
   char *type, *dstHandle, *message;
   type = calloc(1, sizeof(char));
   
   //search for % type
   while ((in_buf[inNdx] != '%') && (inNdx < in_len)){
      inNdx++;
   }
   inNdx++;
   type[0] = in_buf[inNdx];
//printf("type: %s\n", type);
 
   //BROAdCAST
   if (*type == 'B' || *type == 'b'){
//printf("Case 'B'\n");
       flag = 4;
       message = calloc(in_len, sizeof(char));

       //skip white space
       inNdx++; 
       while ((in_buf[inNdx] == ' ') && (inNdx < in_len)){
          inNdx++;
       }

       //grab message
       while (inNdx < in_len){
          message[msgNdx] = in_buf[inNdx];
          msgNdx++;
          inNdx++;
       }
//printf("Message: %s\n", message);
       //make the packet and store the length of the packet
       sendLen =  PacketBuilder(flag, thisClient, NULL, message, msgNdx);
   }

   //MESSAGE
   else if (*type == 'M' || *type == 'm') {
      struct client destClient;
      destClient.handle = calloc(MAXHANDLE, sizeof(char));
      destClient.handleLen = 0;

      flag = 5;
      dstHandle = calloc(MAXHANDLE, sizeof(char));
      message = calloc(in_len, sizeof(char));

       //skip white space
       inNdx++; 
       while ((in_buf[inNdx] == ' ') && (inNdx < in_len)){
          inNdx++;
       }

       //grab destination client's handle
       int hndlNdx = 0;
       while ((in_buf[inNdx] != ' ') && (inNdx < in_len)){
          destClient.handle[hndlNdx] = in_buf[inNdx];
          destClient.handleLen++;
          hndlNdx++;
          inNdx++;
//printf("inNdx: %d\n", inNdx);
       }
//printf("dest handle: %s\n", destClient.handle);

       //skip white space
       inNdx++; 
       while ((in_buf[inNdx] == ' ') && (inNdx < in_len)){
          inNdx++;
       }

       //grab message
       while (inNdx < in_len){
          message[msgNdx] = in_buf[inNdx];
          msgNdx++;
          inNdx++;
       }
//printf("Message: %s\n", message);

      //make the packet and store the length of the packet
      sendLen =  PacketBuilder(flag, thisClient, &destClient, message, msgNdx);
  }
   //EXIT
   else if (*type == 'E' || *type == 'e') {
//printf("Case 'E'\n");
      flag = 8;
      sendLen = PacketBuilder(flag, thisClient, NULL, NULL, 0);   
   }
   //LIST
   else if(*type == 'L' || *type == 'l') {
//printf("Case 'L'\n");
      flag = 10;
      sendLen = PacketBuilder(flag, thisClient, NULL, NULL, 0);   
   }
   else{
      printf("\nInvalid Command.");
   }   

   return flag;
}

void SetupClient(int argc, char *argv[], struct client *thisClient  ) {
   int hndlNdx = 0, handleLen = 0, sent; 
   int flag;
   /* check command line arguments  */
   if (argc != 4){
      printf("usage: %s handle host-name port-number \n", argv[0]);
      exit(1);
   }

   //get the handle
   thisClient->handle = calloc(MAXHANDLE, sizeof(char));
   handleLen = strlen(argv[1]);
   while ((hndlNdx < handleLen)) {
         thisClient->handle[hndlNdx] = argv[1][hndlNdx];
         hndlNdx++;
   }
//printf("thisClient->handle: %s\n", thisClient->handle);
   thisClient->handleLen = handleLen;

   //setup TCP client socket
   thisClient->socketNum = tcpClientSetup(argv[2], argv[3]);// 1 = host-name, 2 = port-num
   thisClient->isAlive = 1;

   //clear the sending packet
   free(send_packet);
   send_packet = calloc(MAXBUF, sizeof(char));

   //get the number of bytes in the packet
   sendLen = PacketBuilder(1, thisClient, NULL, NULL, 0);

//printf("Attempt to send length: %d\n", sendLen);//: %02x\n", send_packet);

   //send the data
   sent = send(thisClient->socketNum, send_packet, sendLen, 0);//1 = fd of socket, 2 = data to send, 3 = #of bytes to send
   if(sent < 0){
	perror("send call");
	exit(-1);
   }

   //allocate space for receiving packet buffer
   rec_packet = calloc(MAXBUF, sizeof(char));
   recLen = 0;

   //receive the data
   if((recLen = recv(thisClient->socketNum, rec_packet, MAXBUF, 0)) < 0) {
      printf("error receiving reply");
      exit(-1); 
   }

   //print out what needs to be printed out from the reply

   /*
printf("Received reply: ");
int j =0; 
for(j = 0; j < recLen ; j++) { 
printf("%02x", rec_packet[j]); 
} 
printf("\n"); 
*/
   flag = ProcessFlag(rec_packet);
   RespondToPacket(flag, recLen, rec_packet, thisClient->socketNum, thisClient);
 
}

int PacketBuilder(int flag, struct client *srcClient, struct client *dstClient, char* message, int messageLen) {
   struct chatHeader cHead;
   cHead.length  = 0;
   char *cHeadBuf = calloc(CHAT_HEAD_LEN, sizeof(char));

   send_packet = calloc(MAXBUF, sizeof(char));
   sendLen = 0;
   cHead.flag = 0;

   switch (flag) {

      //initial client to server: header + handleLen(1) + handle(handleLen)
      case 1:
         cHead.flag |= flag;
         cHead.length = CHAT_HEAD_LEN + 1 + srcClient->handleLen;

         cHeadBuf[2] |= flag;
         memcpy(cHeadBuf, &cHead.length, sizeof(short));
         
         //chat header portion
         memcpy(send_packet, cHeadBuf, 3);
         //extra portion
         memcpy(&send_packet[3], &srcClient->handleLen, sizeof(char));
         memcpy(&send_packet[4], srcClient->handle, srcClient->handleLen);
//printf("packet built %02x\n", send_packet);
         break;
      
      //Broadcast
      case 4:
         cHead.flag |= flag;

         cHead.length = CHAT_HEAD_LEN + 1 + srcClient->handleLen + messageLen;

         cHeadBuf[2] |= flag;
         memcpy(cHeadBuf, &cHead.length, sizeof(short));

//printf("chead len: %hu\n", cHead.length);
         //fill out the final buffer
         //add the chat-head buffer
         memcpy(send_packet, cHeadBuf, 3);
         //add handle length
         memcpy(&send_packet[3], &srcClient->handleLen, sizeof(char));
         //add handle
         memcpy(&send_packet[4], srcClient->handle, srcClient->handleLen*sizeof(char));
         //add message
         memcpy(&send_packet[4+srcClient->handleLen], message, messageLen); 
         break;

      //Message
      case 5:
         cHead.flag |= flag;
         cHead.length = CHAT_HEAD_LEN + 1 + srcClient->handleLen + 1 + dstClient->handleLen + messageLen;

         cHeadBuf[2] |= flag;
         memcpy(cHeadBuf, &cHead.length, sizeof(short));

         //fill out the final buffer
         //add the chat-head buffer
         memcpy(send_packet, cHeadBuf, 3);
         //add dest handle length
         memcpy(&send_packet[3], &dstClient->handleLen, sizeof(char));
//printf("dstClient.handleLen: %d\n", dstClient->handleLen);
         //add dest handle
         memcpy(&send_packet[4], dstClient->handle, dstClient->handleLen);
//printf("dstClient.handle: %s\n", dstClient->handle);
         //add source handle length
         memcpy(&send_packet[4 + dstClient->handleLen], &srcClient->handleLen, sizeof(char));
//printf("srcClient.handleLen: %d\n", srcClient->handleLen);
         //add source handle
         memcpy(&send_packet[4 + dstClient->handleLen + 1], srcClient->handle, srcClient->handleLen);
//printf("srcClient.handle: %s\n", srcClient->handle);

         //add message
         memcpy(&send_packet[5 + dstClient->handleLen + srcClient->handleLen], message, messageLen); 
//printf("message: %s\n", message);
 //        printf("\n$:");
         break;

      //client wants to exit
      case 8:
         cHead.flag |= flag;
         cHead.length = CHAT_HEAD_LEN;
         cHeadBuf[2] |= flag;
         memcpy(cHeadBuf, &cHead.length, sizeof(short));

         //fill out the final buffer
         memcpy(send_packet, cHeadBuf, cHead.length);
         break;
 
      //request list of handles
      case 10:
         cHead.flag |= flag;
         cHead.length = CHAT_HEAD_LEN;

         cHeadBuf[2] |= flag;
         memcpy(cHeadBuf, &cHead.length, sizeof(short));

         //fill out the final buffer
         memcpy(send_packet, cHeadBuf, cHead.length);
         break;
      }
//printf("newPacket. Length: %d\n", cHead.length); 
  return cHead.length;
} 
