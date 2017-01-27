/******************************************************************************
* tcp_server.c
* Chris Casil, 9am Lab
* CPE 464 - Program 2
*****************************************************************************/
#include "networks.h"
#include "testing.h"

Client *client_table;

void initNormalHeader(NormalHeader *header, u_int *seqNum, u_short length, u_char flag) {
   header->seqNum = htons(++*seqNum);
   header->packetLength = htons(sizeof(NormalHeader) + length);
   header->flag = flag;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */
   
int tcp_recv_setup(int argc, char *argv[]) {
   int server_socket= 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */
   int port = 0;
   if (argc == 2){
      port = atoi(argv[1]);
   }
   else if(argc > 2)
   {
     printf("usage: %s port_number(optional) \n", argv[0]);
     exit(1);
   }

   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0)
   {
      perror("socket call");
      exit(1);
   }

   local.sin_family= AF_INET;         //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= htons(port);                 //let system choose the port

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

   printf("Server is using port %d \n", ntohs(local.sin_port));
   
   return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

void tcp_listen(int server_socket, int back_log) {
   if (listen(server_socket, back_log) < 0) {
      perror("listen call");
      exit(-1);
   }
}


int checkHandle(int tableCount, u_char *message) {
   int i = 0;

   while( i < tableCount) {
      if (!(strcmp((char *)client_table[i].handle, (char*)(message + 1)))) {
         return 1;
      }
      i++;
   }
   return 0;
}

void parseClient(Client *client, Message *message) {
   memcpy(&client->handle_len, message->text_message, sizeof(u_char) );
   memcpy(client->handle, message->text_message + sizeof(u_char), client->handle_len);
   client->handle[(int)client->handle_len] = '\0';
}


int sendNormalHeader(int socketCount, u_int *seqNum, u_char flag) {
   NormalHeader *header = malloc(sizeof(NormalHeader)); //might not need check later
   initNormalHeader(header, seqNum, 0, flag);
   return send(socketCount, header, ntohs(header->packetLength), 0);

}

void freeThis(int tableCount, int cSocket) {
   int i = 0;
   while( i < tableCount) {
      if(client_table[i].socketCount == cSocket) {
         client_table[i].socketCount = 0;
         client_table[i].handle[0] = 0;
      }
      i++;
   }
}

void broadcastClients(int tableCount, Message *message, int cSocket){
   int i = 0;

   while( i < tableCount) {
      if (client_table[i].socketCount != 0 && client_table[i].socketCount != cSocket) {
         if(send(client_table[i].socketCount, message, ntohs(message->header.packetLength), 0) < 0) {
            perror("send call");
            exit(-1);
         }
      }
      i++;
   }
}

void sendMessage(int tableCount, Message *message, int cSocket){
   int i = 0, check = -1, offset = 0;;
   u_char destLen = 0;
   u_char *dest_name = malloc(sizeof(255));
   Message *newMessage = malloc(sizeof(Message));
   memcpy(&destLen, message->text_message, sizeof(u_char));
   offset += sizeof(u_char);
   memcpy(dest_name, message->text_message + sizeof(u_char), destLen);
   offset += destLen;
   while(i < tableCount) {
      if (!(strcmp((char *)client_table[i].handle, (char*)(dest_name)))){
         check = i;
      }
      i++;
   }
   if (check == -1) {
      initNormalHeader(&newMessage->header, &(message->header.seqNum), offset, FLAG_7);
      memcpy(newMessage->text_message, &destLen, sizeof(u_char));
      memcpy(newMessage->text_message + sizeof(u_char), dest_name, destLen);
      if(send(cSocket, newMessage, ntohs(newMessage->header.packetLength), 0) < 0) {
         perror("send call");
         exit(-1);
      }
   }
   else {
      memcpy(newMessage->text_message, &destLen, sizeof(u_char));
      memcpy(newMessage->text_message + sizeof(u_char), dest_name, destLen);
      initNormalHeader(&newMessage->header, &(message->header.seqNum), offset, FLAG_6);
      if(send(cSocket, newMessage, ntohs(newMessage->header.packetLength), 0 ) < 0) {
         perror("send call");
         exit(-1);
      }
      if(send(client_table[check].socketCount, message, ntohs(message->header.packetLength), 0) < 0) {
         perror("send call");
         exit(-1);
      }
   }
}

u_int clientNum(int tableCount) {
   int i =0, numClients = 0;
   while(i < tableCount) {
      if(client_table[i].socketCount) {
         numClients++;
      }
      i++;
   }
   return numClients;
}

void flag11(Message *message, int tableCount, int cSocket) {
   Message *newMessage = malloc(sizeof(Message));
   u_int numClients = htonl(clientNum(tableCount));
   initNormalHeader(&newMessage->header, &(message->header.seqNum), sizeof(int), FLAG_11);
   memcpy(newMessage->text_message, &numClients, sizeof(u_int));
   if(send(cSocket, newMessage, ntohs(newMessage->header.packetLength), 0 ) < 0) {
      perror("send call");
      exit(-1);
   }
}

void flag12(Message *message, int tableCount, int cSocket) {
   int i = 0, data_len = 0;
   Message *newMessage = malloc(sizeof(Message));
   initNormalHeader(&newMessage->header, &(message->header.seqNum), 0, FLAG_12);
   newMessage->header.packetLength = htons(0);
   
   if(send(cSocket, &newMessage->header, sizeof(NormalHeader), 0) < 0) {
      perror("send call");
      exit(-1);
   }
   
   while(i < tableCount) {
      if (client_table[i].socketCount) {
         memcpy(newMessage->text_message + data_len, &client_table[i].handle_len, sizeof(u_char));
         data_len += sizeof(u_char);
         memcpy(newMessage->text_message+ data_len, client_table[i].handle, client_table[i].handle_len);
         data_len +=client_table[i].handle_len;
      }
      i++;
   }
   
   if(send(cSocket, newMessage->text_message, data_len, 0) < 0) {
      perror("send call");
      exit(-1);
   }   
}

void initMessage(Message *message, int tableCount, int cSocket, int location) {
   int error_flag; 
   
   message->header.seqNum = ntohs(message->header.seqNum);
   if (message->header.flag == FLAG_1) {
      error_flag = checkHandle( 
      tableCount, message->text_message);
      if (!error_flag) {
         parseClient(client_table + location, message);
         if(sendNormalHeader(cSocket, &(message->header.seqNum), FLAG_2) < 0) {
            perror("send call");
            exit(-1);
         }
      }
      else {
         freeThis( tableCount, cSocket);
         if(sendNormalHeader(cSocket, &(message->header.seqNum), FLAG_3) < 0) {
            perror("send call");
            exit(-1);
         }
         close(cSocket);
      }
   }

   switch(message->header.flag) {
      case FLAG_B: broadcastClients( tableCount, message, cSocket); break;
      case FLAG_M: sendMessage( tableCount, message, cSocket); break;
      case FLAG_E: ; 
      freeThis( tableCount, cSocket);
      if(sendNormalHeader (cSocket, &(message->header.seqNum), FLAG_ACK) < 0){
         perror("send call");
      }
      close(cSocket);
      break;
      case FLAG_L: 
      flag11(message, tableCount, cSocket);
      flag12(message, tableCount, cSocket);
      break;
   }
}



int findSocketCount(int tableCount, int max) {
   int i = 0;
   while(i < tableCount) {
      if (max < client_table[i].socketCount) {
         max = client_table[i].socketCount;
      }
      i++;
   }
   return max;
}

//dyanmic
int checkTable(int tableCount) {
   int i = 0, count = 0;
   
   while(i < tableCount) {  
      if((client_table + i)->socketCount) {
         count++;
      }
      i++;
   }

   if (tableCount-1 == count) {
      client_table = realloc(client_table, tableCount *2);
      tableCount = tableCount * 2;
   }
   return tableCount;
}

void clientSelect(int tableCount, Message *message, int bufSize, int server_socket) {
   int i = 0, newSocket = 0, msgLen = 0;    
   fd_set fdvar;
   int socketCount = findSocketCount(tableCount, server_socket);

   FD_ZERO(&fdvar);
   FD_SET(server_socket, &fdvar); 
   while(i < tableCount){
      if (client_table[i].socketCount > 0) {
         FD_SET(client_table[i].socketCount, &fdvar);
      }
      i++;
   }

   if (select(socketCount + 1, (fd_set *) &fdvar, (fd_set *) 0, (fd_set *) 0, NULL) < 0) {
      perror("select call");
   }


   if(FD_ISSET(server_socket, &fdvar)) {

      if ((newSocket= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
         perror("accept call");
         exit(-1);
      }
      for (i=0; i < tableCount; i++) {
         if(!(client_table[i].socketCount)) {
            client_table[i].socketCount = newSocket;
            break;
         }
      }
   }
   i =0;
   while(i < tableCount) {
      if (FD_ISSET(client_table[i].socketCount, &fdvar)) {
         if ((msgLen  = recv(client_table[i].socketCount, message, bufSize, 0)) < 0) {
            perror("recv call");
            exit(-1);
         }
         else if (!msgLen) {
            freeThis(tableCount, client_table[i].socketCount);
            close(client_table[i].socketCount);
         }
         else {
            initMessage(message, tableCount, client_table[i].socketCount, i);
         }
      }
      i++;
   }            
}

int main(int argc, char **argv) {
   int server_socket= 0, bufSize= MAX_LEN_MSG, tableCount = 20;  
   Message *message = malloc(sizeof(Message));
   client_table = calloc(tableCount, sizeof(Client));
   server_socket = tcp_recv_setup(argc, argv);
   tcp_listen(server_socket, 5);
   while (1) {
      tableCount = checkTable(tableCount);
      clientSelect(tableCount, message, bufSize, server_socket);
   }
}

