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
#include <inttypes.h>

#include "networks.h"
#include "testing.h"
//#define DEBUG

char *buffer;                       //data buffer
int sendLen = 0;                    //amount of data to send
int sent = 0;                       //actual amount of data sent

char *serverName;                   //name of the server we want
int serverSocket;                   //socket descriptor of the server
int clientSocket;                   //socket descriptor of the client
char *clientHandle;                 //handle (name) of the client
uint8_t flag = 1;                   //flag for packet sent
int numClients = 0;


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

void tcp_call_send(char *packet, uint16_t length) {
   int dataSent = 0;

   #ifdef DEBUG
   printf("send - stuck here?\n");
   #endif
   if ((dataSent = send(serverSocket, packet, length, 0) < 0)) {
      perror("send error");
      exit(EXIT_FAIL);
   }
   #ifdef DEBUG
   printf("not stuck.\n");
   #endif
}

void clear() {
   printf("$: ");
   fflush(stdout);
}

void init_receive() {
   int dataReceived;
   struct NormalHeader *header = NULL;

   #ifdef DEBUG
      printf("recv - Stuck here?\n");
   #endif

   if ((dataReceived = recv(serverSocket, buffer, MAX_MSG_LEN, 0)) < 0) {
      perror("recv error");
      exit(EXIT_FAIL);
   }

   #ifdef DEBUG
   printf("Not stuck here.\n");
   printf("Read %d bytes from %d\n", dataReceived, serverSocket);
   #endif

   header = (struct NormalHeader *) buffer;

   if (header->flag == 2) {
   }
   else if (header->flag == 3) {
      fprintf(stderr, "Handle already in use: %s\n", clientHandle);
      close(serverSocket);
      exit(EXIT_FAIL);
   }
   else { 
      printf("init_recv - Not flag 2 or 3. Weird.\n");
      clear();
   }
}

void init_send() {
   char *packet;
   char *packetPointer;
   uint8_t handleLen;
   uint16_t packetLen;

   handleLen = strlen(clientHandle);
   packetLen = sizeof(struct NormalHeader) + 1 + handleLen;
   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);

   *packetPointer = CLIENT_INIT;
   packetPointer += sizeof(uint8_t);

   *packetPointer = handleLen;
   packetPointer += sizeof(uint8_t);

   memcpy(packetPointer, clientHandle, handleLen);

   tcp_call_send(packet, packetLen);
   init_receive();
}

void prepare_message(char *receiverHandle, char *message) {
   uint8_t senderLen = strlen(clientHandle);
   uint8_t receiverLen = strlen(receiverHandle);
   int messageLen = strlen(message + 1);
   char *packet;
   char *packetPointer;
   uint16_t packetLen;

   packetLen = sizeof(struct NormalHeader) + sizeof(uint8_t) + receiverLen + sizeof(uint8_t) + senderLen + messageLen;

   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);
   *packetPointer = MESSAGE;

   packetPointer += sizeof(uint8_t);
   *packetPointer = receiverLen;
   packetPointer += sizeof(uint8_t);

   memcpy(packetPointer, receiverHandle, receiverLen);
   packetPointer += receiverLen;

   *packetPointer = senderLen;
   packetPointer += sizeof(uint8_t);

   memcpy(packetPointer, clientHandle, senderLen);
   packetPointer += senderLen;

   memcpy(packetPointer, message, messageLen + 1);
   packetPointer += messageLen + 1;
   *packetPointer = '\0';
   #ifdef DEBUG
   printf("%s\n", packetPointer);
   #endif
   tcp_call_send(packet, packetLen + 1);
}

void client_message(char *input) {
   char *message;
   char *handle;
   int length;

   strtok(input, " ");
   handle = strtok(NULL, " ");

   length = strlen(handle);
   message = input + length + (sizeof(char) * 4);
   
   #ifdef DEBUG
   printf("%s\n", message);
   #endif

   prepare_message(handle, message);
}

