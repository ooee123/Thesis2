#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "server.h"

static fd_set fds;

/*
 * Gets the port number from the commandline args, if possible
 * Input: argc - number of commandline args
 *        argv - array of commandline args
 * Return: a port number if possible, -1 if none was found
 */
int getPortNum(int argc, char **argv) {
   uint16_t val;
   char *temp;
   /*see if port was entered*/
   if (argc > 1) {
      val = strtol(argv[1], &temp, 10);

      if (temp == argv[1] || *temp != '\0') {
         printf("Did not enter a valid number for port\n");
         exit(EXIT_FAILURE);
      }
      return val;
   }
   return -1;
}

/*
 * Sets up the server's socket
 * Input: si - server info struct
 *        port - port number to open the socket on
 */
void setupServerSocket(serverInfo *si, int port) {
   int server_socket= 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */

   /*create the socket*/
   server_socket= socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0) {
      perror("socket call");
      exit(EXIT_FAILURE);
   }

   local.sin_family= AF_INET;         /*internet family*/
   local.sin_addr.s_addr= INADDR_ANY; /*wild card machine address*/
   local.sin_port= port == -1 ? htons(0) : htons(port); /*get a port*/

   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
      exit(EXIT_FAILURE);
   }
    
   /*get the port name and print it out*/
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
      perror("getsockname call");
      exit(EXIT_FAILURE);
   }

   printf("Server is using port %d \n", ntohs(local.sin_port));
   
   /*save the socket number*/
   si->socketNum = server_socket;
   si->highSock = server_socket;
}

/*
 * Listens for socket connections
 * Input: socketNum - the socket to listen on
 *        back_log - limits outstanding connections in the socket's listne queue
 */
void tcp_listen(int socketNum) {
   if (listen(socketNum, LISTEN_BACKLOG) < 0) {
      perror("listen call");
      exit(EXIT_FAILURE);
   }
}

/*
 * Accepts a new connection
 * Input: socketNum - the socket accepting connections
 * Return: the new client socket number
 */
