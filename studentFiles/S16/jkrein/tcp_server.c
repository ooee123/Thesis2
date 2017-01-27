/******************************************************************************
 * tcp_server.c
 * @author: Jason Krein
 * CPE 464 - Program 1
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
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

#include "networks.h"
#include "packets.h"
#include "tcp_server.h"

uint32_t numConnects;
clientInfo *allClients;

int main(int argc, char *argv[])
{
    numConnects = 1;
    uint32_t server_socket= 0;   //socket descriptor for the server socket

    printf("sockaddr: %lu sockaddr_in %lu\n", sizeof(struct sockaddr), sizeof(struct sockaddr_in));

    int usePort = 0;
    if(argc > 1)
    {
        printf("Using port: %d\n", atoi(argv[1]));
        usePort = atoi(argv[1]);
    }
    //create the server socket
    server_socket= tcp_recv_setup(usePort);

    //look for a client to serve
    waitForClients(server_socket, 5);

    return 1;
}

/*Handle incoming client connections*/
void waitForClients(int32_t serverSocket, int backlog)
{
    int client_socket;
    fd_set checkFds;       //Fields data will come in on. Client socks
                            //Don't need write fields apparently
    printf("Creating socket with elements of size %d\n", sizeof(clientInfo));
    allClients = (clientInfo *)calloc(1, sizeof(clientInfo));
    allClients[0].socket = serverSocket;
    //Continuously wait for clients and check input
    
    if(listen(serverSocket, backlog) < 0)
    {
        perror("Couldn't listen");
        exit(-1);
    }
    
    while(1)
    {
        int inc;
        int maxSock = 0;
        //Add each client socket to the set
        FD_ZERO(&checkFds);
        for(inc = 0; inc < numConnects; inc++)
        {
            //Check if the info is still connected. if it's -1, it's not
            int attSock = allClients[inc].socket;
            if(attSock > 0)
            {
                maxSock = max(maxSock, allClients[inc].socket);
                FD_SET(attSock, &checkFds);  //Set all the sockets to be checked
            }
        }
        select(maxSock + 1, (fd_set *) &checkFds, 0, 0, 0);
        //If someone is ready to connect, accept and add them to the list of connected
        //people
        if(FD_ISSET(serverSocket, &checkFds))
        {
            client_socket = accept(serverSocket, 0, 0);
            allClients = connectClient(client_socket);
        }
        //See if any messages have come in
        checkClientSockets(checkFds);
    }
}

int max(int num1, int num2)
{
    if(num1 > num2)
    {
        return num1;
    }
    return num2;
}

//Called when a client is connected, just sets the socket up
clientInfo *connectClient(int sockNum)
{
    int checkInfo;
    int addedStruct = 0;
    for(checkInfo = 0; checkInfo < numConnects; checkInfo ++)
    {
        int checkSock = allClients[checkInfo].socket;
        if(checkSock <= 0)
        {
            allClients[checkInfo].socket = sockNum;
            addedStruct = 1;
            break;
        }
    }
    //If there were no open spaces, realloc and add a clientInfo
    if(!addedStruct)
    {
        numConnects++;
        printf("Had to realloc\n");
        allClients = (clientInfo *)realloc(allClients, sizeof(clientInfo) * (numConnects));
        allClients[numConnects - 1].socket = sockNum;
    }
    return allClients;
}

//checks all the sockets to see if they're ready to read
void checkClientSockets(fd_set msgFds)
{
    int inc = 0;
    char *buf;
    int buffer_size = 1024;
    buf = (char *)malloc(buffer_size);
    int msgLen;

    for(inc = 1; inc < numConnects; inc ++)
    {
        int checkSock = allClients[inc].socket;
        if(checkSock > 0)
        {
            if(FD_ISSET(checkSock, &msgFds))
            {
                msgLen = checkValidMsg(checkSock, buf);
                if(msgLen > 0)
                {
                   printf("Message rec from socket %d, len = %d\n", checkSock, msgLen);
                   handleMessages(buf, checkSock);
                }
            }
        }
    }
    free(buf);
}

