/******************************************************************************
 * tcp_client.c
 *
 *****************************************************************************/

#include "tcp.h"


void closeAndExit(char *errorMesg, int socketNum) {
   perror(errorMesg);
   close(socketNum);
   exit(-1);
}

// Create and send packet to request handle from server
void initSend(char* handle, int socketNum) {
   char *sendBuf;
   int handleLen = strlen(handle);
   uint16_t packetLen = NORMAL_HEAD_LEN + 1 + handleLen;
   uint16_t packetLenNet = htons(packetLen);
   uint8_t flag;
   uint32_t index = 0;
   int sent = 0;

   if (handleLen > 255) {
      printf("Handle length too long\n");
      exit(-1);
   }   

   // Assemble and send packet with requested handle
   sendBuf = malloc(packetLen);

   flag = FLAG_ONE;

   memcpy(sendBuf, &packetLenNet, index += 2);
   memcpy(sendBuf + index, &flag, 1);
   index += 1;
   memcpy(sendBuf + index, &handleLen, 1);
   index += 1;
   memcpy(sendBuf + index, handle, handleLen);

   // now send the data 
   sent =  send(socketNum, sendBuf, packetLen, 0);  //***
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
  
   free(sendBuf);
}

// Blocks until receives response from server. Returns the flag value in the response
uint8_t initRecv(int socketNum) {
   uint8_t responseFlag = 0;
   int mesglen = 0;
   fd_set fdvar; 
   char *data = malloc(NORMAL_HEAD_LEN);

   // Block and wait for response
   FD_ZERO(&fdvar);  //reset variables
   FD_SET(socketNum, &fdvar);  //set to monitor server's socket

   // SELECT
   select(socketNum + 1, (fd_set *) &fdvar, (fd_set *) 0, (fd_set *) 0, NULL);  //*****

   if (FD_ISSET(socketNum, &fdvar)) {   //ready to receive data from server
      mesglen = recvFunc(socketNum, data);  //TCP recv
   }

   if (mesglen < 0) {
      printf("recv call fail");
      exit(-1);
   }

   responseFlag = data[FLAG_OFFSET];  

   return responseFlag;
}

void init(char* handle, int socketNum) {
   uint8_t responseFlag = 0;  

   initSend(handle, socketNum);
   responseFlag = initRecv(socketNum);

   if (responseFlag == FLAG_TWO) { 
      //Good to go, continue to next state
   }
   else if (responseFlag == FLAG_THREE) {  
      printf("Handle already in use: %s\n", handle);
      close(socketNum);
      exit(1);
   }
   else {
      printf("Bad flag response in init: %d\n", responseFlag);
      exit(-1);
   }
}

int wait(int socketNum) {
   fd_set fdvar;

   printf("$: ");
   fflush(stdout); //flush std out

   // Block and wait for response
   FD_ZERO(&fdvar);  //reset variables
   FD_SET(socketNum, &fdvar);  //set to monitor server's socket
   FD_SET(STD_IN_FD, &fdvar);  //set to monitor stdin


   // SELECT
   if (select(socketNum + 1, (fd_set *) &fdvar, (fd_set *) 0, (fd_set *) 0, NULL) < 0) {
      perror("select wait");
      exit(-1);
   }  //*****

   if (FD_ISSET(STD_IN_FD, &fdvar)) {   //ready to receive data from user
      return FROM_STD_IN;
   }
   else if (FD_ISSET(socketNum, &fdvar)) {   //ready to receive data from server
      return FROM_SERVER;
   }

   return -1;
}