int tcp_accept(int socketNum) {
   int client_socket = 0;
   if ((client_socket= accept(socketNum, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
      perror("accept call");
      exit(EXIT_FAILURE);
   }
   return client_socket;
}

/*
 * Removes a handle from the linked list, clears it from the fd_set, and
 *  recalculates the highest socket number
 * Input: si - server info struct
 *        socketNum - socket number of the node to remove
 */
void removeSocket(serverInfo *si, int socketNum) {
   freeHandle(&si->list, socketNum);
   FD_CLR(socketNum, &si->fdvar);

   /*recalculate highest socket number if needed*/
   if (si->highSock == socketNum) {
      si->highSock = findHighestSocket(si->list);  
   }
}

/*
 * Checks the first package sent by a client that verifies its handle
 * Input: si - server info struct
 *        senderSocket - the socket number of the client
 */
void checkInitPackage(serverInfo *si, int senderSocket) {
   int flag = FLAG_CLIENT_HANDLE_ERR;
   header *h = (header *)si->recBuf;
   char handle[MAX_HANDLE_LEN];

   /*get the handle in question from the buffer*/
   extractHandle(handle, si->recBuf, sizeof(header), ntohs(h->len) - sizeof(header));

   /*make sure the handle isn't already used*/
   if (!findHandle(si->list, handle)) {
      flag = FLAG_CLIENT_HANDLE_GOOD;
      setHandle(si->list, senderSocket, handle);
      si->listSize++;
   }
   else {
      removeSocket(si, senderSocket);
   }
   createHeader(&si->sendBuf, flag, sizeof(header));
   si->sendLen = sizeof(header);
}

/*
 * Builds a message packet, this can be either a broadcast or a message
 * Input: si - server info struct
 *        initLen - offset for where data goes
 *        senderLoc - offset for where to put the sender's handle
 */
void buildMessage(serverInfo *si, int initLen, int senderLoc) {
   int msgLen = sizeof(header);
   header *h = (header *)si->recBuf;
   int dataLoc = sizeof(header) + 1 + si->recBuf[3] + initLen;

   /*copy in the sender*/
   memcpy(si->sendBuf + sizeof(header), si->recBuf + senderLoc + 1, si->recBuf[senderLoc]);
   si->sendBuf[3 + si->recBuf[senderLoc]] = ':';
   si->sendBuf[4 + si->recBuf[senderLoc]] = ' ';
   msgLen += 2 + si->recBuf[senderLoc];

   /*copy in msg*/
   memcpy(si->sendBuf + msgLen, si->recBuf + dataLoc, ntohs(h->len) - dataLoc);
   msgLen += ntohs(h->len) - dataLoc;

   /*create header*/
   createHeader(&si->sendBuf, h->flag, msgLen);
   si->sendLen = msgLen;
}

/*
 * Fills in data for a message packet and checks to make sure that
 *  the destination handle exists
 * Input: si - server info struct
 *        senderSocket - the socket that is sending the packet
 * Return: the socket to send the packet to
 */
int startMessage(serverInfo *si, int senderSocket) {
   int nameLoc = sizeof(header) + 1 + si->recBuf[3];
   char name[MAX_HANDLE_LEN + 1];
   int msgLen = 1 + si->recBuf[3];
   int res;

   /*get the destination name ane make sure it exists*/
   extractHandle(name, si->recBuf, NAME_OFFSET, si->recBuf[3]);
   res = findHandle(si->list, name);

   /*handle not found, send handle not found packet*/
   if (res == 0) {
      msgLen = sizeof(header) + strlen(name) + 1;
      createHeader(&si->sendBuf, FLAG_BAD_HANDLE, msgLen);
      memcpy(si->sendBuf + NAME_OFFSET, name, strlen(name));
      si->sendBuf[3] = strlen(name);
      si->sendLen = msgLen;
      res = senderSocket;
   }
   else {
      buildMessage(si, 1 + si->recBuf[msgLen + sizeof(header)], nameLoc);
   }
   
   return res;
}

/*
 * Sends the list of handles to a socket
 * Input: si - server info struct
 */
void sendHandleList(serverInfo *si) {
   handleNode *cur = si->list;
   si->sendLen = sizeof(header);
   int curSize;

   createHeader(&si->sendBuf, FLAG_LIST, 0);

   while (cur != NULL) {
      curSize = strlen(cur->handle);
      /*check if theres room in the buffer*/
      if (si->sendLen + 1 + curSize < BUF_SIZE) {

         /*put size of handle*/
         si->sendBuf[si->sendLen] = curSize;
         si->sendLen++;

         /*put handle*/
         memcpy(si->sendBuf + si->sendLen, cur->handle, curSize);
         si->sendLen += curSize;
      }
      else {
         sendMessage(si->sendBuf, si->processList, si->sendLen);
         si->sendLen = sizeof(header);
         createHeader(&si->sendBuf, FLAG_LIST, 0);
      }
      cur = cur->next;
   }

   /*send any remaining info left*/
   if (si->sendLen != sizeof(header)) {
      sendMessage(si->sendBuf, si->processList, si->sendLen);
   }
   si->processList = 0;
}

/*
 * Parses a packet based on its flag and performs an appropriate action
 * Input: si - server info struct
 *        senderSocket - the socket that sent the data
 * Return: the socket to send the packet to
 */
int parsePacket(serverInfo *si, int senderSocket) {
   uint32_t *val;
   header *h = (header *)si->recBuf;
   int msgReceiver = senderSocket; /*most resposnes sent back to sender*/

   switch (h->flag) {
      case FLAG_CLIENT_INIT:
         checkInitPackage(si, senderSocket);
         break;
      case FLAG_BROADCAST:
         buildMessage(si, 0, sizeof(header));
         msgReceiver = SEND_ALL; 
         break;
      case FLAG_MESSAGE:
         msgReceiver = startMessage(si, senderSocket);
         break;
      case FLAG_REQ_EXIT:
         removeSocket(si, senderSocket);
         si->listSize--;

         createHeader(&si->sendBuf, FLAG_ACK_EXIT, sizeof(header));
         si->sendLen = sizeof(header);
         break;
      case FLAG_REQ_LIST:
         /*add the the number of headers into the buffer*/
         val = (uint32_t *) (si->sendBuf + sizeof(header));
         *val = htonl(si->listSize);
         memcpy(si->sendBuf + sizeof(header), (uint8_t *)val, sizeof(uint32_t));

         si->sendLen = sizeof(header) + sizeof(uint32_t);
         si->processList = senderSocket;
         createHeader(&si->sendBuf, FLAG_LIST_SIZE, si->sendLen);
         break;
      default:
         printf("Error: server received packet with flag %d\n", h->flag);
   }
   return msgReceiver;
}

/*
 * Sends a message to all clients except the sender
 * Input: si - server info struct
 *        sender - socket number of the sender
 */
void sendBroadcast(serverInfo *si, int sender) {
   handleNode *cur = si->list;
   while (cur != NULL) {
      if (sender != cur->socketNum) {
         sendMessage(si->sendBuf, cur->socketNum, si->sendLen);
      }
      cur = cur->next;
   }
}

/*
 * Checks the sockets for data, if there is then the packets are parsed and
 *  a response is sent
 * Input: si - server info struct
 */
void checkConnections(serverInfo *si) {
   handleNode *cur = si->list;
   int res;

   /*go through linked list*/
   while (cur != NULL) {
      if (FD_ISSET(cur->socketNum, &si->fdvar)) {

         /*get the entire msg, can block since user input will be so slow*/
         if (receiveMessage(&si->recBuf, INIT_RECV, cur->socketNum, BLOCKING_SERVER)) {

            res = parsePacket(si, cur->socketNum);
            
            /*send message*/
            if (res == SEND_ALL) {
               sendBroadcast(si, cur->socketNum);
            }
            else {
               sendMessage(si->sendBuf, res, si->sendLen);
            }

            /*clear buffers*/
            memset(si->recBuf, 0, BUF_SIZE);
            memset(si->sendBuf, 0, BUF_SIZE);
         }
         /*remove a socket, it exited (probably by Ctrl+c)*/
         else {
            removeSocket(si, cur->socketNum);
            si->listSize--;
         }
      }
      cur = cur->next;
   }

   /*Send the list of client handles*/
   if (si->processList) {
      sendHandleList(si);
   }
}

/*
 * Checsk sockets for input from clients
 * Input: si - server info struct
 */
void checkFds(serverInfo *si) {
   int res;
   handleNode *cur = si->list;

   /*set the fd_set*/
   FD_ZERO(&si->fdvar);
   FD_SET(si->socketNum, &si->fdvar);
   while (cur) {
      FD_SET(cur->socketNum, &si->fdvar);
      cur = cur->next;
   }

   res = selectCall(&si->fdvar, si->highSock);

   if (res > 0) {
      /*new connection*/
      if (FD_ISSET(si->socketNum, &si->fdvar)) {
         res = tcp_accept(si->socketNum);
         initHandle(&si->list, res);

         FD_SET(res, &fds);
         si->highSock = si->highSock < res ? res : si->highSock;
      }
      checkConnections(si);
   }
}

/*
 * Initialize serverInfo struct with intial values
 * Input: si - struct to initialize
 *        argc - number of commandeline args
 *        argv - list of commanline args
 */
void initServerInfo(serverInfo *si, int argc, char **argv) {
   int maxSize = sizeof(header) + MAX_HANDLE_LEN + MAX_HANDLE_LEN + 2 + BUF_SIZE;
   si->recBuf = (uint8_t *)malloc(maxSize);
   si->sendBuf = (uint8_t *)malloc(maxSize);
   si->sendLen = 0;
   si->listSize = 0;
   si->processList = 0;
   setupServerSocket(si, getPortNum(argc, argv));
}

/*
 * Free memory allocated memory
 * Input: si - the struct to free
 */
void cleanup(serverInfo *si) {
   handleNode *cur = si->list;
   while (cur) {
      freeHandle(&si->list, cur->socketNum);
      cur = si->list;
   }
   free(si->recBuf);
   free(si->sendBuf);
   free(si);
}

int main(int argc, char **argv) {
   serverInfo *si = (serverInfo *)malloc(sizeof(serverInfo));
   
   initServerInfo(si, argc, argv);
   tcp_listen(si->socketNum);

   /*loop forever*/
   while (1)
      checkFds(si);

   cleanup(si);
   return 0;
}