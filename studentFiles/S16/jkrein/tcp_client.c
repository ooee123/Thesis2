/******************************************************************************
 * tcp_client.c
 * @author: Jason Krein
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
#include "testing.h"
#include "packets.h"
#include "tcp_client.h"

void printDm(char *message);
int socket_num; //Socket to send stuff/recieve from server
char *myHandle;
int numHandles = 0; //When asking for a list of handles, this gets set by
                    //one of the packets with flag 11
int continueMessage = 0;    //True if we just sent a message to server.
                            //Used to determine if we should send more msg
                            //or not

int main(int argc, char * argv[])
{
    int send_len= 0;        //amount of data to send


    /* check command line arguments  */
    if(argc!= 4)
    {
        printf("usage: %s handle host-name port-number \n", argv[0]);
        exit(1);
    }

    /* set up the socket for TCP transmission  */
    socket_num= tcp_send_setup(argv[2], argv[3]);

    /*Set their handle*/
    myHandle = argv[1];
    /*Send handle to the server*/
    establishHandle();

    /*Start waiting for input*/
    runClient();
    
    send_len = 0;
    //while ((send_buf[send_len] = getchar()) != '\n' && send_len < 80)
    close(socket_num);

    return 0;
}

void establishHandle()
{
   /*Send the handle and then wait for a response*/
    //Handle is gonna be argv[1]
    char handleLen = strlen(myHandle);
    short packetSize = sizeof(struct normal_header) + 1 + handleLen;
//    printf("Packet size = %d\n", packetSize);
    char *packet = malloc(packetSize);
    short nPacketSize = htons(packetSize);
    memcpy(packet, &nPacketSize, 2);
    packet[2] = INITIAL_CLIENT;
    packet[3] = handleLen;
    memcpy(packet + 4, myHandle, handleLen);
//    printf("Sending name: ");
//    printChars(packet + 4, handleLen);
    send(socket_num, packet, packetSize, 0);
    free(packet);
}

void runClient()
{
    fd_set recFd;           //Fields to receive on
    static struct timeval timeout;
    int bufsize;

    bufsize= 1229;
    char *buf = malloc(bufsize);

    printf("$: ");
    fflush(stdout);
    //Keep on doing input stuff until user exits
    while(1)
	{
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        FD_ZERO(&recFd);
        FD_SET(socket_num, &recFd);
        FD_SET(STDIN_FILENO, &recFd);
        select(socket_num + 1, &recFd, 0, 0, 0);
        if(FD_ISSET(socket_num, &recFd))
        {
            int len = checkValidMsg(socket_num, buf);
            if(len > 0)
            {
//                printf("Got a whole message, len = %d\n", len);
                int didPrint = procServMessage(buf);
                if(didPrint)
                {
                    printf("$: ");
                }
            }
            fflush(stdout);
        }
        if(FD_ISSET(STDIN_FILENO, &recFd))
        {
            //max would be %m handle of len 255, msg 1000
            char option[3];
            fgets(option, 3, stdin);
//            printf("Thanks, you said %s\n", option);
            int expResp = parseOption(option);
            //Stop buffering and just print it
            if(!expResp)
            {
                printf("$: ");
                fflush(stdout);
            }
        }
    }

}

//Send server a list request
void requestList()
{
    normal_header *reqHeader = makeNormHead(GET_HANDLES, NORMLEN);
    int sent = send(socket_num, reqHeader, NORMLEN, 0);
    if(sent < 0)
    {
        perror("Request List call");
        exit(-1);
    }
}

void clearStdin()
{
    int clear;
    do
    {
        clear = fgetc(stdin);
    } while(clear != '\n' && clear != EOF);
}