// Ready to read mesg data from stdin, formatted: handle [text]
// Then send the message to server
void inM(char *handle, int socketNum) {
   char *sendBuf;
   char mesgBuf[1000];
   char destHandle[256]; 
   uint8_t flag = FLAG_FIVE;
   uint8_t handleLen = strlen(handle);
   uint8_t destHandleLen = 0;
   uint32_t index = 0;
   uint16_t mesgLen = 0;
   uint16_t packetLen;
   uint16_t packetLenNet;
   int sent = 0, firstTime = 1, quit = 0, newLine = 0;

   while (destHandleLen < 256) {
      destHandle[destHandleLen] = getchar();
      if (destHandle[destHandleLen] == '\n') {
         newLine = 1;
 
         mesgBuf[0] = NULL_CHAR;
   
         packetLen = NORMAL_HEAD_LEN + 1 + destHandleLen + 1 + handleLen + mesgLen;
         packetLenNet = ntohs(packetLen);

         // Assemble and send packet with requested handle
         sendBuf = malloc(packetLen);

         memcpy(sendBuf, &packetLenNet, index += 2);
         memcpy(sendBuf + index, &flag, 1);
         index += 1;
         memcpy(sendBuf + index, &destHandleLen, 1);  //dest handle
         index += 1;
         memcpy(sendBuf + index, destHandle, destHandleLen);
         index += destHandleLen;
         memcpy(sendBuf + index, &handleLen, 1);  //user's handle
         index += 1;
         memcpy(sendBuf + index, handle, handleLen);
         index += handleLen;
         memcpy(sendBuf + index, mesgBuf, mesgLen);

         // now send the data 
         sent = send(socketNum, sendBuf, packetLen, 0);  //***

         if(sent < 0)
         {
            perror("send call");
            exit(-1);
         }

         break;
      }
      else if(destHandle[destHandleLen] == ' '){
         break;
      }
      destHandleLen++;
   }
   if (destHandleLen < 1) {
      printf("Bad handle\n");
      exit(-1);
   }  

   // Fill message buffer, could be wrong**
   while ((firstTime || mesgLen == 1000 - 1) && !newLine) {
      mesgLen = 0;
      while ((mesgBuf[mesgLen] = getchar()) != '\n' && mesgLen < 1000) {
         mesgLen++;
      }

      if (mesgBuf[0] == '\n' && !firstTime) {
         break;
      }
      //insert the terminating char
      if (mesgLen == 1000 - 1 && mesgBuf[mesgLen] == '\n') {
         mesgBuf[mesgLen] = '\0';
         quit = 1;
      }
      mesgBuf[mesgLen] = '\0';
      mesgLen++;  
   
      packetLen = NORMAL_HEAD_LEN + 1 + destHandleLen + 1 + handleLen + mesgLen;
      packetLenNet = ntohs(packetLen);

      // Assemble and send packet with requested handle
      sendBuf = malloc(packetLen);

      memcpy(sendBuf, &packetLenNet, index += 2);
      memcpy(sendBuf + index, &flag, 1);
      index += 1;
      memcpy(sendBuf + index, &destHandleLen, 1);  //dest handle
      index += 1;
      memcpy(sendBuf + index, destHandle, destHandleLen);
      index += destHandleLen;
      memcpy(sendBuf + index, &handleLen, 1);  //user's handle
      index += 1;
      memcpy(sendBuf + index, handle, handleLen);
      index += handleLen;
      memcpy(sendBuf + index, mesgBuf, mesgLen);

      //sendBuf[packetLen] = NULL_CHAR;

      // now send the data 
      sent = send(socketNum, sendBuf, packetLen, 0);  //***

      if(sent < 0)
      {
         perror("send call");
         exit(-1);
      }
      firstTime = 0;
      if (quit) {
         break;
      }
   }
   
   free(sendBuf);
}

