#include "testing.h"
#include "chatShared.h"
#include "networks.h"
#include "server.h"
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

int serverFD;
int *clientFDs = NULL;
char **handles = NULL;
int clientNum = 0;


int main(int argc, char *argv[]) {
  serverFD = tcpServerSetup(0);
  process();
  return 0;
}

/*
 * Give it a FD and it'll expand the arrays
 */
void addClient(int fd) {
  clientNum++;

  clientFDs = realloc(clientFDs, sizeof(int) * clientNum);
  clientFDs[clientNum-1] = fd;

  handles = realloc(handles, sizeof(char *) * clientNum);
  handles[clientNum-1] = NULL;
}

void setClientName(void *packet, int fd) {
  int i = 0;
  char *name = NULL;
  getFirstHandle(packet, &name);


  for (i = 0; i < clientNum; i++) {
    if(clientFDs[i] == fd) {
      if(getFDForName(name) != -1) {
        free(name);
        sendPacketFlag(CONNECT_ERROR, clientFDs[i]);
        removeClient(fd);
      } else {
        handles[i] = name;
        sendPacketFlag(CONNECT_ACK, clientFDs[i]);
        printf("Welcome: %s\n", handles[i]);
      }
      return;
    }
  }
}
/*
  Returns -1 if not found
*/
int getFDForName(char *name) {
  int i = 0;

  for (i = 0; i < clientNum; i++) {
    if(handles[i] != NULL && strcmp(name, handles[i]) == 0) {
      return clientFDs[i];
    }
  }
  return -1;
}

/*
 * Give it a fd and it'll remove it from the arrays
 */
void removeClient(int fd) {
  int i = 0;
  int *newClientFDs;
  char **newHandles;
  int foundFD = 0;
  clientNum--;

  newClientFDs = (int *)malloc(sizeof(int) * clientNum);
  newHandles = (char **)malloc(sizeof(char *) * clientNum);

  for(i = 0; i <= clientNum; i++) {
    if(clientFDs[i] != fd) {
      if(!foundFD) {
        newClientFDs[i] = clientFDs[i];
        newHandles[i] = handles[i];
      } else {
        newClientFDs[i - 1] = clientFDs[i];
        newHandles[i - 1] = handles[i];
      }
    } else {
      foundFD = 1;
      if(handles[i] != NULL) {
        printf("Bye %s\n", handles[i]);
      }
      free(handles[i]);
      close(clientFDs[i]);
    }
  }

  free(handles);
  free(clientFDs);
  handles = newHandles;
  clientFDs = newClientFDs;

}

int getFDForInput() {
  fd_set fdvar;
  int i = 0;
  int maxFD = serverFD + 1;

  FD_ZERO(&fdvar);
  FD_SET(serverFD,&fdvar);
  for(i = 0; i < clientNum; i++) {
      FD_SET(clientFDs[i],&fdvar);
  }

  if (clientNum > 0) {
    maxFD = clientFDs[clientNum-1] + 1;
  }

  if(select(maxFD,(fd_set *) &fdvar, NULL, NULL, NULL) == -1) {
    perror("Error selecting FD to read from");
    exit(1);
  }

  if(FD_ISSET(serverFD, &fdvar)) {
    return serverFD;
  } else {
    for(i = 0; i < clientNum; i++) {
        if(FD_ISSET(clientFDs[i], &fdvar)) {
          return clientFDs[i];
        }
    }
  }
  return -1;
}

void acceptClient() {
  int clientFD = tcpAccept(serverFD);
  addClient(clientFD);
}

void process() {
  int fdToProcess;
  while(1) {
    fdToProcess = getFDForInput();
    if(fdToProcess == serverFD) {
      acceptClient();
    } else {
      processPacket(fdToProcess);
    }
  }
}

void processPacket(int fd) {
  uint8_t *packet = malloc(MAXBUF);
  int numBytes = 0;
  if ((numBytes = recv(fd, packet, MAXBUF, 0)) < 0)
	{
		perror("recv call");
		exit(-1);
	}
  if(numBytes == 0) {
    removeClient(fd);
  }
  switch(((header *) packet)->flag) {
    case CLIENT_CONNECT:
      setClientName(packet, fd);
      break;
    case CLIENT_EXIT:
      sendPacketFlag(EXIT_ACK, fd);
      removeClient(fd);
      break;
    case CLIENT_BROADCAST:
      broadCast(fd, packet);
      break;
    case CLIENT_MESSAGE:
      forwardMessage(fd, packet);
      break;
    case CLIENT_HANDLES:
      sendPacketHandleAck(HANDLES_ACK, fd);
      sendHandles(fd);
      break;
    default:
      break;
  }
  free(packet);
}

void broadCast (int senderFD, void *packet) {
  int i = 0;
  for(i = 0; i < clientNum; i++) {
    if(clientFDs[i] != senderFD) {
      if(sendPacket(packet, clientFDs[i]) < 0) {
        perror("Packet Broadcast");
        exit(1);
      }
    }
  }
}

void forwardMessage (int senderFD, void *packet) {
  int i = 0;
  char *dest = NULL;
  int sent = 0;
  getDestHandle(packet, &dest);

  for(i = 0; i < clientNum; i++) {
    if(strcmp(dest, handles[i]) == 0) {
      if(sendPacket(packet, clientFDs[i]) < 0) {
        perror("Packet Broadcast");
        exit(1);
      }
      sent = 1;
      break;
    }
  }
  if(sent == 0) {
    sendPacketFlag(MESSAGE_ERROR, senderFD);
  }
  free(dest);
}

void sendPacketHandleAck(uint8_t flag, int socket) {
  packetHandleAck head;
  headerSetLen((header *)&head, (uint16_t)(sizeof(packetHandleAck) - sizeof(header)));
  packetHandleAckSetNum(&head, clientNum);
  head.head.flag = flag;
  if(send(socket, &head, sizeof(packetHandleAck), 0) < 0) {
    perror("Flag Packet Send");
    exit(1);
  }
}

void sendPacketHandle(int socket, char *handle) {
  int hLen = strlen(handle);
  void *packet = makePacketHandle(SERVER_HANDLE, hLen, handle);
  if(sendPacket(packet, socket) < 0) {
    perror("Packet Connection");
    exit(1);
  }
  free(packet);
}

void sendHandles(int socket) {
  int i = 0;
  for(i = 0; i < clientNum; i++) {
    sendPacketHandle(socket, handles[i]);
  }
}
