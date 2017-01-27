/******************************************************************************
 * tcp_client.c
 *
 *****************************************************************************/
#include "testing.h"
#include "tcp_read.h"

int fillFlag() {
   // Return 255 for Error in filling flag. anything else for success.
   char first, second, temp;
   uint8_t flag = 255;
   if (((first = getchar()) == EOF) || ((second = getchar()) == EOF)) { //invalid command
      flag = 255;
   }
   
   if (first == '%') {
      if (tolower(second) == 'm') { //Client messaging individual client
         flag = 5;
      }
      else if (tolower(second) =='b') { //Client broadcasting to all clients
         flag = 4;
      }
      else if (tolower(second) == 'l') { //Client Getting list of Handles
         flag = 10;
      }
      else if (tolower(second) == 'e') { //Client Exiting
         flag = 8;
      }
      else {
         flag = 255;
      } 
   }
   else {
      flag = 255;
   }
   if ((temp = getchar()) == '\n') { //Get space
      ungetc(temp, stdin);
   }
   else if (temp != ' ') {
      flag = 255;
   }
   return flag;
}





void checkHandle(int socketNum, Header *header, char *handle) {
   char *buffer;
   int selReturn;
   char *handlePacket = calloc(strlen(handle) + 1, sizeof(char));
   *handlePacket = strlen(handle);
   memcpy(handlePacket + 1, handle, strlen(handle));
   char *packet = genPacket(handlePacket, 1, SIZE_HEADER + strlen(handle) + 1, 1); //calloc packet 1
   fd_set rfds;
   FD_ZERO(&rfds);
   FD_SET(socketNum, &rfds);
   if(sendPacket(packet, socketNum, 1)) {
      perror("send call");
      exit(-1);
   }
   selReturn = select(socketNum + 1, &rfds, NULL, NULL, NULL);
   if (selReturn == -1) {
      perror("select()");
   }
   else if (selReturn) {
      while(1) {
         while ((buffer = readDataFromSocket(socketNum, 2)) == NULL) { //calloc buffer 1
         }
         if (*(buffer + 2) == 3) { //Error. Already exists 
            printf("Handle already in use: %s\n", handle);
            free(buffer);
            exit(0);
         }
         else if (*(buffer + 2) == 2) { //Good Handle
         }
         free(buffer); //free buffer 1
         break;
      }
   }
}


char *getData(char *srcHandle, uint8_t flagPrev) {
   char *packet;
   uint8_t flag = flagPrev, hasHandle = 0;
   int sendLen = 0, count = 0, more = 1, packetLen = 0;        //amount of data to send
   char buff[MAX_USER_INPUT], c;
   char *message;
   if (flag == 0) {
      flag = fillFlag();
      if (flag == 255) {
         printf("invalid command\n$: ");
         while((c = getchar()) != '\n' && c != EOF); //clear stdin
         return NULL;
      }
   }
   while ((buff[sendLen] = getchar()) != '\n' && count < MAX_TEXT_SIZE  && sendLen < MAX_USER_INPUT) {
      if ((flag == 4) || (flag == 5 && hasHandle)) {
         count++;
      }
      else if (flag == 5 && !(hasHandle) && (buff[sendLen] == ' ')) {
         hasHandle = 1;
      }
	   sendLen++;
   }
   if ((flag == 5 || flag == 4) && buff[sendLen] == '\n') {
      more = 0;
   }
   ungetc(buff[sendLen], stdin);
   message = genMessage(flag, sendLen, srcHandle, buff, more);
   if (message) {
      packetLen = strlen(message);
   }
   packetLen += SIZE_HEADER;
   packet = genPacket(message, flag, packetLen + SIZE_HEADER, 1);
   return packet;
}



void blockForHandles(uint32_t numHandles, int socketNum) {
   uint8_t handleLen;
   char *message, *buff;
   while (numHandles--) {
      while(!(buff = readDataFromSocket(socketNum, 1))) {
      }
      handleLen = *((uint8_t *)buff);
      message = calloc(handleLen + 1, sizeof(char));
      memcpy(message, buff + 1, handleLen);
      message[handleLen] = 0;
      printf("  %s\n", message);
      free(message);
      free(buff);
   }
   printf("$: ");
}


