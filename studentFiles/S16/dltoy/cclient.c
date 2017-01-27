#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#include "cclient.h"

/*
 * Gets an argument from the commandline args
 * Input: argc: number of commandline args
 *        argv: array of commandline args
 *        ndx: index into argv
 *        dest: place to copy the arg to
 */
void getArg(int argc, char **argv, int ndx, char **dest) {
   /*make sure theres an arg there*/
   if (argc < ndx + 1) {
      printf("Usage: cclient handle server-name server-port\n");
      exit(EXIT_FAILURE);
   }
   /*copy it to a location*/
   *dest = (char *)malloc(sizeof(char) * strlen(argv[ndx]));
   if (dest == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
   }
   memmove(*dest, argv[ndx], strlen(argv[ndx]));
}

/*
 * Parses the commandline args
 * Input: ci - client info struct
 *        argc - number of commandline args
 *        argv - array of commandline args
 */
void parseArgs(clientInfo *ci, int argc, char **argv) {
   char *str, *temp;
   unsigned long val;
   getArg(argc, argv, 1, &ci->handle);
   getArg(argc, argv, 2, &ci->ipAddr);
   getArg(argc, argv, 3, &str);

   /*make sure handle is under allowed handle length*/
   if (strlen(ci->handle) >= MAX_HANDLE_LEN) {
      printf("Handle is too long, max length is 255\n");
      exit(EXIT_FAILURE);
   }

   /*convert port number to a number*/
   val = strtol(str, &temp, 10);

   if (temp == str || *temp != '\0') {
      printf("Did not enter a valid number for port\n");
      exit(EXIT_FAILURE);
   }
   ci->port = (uint16_t)val;
   free(str);
}

/*
 * Sets up the client's socket
 * Input: ci - client info struct
 */
void setupClientSocket(clientInfo *ci)
{
   int socket_num;
   struct sockaddr_in remote;       /* socket address for remote side */
   struct hostent *hp;              /* address of remote host         */

   /*create the socket*/
   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket call");
      exit(EXIT_FAILURE);
   }

   /*designate the addressing family*/
   remote.sin_family= AF_INET;

   /*get the address of the remote host and store*/
   if ((hp = gethostbyname(ci->ipAddr)) == NULL)
   {
     printf("Error getting hostname: %s\n", ci->ipAddr);
     exit(EXIT_FAILURE);
   }
    
   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   /*get the port used on the remote side and store*/
   remote.sin_port= htons(ci->port);

   if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
   {
      perror("connect call");
      exit(EXIT_FAILURE);
   }

   ci->socketNum = socket_num;
}

/*
 * Cleans up memory
 * Input: ci - client info struct
 */
void cleanup(clientInfo *ci) {
   free(ci->ipAddr);
   free(ci->handle);
   free(ci);
}

/*
 * Displays a message to the screen
 * Input: ci - client info struct
 */
void displayMsg(clientInfo *ci) {
   printf("\n%s\n", ci->recBuf + sizeof(header));
}

/*
 * Saves the number of handles from a message with flag 11
 * Input: ci - client info struct
 */
void saveHandleNum(clientInfo *ci) {
   ci->numHandles = ntohl(*((uint32_t *)(ci->recBuf + sizeof(header))));
}

/*
 * Prints out the list of current handles using packets from the server, this will
 *  block until it receives all the handles
 * Input: ci - client info struct
 */
void processHandleList(clientInfo *ci) {
   char name[MAX_HANDLE_LEN];
   uint8_t curLen;

   /*get rid of the flag 12*/
   receiveMessage(&ci->recBuf, ONLY_ONE, ci->socketNum, HANDLE_LIST);
   if (ci->recBuf[0] != FLAG_LIST) {
      return;
   }

   printf("Number of Clients: %u\n", ci->numHandles);
   
   /*receive all the handles*/
   while (ci->numHandles > 0) {
      /*get a length*/
      receiveMessage(&ci->recBuf, ONLY_ONE, ci->socketNum, HANDLE_LIST);
      curLen = ci->recBuf[0];

      /*get handle*/
      receiveMessage(&ci->recBuf, curLen, ci->socketNum, HANDLE_LIST);

      /*print handle*/
      extractHandle(name, ci->recBuf, 0, curLen);
      printf("  %s\n", name);
      ci->numHandles--;
   }
}

/*
 * Parses a packet based on its flag and performs an appropriate action
 * Input: ci - client info struct
 * Return: true when the client can exit
 */
