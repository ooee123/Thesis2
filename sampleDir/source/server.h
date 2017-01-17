#ifndef __chatServer__
#define __chatServer__

void addClient(int fd);
void removeClient(int fd);
void setClientName(void *packet, int fd);
int getFDForInput();
void acceptClient();
void process();
void processPacket(int fd);
int getFDForName(char *name);
void broadCast (int senderFD, void *packet);
void forwardMessage (int senderFD, void *packet);
void sendPacketHandleAck(uint8_t flag, int socket);
void sendPacketHandle(int socket, char *handle);
void sendHandles(int socket);

#endif