void receive_message(int packetLen) {
   char *message;
   char *senderHandle;
   uint8_t senderLen;
   uint8_t destLen;
   int messageLen;

   #ifdef DEBUG
   printf("got a message\n");
   #endif

   destLen = *(buffer + sizeof(struct NormalHeader));

   senderLen = *(buffer + sizeof(struct NormalHeader) + sizeof(uint8_t) + destLen);
   senderHandle = malloc(senderLen + 1);
   memcpy(senderHandle, buffer + sizeof(struct NormalHeader) + sizeof(uint8_t) + destLen + sizeof(uint8_t), senderLen);
   messageLen = packetLen - sizeof(struct NormalHeader) - sizeof(uint8_t) - destLen - sizeof(uint8_t) - senderLen;
   #ifdef DEBUG
   printf("Message length: %d\n", messageLen);
   #endif
   message = malloc(messageLen + 1);
   strcpy(message, buffer + sizeof(struct NormalHeader) + sizeof(uint8_t) + destLen + sizeof(uint8_t) + senderLen);
   message[messageLen] = '\0';

   printf("\n%s: %s\n", senderHandle, message);
}

void prepare_broadcast(char *message) {
   char *packet;
   char *packetPointer;
   uint8_t senderLen = strlen(clientHandle);
   int messageLen = strlen(message);
   uint16_t packetLen;

   packetLen = sizeof(struct NormalHeader) + sizeof(uint8_t) + senderLen + messageLen;

   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);

   *packetPointer = BROADCAST;
   packetPointer += sizeof(uint8_t);

   *packetPointer = senderLen;
   packetPointer += sizeof(uint8_t);

   memcpy(packetPointer, clientHandle, senderLen);
   packetPointer += senderLen;

   strcpy(packetPointer, message);

   tcp_call_send(packet, packetLen);
}

void client_broadcast(char *input) {
   char *message;

   strtok(input, " ");

   message = input + (sizeof(char) * 3);

   prepare_broadcast(message);
}

void receive_broadcast(int packetLen) {
   char *message;
   char *senderHandle;
   uint8_t handleLen;
   int messageLen;

   handleLen = *(buffer + sizeof(struct NormalHeader));
   senderHandle = malloc(handleLen + 1);
   memcpy(senderHandle, buffer + sizeof(struct NormalHeader) + sizeof(uint8_t), handleLen);
   senderHandle[handleLen] = '\0';

   messageLen = packetLen - sizeof(struct NormalHeader) - sizeof(uint8_t) - handleLen;
   message = (char *) malloc(messageLen);
   strcpy(message, buffer + sizeof(struct NormalHeader) + sizeof(uint8_t) + handleLen);
   message[messageLen] = '\0';

   printf("\n%s: %s\n", senderHandle, message);
}

void client_list() {
   char *packet;
   char *packetPointer;
   int packetLen;

   packetLen = sizeof(struct NormalHeader);
   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);

   *packetPointer = CLIENT_LIST;

   tcp_call_send(packet, packetLen);
}

void receive_num_clients() {

   numClients = *(buffer + sizeof(struct NormalHeader));

   printf("Number of clients: %d\n", numClients);
}

void receive_client_list() {
   int i = 0;
   char *tempHandle = malloc(MAX_HANDLE_LEN);
   uint8_t tempHandleLen;
   char *packetPointer;

   packetPointer = buffer + sizeof(struct NormalHeader);
      for (i = 0; i < numClients; i++) {
      tempHandleLen = *packetPointer;
      packetPointer += sizeof(uint8_t);
      memcpy(tempHandle, packetPointer, tempHandleLen);
      tempHandle[tempHandleLen] = '\0';
      packetPointer += tempHandleLen;

      printf("\t%s\n", tempHandle);
   }  
}

void err_no_client(uint8_t flag) {
   uint8_t handleLen = *(buffer + sizeof(struct NormalHeader));
   char *handle;

   handle = malloc(handleLen + 1);
   memcpy(handle, buffer + sizeof(struct NormalHeader) + sizeof(uint8_t), handleLen);
   handle[handleLen] = '\0';

   if (flag == SERVER_BAD) {
      fprintf(stderr, "Handle already in use: %s\n", handle);     
   }
   else if (flag == MESSAGE_ERR) {
      fprintf(stderr, "Client with handle %s does not exist.\n", handle);
   }
}

