
#include "testing.h"
#include "socketWrapper.h"
#include "packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include <sys/select.h>

//#define DEBUG

#ifdef DEBUG 
    #define DEBUG_PRINTF(str, ...) \
        fprintf(stderr, "%d: ", __LINE__); \
        fprintf(stderr, str, __VA_ARGS__);
#else
    #define DEBUG_PRINTF(str, ...)
#endif

#define INIT_NUM_CLIENTS 4

typedef struct {
    int sockfd;
    char handle[MAX_HANDLE_LEN];
    char dataBuf[MAX_PACKET_LEN];
    int dataBufIdx;
} Client_t;

static Client_t* clients;
static int numClients;

void processArgs(int argc, char** argv, int* port);
int initNetwork(int port);
int initNetwork(int port);
void serverFSM(int sockfd);

int main(int argc, char** argv)
{
    int port = 0;
    int sockfd;

    processArgs(argc, argv, &port);

    sockfd = initNetwork(port);

    clients = calloc(INIT_NUM_CLIENTS, sizeof(Client_t));
    if (clients == NULL)
    {
        perror("calloc");
        return -1;
    }

    numClients = INIT_NUM_CLIENTS;

    serverFSM(sockfd);

    return 0;
}

void processArgs(int argc, char** argv, int* port)
{
    if (argc > 1)
    {
        *port = atoi(argv[1]);
    }
}

int initNetwork(int port)
{
    int sockfd;
    struct sockaddr_in s_in;
    socklen_t len = sizeof(s_in);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(-1);
    }

    s_in.sin_family = AF_INET;
    s_in.sin_addr.s_addr = INADDR_ANY;
    s_in.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&s_in, (socklen_t)len) == -1)
    {
        perror("bind");
        exit(-1);
    }

    if (getsockname(sockfd, (struct sockaddr*)&s_in, (socklen_t*)&len) < 0)
    {
        perror("getsockname");
        exit(-1);
    }

    printf("Server is using port %d\n", ntohs(s_in.sin_port));

    if (listen(sockfd, 20) == -1)
    {
        perror("listen");
        exit(-1);
    }

    return sockfd;
}

//Allocate memory for a new client and add them to the array
void addNewClient(int sockfd)
{
    int idx;
    int clientfd;
    Client_t* newClients;

    DEBUG_PRINTF("Adding new client%s\n", " ");

    clientfd = accept(sockfd, (struct sockaddr*)0, (socklen_t*)0);

    if (clientfd == -1)
    {
        perror("accept");
        exit(-1);
    }

    for (idx = 0; idx < numClients; idx++)
    {
        if (clients[idx].sockfd == 0)
        {
            break;
        }
    }

    if (idx == numClients)
    {
        newClients = calloc(numClients * 2, sizeof(Client_t));

        memcpy(newClients, clients, numClients * sizeof(Client_t));
        clients = newClients;
        numClients *= 2;
    }


    clients[idx].sockfd = clientfd;
    memset(clients[idx].handle, 0, MAX_HANDLE_LEN);
    memset(clients[idx].dataBuf, 0, MAX_PACKET_LEN);
    clients[idx].dataBufIdx = 0;
}

//Remove a client from the array. Clean up memory
void cleanupClient(int clientNum)
{
    DEBUG_PRINTF("Cleaning %s\n", clients[clientNum].handle);

    memset(&clients[clientNum], 0, sizeof(Client_t));
}

void processInitPacket(int clientNum)
{
    int handleLen;
    int idx;
    char tempHandle[MAX_HANDLE_LEN];

    memset(tempHandle, 0, MAX_HANDLE_LEN);

    decodeInitPacket(clients[clientNum].dataBuf,
                     tempHandle,
                     &handleLen);

    for (idx = 0; idx < numClients; idx++)
    {
        if (strcmp(clients[idx].handle, tempHandle) == 0)
        {
            sendPacketHandleExists(clients[clientNum].sockfd);
            break;
        }
    }

    if (idx == numClients)
    {
        sendPacketInitResp(clients[clientNum].sockfd);
        strcpy(clients[clientNum].handle, tempHandle);
        DEBUG_PRINTF("%s joined the server\n", clients[clientNum].handle);
    }
}

void processBroadcastPacket(int clientNum)
{
    int idx;

    for (idx = 0; idx < numClients; idx++)
    {
        if (clients[idx].sockfd != 0 &&
            idx != clientNum)
        {
            sendRawPacket(clients[idx].sockfd, clients[clientNum].dataBuf);
        }
    }
}

