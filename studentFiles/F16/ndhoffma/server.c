#include "network.h"

static int svrSocket;

static fd_set sSet;
static struct timeval time;

static char buf[MAX_BUF_SIZE];
static char *bp;
static int i;

static struct user *uList;
static int uListSize;
static int numUsers;
static int numSocks;

int handleKnown(struct handle h);
int recFlag1(int skt, struct chat_header cHead);
void addHandle(int skt, struct handle h);
void sendFlag(int skt, uint8_t flag);
void handlePacket(int skt);
void sReadSockets();
void sendHandleList(int skt, struct chat_header ch); 
void broadcastMsg(int skt, struct chat_header ch);
void fwdMsg(int skt, struct chat_header);
void sendHandleList(int skt, struct chat_header);

int handleKnown(struct handle h) {
   char h1[MAX_HANDLE_LEN + 1];
   char h2[MAX_HANDLE_LEN + 1];

   memcpy(h1, h.handle, h.hLen);
   h1[h.hLen] = '\0';

   for (i = 0; i < numUsers; i++) {
      memcpy(h2, uList[i].handle, uList[i].hLen);
      h2[uList[i].hLen] = '\0';

      if (uList[i].hLen == h.hLen && !strcmp(h1, h2)) {
         return uList[i].socket;
      }
   }

   return 0;
}

int recFlag1(int skt, struct chat_header cHead) {
   struct handle fHead;
   ssize_t bytes;

   if ((bytes = recv(skt, buf, sizeof(struct handle), 0)) < 0) {
      perror("receive call");
      exit(-1);
   }

   memcpy(&fHead, buf, sizeof(uint8_t));
   bp = buf + sizeof(uint8_t);

   memcpy(fHead.handle, bp, fHead.hLen);
   bp += fHead.hLen;

   printf("Received Flag 1:\n\tlength = %d\thandle = %.*s\n", fHead.hLen, fHead.hLen, fHead.handle);

   if (!handleKnown(fHead)) {
      addHandle(skt, fHead);
      return 1;
   }

   return 0;
}

void addHandle(int skt, struct handle h) {
   struct user *tempList;

   if (numUsers >= uListSize - 1) {
      tempList = uList;
      uList = calloc(uListSize * 10, sizeof(struct user));
      uListSize *= 10;
      memcpy(uList, tempList, numSocks * sizeof(struct user));
      free(tempList);
   }

   memcpy(&uList[numUsers], &h, sizeof(struct handle));
   numUsers++;
}

void sendFlag(int skt, uint8_t flag) {
   struct chat_header h;

   h.length = 0;
   h.flag = flag;

   memcpy(buf, &h, sizeof(struct chat_header));

   printf("send flag %d to %d\n", flag, skt);
   if (send(skt, buf, sizeof(struct chat_header), 0) < 0) {
      perror("Error in send call: ");
      exit(-1);
   }

   printf("sent Flag %d to socket %d\n", flag, skt);
}

void broadcastMsg(int skt, struct chat_header ch) {

}

void fwdMsg(int skt, struct chat_header ch) {
   ssize_t bytes;
   struct handle destHandle;
   int destSock;

   if ((bytes = recv(uList[i].socket, buf, ch.length - sizeof(struct chat_header), 0)) < 0) { 
      perror("Error in recv() call: ");
      exit(-1);
   } else if (bytes > 0) {
      memcpy(&destHandle, buf, sizeof(struct handle));

      if ((destSock = handleKnown(destHandle)) <= 0) {
         ch.length = sizeof(struct chat_header) + destHandle.hLen + 1;
         ch.flag = FLAG_7;
      
         memcpy(buf, &ch, sizeof(struct chat_header));
         bp = buf + sizeof(struct chat_header);
         memcpy(bp, &destHandle, sizeof(struct handle));

         if (send(skt, buf, sizeof(struct chat_header) + destHandle.hLen + 1, 0) < 0) {
            perror("Error in server send call: ");
            exit(-1);
         }
      } else {
         if (send(destSock, buf, bytes, 0) < 0) {
            perror("Error in server send call: ");
            exit(-1);
         }
         printf("forwarded a message\n");
      }
   
   }
 
     
}

void sendHandleList(int skt, struct chat_header ch) {

}

void handlePacket(int skt) {
   struct chat_header cHead;

   memcpy(&cHead, buf, sizeof(struct chat_header));
   cHead.length = ntohs(cHead.length);
   bp = buf + sizeof(struct chat_header);

   printf("handling a packet. FLAG => %d\n", cHead.flag);

   switch (cHead.flag) {
      case FLAG_1:
         if (recFlag1(skt, cHead))
            sendFlag(skt, FLAG_2);
         else
            sendFlag(skt, FLAG_3);
         break;
      case FLAG_4:
         broadcastMsg(skt, cHead);
         break;
      case FLAG_5:
         fwdMsg(skt, cHead);
         break;
      case FLAG_8:
         sendFlag(skt, FLAG_9);
         break;
      case FLAG_10:
         sendHandleList(skt, cHead);
         break;
   }

}

void connectNewSocket() {
   int newSocket;
   struct user *tempList;

   newSocket = serverAccept(svrSocket);

   printf("New Client Connected to Server\n\tsocket fd: %d\n", newSocket);

   if (numUsers >= uListSize - 1) {
      tempList = uList;
      uList = calloc(uListSize * 10, sizeof(struct user));
      uListSize *= 10;
      memcpy(uList, tempList, sizeof(uListSize) * sizeof(struct user));
      free(tempList);
   }

   uList[numSocks].socket = newSocket;
   numSocks++;
}

void sReadSockets() {
   int bytes;

   /* doint forget to check the svrSocket for waiting connecting sockets */
   if (FD_ISSET(svrSocket, &sSet)) {
      connectNewSocket();
   }

   for (i = 0;i < numSocks; i++) {

      if (uList[i].socket != 0 && FD_ISSET(uList[i].socket, &sSet)) { 
   
         if ((bytes = recv(uList[i].socket, buf, sizeof(struct chat_header), 0)) < 0) { 
            perror("Error in recv() call: ");
            exit(-1);
         } else if (bytes > 0) {
            handlePacket(uList[i].socket);
         }
      }
   }
}

void loop() {
   int selectReturn;

   while (1) {

      FD_ZERO(&sSet);
      FD_SET(svrSocket, &sSet);

      for (i = 0; i < numSocks; i++) {
         if (uList[i].socket != 0)
            FD_SET(uList[i].socket, &sSet);
      }

      if ((selectReturn = select(FD_SETSIZE, &sSet, (fd_set *)0, (fd_set *)0, &time)) < 0) {
         perror("Error in select call: ");
         exit(-1);
      } else if (selectReturn > 0) {
         sReadSockets();
      }
   }

}

void init(int argc, char *argv[]) {
   uListSize = 10;
   numUsers = 0;
   numSocks = 0;
   uList = calloc(uListSize, sizeof(struct user));

   time.tv_sec = 0;
   time.tv_usec = 0;

   svrSocket = parseServerArgs(argc, argv);
}

int main(int argc, char *argv[]) {

   init(argc, argv);
   loop();

   return 0;
}
