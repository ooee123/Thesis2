#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

//Bradley Schmidt 464 9am

struct Client {
	int socket;
	u_char handleLength;
	uint32_t listLength;
	uint32_t expecting;
	uint32_t have;
	u_char *handle;
	u_char *packet;
} __attribute__((packed));

void checkArgs(int argc, char *argv[]);
void runClient(char *handle, char *hostName, char *port);
void parseCommand(struct Client *client, char *input);
void sendMessage(struct Client *client, u_char *input);
uint32_t buildMessage(struct Client *client, u_char *input, u_char **packet, u_char *destHandleSize, uint32_t msgLenToSend);
void makeNormalHeader(u_char *header, uint16_t packetLength, u_char flag);
int parseMessageInput(u_char *input, u_char *destHandleSize, uint32_t *messageSize);
void sendPacket(int socketNum, u_char *packet, uint16_t totalToSend);
void connectToServer(struct Client *client, char *hostName, char *port);
void initializeClient(struct Client **client, char *handle);
int setupDescriptors(fd_set *fdSet, struct Client *client);
void receiveMessage(struct Client *client);
void receiveMessageError(struct Client *client);
void sendBroadcast(struct Client *client, u_char *input);
int parseBroadcastInput(u_char *input, uint32_t *broadcastSize);
uint32_t buildBroadcast(struct Client *client, u_char *input, u_char **packet, uint32_t msgLenToSend);
void requestList(struct Client *client);
void receiveListLength(struct Client *client);
void receiveList(struct Client *client);
void requestExit(struct Client *client);
void exitClient(struct Client *client);
void fetchData(struct Client *client);
int processLength(struct Client *client);	
int parseFlag(struct Client *client);
void clearPacket(struct Client *client);

#endif