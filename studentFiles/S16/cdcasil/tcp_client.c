/******************************************************************************
* tcp_client.c
* Chris Casil, 9am Lab
* CPE 464 - Program 2
*****************************************************************************/

#include "networks.h"
#include "testing.h"


void initNormalHeader(NormalHeader *header, u_int *seqNum, u_short length, u_char flag) {
   header->seqNum = htons(++*seqNum);
   header->packetLength = htons(sizeof(NormalHeader) + length);
   header->flag = flag;
}

void initClientHandle(int argc, char * argv[], u_char *clientHandle) {
   if (strlen(argv[1]) > MAX_HANDLE) { 
      printf("Handle name is longer than %d\n", MAX_HANDLE); 
      exit(1); 
   }
   if(argc != 4) {
      printf("%s: not enought arguments \n", argv[0]);
      exit(1);
   }
   memcpy(clientHandle, argv[1], strlen(argv[1]));
}

int initTargetHandle(char *cmd, u_char *handle) {
   int str = 0, handleCount = 0;
   while(str < strlen(cmd)) {
      if(handleCount == MAX_HANDLE || cmd[str] == ' ' || cmd[str] == '\n') {
         handle[handleCount] = '\0';
         return handleCount;
      }
      handle[handleCount] = cmd[str];
      handleCount++;
      str++;
   }
   handle[handleCount] = '\0';
   return handleCount;
}

void initMessage(Message *message) {
   int messageLen = ntohs(message->header.packetLength) - sizeof(NormalHeader) - sizeof(u_char);
   u_char dest_len = 0, sender_len = 0;
   u_char *dest_name = malloc(sizeof(255));
   u_char *sender_name = malloc(sizeof(255));
   u_char *parsed_message = malloc(MAX_LEN_MSG);
   int offset = 0;
   memcpy(&dest_len, message->text_message, sizeof(u_char));
   offset += sizeof(u_char);
   memcpy(dest_name, message->text_message + offset, dest_len);
   offset += dest_len;
   messageLen -= dest_len;
   memcpy(&sender_len, message->text_message + offset, sizeof(u_char));
   offset += sizeof(u_char);
   messageLen -= sizeof(u_char);
   memcpy(sender_name, message->text_message + offset, sender_len);
   offset += sender_len;
   messageLen -= sender_len;
   memcpy(parsed_message, message->text_message + offset, messageLen);
   printf("\n%s: %s \n", sender_name, parsed_message);
   printf("$: ");
   fflush(stdout);
}

int getData(char* input) {
   int len = 0; 

   while ((input[len] = getchar()) != '\n' && len < MAX_LEN_MSG - 1) {
      len++;
   }

   input[len] = '\n';
   return len;
}

u_short rallyMessage(char* input, u_char *message) {
   int messageLen= 0, messageNdx = 0;
   if (input[messageNdx++] != '\n') {
      for( messageLen = 0; ((message[messageLen] = input[messageNdx]) != '\n' && messageLen < MAX_LEN_MSG - 1); messageLen++){
         messageNdx++;
      }
   }
   message[messageLen] = '\0';
   return messageLen + 1;
}