void handleMessages(char *message, int recSocket)
{
    normal_header *header = (normal_header *)message;
    u_short expLen = ntohs(header -> packetLen);
    u_char flag = header -> flag;
    if(flag == INITIAL_CLIENT)
    {
        initialConnect(recSocket, message);
    }
    else if(flag == GET_HANDLES)
    {
        sendHandles(message, recSocket);
    }
    else if(flag == DIRECT_MESSAGE)
    {
        gotDirectMessage(message, recSocket, expLen);
    }
    else if(flag == CLIENT_BCAST)
    {
        gotBroadcast(message, recSocket, expLen);
    }
    else if(flag == CLIENT_EXIT)
    {
        normal_header *exitHead = makeNormHead(NORMLEN, ACK_EXIT);
        send(recSocket, exitHead, NORMLEN, 0);
        free(exitHead);
        closeSocket(recSocket);
    }
}


void gotBroadcast(char *message, int recSocket, u_short expLen)
{
        //Check every socket, make sure it's not == to recSocket, send to it
        int castTo;
        for(castTo = 1; castTo < numConnects; castTo ++)
        {
            int sockToSend = allClients[castTo].socket;
            if(sockToSend != recSocket && sockToSend > 0)
            {
                send(sockToSend, message, expLen, 0);
            }
        }
}


void gotDirectMessage(char *message, int recSocket, u_short expLen)
{
        //if it's a direct message, get the socket of the destination handle
        //the message is char *message
        u_char destHandleLen = message[NORMLEN];
        int destSock = getHandleSock(destHandleLen, message + NORMLEN + 1);
        if(destSock > 0)
        {
            send(destSock, message, expLen, 0);
        }
        else
        {
            //Send the error packet back to src
            short errLen = NORMLEN + destHandleLen + 1;
            char *errorPack = (char *)makeNormHead(errLen, HANDLE_DNE);
            errorPack[NORMLEN] = destHandleLen;
            memcpy(errorPack + NORMLEN + 1, message + NORMLEN + 1, destHandleLen);
            send(recSocket, errorPack, errLen, 0);
            free(errorPack);
        }
}



void sendHandles(char *message, int recSocket)
{
    char *num = (char *)makeNormHead(NORMLEN + 4, SEND_NUM_HANDLES);
    //add room for the int number of things
    num = (char *)realloc(num, NORMLEN + 4);
    uint32_t numHandles = 0;

    int check;
    
    for(check = 1; check < numConnects; check ++)
    {
        int32_t sockNum = allClients[check].socket;
        if(sockNum > 0)
        {
            numHandles ++;
        }
    }

    numHandles = htonl(numHandles);
    memcpy(&num[NORMLEN], &numHandles, 4);
    send(recSocket, num, NORMLEN + 4, 0);
    //After sending the number of clients, create the list of them
    free(num);


    send(recSocket, makeNormHead(0, SEND_HANDLES), 3, 0);

    for(check = 1; check < numConnects; check ++)
    {
        int32_t sockNum = allClients[check].socket;
        //Make sure this wasn't the guy that requested
        if(sockNum > 0)
        {
            //Realloc, add the name + len to it, add to sendSize
            u_char len = allClients[check].handleLen;
            char *name = allClients[check].handle;
            char thisHandle[len + 1];
            memcpy(thisHandle, &len, 1);
            memcpy(thisHandle + 1, name, len);
            send(recSocket, thisHandle, len + 1, 0);
        }
    }

    //Set expected len
    //short networkSize = htons(sendSize);
    //memcpy(sendAllHandles, &networkSize, 2);
}




void initialConnect(int recSocket, char *message)
{
        client_handle_info *cHandle = (client_handle_info *)message;
        char len = cHandle -> handleLen;
        char *handlePtr = (char *)malloc(len);  //Don't free this, it's stored in connections
        memcpy(handlePtr, &(cHandle -> handle), len);
        int couldAdd = addHandle(len, handlePtr, recSocket);
        if(couldAdd > 0)
        {
            send(recSocket, makeNormHead(NORMLEN, HANDLE_CONFIRM), NORMLEN, 0);
        }
        else
        {
            send(recSocket, makeNormHead(NORMLEN, HANDLE_EXISTS), NORMLEN, 0);
        }
}





normal_header *makeNormHead(short len, char flag)
{
    if(len < NORMLEN)
    {
        len = NORMLEN;
    }
    normal_header *toSend = (normal_header *)malloc(len);
    toSend -> packetLen = htons(len);
    toSend -> flag = flag;
    return toSend;
}