void tcp_call_receive() {
   int messageLen;
   uint8_t flag;

   #ifdef DEBUG
   printf("recv - stuck here\n");
   #endif

   if ((messageLen = recv(serverSocket, buffer, MAX_MSG_LEN, 0)) < 0) {
      perror("recv error");
      exit(EXIT_FAIL);
   }

   #ifdef DEBUG
   printf("not stuck.\n");
   printf("Read %d bytes from %d\n\n", messageLen, serverSocket);
   #endif

   if (messageLen == 0) {
      fprintf(stderr, "Server terminated\n");
      exit(EXIT_FAIL);
   }

   flag = *(buffer + sizeof(uint16_t)); 

   #ifdef DEBUG
   printf("FLAG: %d\n", flag);
   #endif

   switch(flag) {
      case SERVER_GOOD:
         clear();
         break;
      case SERVER_BAD:
         err_no_client(SERVER_BAD);
         clear();
         break;
      case BROADCAST:
         receive_broadcast(messageLen);
         clear();
         break;
      case MESSAGE:
         receive_message(messageLen);
         clear();
         break;
      case MESSAGE_ERR:
         err_no_client(MESSAGE_ERR);
         clear();
         break;
      case SERVER_ACK:
         close(clientSocket);
         exit(EXIT_SUCCESS);
         break;
      case NUM_CLIENTS:
         receive_num_clients();
         break;
      case LIST_CLIENTS:
         receive_client_list();
         clear();
         break;
      default:
         printf("Uhhh... Unexpected flag: %d\n", flag);
         break;
   }
}

void client_exit() {
   char *packet;
   char *packetPointer;
   uint16_t packetLen;

   packetLen = sizeof(struct NormalHeader);
   packet = malloc(packetLen);
   packetPointer = packet;

   *packetPointer = htons(packetLen);
   packetPointer += sizeof(uint16_t);

   *packetPointer = CLIENT_EXIT;

   tcp_call_send(packet, packetLen);
}

void command_branch(char *input) {
   if (input[0] == '%') {
      switch (input[1]) {
         case 'M':
         case 'm':
            client_message(input);
            clear();
            break;
         case 'B':
         case 'b': 
            client_broadcast(input);
            clear();
            break;
         case 'L':
         case 'l':
            client_list();
            clear();
            break;
         case 'E':
         case 'e':
            client_exit();
            break;
         default:
            fprintf(stderr, "Invalid command\n");
            clear();
            break;
      }
   }
   else {
      fprintf(stderr, "Invalid command\n");
      clear();
   }
}

void tcp_call_select() {
   fd_set fds;
   char *input;
   int length = 0;

   FD_ZERO(&fds);

   FD_SET(clientSocket, &fds);
   FD_SET(serverSocket, &fds);

   #ifdef DEBUG
   printf("select - stuck here?\n");
   #endif

   if (select(FD_SETSIZE, (fd_set *) &fds, NULL, NULL, NULL) < 0) {
      perror("select error");
      exit(EXIT_FAIL);
   }

   #ifdef DEBUG
   printf("not stuck\n");
   #endif

   if (FD_ISSET(serverSocket, &fds)) {
      #ifdef DEBUG
      printf("Reading a packet sent from server\n");
      #endif
      tcp_call_receive();
   }

   if (FD_ISSET(clientSocket, &fds)) {
      input = (char *) malloc(MAX_MSG_LEN);

      while ((input[length] = getchar()) != '\n') {
         length++;
      }
      input[length] = '\0';

      #ifdef DEBUG
      printf("Sending: %s\n", input);
      #endif

      command_branch(input);
   }

   #ifdef DEBUG
   printf("This should repeat\n");
   #endif
}

int main(int argc, char * argv[]) {
   if(argc != 4 || strlen(argv[1]) > MAX_HANDLE_LEN) {  
      printf("usage: cclient handle server-name server-port \n");
      exit(EXIT_FAIL);
   }
   strcpy(clientHandle = malloc(strlen(argv[1]) + 1), argv[1]);
   strcpy(serverName = malloc(strlen(argv[2] + 1)), argv[2]);
   serverSocket = tcp_send_setup(argv[2], argv[3]);
   buffer = (char *) malloc(MAX_MSG_LEN);
   clientSocket = 0;
   clear();
   init_send();
   while (1) tcp_call_select();
   close(serverSocket);
   close(clientSocket);
   return 0;
}
