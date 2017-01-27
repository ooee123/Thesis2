#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#define HANDLE_NOT_FOUND -1
#define HANDLE_ALREADY_EXISTS -2
#define SUCCESS 0

typedef struct HandleNode {
	u_char handleLen;
	int socket;
	char *handle;
	struct HandleNode *next;
} HandleNode;

typedef struct {
	int server_socket; //socket descriptor for the server socket
	int client_socket; //socket descriptor for the client socket
	char *buf; //buffer for receiving from client
	HandleNode *handleList;
   fd_set fdSet;
   int highestFd;
} Server;

void handleRequest(Server *s);
void handleInitialPacket(Server *s);
void setup(int argc, char *argv[], Server *s);
void cleanup(Server *s);
void processNextRequest(Server *s);
void handleMessagePacket(Server *s);
int addUser(char *handle, u_char handleLen, Server *s);
int lookupSocketForHandle(char *handle, u_char handleLen, Server *s);
void printAllHandles(Server *s);
int getNextReadyFd(Server *s);
void removeUserWithSocket(int socketNum, Server *s);
void resetFdSet(Server *s);
void handleBroadcastPacket(Server *s);
void sendHandles(Server *s);

#endif
