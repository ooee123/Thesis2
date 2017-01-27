#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

//Bradley Schmidt 464 9am

struct Client {
	int socket;
	u_char handleLength;
	uint32_t expecting;
	uint32_t have;
	u_char *handle;
	u_char *packet;	
} __attribute__((packed));

void checkArgs(char **port, int argc, char *agrv[]);
void runServer(char *port);
uint32_t setupDescriptors(fd_set *fdSet, uint32_t serverSocket, struct Client *clients, uint32_t count);
int addNewSocket(uint32_t serverSocket, struct Client *clients, uint32_t count);
int increaseClientCapacity(struct Client **clients, uint32_t count);
void fetchData(struct Client *client);
int processLength(struct Client *client);
int parseFlag(struct Client *client, struct Client **clients, uint32_t count, int capacity);
int addNewClient(struct Client *client, struct Client *clients, uint32_t count);
void makeNormalHeader(u_char *header, uint16_t packetLength, u_char flag);
void sendPacket(int socketNum, u_char *packet, uint16_t totalToSend);
void deleteClient(struct Client *delete, struct Client **clients, uint32_t count, int capacity);
void clearPacket(struct Client *client);
void transmitMessage(struct Client *client, struct Client *clients, uint32_t count);
void transmitBroadcast(struct Client *client, struct Client *clients, uint32_t count);
void sendClientListLength(struct Client *client, uint32_t count);
void sendClientList(struct Client *client, struct Client *clients, uint32_t count);
void sendExitPermission(struct Client *client);
int getClientSocket(u_char *handle, struct Client *clients, uint32_t count);
void replyToBadMessage(struct Client *client, u_char handleLength);
int tcp_recv_setup(char *port);

#endif