#include "testing.h"
#include "chatShared.h"
#include "networks.h"
#include "cclient.h"
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

int socketFD = 0;
int isConnected = 0;
char *myHandle = NULL;

int main(int argc, char * argv[]) {

  if (argc != 4) {
		printf("usage: %s handle host-name port-number \n", argv[0]);
		exit(1);
	}

  socketFD = tcpClientSetup(argv[2], argv[3]);
  myHandle = argv[1];
  sendPacketConnection(socketFD);
  process();
  return 0;
}

void sendPacketConnection(int socket) {
  int hLen = strlen(myHandle);
  void *packet = makePacketHandle(CLIENT_CONNECT, hLen, myHandle);
  if(sendPacket(packet, socket) < 0) {
    perror("Packet Connection");
    exit(1);
  }
  free(packet);
}

void sendPacketBroadCast(char *mssg, int socket) {
  char *toSend;
  void *packet;
  int hLen = strlen(myHandle);
  int maxMssgLen = MAXBUF - hLen - sizeof(header) - 1;
  /* plus one to account for null */
  int mssgNum = (strlen(mssg) + 1) / maxMssgLen;
  int consumed = 0;
  /* add mssgNum because that's how many nulls we'll end up with
     which we need to account for in the packet*/
  int theRest = ((strlen(mssg) + 1) % maxMssgLen) + mssgNum;

  toSend = malloc(maxMssgLen);
  while(mssgNum > 0) {
    memcpy(toSend, mssg + consumed, maxMssgLen - 1);
    toSend[maxMssgLen - 1] = 0;
    consumed += maxMssgLen - 1;
    packet = makePacketBroadcast(CLIENT_BROADCAST, hLen, myHandle, toSend);
    if(sendPacket(packet, socket) < 0) {
      perror("Packet Broadcast");
      exit(1);
    }
    free(packet);
    mssgNum--;
  }
  free(toSend);
  toSend = malloc(theRest);
  memcpy(toSend, mssg + consumed, theRest);
  packet = makePacketBroadcast(CLIENT_BROADCAST, hLen, myHandle, toSend);
  if(sendPacket(packet, socket) < 0) {
    perror("Packet Broadcast");
    exit(1);
  }
  free(packet);
  free(toSend);

}

void sendPacketMssg(char *mssg, char *dest, int socket) {
  char *toSend;
  void *packet;
  int srcLen = strlen(myHandle);
  int destLen = strlen(dest);
  int maxMssgLen = MAXBUF - (srcLen + 1) - (destLen + 1) - sizeof(header);
  /* plus one to account for null */
  int mssgNum = (strlen(mssg) + 1) / maxMssgLen;
  int consumed = 0;
  /* add mssgNum because that's how many nulls we'll end up with
     which we need to account for in the packet*/
  int theRest = ((strlen(mssg) + 1) % maxMssgLen) + mssgNum;

  toSend = malloc(maxMssgLen);
  while(mssgNum > 0) {
    memcpy(toSend, mssg + consumed, maxMssgLen - 1);
    toSend[maxMssgLen - 1] = 0;
    consumed += maxMssgLen - 1;
    packet = makePacketMssg(CLIENT_MESSAGE, destLen, dest,
    							 srcLen, myHandle, toSend);
    if(sendPacket(packet, socket) < 0) {
      perror("Packet Message");
      exit(1);
    }
    free(packet);
    mssgNum--;
  }
  free(toSend);
  toSend = malloc(theRest);
  memcpy(toSend, mssg + consumed, theRest);
  packet = makePacketMssg(CLIENT_MESSAGE, destLen, dest,
                 srcLen, myHandle, toSend);
  if(sendPacket(packet, socket) < 0) {
    perror("Packet Message");
    exit(1);
  }
  free(packet);
  free(toSend);

}


int getFDForInput() {
  fd_set fdvar;
  int maxFD = socketFD + 1;

  FD_ZERO(&fdvar);
  FD_SET(socketFD,&fdvar);
  if(isConnected) {
    FD_SET(fileno(stdin),&fdvar);
  }

  if(select(maxFD,(fd_set *) &fdvar, NULL, NULL, NULL) == -1) {
    perror("Error selecting FD to read from");
    exit(1);
  }

  if(FD_ISSET(socketFD, &fdvar)) {
    return socketFD;
  }
  if(isConnected && FD_ISSET(fileno(stdin), &fdvar)) {
    return fileno(stdin);
  }
  return -1;
}