int parseOption(char *option)
{
    if(option[0] == '%')
    {
        char cmdLtr = option[1];
        if(cmdLtr == 'l' || cmdLtr == 'L')
        {
            clearStdin();
            requestList();
            return 1;
        }
        else if(cmdLtr == 'm' || cmdLtr == 'M')
        {
            //parse the rest as a message
            //Do 256 handle len to scan the null
            char *handle = malloc(256);
            char *fullMsg = malloc(MAXTEXT);
            char *msg = fullMsg;
            scanf("%s", handle);
            
            fgets(msg, MAXTEXT, stdin);
            char firstLtr = msg[0];
            if(firstLtr == ' ')
            {
                msg = fullMsg + 1;
            }
            char endLtr = msg[strlen(msg)-1];
            sendDm(msg, handle);
//            printf("Input is now %s\n", msg);
//            printf("handle = %s\n", handle);
            //If this wasn't the end of the input, do fgets again
            while(endLtr != '\n')
            {
                fgets(msg, MAXTEXT, stdin);
                endLtr = msg[strlen(msg)-1];
                sendDm(msg, handle);
                //printf("Input is now %s\n", msg);
//                printf("handle = %s\n", handle);
            }
            free(handle);
            free(fullMsg);
            return 0;
        }
        else if(cmdLtr == 'b' || cmdLtr == 'B')
        {
            //Parse as broadcast
            char *fullMsg = malloc(MAXTEXT);
            char *msg = fullMsg;
            fgets(msg, MAXTEXT, stdin);
            char firstLtr = msg[0];
            if(firstLtr == ' ')
            {
                msg ++;
            }
            char endLtr = msg[strlen(msg)-1];
            sendBcast(msg);
            while(endLtr != '\n')
            {
                fgets(msg, MAXTEXT, stdin);
                endLtr = msg[strlen(msg)-1];
                sendBcast(msg);
            }
            free(fullMsg);
            return 0;
        }
        else if(cmdLtr == 'e' || cmdLtr == 'E')
        {
            //Parse as exit
            normal_header *exitSend = makeNormHead(CLIENT_EXIT, NORMLEN);
            send(socket_num, exitSend, NORMLEN, 0);
            return 1;
        }
    }
    clearStdin();
    printf("Invalid command\n");
    return 0;
}

void sendBcast(char *msg)
{
    int castLen = strlen(msg);
    //Make sure there's no rogue new lines at the end
    char endLtr = msg[strlen(msg) - 1];
    char srcHandLen = strlen(myHandle);
    if(endLtr == '\n')
    {
        msg[strlen(msg)-1] = 0;
    }
    else
    {
        castLen += 1;
    }
    u_short totalLen = NORMLEN + castLen + srcHandLen + 1;
    char *toSend = malloc(totalLen);
    normal_header *nHead = makeNormHead(CLIENT_BCAST, totalLen);
    memcpy(toSend, nHead, NORMLEN);
    memcpy(toSend + NORMLEN, &srcHandLen, 1);
    memcpy(toSend + NORMLEN + 1, myHandle, srcHandLen);
    memcpy(toSend + NORMLEN + 1 + srcHandLen, msg, castLen);
    send(socket_num, toSend, totalLen, 0);
    free(toSend);
}

void sendDm(char *msg, char *handle)
{
    int msgLen = strlen(msg);   //This will include the null character
                                //if we ended on a newline, otherwise add 1
    u_char destHandLen = strlen(handle);
    u_char srcHandLen = strlen(myHandle);
    //total len = message + destination hand + src hand + dest hand len 
    //+ src len + normal header
    char endLtr = msg[strlen(msg)-1];
    if(endLtr == '\n')
    {
        msg[strlen(msg)-1] = 0;
    }
    else
    {
        msgLen += 1;
    }
    u_short totalLen = msgLen + destHandLen + srcHandLen + 1 + 1 + NORMLEN;
//    printf("totalLen = %d + %d + %d + 2 + %d\n", msgLen, destHandLen, srcHandLen, NORMLEN); 
    //Construct the message packet
    char *toSend = malloc(totalLen);
    normal_header *nHead = makeNormHead(DIRECT_MESSAGE, totalLen);
    memcpy(toSend, nHead, NORMLEN);
    memcpy(toSend + NORMLEN, &destHandLen, 1);
    memcpy(toSend + NORMLEN + 1, handle, destHandLen);
    memcpy(toSend + NORMLEN + 1 + destHandLen, &srcHandLen, 1);
    memcpy(toSend + NORMLEN + 1 + destHandLen + 1, myHandle, srcHandLen);
    memcpy(toSend + NORMLEN + 1 + destHandLen + 1 + srcHandLen, msg, msgLen);
    send(socket_num, toSend, totalLen, 0);
    free(toSend);
}

normal_header *makeNormHead(char flag, u_short len)
{
    normal_header *toSend = (normal_header *)malloc(len);
    toSend -> packetLen = htons(len);
    toSend -> flag = flag;
    return toSend;
}