int parsePacket(clientInfo *ci) {
   int res = 0;
   header *h = (header *)(ci->recBuf);
   char name[MAX_HANDLE_LEN];

   switch (h->flag) {
      case FLAG_CLIENT_HANDLE_GOOD:
         /*confirmed handle... nothing to do*/
         break;
      case FLAG_CLIENT_HANDLE_ERR:
         printf("Handle already in use %s\n", ci->handle);
         cleanup(ci);
         exit(EXIT_FAILURE);
         break;
      case FLAG_BROADCAST:
         displayMsg(ci);
         break;
      case FLAG_MESSAGE:
         displayMsg(ci);
         break;
      case FLAG_BAD_HANDLE:
         extractHandle(name, ci->recBuf, sizeof(header) + 1, ci->recBuf[3]);
         printf("Client with handle %s does not exist\n", name);
         break;
      case FLAG_ACK_EXIT:
         res = 1;
         break;
      case FLAG_LIST_SIZE:
         saveHandleNum(ci);
         break;
      default:
         printf("Error: client received a packet with flag %d\n", h->flag);
   }
   return res;
}

/* 
 * Verifies the handle with the server by sending the initial packet and
 *  then waiting for a response
 * Input: ci - client info struct
 */
void verifyHandle(clientInfo *ci) {
   int sendLen = strlen(ci->handle) + sizeof(header);

   /*create message to send*/
   createHeader(&ci->sendBuf, FLAG_CLIENT_INIT, sendLen);
   memcpy(ci->sendBuf + sizeof(header), ci->handle, strlen(ci->handle));
   sendMessage(ci->sendBuf, ci->socketNum, sendLen);
   
   /*parse server reply*/
   receiveMessage(&ci->recBuf, INIT_RECV, ci->socketNum, BLOCKING_CLIENT);
   parsePacket(ci);
}

/*
 * Builds a broadcast message
 * Input: ci - client info struct
 */
void buildBroadcast(clientInfo *ci) {
   uint16_t msgLen = sizeof(header) + 1 + strlen(ci->handle);

   /*copy in handle and its length*/
   ci->sendBuf[sizeof(header)] = strlen(ci->handle);
   memcpy(ci->sendBuf + sizeof(header) + 1, ci->handle, strlen(ci->handle));

   /*copy in message if there is one*/
   if (ci->sendLen > 3) {
      memmove(ci->sendBuf + msgLen, ci->recBuf + 3, ci->sendLen - 3);
      msgLen += ci->sendLen - 3;
   }

   /*create header*/
   createHeader(&ci->sendBuf, FLAG_BROADCAST, msgLen);
   ci->sendLen = msgLen;
}

/*
 * Builds a message to be sent to one other client
 * Input: ci - client info struct
 * Return: true on no error
 */
int buildMessage(clientInfo *ci) {
   uint16_t msgLen = sizeof(header) + 1;
   int recIter = 3; /*keeps track of where we are in the recBuf*/

   /*add destination handle - copy from recBuf*/
   while (ci->recBuf[recIter] != 0 && ci->recBuf[recIter] != ' ') {
      ci->sendBuf[msgLen++] = ci->recBuf[recIter++];
   }

   /*no name entered*/
   if (recIter == 3) {
      printf("Usage: %%M client-handle <message>\n");
      return 0;
   }

   /*add in length of destination handle*/
   ci->sendBuf[3] = recIter - 3;
   recIter++;

   /*add sender handle*/
   ci->sendBuf[msgLen++] = strlen(ci->handle);
   memcpy(ci->sendBuf + msgLen, ci->handle, strlen(ci->handle));
   msgLen += strlen(ci->handle);

   /*copy in message*/
   if (ci->sendLen >= recIter) {
      memmove(ci->sendBuf + msgLen, ci->recBuf + recIter, ci->sendLen - recIter);
      msgLen += ci->sendLen - recIter;
   }

   /*add header*/
   createHeader(&ci->sendBuf, FLAG_MESSAGE, msgLen);
   ci->sendLen = msgLen;
   return 1;
}

/*
 * Processes the input from the user and prepares a packet to be sent
 * Input: ci - client info struct
 * Return true if the data can be sent to the server
 */
int processUserInput(clientInfo *ci) {
   int rtn = 1;
   /*check for valid input*/
   if (ci->recBuf[0] == '%' && isalpha(ci->recBuf[1]) && 
      (ci->recBuf[2] == ' ' || ci->recBuf[2] == '\n' || ci->recBuf[2] == 0)) {
      switch (toupper(ci->recBuf[1])) {
         case 'M':
            if (buildMessage(ci) == 0)
               rtn = 0;
            break;
         case 'B':
            buildBroadcast(ci);
            break;
         case 'L':
            createHeader(&ci->sendBuf, FLAG_REQ_LIST, sizeof(header));
            ci->sendLen = sizeof(header);
            break;
         case 'E':
            createHeader(&ci->sendBuf, FLAG_REQ_EXIT, sizeof(header));
            ci->sendLen = sizeof(header);
            break;
         default:
            printf("Invalid command\n");
            rtn = 0;
      }
   }
   else {
      printf("Invalid command\n");
      rtn = 0;
   }
   return rtn;
}