void sendNormalHeader(int socketNum, uint8_t flag) {
   char sendBuf[NORMAL_HEAD_LEN];
   uint16_t packetLen = NORMAL_HEAD_LEN;
   uint16_t packetLenNet = htons(packetLen);
   int sent = 0;
   
   memcpy(sendBuf, &packetLenNet, FLAG_OFFSET);
   memcpy(sendBuf + FLAG_OFFSET, &flag, 1); 

   sent =  send(socketNum, sendBuf, packetLen, 0);  //***
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

// parameter handle is the user's handle, socketNum is the server's socket
void inB(char *handle, int socketNum, int newLineFlag) {
   char *sendBuf;
   char mesgBuf[1000];
   uint8_t flag = FLAG_FOUR;  //broadcast
   uint8_t handleLen = strlen(handle);
   uint32_t index = 0;
   uint16_t mesgLen = 0;
   uint16_t packetLen;
   uint16_t packetLenNet;
   int sent = 0, firstTime = 1, quit = 0;

   // Fill message buffer
   while ((firstTime || mesgLen == 1000)) {
      mesgLen = 0;

      if (newLineFlag) {  //means empty message
         mesgBuf[0] = NULL_CHAR;
      }
      else {
         while ((mesgBuf[mesgLen] = getchar()) != '\n' && mesgLen < 1000) {
            mesgLen++;
         }

         if (mesgBuf[0] == '\n' && !firstTime) {
            break;
         }
         //insert the terminating char
         if (mesgLen == 1000 - 1 && mesgBuf[mesgLen] == '\n') {
            mesgBuf[mesgLen] = NULL_CHAR;
            quit = 1;
         }
         mesgBuf[mesgLen] = NULL_CHAR;
      }
      
      mesgLen++;  
   
      packetLen = NORMAL_HEAD_LEN + 1 + handleLen + mesgLen;
      packetLenNet = ntohs(packetLen);

      // Assemble and send packet with requested handle
      sendBuf = malloc(packetLen);

      memcpy(sendBuf, &packetLenNet, index += 2);
      memcpy(sendBuf + index, &flag, 1);
      index += 1;
      memcpy(sendBuf + index, &handleLen, 1);  //user's handle
      index += 1;
      memcpy(sendBuf + index, handle, handleLen);
      index += handleLen;
      memcpy(sendBuf + index, mesgBuf, mesgLen);

      // now send the data 
      sent = send(socketNum, sendBuf, packetLen, 0);  //***

      if(sent < 0)
      {
         perror("send call");
         exit(-1);
      }
      firstTime = 0;
      if (quit) {
         break;
      }
   }
   
   free(sendBuf);
}

void printClientMesg(char *data, char *handle) {
   char senderHandle[255 + NULL_CHAR_LEN]; 
   uint8_t handleLen = strlen(handle);
   uint8_t senderHandleLen;
   uint32_t index = 0;
   uint16_t packetLen;
   uint16_t packetLenNet;

   memcpy(&packetLenNet, data, FLAG_OFFSET);
   packetLen = ntohs(packetLenNet);

   index = NORMAL_HEAD_LEN + 1 + handleLen; //sits in front of sender handle
   senderHandleLen = data[index];
   index += 1;
   memcpy(senderHandle, data + index, senderHandleLen);
   senderHandle[senderHandleLen] = NULL_CHAR;
   index += senderHandleLen;  //index now points to the message, which is null terminated

   printf("\n%s: %s\n", senderHandle, data + index);
}

void printBroadcast(char *data) {
   char senderHandle[255 + NULL_CHAR_LEN]; 
   uint8_t senderHandleLen;
   uint32_t index = NORMAL_HEAD_LEN;
   uint16_t packetLen;
   uint16_t packetLenNet;

   memcpy(&packetLenNet, data, FLAG_OFFSET);
   packetLen = ntohs(packetLenNet);

   senderHandleLen = data[index];
   index += 1;
   memcpy(senderHandle, data + index, senderHandleLen);
   senderHandle[senderHandleLen] = NULL_CHAR;
   index += senderHandleLen;  //index now points to the message, which is null terminated

   printf("\n%s: %s\n", senderHandle, data + index);
}

void printHandleList(int numOfClients, char *data) {
   int i, index = NORMAL_HEAD_LEN;

   for (i = 0; i < numOfClients; i++) {
      printf("\t%.*s\n", data[index], data + index + 1);
      index += (1 + data[index]);
   }
}

//Finish implementing*****
//handle is user's handle, socketNum is server's socket
void dataFromServer(char *handle, int socketNum) {
   char *data = malloc(NORMAL_HEAD_LEN);
   int bufLen = -1, responseFlag = -1;
   uint8_t destHandleLen = 0;
   char destHandle[255 + NULL_CHAR_LEN];
   int numOfClientsNet;
   static int numOfClients = 0;

   bufLen = recvFunc(socketNum, data);

   responseFlag = data[FLAG_OFFSET];

   switch (responseFlag) {
      case FLAG_FOUR:
         printBroadcast(data);
         break;
      case FLAG_FIVE: 
         printClientMesg(data, handle);
         break;
      case FLAG_SEVEN:
         destHandleLen = data[NORMAL_HEAD_LEN];
         memcpy(destHandle, data + NORMAL_HEAD_LEN + 1, destHandleLen);
         destHandle[destHandleLen] = NULL_CHAR;
         printf("Client with handle %s does not exist.\n", destHandle);
         break;
      case FLAG_NINE:
         printf("\n");
         exit(0);
         break;
      case FLAG_ELEVEN:
         memcpy(&numOfClientsNet, data + NORMAL_HEAD_LEN, sizeof(int));
         numOfClients = ntohs(numOfClientsNet);
         printf("Number of clients: %d\n", numOfClients);
         break;
      case FLAG_TWELVE:
         printHandleList(numOfClients, data);
         break;
      default:
         printf("Bad flag value from server: %d\n", responseFlag);
         exit(-1);
   }

}



void primaryStateMachine(int argc, char * argv[]) {
   enum{INIT, WAIT, DATA_FROM_SERVER, IN_M, IN_B, IN_L, IN_E};
   static int state = INIT;
   int socketNum = 0;          //socket descriptor
   char *handle = argv[1]; //user's handle
   int waitReturn = -2;
   int bytesRead = -1;
   char letterBuf[3];
   int newLineFlag = 0;

   //add in a loop
   while (1) {
   //select happens in the wait state, so any time it is in another state, can
   // assume data is ready to recv
   switch(state) {
      case INIT:
         // check command line arguments 
         if(argc!= 4)
         {
            printf("usage: %s host-name port-number \n", argv[0]);
            exit(1);
         }

         /* set up the socket for TCP transmission  */
         socketNum = tcp_send_setup(argv[2], argv[3]);  //testing, add back in******

         init(handle, socketNum);
         state = WAIT; 
         break;
      case WAIT:
         waitReturn = wait(socketNum);
         if (waitReturn == FROM_STD_IN) {
            //parse the %letter, then choose appropriate state
            //read 3 bytes for the percent and the letter and the space
            bytesRead = read(STD_IN_FD, letterBuf, 3);  

            if (bytesRead < 2) {
               printf("command usage: percent[letter]\n");
            }

            if (letterBuf[1] == TYPE_M_1 || letterBuf[1] == TYPE_M_2) {
               state = IN_M;
            }
            else if (letterBuf[1] == TYPE_B_1 || letterBuf[1] == TYPE_B_2) {
               if (letterBuf[2] == '\n') {
                  newLineFlag = 1;
               }
               state = IN_B;
            }
            else if (letterBuf[1] == TYPE_L_1 || letterBuf[1] == TYPE_L_2) {
               state = IN_L;
            }
            else if (letterBuf[1] == TYPE_E_1 || letterBuf[1] == TYPE_E_2) {
               state = IN_E;
            }
            else {
               printf("Invalid command\n");
            }
         }
         else if (waitReturn == FROM_SERVER) {
            state = DATA_FROM_SERVER;
         } 
         break;
      case DATA_FROM_SERVER:
         dataFromServer(handle, socketNum);
         state = WAIT; 
         break;
      case IN_M:
         inM(handle, socketNum);
         state = WAIT; 
         break;
      case IN_B:
         inB(handle, socketNum, newLineFlag);
         state = WAIT; 
         break;
      case IN_L:
         sendNormalHeader(socketNum, FLAG_TEN);  //request list
         state = WAIT; 
         break;
      case IN_E:
         sendNormalHeader(socketNum, FLAG_EIGHT); //request to exit
         state = WAIT; 
         break;
      default:
         break;
   }   
   } //end of loop
}

int main(int argc, char * argv[])
{
    primaryStateMachine(argc, argv);
    return 0;
    
}


int tcp_send_setup(char *host_name, char *port)
{
    int socket_num;
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host

    // create the socket
    if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror("socket call");
      exit(-1);
    }
    

    // designate the addressing family
    remote.sin_family= AF_INET;

    // get the address of the remote host and store
    if ((hp = gethostbyname(host_name)) == NULL)
    {
      printf("Error getting hostname: %s\n", host_name);
      exit(-1);
    }
    
    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

    // get the port used on the remote side and store
    remote.sin_port= htons(atoi(port));

    if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
    {
      perror("connect call");
      exit(-1);
    }

    return socket_num;
}