int procServMessage(char *msg)
{
    normal_header *begHeader = (normal_header *)msg;
    u_char flag = begHeader -> flag;
    if(flag == HANDLE_EXISTS)
    {
        printf("Handle already in use:%s\n", myHandle);
        close(socket_num);
        exit(0);
    }
    else if(flag == ACK_EXIT)
    {
        close(socket_num);
        exit(0);
    }
    else if(flag == HANDLE_DNE)
    {
        printNoHandle(msg);
        return 1;
    }
    else if(flag == DIRECT_MESSAGE)
    {
        printDm(msg);
        return 1;
    }
    else if(flag == CLIENT_BCAST)
    {
        printBcast(msg);
        return 1;
    }
    //If flag is 11, set the global "numHandles" int.
    else if(flag == SEND_NUM_HANDLES)
    {
        int *numSending = (int *)(msg + NORMLEN);
        numHandles = ntohl(*numSending);
        printf("expecting %d handles\n", numHandles);
        return 0;
    }
    else if(flag == SEND_HANDLES)
    {
        printf("Got send handles flag\n");
        recieveHandles(socket_num);
        return 1;
    }
    return 0;
}

void printNoHandle(char *message)
{
    u_char attLen = message[NORMLEN];
//    printf("attLen = %d\n", attLen);
    char *attHandle = calloc(attLen + 1, 1);
    memcpy(attHandle, message + NORMLEN + 1, attLen);
    printf("\nClient with handle %s does not exist\n", attHandle);
}

void printBcast(char *message)
{
    u_char srcLen = message[NORMLEN];
    char *srcHandle = calloc(srcLen + 1, 1);
    memcpy(srcHandle, message + NORMLEN + 1, srcLen);
    printf("\n%s: %s\n", srcHandle, message + NORMLEN + 1 + srcLen);
}

void printDm(char *message)
{
    //handle: message
    u_char destLen = message[NORMLEN]; //Skip dest len
//    printf("destLen = %d\n", destLen);
    u_char srcLen = message[NORMLEN + destLen + 1];
//    printf("Src len = %d\n", srcLen);
    char *srcHandle = calloc(srcLen + 1, 1);
    memcpy(srcHandle, message + NORMLEN + destLen + 2, srcLen);

    printf("\n%s: %s\n", srcHandle, message + NORMLEN + destLen + srcLen + 2);
}

void recieveHandles(int socketNum)
{
    printf("Number of clients: %d\n", numHandles);
    //First thing recv'd here will be the next handle's len
    while(numHandles > 0)
    {
        char handLen;
        safeRecv(socketNum, &handLen, 1);
//        printf("next handle expected len = %d\n", handLen);
        char *handle = calloc(handLen + 1, 1);
        safeRecv(socketNum, handle, handLen);
        printf("  %s\n", handle);
        free(handle);
        numHandles --;
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
//TODO
//if numHandles > 0, gotta do some special stuff. write a diff functions
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
//                printf("Waiting for len = %d\n", msgLen);
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
            //if msgLen == 0, have to do special handle receiving stuff
            if(msgLen == 0)
            {
                //get the flag
                safeRecv(socketNum, &incompData[lenRecd], 1); //Get the flag
                printf("got flag num = %d\n", incompData[lenRecd]);
                memcpy(data, incompData, NORMLEN);
                toRet = NORMLEN;
                lenRecd = 0;
                state = START;
            }
            else
            {
                lenRecd += safeRecv(socketNum, &incompData[lenRecd], msgLen - lenRecd);
                if(lenRecd == msgLen)
                {
                    memcpy(data, incompData, lenRecd);
                    toRet = lenRecd;
                    lenRecd = 0;
                    state = START;
                }
            }
        break;
            
        default:
            printf("Shouldn't get here\n");
    }
    return toRet;
}

int safeRecv(int socket_num, char *buf, int bufSize)
{
    int len = recv(socket_num, buf, bufSize, 0);
    if(len <= 0)
    {
        perror("Recieve call");
        exit(-1);
    }
    return len;
}



/*This method prints len characters from message.*/
void printChars (char *message, int len)
{
    int i = 0;
    for(i = 0; i < len; i++)
    {
        printf("%c", message[i]);
    }
    printf("\n");
}


int tcp_send_setup(char *host_name, char *port)
{
    int socket_num;
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host

    // create the socket
    if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    perror("socket call");
	    exit(-1);
	}
    

    // designate the addressing family
    remote.sin_family= AF_INET;

    // get the address of the remote host and store
    if ((hp = gethostbyname(host_name)) == NULL)
	{
	  printf("Error getting hostname: %s\n", host_name);
	  exit(-1);
	}
    
    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

    // get the port used on the remote side and store
    remote.sin_port= htons(atoi(port));

    if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
    {
	perror("connect call");
	exit(-1);
    }

    return socket_num;
}