void process() {
  int fdToProcess;
  while(1) {
    printf("$: ");
    fflush(stdout);
    fdToProcess = getFDForInput();
    if(fdToProcess == fileno(stdin)) {
      processInput();
    } else {
      printf("\n");
      processPacket();
    }
  }
}

void processPacket() {
  uint8_t *packet = malloc(MAXBUF);
  uint8_t *toFree = packet;
  int numBytes = 0;
  if ((numBytes = recv(socketFD, packet, MAXBUF, 0)) < 0)
	{
		perror("recv call");
		exit(-1);
	}
  if(numBytes == 0) {
    printf("Server terminated\n");
    close(socketFD);
    exit(1);
  }
  while(numBytes > 0) {
    switch(((header *) packet)->flag) {
      case CONNECT_ACK:
        printf("Connected to server\n");
        isConnected = 1;
        break;
      case CONNECT_ERROR:
        printf("Handle already in use: %s\n", myHandle);
        exit(1);
        break;
      case EXIT_ACK:
        printf("Goodbye!\n");
        exit(0);
        break;
      case CLIENT_BROADCAST:
        printBroadcast(packet);
        break;
      case MESSAGE_ERROR:
        printf("No such handle connected to the server\n");
        break;
      case CLIENT_MESSAGE:
        printMessage(packet);
        break;
      case SERVER_HANDLE:
        printHandle(packet);
        break;
      case HANDLES_ACK:
        printf("Number of clients: %u\n", packetHandleAckGetNum((packetHandleAck *)packet));
        //processHandles(packetHandleAckGetNum((packetHandleAck *)packet));
        break;
      default:
        printf("unknown packet %d with size %d\n", ((header *) packet)->flag,numBytes);
        break;
    }
    packet += headerGetLen((header *) packet) + sizeof(header);
    numBytes -= headerGetLen((header *) packet) + sizeof(header);
  }
  free(toFree);
}

void processInput() {
  int didWork = 0;
  unsigned long inputNum = -1;
  char *line = NULL;
  char *lineToFree = NULL;
  char *cmd = NULL;
  char *mssg = NULL;
  char *dst = NULL;

  getline(&line, (size_t *)&inputNum, stdin);
  lineToFree = line;
  cmd = strtok(line, " \n\t");

  if (cmd != NULL){
    if(strcmp(cmd, "%E") == 0 || strcmp(cmd, "%e") == 0) {
      sendPacketFlag(CLIENT_EXIT, socketFD);
      didWork = 1;
    }
    if(strcmp(cmd, "%B") == 0 || strcmp(cmd, "%b") == 0) {
      mssg = strtok(NULL, "\n");
      if(mssg == NULL) {
        printf("Invalid command\n");
      } else {
        sendPacketBroadCast(mssg, socketFD);
      }
      didWork = 1;
    }
    if(strcmp(cmd, "%M") == 0 || strcmp(cmd, "%m") == 0) {
      dst = strtok(NULL, " \n\t");
      if(dst == NULL) {
        printf("Invalid command\n");
      } else {
        mssg = strtok(NULL, "\n");
        if(mssg == NULL) {
          printf("Invalid command\n");
        } else {
          sendPacketMssg(mssg, dst, socketFD);
        }
      }
      didWork = 1;
    }
    if(strcmp(cmd, "%L") == 0 || strcmp(cmd, "%l") == 0) {
      sendPacketFlag(CLIENT_HANDLES, socketFD);
      didWork = 1;
    }
  }


  free(lineToFree);
  if(didWork == 0) {
    printf("Invalid command\n");
  }
}

void processHandles(int handleNum) {
  int i = 0;
  /*stop the client from reading stdin */
  isConnected = 0;
  for(i = 0; i < handleNum; i++) {
    getFDForInput();
    processPacket();
  }
  isConnected = 1;
}

void printBroadcast(void *packet) {
  char *sender = NULL;
  char *mssg = NULL;
  getFirstHandle(packet, &sender);
  getBroadCastMssg(packet, &mssg);
  printf("%s: %s\n", sender, mssg);
  free(mssg);
  free(sender);
}

void printMessage(void *packet) {
  char *sender = NULL;
  char *mssg = NULL;
  getSrcHandle(packet, &sender);
  getMssg(packet, &mssg);
  printf("%s: %s\n", sender, mssg);
  free(mssg);
  free(sender);
}

void printHandle(void *packet) {
  char *handle = NULL;
  getFirstHandle(packet, &handle);
  printf("  -%s\n", handle);
  free(handle);
}
