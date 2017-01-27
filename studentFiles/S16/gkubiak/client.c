
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

static char* myHandle;
static int dataBufSize;

int processArgs(int argc, char** argv, char** handle, char** server, int* port);
int initNetwork(char* server, int port);
int requestHandle(int sockfd, char* handle);
void clientFSM(int sockfd);
void printPrompt();

int main(int argc, char** argv)
{
    char* server;
    int port;
    int sockfd;

    setbuf(stdout, NULL);

    if (processArgs(argc, argv, &myHandle, &server, &port) == -1)
    {
        return 1;
    }

    sockfd = initNetwork(server, port);
    if (requestHandle(sockfd, myHandle) == -1)
    {
        printf("Handle already in use\n");
        return 1;
    }

    printPrompt();

    clientFSM(sockfd);

    return 0;
}

int processArgs(int argc, char** argv, char** handle, char** server, int* port)
{
    if (argc < 4)
    {
        printf("Not enough arguments\n");
        return -1;
    }

    if (strlen(argv[1]) > 255)
    {
        printf("Handle name too long (255 max)\n");
        return -1;
    }

    *handle = argv[1];

    if (strlen(argv[2]) > 1024)
    {
        printf("Server name too long (1024 max)\n");
        return -1;
    }

    *server = argv[2];

    *port = atoi(argv[3]);

    if (*port == 0)
    {
        printf("Invalid port number\n");
        return -1;
    }

    return 0;
}

int initNetwork(char* server, int port)
{
    int sockfd;
    int ret;
    struct sockaddr_in s_in;
    struct hostent *hp;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        perror("socket");
        exit(-1);
    }

    s_in.sin_family = AF_INET;

    hp = gethostbyname(server);
    if (hp == NULL)
    {
        perror("gethostbyname");
        exit(-1);
    }

    memcpy((char*)&s_in.sin_addr, (char*)hp->h_addr, hp->h_length);
    s_in.sin_port = htons(port);

    ret = connect(sockfd, (struct sockaddr*)&s_in, sizeof(struct sockaddr_in));
    if (ret == -1)
    {
        perror("connect");
        exit(-1);
    }

    return sockfd;
}

//Request the given handle. Block until response received
int requestHandle(int sockfd, char* handle)
{
    char initRespBuf[(sizeof(PacketHeader_t) + 1)];
    int bufIdx = 0;
    PacketHeader_t* packet;

    sendPacketInit(sockfd, handle);

    while(readPacket(sockfd, initRespBuf, &bufIdx) == 0)
    {}

    packet = (PacketHeader_t*)initRespBuf;
    if (packet->flags == PFLAG_INIT_RESP)
    {
        return 0;
    } else if (packet->flags == PFLAG_HANDLE_EXISTS) {
        return -1;
    } else {
        return -1;
    }
}

void printPrompt()
{
    printf("$: ");
}

//Process and print a broadcast packet
void processBroadcastPacket(char* buf)
{
    char handle[MAX_HANDLE_LEN];
    int handleLen;
    char msg[MAX_MSG_LEN];
    int msgLen;

    memset(handle, 0, MAX_HANDLE_LEN);

    decodeBroadcastPacket(buf, handle, &handleLen, msg, &msgLen);

    printf("\n%s: %s", handle, msg);
    printPrompt();
}

//Process and print a message packet
void processMessagePacket(char* buf)
{
    char srcHandle[MAX_HANDLE_LEN];
    char dstHandle[MAX_HANDLE_LEN];
    int srcHandleLen = 0;
    int dstHandleLen = 0;
    char msg[MAX_MSG_LEN];
    int msgLen;

    memset(srcHandle, 0, MAX_HANDLE_LEN);
    memset(dstHandle, 0, MAX_HANDLE_LEN);

    decodeMessagePacket(buf,
                        srcHandle, &srcHandleLen,
                        dstHandle, &dstHandleLen,
                        msg, &msgLen);

    printf("\n%s: %s", srcHandle, msg);
    printPrompt();
}

void processBadHandlePacket(char* buf)
{
    char handle[MAX_HANDLE_LEN];
    int handleLen = 0;

    memset(handle, 0, MAX_HANDLE_LEN);

    decodeBadHandlePacket(buf, handle, &handleLen);

    printf("Client with handle %s does not exist.\n", handle);
    printPrompt();
}

void processHandleListNumPacket(char** buf)
{
    int numHandles = 0;

    decodeHandleListNumPacket(*buf, &numHandles);

    //Allocate enough memory for handle list
    free(*buf);
    *buf = calloc(numHandles + 1, MAX_HANDLE_LEN);
    if (*buf == NULL)
    {
        perror("calloc");
        exit(-1);
    }
    dataBufSize = (numHandles + 1) * MAX_HANDLE_LEN;
}

