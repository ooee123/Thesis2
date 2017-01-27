#ifndef CLIENT_H
#define CLIENT_H

#define C_INIT 1
#define S_INIT_GOOD 2
#define S_INIT_ERR 3
#define C_BROADCAST 4
#define C_MESSAGE 5
#define S_MSG_GOOD 6
#define S_MSG_ERR 7
#define C_EXIT 8
#define S_EXIT_ACK 9
#define C_LIST 10
#define S_LIST 11
#define F_HANDLE 12

void getHandle(int argc, char * argv[]);
void getServer(int argc, char * argv[]);
void sendInit();
void clientSelect();
void serverListen();
void safeSend(char *packet, int packetLength);
void initGet();
void broadcast(char *buffer);
void getBroadcast(char * buff, int messlen);
void c_exit(char *buffer);
void getExit(char * buff, int messlen);
void message(char *buffer);
void getMessage(char * buff, int messlen);
void c_list(char * buffer);
void  getListCount(char * buff, int messlen);
void listHandles();
void getListHandle(char * buff, int messlen);
void startupClient();

#endif