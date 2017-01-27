//Nick Verbos
//tcp server for chat program

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

#include "testing.h"
#include "networks.h"
#include "tcp_server.h"


int main(int argc, char *argv[]) {
	Server server;

	setup(argc, argv, &server);
   

   while(1){
	   processNextRequest(&server);
   }
	
	/* close the sockets */
	cleanup(&server);
	
	return SUCCESS;
}


void setup(int argc, char *argv[], Server *s) {
	s->server_socket = 0;   //socket descriptor for the server socket
	s->client_socket = 0;   //socket descriptor for the client socket
	s->buf = NULL;        //buffer for receiving from client
	s->handleList = NULL;
   FD_ZERO(&s->fdSet);
   int port = 0;

   if(argc == 2){
      if((port = atoi(argv[1]))){
         //port selected
      } else {
         puts("Invalid port number.  Exiting.");
         exit(-1);
      }
   }
	
	//create packet buffer
	if ((s->buf = (char *) malloc(MAXBUF)) == NULL)
	{
		perror("malloc() error");
		exit(-1);
	}
	
	//create the server socket
	s->server_socket = tcpServerSetup(port);
   FD_SET(s->server_socket, &s->fdSet);
   s->highestFd = s->server_socket;
}

/* Closes all connections and frees dynamic memory
 */
void cleanup(Server *s){
	close(s->server_socket);
	close(s->client_socket);
}

/* Wait for a request and then set up the server to handle it
 */