//Process a complete packet
void processPacket(char** buf, int bufLen)
{
    PacketHeader_t* packet = (PacketHeader_t*)*buf;

    DEBUG_PRINTF("Got packet with type %d\n", packet->flags);

    switch (packet->flags)
    {
        case PFLAG_BROADCAST:
            processBroadcastPacket(*buf);
            break;

        case PFLAG_MESSAGE:
            processMessagePacket(*buf);
            break;

        case PFLAG_BAD_HANDLE:
            processBadHandlePacket(*buf);
            break;

        case PFLAG_EXITING_ACK:
            exit(0);

        case PFLAG_HANDLE_LIST_NUM:
            processHandleListNumPacket(buf);
            break;

        case PFLAG_HANDLE_LIST:
            printHandleListPacket(*buf);
            printPrompt();

            free(*buf);
            *buf = calloc(1, MAX_PACKET_LEN);
            if (*buf == NULL)
            {
                perror("calloc");
                exit(-1);
            }
            dataBufSize = MAX_PACKET_LEN;
            break;

        default:
            printf("Unknown Packet\n");
            break;
    }
}

//Parse a Message command from the console
void parseMessageText(int sockfd, char* buffer, int len)
{
    char* handle = calloc(1, len + 1);
    char msgTempBuffer[MAX_MSG_LEN];
    int msgLen;

    if (handle == NULL)
    {
        perror("calloc");
        exit(-1);
    }

    sscanf(buffer, " %s", handle);

    if (strlen(handle) > (MAX_HANDLE_LEN - 1))
    {
        printf("Invalid Handle\n");
        printPrompt();
    }

    while (*buffer == ' ')
    {
        buffer++;
    }

    if (*buffer == 0)
    {
        printf("Invalid Command\n");
        printPrompt();
    }

    buffer += strlen(handle);

    while (*buffer == ' ')
    {
        buffer++;
    }

    if (*buffer == 0)
    {
        printf("Invalid Command\n");
        printPrompt();
        return;
    }

    while (strlen(buffer) > 0)
    {
        memset(msgTempBuffer, 0, MAX_MSG_LEN);

        if (strlen(buffer) < (MAX_MSG_LEN - 1))
        {
            msgLen = strlen(buffer);
        } else {
            msgLen = (MAX_MSG_LEN - 1);
        }

        memcpy(msgTempBuffer, buffer, msgLen);

        sendPacketMessage(sockfd, myHandle, handle, msgTempBuffer);

        buffer += msgLen;
    }

    printPrompt();
}

void parseBroadcastText(int sockfd, char* buffer, int len)
{
    char msgTempBuffer[MAX_MSG_LEN];
    int msgLen;

    while (*buffer == ' ')
    {
        buffer++;
    }

    if (*buffer == 0)
    {
        printf("Invalid Command\n");
        printPrompt();
        return;
    }

    while (strlen(buffer) > 0)
    {
        memset(msgTempBuffer, 0, MAX_MSG_LEN);

        if (strlen(buffer) < (MAX_MSG_LEN - 1))
        {
            msgLen = strlen(buffer);
        } else {
            msgLen = (MAX_MSG_LEN - 1);
        }

        memcpy(msgTempBuffer, buffer, msgLen);

        sendPacketBroadcast(sockfd, myHandle, msgTempBuffer);

        buffer += msgLen;
    }

    printPrompt();
}

void processConsoleBuffer(int sockfd, char* buffer, int len)
{
    int idx = 0;

    while (idx < len)
    {
        if (buffer[idx] == '%')
        {
            idx++;
            switch (buffer[idx])
            {
                case 'm':
                case 'M':
                    parseMessageText(sockfd, &buffer[idx+1], len - (idx + 1));
                    return;

                case 'b':
                case 'B':
                    parseBroadcastText(sockfd, &buffer[idx+1], len - (idx + 1));
                    return;

                case 'l':
                case 'L':
                    sendPacketHandleListReq(sockfd);
                    return;

                case 'e':
                case 'E':
                    sendPacketExiting(sockfd);
                    return;
            
                default:
                    printf("Invalid Command\n");
                    printPrompt();
                    return;
            }
        }
    }
}

void processConsole(int sockfd)
{
    char* buffer;
    int len;

    buffer = calloc(1, 16384);

    if ((len = read(STDIN_FILENO, buffer, 16384)) == -1)
    {
        perror("read");
        exit(-1);
    }

    processConsoleBuffer(sockfd, buffer, len);

    free(buffer);
}

//Overall state machine for the client. Holds the select call
void clientFSM(int sockfd)
{
    fd_set readfds;
    char* readBuf;
    int bufIdx = 0;
    int ret;

    readBuf = calloc(1, MAX_PACKET_LEN);
    if (readBuf == NULL)
    {
        perror("calloc");
        exit(-1);
    }
    dataBufSize = MAX_PACKET_LEN;
   

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        wrapSelect(sockfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &readfds))
        {
            ret = readPacket(sockfd, readBuf, &bufIdx);
            if (ret == 1)
            {
                processPacket(&readBuf, bufIdx);

                memset(readBuf, 0, dataBufSize);
                bufIdx = 0;
            } else if (ret == -1) {
                printf("Server Terminated\n");
                exit(0);
            }
        } 

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            processConsole(sockfd);
        }

    }
}