//Returns 1 if handle was added, -1 if it was taken and need to return error
int addHandle(char len, char *handle, int sock)
{
    if(getHandleSock(len, handle) != -1)
    {
        return -1;
    }
    else
    {
        int check;
        for(check = 0; check < numConnects; check++)
        {
            if(allClients[check].socket == sock)
            {
                allClients[check].handleLen = len;
                allClients[check].handle = handle;
                return 1;
            }
        }
    }
    return -1;
}

//If handle isn't in the list, return -1, otherwise return socket associated w/ handle
int getHandleSock(char len, char *handle)
{
    int check;
    for(check = 0; check < numConnects; check ++)
    {
        if(allClients[check].handleLen > 0 && 
            allClients[check].handleLen == len && memcmp(allClients[check].handle, handle, len) == 0)
        {
            return allClients[check].socket;
        }
    }
    return -1;
}


//Sets the info with the given socket number to -1
void closeSocket(int toClose)
{
    int check = 0;
    close(toClose);
    while(allClients[check].socket != toClose && check < numConnects)
    {
        check ++;
    }
    if(check > numConnects)
    {
        printf("Wrecked\n");
    }
    else
    {
        allClients[check].socket = -1;
        allClients[check].handleLen = 0;
    }
}

//Returns 0 if not valid message, otherwise length of the message
int checkValidMsg(int socketNum, char *data)
{
    enum PHASE {START, WAITLEN, PROCDATA};
    static enum PHASE state = START;
    static short msgLen;
    static int lenRecd = 0;
    static char incompData[MAX_MSGLEN];
    int toRet = 0;
    switch (state)
    {   
        case START: 
            lenRecd = safeRecv(socketNum, incompData, SIZELEN);
            //If we got enough to determine size, put that into
            //the short
            if(lenRecd == SIZELEN)
            {   
                memcpy(&msgLen, incompData, SIZELEN);
                msgLen = ntohs(msgLen);
                state = PROCDATA;
            }
            else if(lenRecd == 0)
            {
                printf("Closing socket\n");
                closeSocket(socketNum);
            }
            else
            {
                state = WAITLEN;
            }
        break;

        case WAITLEN:
            lenRecd += safeRecv(socketNum, &incompData[1], 1);
            memcpy(&msgLen, incompData, SIZELEN);
            msgLen = ntohs(msgLen);
            state = PROCDATA;

        break;

        case PROCDATA:
            lenRecd += safeRecv(socketNum, &incompData[lenRecd], msgLen - lenRecd);
            if(lenRecd == msgLen)
            {
                memcpy(data, incompData, lenRecd);
                toRet = lenRecd;
                lenRecd = 0;
                state = START;
            }
            else if(lenRecd == 0)
            {
                printf("Closing socket\n");
                closeSocket(socketNum);
            }
        break;

        default:
            printf("Shouldn't get here\n");
    }
    return toRet;
}

int safeRecv(int socket_num, char *buf, int bufSize)
{
    int msgLen = recv(socket_num, buf, bufSize, 0);
    if(msgLen < 0)
    {
       perror("Recv call");
    }
    return msgLen;
}





/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup(int socketNum)
{
    int server_socket= 0;
    struct sockaddr_in local;      /* socket address for local side  */
    socklen_t len= sizeof(local);  /* length of local address        */

    /* create the socket  */
    server_socket= socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
    {
      perror("socket call");
      exit(1);
    }

    local.sin_family= AF_INET;         //internet family
    local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
    local.sin_port= htons(socketNum);                 //let system choose the port


    /* bind the name (address) to a port */
    if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
      {
	perror("bind call");
	exit(-1);
      }
    
    //get the port name and print it out
    if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0)
      {
	perror("getsockname call");
	exit(-1);
      }

    printf("socket has port %d \n", ntohs(local.sin_port));
	        
    return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

int tcp_listen(int server_socket, int back_log)
{
  int client_socket= 0;
  if (listen(server_socket, back_log) < 0)
    {
      perror("listen call");
      exit(-1);
    }
  
  if ((client_socket= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
    {
      perror("accept call");
      exit(-1);
    }
  
  return(client_socket);
}

