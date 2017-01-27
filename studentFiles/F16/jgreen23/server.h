#ifndef SERVER_H
#define SERVER_H

#define C_INIT 1
#define S_INIT_GOOD 2
#define S_INIT_ERR 3
#define C_BROADCAST 4
#define C_MESSAGE 5
#define S_MSG_ERR 7
#define C_EXIT 8
#define S_EXIT_ACK 9
#define C_LIST 10
#define S_LIST 11
#define F_HANDLE 12

int tcpSetup(int port);
void safeListen();
void safeSelect();
void handle_message(int sock);
int getClient(int sock);
void decodeMessage();
void getInit(int sock, char *buffer, int messlen);
void safeSend(int socket, char * packet, int packetLen);
void getBroad(int sock, char * buff, int messlen);
void s_exit(int sock, char * buff, int messlen);
void getMessage(int sock, char * buff, int messlen);
void getList(int sock, char * buff, int messlen);
void sendNumHandles(int sock);
void sendHandle(int sock, char * handle);
void sendMessage(char * handle, char * buff, int messlen);
void sendError(int sock, int len, char * handle);

#endif