void processMessagePacket(int clientNum)
{
    int idx;
    char srcHandle[MAX_HANDLE_LEN];
    char dstHandle[MAX_HANDLE_LEN];
    int srcHandleLen;
    int dstHandleLen;
    char msg[MAX_MSG_LEN];
    int msgLen;

    memset(srcHandle, 0, MAX_HANDLE_LEN);
    memset(dstHandle, 0, MAX_HANDLE_LEN);

    decodeMessagePacket(clients[clientNum].dataBuf,
                        srcHandle, &srcHandleLen,
                        dstHandle, &dstHandleLen,
                        msg, &msgLen);

    DEBUG_PRINTF("Searching for handle: %s\n", dstHandle);

    for (idx = 0; idx < numClients; idx++)
    {
        if (strcmp(dstHandle, clients[idx].handle) == 0)
        {
            DEBUG_PRINTF("Found handle: %s\n", clients[idx].handle);

            sendRawPacket(clients[idx].sockfd, clients[clientNum].dataBuf);
            break;
        }
    }

    if (idx == numClients)
    {
        DEBUG_PRINTF("Couldn't find handle%s\n", " ");
        sendPacketBadHandle(clients[clientNum].sockfd, dstHandle);
    }
}

void processExitingPacket(int clientNum)
{
    sendPacketExitingAck(clients[clientNum].sockfd);

    cleanupClient(clientNum);
}

void processListReqPacket(int clientNum)
{
    char** handleList;
    int connectedClients = 0;
    int idx;

    handleList = calloc(numClients + 1, sizeof(char*));
    if (handleList == NULL)
    {
        perror("calloc");
        exit(-1);
    }

    for (idx = 0; idx < numClients; idx++)
    {
        if (clients[idx].sockfd)
        {
            handleList[connectedClients] = clients[idx].handle;
            connectedClients++;
        }
    }

    sendPacketHandleListNum(clients[clientNum].sockfd, connectedClients);

    sendPacketHandleList(clients[clientNum].sockfd, handleList);

    free(handleList);
}

//Process then incoming packets
void processPacket(int clientNum)
{
    PacketHeader_t* header;

    header = (PacketHeader_t*)clients[clientNum].dataBuf;

    DEBUG_PRINTF("Got packet of type: %d\n", header->flags);

    switch (header->flags)
    {
        case PFLAG_INIT:
            processInitPacket(clientNum);
            break;

        case PFLAG_BROADCAST:
            processBroadcastPacket(clientNum);
            break;

        case PFLAG_MESSAGE:
            processMessagePacket(clientNum);
            break;

        case PFLAG_EXITING:
            processExitingPacket(clientNum);
            break;

        case PFLAG_HANDLE_LIST_REQ:
            processListReqPacket(clientNum);
            break;

        default:
            printf("Unknown Packet\n");
    }

}

//Read in the packet from the socket until it's full
void processClient(int clientNum)
{
    Client_t* thisClient;
    int ret;

    DEBUG_PRINTF("Processing Client: %d\n", clientNum);

    thisClient = &clients[clientNum];

    ret = readPacket(thisClient->sockfd, thisClient->dataBuf, &thisClient->dataBufIdx);

    if (ret == 1)
    {
        processPacket(clientNum);

        memset(thisClient->dataBuf, 0, MAX_PACKET_LEN);
        thisClient->dataBufIdx = 0;
    } else if (ret == -1) {
        cleanupClient(clientNum);
    }

}

//Overall state maching for the server. Holds to select call.
void serverFSM(int sockfd)
{
    fd_set readfds;
    int maxfd;
    int idx;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        maxfd = sockfd;

        for (idx = 0; idx < numClients; idx++)
        {
            if (clients[idx].sockfd)
            {
                FD_SET(clients[idx].sockfd, &readfds);
                if (clients[idx].sockfd > maxfd)
                {
                    maxfd = clients[idx].sockfd;
                }
            }
        }

        wrapSelect(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &readfds))
        {
            addNewClient(sockfd);
            continue;
        }

        for (idx = 0; idx < numClients; idx++)
        {
            if (clients[idx].sockfd && 
                FD_ISSET(clients[idx].sockfd, &readfds))
            {
                processClient(idx);
            }
        }

    }
}