int decipherPacket(Header *header, char *data, int socketNum) {
   uint8_t srcHandLen, destHandLen;
   int textLen, term = 1;
   char *message = NULL; 
   uint8_t flag = header->flag, clear = 1;
   static uint32_t numHandles = 0;
   if (flag == 4) { //Broadcast %B
      textLen = ntohs(header->length) - SIZE_HEADER - 1 - *data;
      message = calloc(ntohs(header->length) - SIZE_HEADER + 2, sizeof(char));
      memcpy(message , data + 1, *data);
      memcpy(message + *data, ": ", 2);
      strcpy(message + *data + 2, data + *data + 1 );
   }
   else if (flag == 5){ //Client to client %M
      destHandLen = *((uint8_t *)data);
      srcHandLen = *((uint8_t *)(data + 1 + destHandLen));
      textLen = ntohs(header->length) - SIZE_HEADER - 2 - destHandLen;
      message = calloc(textLen, sizeof(char));
      memcpy(message, data + 2 + destHandLen, srcHandLen); //Copy src handle
      memcpy(message + srcHandLen, ": ", 2); //Copy ": "
      strcpy(message + srcHandLen + 2, data + 2 + destHandLen + srcHandLen); //Copy Message
   }
   else if (flag == 7) { //Client does not exist
      destHandLen = *((uint8_t *)data);
      message = calloc(destHandLen + 37, sizeof(char));
      memcpy(message, "Client with handle ", 19);
      memcpy(message + 19, data + 1, destHandLen);
      strcpy(message + 19 + destHandLen, " does not exist.");
   }
   else if (flag == 9){
      term = 0; 
   }
   else if (flag == 11){
      numHandles = ntohl(*((uint32_t *)data));   
      clear = 0;
   }
   else if (flag == 12){
      message = calloc(30, sizeof(char)); //10 for length of max 4 bytes number in decimal
      message = memcpy(message, "Number of clients: ", 19); //
      sprintf(message + 19, "%d", numHandles);
      clear = 1; 
   }
   else {
   }
   if (message) {
      if (flag == 7) {
         printf("\r%s\n$:", message);
      }
      else if (flag == 12) {
         printf("\r%s\n", message);
      }
      else  {
         printf("\r$: \n%s\n$: ", message);
      }
      free(message); //free message1
   }
   if (flag == 12) {
      blockForHandles(numHandles, socketNum);
   }
   if (clear) {
      numHandles = 0;
   }
   return term;
}

int clientAction(Socket *activeSocket, SocketBin *socketBin, int socketNum, char *srcHandle) {
   char *message, temp;           //buffer for receiving from client
   int clientSocket = 0;
   int flag = 0, length, term = 1;
   Header *header;
   int command = 0;
   char *packet;

   if (activeSocket->socketNum == socketNum) { //Data from Server
      while(!(packet = readDataFromSocket(socketNum, 2))) {
      }
      header = (Header *)packet;
      if ((packet + SIZE_HEADER) && strcmp(packet + SIZE_HEADER, "END") == 0 && header->flag == 0) { //Server terminated
         removeSocket(socketBin, socketNum);   
         printf("Server Terminated\n");
         return term = 0;
      }
      length = ntohs(header->length);
      flag = header->flag;
      message = packet + SIZE_HEADER;
      term = decipherPacket(header, message, socketNum);
      free(packet);
   }
   else { //Data from STDIN
      clientSocket = activeSocket->socketNum;
      while ((temp = getchar()) != '\n') {
         ungetc(temp, stdin);
         packet = getData(srcHandle, flag);
         if (packet == NULL) {
            return term = 1;
         }
         command = 1;
         flag = *(packet + 2);
         sendPacket(packet, socketNum, 1);
      }
     printf("$: ");
   }
   return term;
}
   

void startClient(int socketNum, int argc, char **argv) {
   Socket *activeSocket;
   SocketBin *socketBin = calloc(sizeof(SocketBin), 1); //calloc socketBin1
   increaseSocketBin(socketBin);
   addSocket(socketBin, NULL, 0);
   addSocket(socketBin, argv[1], socketNum);
   int max = socketNum;
   fd_set readFds, writeFds;
   FD_ZERO(&readFds);
   FD_ZERO(&writeFds);
   printf("$: ");
   while (1) {   
      FD_ZERO(&readFds);
      FD_SET(socketNum, &readFds);
      FD_SET(0, &readFds);
      activeSocket = listenSockets(&readFds, max, socketBin);
      if (clientAction(activeSocket, socketBin, socketNum, argv[1]) == 0) {
         break;
      }
   }
   removeSocket(socketBin, (socketBin->sockets + 1)->socketNum);
   free(socketBin);
   close(socketNum);
}

int main(int argc, char * argv[])
{
   int socketNum;         //socket descriptor
   setbuf(stdout, NULL);
   Header header;

   /* check command line arguments  */
   if(argc!= 4) {
      printf("usage: %s client-name host-name port-number \n", argv[0]);
	   exit(1);
   }

   /* set up the socket for TCP transmission  */
   socketNum = tcp_send_setup(argv[2], argv[3]);
   checkHandle(socketNum, &header, argv[1]); //
   startClient(socketNum, argc, argv);
   return 0; // exit
}



int tcp_send_setup(char *host_name, char *port) {
   int socket_num;
   struct sockaddr_in remote;       // socket address for remote side
   struct hostent *hp;              // address of remote host

   // create the socket
   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	   perror("socket call");
	   exit(-1);
	}
   

   // designate the addressing family
   remote.sin_family= AF_INET;

   // get the address of the remote host and store
   if ((hp = gethostbyname(host_name)) == NULL) {
	   printf("Error getting hostname: %s\n", host_name);
	   exit(-1);
	}
   
   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   // get the port used on the remote side and store
   remote.sin_port= htons(atoi(port));

   if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0) {
	   perror("connect call");
	   exit(-1);
   }

   return socket_num;
}