int tcp_send_setup(char *host_name, char *port) {
   int socketNum;
   struct sockaddr_in remote;       // socket address for remote side
   struct hostent *hp;              // address of remote host

   // create the socket
   if ((socketNum = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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

   if(connect(socketNum, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
   {
      perror("connect call");
      exit(-1);
   }

   return socketNum;
}

int handleMessage(u_char *clientHandle, char *cmd, int socketNum, u_int *seqNum, u_char flag) {
   Message *message = malloc(sizeof(Message));
   u_char dest_size = 0;
   u_char *handle_name_dest = malloc(sizeof(255));
   u_char send_size = strlen((char*)clientHandle);
   u_char text[MAX_LEN_MSG];
   u_short messageLen;
   u_short dataLen = 0;

   if (flag == FLAG_B) {
      messageLen = rallyMessage(cmd - 1, text);
   }
   else if (flag == FLAG_M) {
      dest_size = initTargetHandle(cmd, handle_name_dest);
      messageLen = rallyMessage(cmd + dest_size, text);
      
      memcpy(message->text_message, &dest_size, sizeof(u_char));
      
      dataLen += sizeof(u_char);
      memcpy(message->text_message +  dataLen, handle_name_dest, dest_size);
      dataLen += dest_size;
   }
   

   memcpy(message->text_message +  dataLen, &send_size, sizeof(u_char));
   dataLen += sizeof(u_char);
   memcpy(message->text_message + dataLen, clientHandle, send_size);
   dataLen += send_size;
   memcpy(message->text_message +  dataLen, text, messageLen);
   dataLen += messageLen;

   initNormalHeader(&message->header, seqNum, dataLen, flag);

   return send(socketNum, message, ntohs(message->header.packetLength), 0);
}


int normalHeader(int socketNum, u_int *seqNum, u_char flag) {
   NormalHeader *header = malloc(sizeof(NormalHeader));

   initNormalHeader(header, seqNum, sizeof(NormalHeader), flag);
   return send(socketNum, header, ntohs(header->packetLength), 0);
}

void nextCmd(char *input, u_char *clientHandle, u_int *seqNum, int socketNum)  {
   int len = 0, sent = 0;
   input[len] = getchar();

   while (input[len] != ' ' && input[len] != '\n' && len < 3) {
      len++;
      input[len] = getchar();
   }
   if (tolower(*++input) == 'm') {
      if(!getData(input)) {
         printf("Error, no handle given\n");
         
      }
      else
         sent =  handleMessage(clientHandle, input, socketNum, seqNum, FLAG_M);
   }
   else if(tolower(*input) == 'b') {
      getData(input);
      sent =  handleMessage(clientHandle, input, socketNum, seqNum, FLAG_B);

   }
   else if(tolower(*input) == 'l') {
      sent = normalHeader(socketNum, seqNum, FLAG_L);
   }
   else if(tolower(*input) == 'e') {
      sent = normalHeader(socketNum, seqNum, FLAG_E);
   }
   else {
      printf("Invalid Command\n");
      printf("$: ");
      fflush(stdout);
   }

   if(sent < 0) {
      perror("send call");
      exit(-1);
   }
}

void flag1(int socketNum, u_int *seqNum, u_char *clientHandle) {
   u_short dataLen = 0;
   Message *message = malloc(sizeof(Message));
   u_char send_size = strlen((char*)clientHandle);
   memcpy(message->text_message, &send_size, sizeof(u_char));
   dataLen += sizeof(u_char);
   memcpy(message->text_message + dataLen, clientHandle, send_size);
   dataLen += send_size;
   initNormalHeader(&message->header, seqNum, dataLen, FLAG_1);
   if(send(socketNum, message, ntohs(message->header.packetLength), 0) < 0) {
      perror("send call");
      exit(-1);
   }
   printf("$: ");
   fflush(stdout);
}

void checkConnection(int socketNum, u_char *clientHandle){
   int messageLen = 0, flag = 0;
   Message *message = malloc(sizeof(Message));
   while (!flag) {
      if ((messageLen = recv(socketNum, message, MAX_LEN_MSG, 0)) < 0) {
         perror("recv call");
         exit(-1);
      }
      if (message->header.flag == FLAG_2) {
         flag = 1;
      }
      else if (message->header.flag == FLAG_3) {
         flag = -1;
      }
   }
   if (flag == -1) {
      printf("Handle already in use: %s\n", clientHandle);
      close(socketNum);
      exit(1);
   }
}

void broadcast(Message *message) {
   int messageLen = ntohs(message->header.packetLength) - sizeof(NormalHeader) - sizeof(u_char);
   u_char *parsed_message = malloc(MAX_LEN_MSG);
   u_char handle_len = 0;
   u_char *handle_name = malloc(sizeof(255));
   int offset = 0;
   memcpy(&handle_len, message->text_message, sizeof(u_char));
   offset += sizeof(u_char);
   messageLen -= handle_len;
   memcpy(handle_name, message->text_message + offset, handle_len);
   offset += handle_len;
   memcpy(parsed_message, message->text_message + offset, messageLen);
   printf ("\n%s: %s\n", handle_name, parsed_message);
   printf("$: ");
   fflush(stdout);
}

void clientError(Message *message){
   u_char handle_len = 0;
   u_char *handle_name = malloc(sizeof(255));
   memcpy(&handle_len, message->text_message, sizeof(u_char));
   memcpy(handle_name, message->text_message + sizeof(u_char), handle_len);
   printf("Client with handle %s does not exist\n", handle_name);
   printf("$: ");
   fflush(stdout);
}

void printClients(Message * message, u_int *num_clients, int socketNum) {
   int i = 0, messageLen = 0;
   Message *new_message;
   u_char handle_len = 0;
   printf("Number of clients: %d\n", ntohl(*num_clients));
   
   while( i < ntohl(*num_clients)) {
      new_message = malloc(sizeof(Message));
      memcpy(&handle_len, message->text_message + messageLen, sizeof(u_char));
      messageLen += sizeof(u_char);
      memcpy(new_message->text_message, message->text_message + messageLen, handle_len);
      messageLen += handle_len;
      printf("   %s\n", new_message->text_message);
      free(new_message);
      i++;
   }
}

void serverSelect(int socketNum, char *input, u_char *clientHandle, u_int *seqNum, u_int *num_clients) {
   int messageLen = 0;
   fd_set fdset;
   
   struct timeval timeout;
   timeout.tv_sec = 1;
   timeout.tv_usec = 0;
   Message *message = malloc(sizeof(Message));
   FD_ZERO(&fdset);
   FD_SET(0, &fdset);
   FD_SET(socketNum, &fdset);

   if (select(socketNum + 1, (fd_set *) &fdset, (fd_set *) 0, (fd_set *) 0, NULL) < 0) {
      perror("select call");
      exit(-1);
   }
   
   if (FD_ISSET(socketNum, &fdset)) {
      if ((messageLen = recv(socketNum, message, MAX_LEN_MSG, 0)) < 0) {
         perror("recv call");
         exit(-1);
      } else if (!messageLen) {
         printf("Server Terminated\n");
         close(socketNum);
         exit(1);
      }

      switch(message->header.flag) {
         case FLAG_B: broadcast(message); break;
         case FLAG_M: initMessage(message); break;
         case FLAG_6: ; break;
         case FLAG_7: clientError(message); break;
         case FLAG_ACK: printf("\n"); exit(1); break;
         case FLAG_11: 
         memcpy(num_clients, message->text_message, sizeof(u_int)); 
         break;
         case FLAG_12:
         FD_CLR(0, &fdset);
         printClients(message, num_clients, socketNum);
         printf("$: ");
         fflush(stdout);
         break;
      }
   }
   if (FD_ISSET(0, &fdset)) {
      printf("$: ");
      fflush(stdout);
      nextCmd(input, clientHandle, seqNum, socketNum);
   }
}

int main(int argc, char **argv) {     
   char *send_buf = (char *) malloc(MAX_LEN_MSG);
   u_char *clientHandle = malloc(sizeof(MAX_HANDLE));
   u_int *seqNum = malloc(sizeof(u_int));
   u_int *num_clients = malloc(sizeof(u_int));
   *seqNum = 0, *num_clients = 0;
   initClientHandle(argc, argv, clientHandle);
   int socketNum = tcp_send_setup(argv[2], argv[3]);
   flag1(socketNum, seqNum, clientHandle);
   checkConnection(socketNum, clientHandle);
   while(1) {
      serverSelect(socketNum, send_buf, clientHandle, seqNum, num_clients);
   }
}