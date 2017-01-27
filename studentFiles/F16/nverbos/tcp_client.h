#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#define SUCCESS 0

typedef struct {
	int serverSocket; //socket descriptor
   char *handle; //client handle
   u_char handleLen; //length of client handle
	char *buf; //data buffer
   fd_set fdSet;
   uint32_t handles;
} CClient;

void setup(int argc, char *argv[], CClient *cc);
void sendInitialPacket(CClient *cc);
void waitForResponse(CClient *cc);
void handleNextCommand(CClient *cc);
void sendMessagePacket(char *message, char *destHandle, CClient *cc);
void exitServer(CClient *cc);
void handleNext(CClient *cc);
void printPacket(CClient *cc);
void printIncomingMessage(CClient *cc);
void sendBroadcastPacket(char *message, CClient *cc);
void requestHandleList(CClient *cc);
void handleIncomingHandles(CClient *cc);
void printIncomingBroadcast(CClient *cc);
char *getUserInput();
void printHandle(CClient *cc);
void sendMessageHelper(char *message, char *destHandle, CClient *cc);

#endif