/*
 * Gets input from the client and sends it to the server, may be sent in
 *  multiple sends if the message is too long
 * Input: ci - client info struct to store the input
 */
void sendClientInput(clientInfo *ci) {
   int res;
   ci->sendLen = 0;

   /*send the user input and split if too long*/
   while (((ci->recBuf)[ci->sendLen] = getchar()) != '\n') {
      ci->sendLen++;

      /*if the length of the data is over the buffer size*/
      if ((int)(ci->sendLen - sizeof(header) - 1 - strlen(ci->handle)) > BUF_SIZE - 1) {
         
         (ci->recBuf)[ci->sendLen++] = '\0';
         res = processUserInput(ci);
         if (res > 0) {
            sendMessage(ci->sendBuf, ci->socketNum, ci->sendLen);
            
            /*reset sendlen*/
            ci->sendLen = sizeof(header);

            /*if %M, accomodate for extra handle*/ 
            if (ci->sendBuf[2] == FLAG_MESSAGE) { 
               ci->sendLen += 1 + ci->sendBuf[3];
            }
         }
      }
   }

   (ci->recBuf)[ci->sendLen++] = '\0';
   res = processUserInput(ci);
   if (res > 0) {
      sendMessage(ci->sendBuf, ci->socketNum, ci->sendLen);
   }
}

/*
 * Reads from the server and parses when a message is ready
 * Input: ci - the client info struct
 * Return: true if the client can exit
 */
int getMsg(clientInfo *ci) {
   int res = 0;
   ci->recLen = receiveMessage(&ci->recBuf, ci->recLen, ci->socketNum, NONBLOCKING);

   /*remove the $: if not sending a message or broadcast*/
   if (ci->recBuf[2] != 4 && ci->recBuf[2] != 5) {
      printf("\33[2K\r");
   }

   /*check if received the handle list*/
   if (getLen(ci->recBuf) == 0) {
      processHandleList(ci);
      ci->recLen = 0;
   }
   /*see if an entire packet has been found or if receiving process handle*/
   else if (ci->recLen > 2 && ci->recLen == getLen(ci->recBuf)) {
      res = parsePacket(ci);
      ci->recLen = 0;
   }

   return res;
}

/*
 * Checks the sockets for input using the select call
 * Input: ci - client info struct
 * Return: true when the client can exit
 */
int checkFds(clientInfo *ci) {
   int res, done = 0;

   FD_ZERO(&ci->fdvar);
   FD_SET(0, &ci->fdvar);
   FD_SET(ci->socketNum, &ci->fdvar);

   res = selectCall(&ci->fdvar, ci->socketNum);
   if (res > 0) {
      /*check stdin*/
      if (FD_ISSET(0, &ci->fdvar)) {
         sendClientInput(ci);
      }
      /*check if msg from socket*/
      else if (FD_ISSET(ci->socketNum, &ci->fdvar)) {
         done = getMsg(ci); 
      }

      if (done == 0) {
         printf("$: ");
         fflush(stdout);

         if (ci->recLen == 0) {
            memset(ci->recBuf, 0, BUF_SIZE);
         }
         memset(ci->sendBuf, 0, BUF_SIZE);
      }
   }
   return done == 1;
}

/*
 * Initializes the clientInfo struct that holds data that will be passed around
 * Input: ci - struct to initialize
 *        argc - number of commandline args
 *        argv - array of commandline args
 */
void initClientInfo(clientInfo *ci, int argc, char **argv) {
   int maxSize = sizeof(header) + MAX_HANDLE_LEN + MAX_HANDLE_LEN + 2 + BUF_SIZE;
   ci->recBuf = (uint8_t *)malloc(maxSize);
   ci->sendBuf = (uint8_t *)malloc(maxSize);
   ci->recLen = 0;
   ci->sendLen = 0;
   ci->numHandles = 0;
   parseArgs(ci, argc, argv);
   setupClientSocket(ci);
}

int main(int argc, char **argv) {
   clientInfo *ci = (clientInfo *)malloc(sizeof(clientInfo));
  
   initClientInfo(ci, argc, argv);
   verifyHandle(ci);
   
   /*print out ready state*/
   printf("$: ");
   fflush(stdout);

   while (checkFds(ci) == 0);

   cleanup(ci);
   return 0;
}