void processNextRequest(Server *s){
   ChatHeader ch;
   int readSize, lenPacket;
   int nextFd = getNextReadyFd(s);
	
   if(nextFd == s->server_socket){
      s->client_socket = tcpAccept(s->server_socket);
   } else {
      s->client_socket = nextFd;
   }
   
	if ((readSize = recv(s->client_socket, &ch, sizeof(ChatHeader), 0)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

   memcpy(s->buf, &ch, sizeof(ChatHeader));
   lenPacket = htons(ch.packetLen);
   
   if(readSize == 0){ 
      removeUserWithSocket(s->client_socket, s);
   } else {
      if(lenPacket != sizeof(ChatHeader)){
         if (recv(s->client_socket, s->buf + sizeof(ChatHeader), lenPacket - sizeof(ChatHeader), 0) < 0)
         {
            perror("recv call");
            exit(-1);
         }
      }
	   handleRequest(s);
   }
}

int getNextReadyFd(Server *s){
  	HandleNode *current = s->handleList;
   
   resetFdSet(s);
   if(select(s->highestFd + 1, &s->fdSet, NULL, NULL, NULL) == -1){
      perror("select call");
   }

   if(FD_ISSET(s->server_socket, &s->fdSet)){
      return s->server_socket;
   }

	while(current){
		if(FD_ISSET(current->socket, &s->fdSet)){
         return current->socket;
      }
      current = current->next;
	}
   return -1;
}

/* Determine the request type and send it to the correct helper function to
 * process it properly
 */
void handleRequest(Server *s) {
	ChatHeader *ch = (ChatHeader *)s->buf, response;
	switch (ch->flag) {
		case INITIAL_PACKET :
			handleInitialPacket(s);
			break;

		case MESSAGE :
			handleMessagePacket(s);
			break;

      case EXIT :
			removeUserWithSocket(s->client_socket, s);
         response.packetLen = htons(sizeof(ChatHeader));
         response.flag = ACK;
	      send(s->client_socket, &response, sizeof(ChatHeader), 0);
         close(s->client_socket);
			break;

      case BROADCAST : 
         handleBroadcastPacket(s);
         break;

      case REQUEST_HANDLES : 
         sendHandles(s);
         break;   

		default :
			puts("Unknown packet flag");
         exit(1);
			break;
	}
}

void sendHandles(Server *s){
   ChatHeader ch;
   HandleNode *current = s->handleList;
   uint32_t numHandlesNetwork, numHandles = 0;

   ch.flag = NUMBER_HANDLES;
   ch.packetLen = htons(sizeof(ChatHeader) + sizeof(uint32_t));

   while(current != NULL){
      numHandles++;
      current = current->next;
   }
   numHandlesNetwork = htonl(numHandles);

   memcpy(s->buf, &ch, sizeof(ChatHeader));
   memcpy(s->buf + sizeof(ChatHeader), &numHandlesNetwork, sizeof(uint32_t));

   send(s->client_socket, s->buf, sizeof(ChatHeader) + sizeof(uint32_t), 0);

   current = s->handleList;
   ch.flag = HANDLE;

   while(current != NULL){
      ch.flag = HANDLE;
      ch.packetLen = htons(sizeof(ChatHeader) + sizeof(u_char) + current->handleLen);
      memcpy(s->buf, &ch, sizeof(ChatHeader));
      memcpy(s->buf + sizeof(ChatHeader), &(current->handleLen), sizeof(u_char));
      memcpy(s->buf + sizeof(ChatHeader) + sizeof(u_char), current->handle, current->handleLen);
      send(s->client_socket, s->buf, ntohs(ch.packetLen), 0);
      current = current->next;
   }

}

/* Handles an initial packet by adding the users handle and sending an
 * appropriate response to the client
 */
void handleInitialPacket(Server *s) {
	ChatHeader response;
   char *handle;
   u_char handleLen;

   handleLen = *(s->buf + sizeof(ChatHeader));
   handle = s->buf + sizeof(ChatHeader) + 1;
   
	response.packetLen = htons(sizeof(ChatHeader));

	if(addUser(handle, handleLen, s) == SUCCESS) {
		response.flag = INITIAL_SUCCESS;
	} else {
		response.flag = INITIAL_ERROR;
	}

	send(s->client_socket, &response, sizeof(ChatHeader), 0);
}

/* Forwards a message packet to the appropriate recipient or sends an error to
 * the sender if the recipient does not exist
 */
void handleMessagePacket(Server *s) {
   u_char *flag, *destHandleLen, *fromHandleLen;
   char *destHandle, *fromHandle, *message;
   u_short *packetLenPtr, packetLen;
   int destSocket;

   packetLenPtr = (u_short *)s->buf;
   packetLen = *packetLenPtr;
   flag = (u_char *)s->buf + sizeof(short);
   destHandleLen = flag + 1;
   destHandle = (char *)destHandleLen + 1;
   fromHandleLen = (u_char *)destHandle + *destHandleLen;
   fromHandle = (char *)fromHandleLen + 1;
   message = (char *)fromHandle + *fromHandleLen;

   destSocket = lookupSocketForHandle(destHandle, *destHandleLen, s);
  
   if(destSocket == HANDLE_NOT_FOUND){
      *packetLenPtr = htons(sizeof(ChatHeader) + *destHandleLen + 1);
      *flag = MESSAGE_ERROR;
      send(s->client_socket, s->buf, ntohs(*packetLenPtr), 0);
   } else {
      send(destSocket, s->buf, ntohs(packetLen), 0);
   }
}

void handleBroadcastPacket(Server *s) {
   u_char *flag, *fromHandleLen;
   char *fromHandle, *message;
   u_short *packetLenPtr, packetLen;
   HandleNode *current = s->handleList;

   packetLenPtr = (u_short *)s->buf;
   packetLen = *packetLenPtr;
   flag = (u_char *)s->buf + sizeof(short);
   fromHandleLen = flag + 1;
   fromHandle = (char *)fromHandleLen + 1;
   message = (char *)fromHandle + *fromHandleLen;
   
   while(current){
      if(current->socket != s->client_socket){
         send(current->socket, s->buf, ntohs(packetLen), 0);
      }
      current = current->next;
   } 
}


/* Adds a handle and its socket number to the list of currently active handles
 * Returns: SUCCESS or an error code
 */
int addUser(char *handle, u_char handleLen, Server *s){
	HandleNode *current, *prev, *new;

	current = s->handleList;
   prev = NULL;


	if(lookupSocketForHandle(handle, handleLen, s) != HANDLE_NOT_FOUND) {
		return HANDLE_ALREADY_EXISTS; //This handle name is already being used
	}
   
	new = malloc(sizeof(HandleNode));
	new->handle = malloc(handleLen);
	memcpy(new->handle, handle, handleLen);
	new->handleLen = handleLen;
	new->socket = s->client_socket;
	new->next = NULL;

	while(current){ //Iterate to the end of the list
		prev = current;
		current = current->next;
	}
   
   if(prev){
	   prev->next = new;
   } else {
      s->handleList = new; //The handleList was empty
   }
   
   FD_SET(s->client_socket, &s->fdSet);

   if(s->client_socket > s->highestFd){
      s->highestFd = s->client_socket;
   }

	return SUCCESS;
}

/* Finds the socket number for a given handle or determine that it does not
 * exist
 * Returns: The associated socket number or HANDLE_NOT_FOUND
 */
int lookupSocketForHandle(char *handle, u_char handleLen, Server *s) {
	HandleNode *current = s->handleList;
	
   while(current) {
		if(current->handleLen == handleLen){
			if(memcmp(current->handle, handle, handleLen) == 0){
				return current->socket;
			}
		}
		current = current->next;
	}
   
	return HANDLE_NOT_FOUND;
}

/* Finds the handle number for a given socket or determine that it does not
 * exist
 * Returns: A pointer to the associated handle or NULL if no socket is
 * associated to the given handle
 */
char *lookupHandleForSocket(int socketNum, Server *s) {
	HandleNode *current = s->handleList;

	while(current) {
      if(current->socket == socketNum){
	      return current->handle;
		}
		current = current->next;
	}
   
	return NULL;
}

void removeUserWithSocket(int socketNum, Server *s){
	HandleNode *prev, *current = s->handleList;
   prev = NULL;

	while(current) {
      if(current->socket == socketNum){
         if(prev == NULL){
            free(current->handle);
            free(current);
            s->handleList = NULL;
         } else {
            free(current->handle);
            free(current);
            prev->next = current->next;
         }
		}
      prev = current;
		current = current->next;
	}

   FD_CLR(socketNum, &s->fdSet);
   FD_SET(s->server_socket, &s->fdSet);

   if(s->highestFd == socketNum){
      s->highestFd = s->server_socket;
      current = s->handleList;
      while(current){
         if(current->socket > s->highestFd){
            s->highestFd = current->socket;
         }
         current = current->next;
      }
   }
}


/* Prints out all of the handles that are currently being used
 */
void printAllHandles(Server *s){
	HandleNode *current = s->handleList;

   puts("Print all");
	while(current != NULL){
		current = current->next;
	}
   puts("Exit Print all");
}

void resetFdSet(Server *s){
	HandleNode *current = s->handleList;

	while(current){
      FD_SET(current->socket, &s->fdSet);
		current = current->next;
	}
   FD_SET(s->server_socket, &s->fdSet);
}
