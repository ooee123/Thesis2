#ifndef __chatClient__
#define __chatClient__

void sendPacketConnection(int socket);
int getFDForInput();
void process() ;
void processPacket();
void processInput();
void sendPacketBroadCast(char *mssg, int socket);
void printBroadcast(void *packet);
void sendPacketMssg(char *mssg, char *dest, int socket);
void printMessage(void *packet);
void printHandle(void *packet);
void processHandles(int handleNum);


#endif
