#include "network.h"

static char *cHandle;

static int commSocket;

static fd_set sSet;
static int numSocks;
static struct timeval time; 

static char buf[MAX_BUF_SIZE];
static char *bp;
//static int i;

static struct handle *hList;
static uint32_t numHandles;

void cReadCommSocket();
void readUserInput();
void sendFlag1();
void recMessage(struct chat_header ch);
void recNumHandles();
void recHandleList();

void clearBuffer(int skt) {
   if (recv(skt, buf, MAX_BUF_SIZE, 0) < 0) {
      perror("rec call");
      exit(-1);
   }
}

void recMessage(struct chat_header ch) {
   ssize_t bytes;

   uint8_t srcHandleLen;
   char *srcHandle;
   char *msg;

   if ((bytes = recv(commSocket, buf, ch.length - sizeof(struct chat_header), 0)) < 0) {
      perror("Error in client receive: ");
      exit(-1);
   } else if (bytes > 0) {
      bp = buf;

      if (ch.flag == FLAG_5) {
         bp += strlen(cHandle) + sizeof(uint8_t);
      }

      memcpy(&srcHandleLen, bp, sizeof(uint8_t));
      srcHandleLen = srcHandleLen;
      bp += sizeof(uint8_t);

      srcHandle = calloc(srcHandleLen + 1, sizeof(uint8_t));
      memcpy(srcHandle, bp, srcHandleLen);
      bp += srcHandleLen;
      srcHandle[srcHandleLen] = '\0';

      msg = calloc(MAX_MSG_LEN, sizeof(uint8_t));
      strcpy(msg, bp);

      printf("Message received from %s\nmsg: %s\n", srcHandle, msg);
   }

   clearBuffer(commSocket);
}

void recNumHandles() {
   ssize_t bytes;
   
   if ((bytes = recv(commSocket, buf, sizeof(uint32_t), 0)) < 0) {
      perror("Error in client receive: ");
      exit(-1);
   } else if (bytes > 0) {
      memcpy(&numHandles, buf, sizeof(uint32_t));
      numHandles = ntohl(numHandles);

      hList = calloc(numHandles, sizeof(struct handle));
   }

   clearBuffer(commSocket);
}

void recHandleList() {
   ssize_t bytes;
   uint8_t hLen;
   char han[MAX_HANDLE_LEN];

   while (numHandles) {

      if ((bytes = recv(commSocket, buf, sizeof(uint8_t), 0)) < 0) {
         perror("Error in client receive: ");
         exit(-1);
      } else if (bytes > 0) {
         memcpy(&hLen, buf, sizeof(uint8_t));
      }

      if ((bytes = recv(commSocket, buf, hLen, 0)) < 0) {
         perror("Error in client receive: ");
         exit(-1);
      } else if (bytes > 0) {
         memcpy(han, buf, hLen);
         han[hLen] = '\0';
      }

      printf("\t%s\n", han);

      numHandles--;
   }

   clearBuffer(commSocket);
}

void sendFlag1() {
   struct chat_header h;
   struct handle handle;

   h.length = sizeof(struct chat_header) + strlen(cHandle) + 1;
   h.length = htons(h.length);
   h.flag = FLAG_1;

   handle.hLen = strlen(cHandle);
   memcpy(handle.handle, cHandle, strlen(cHandle));

   memcpy(buf, &h, sizeof(struct chat_header));
   bp = buf + sizeof(struct chat_header);
   memcpy(bp, &handle, sizeof(struct handle));

   if (send(commSocket, buf, ntohs(h.length), 0) < 0) {
      perror("Error in client send call: ");
      exit(-1);
   }
}

void cReadCommSocket() {
   ssize_t bytes;
   struct chat_header cHead;
   struct handle errHandle;

   printf("receiving chat header for %s\n", cHandle);
   
   if ((bytes = recv(commSocket, buf, sizeof(struct chat_header), 0)) < 0) {
      perror("Error in recv() call: ");
      exit(-1);
   } else if (bytes > 0) {
      memcpy(&cHead, buf, sizeof(struct chat_header));
      bp = buf + sizeof(struct chat_header);

      cHead.length = ntohs(cHead.length);

      printf("received chat header for %s\nlength: %d flag: %d\n", cHandle, cHead.length, cHead.flag);

      switch (cHead.flag) {
         case FLAG_2:
            printf("Flag 2 received\n");
            break;
         case FLAG_3:
            printf("Flag 3 received. Handle already exists, exiting program\n");
            exit(0);
            break;
         case FLAG_4:
            printf("flag 4 %s\n", cHandle);
            recMessage(cHead);
            break;
         case FLAG_5:
            recMessage(cHead);
            break;
         case FLAG_7:
            if ((bytes = recv(commSocket, buf, sizeof(struct handle), 0)) < 0) {
               perror("Error in recv() call: ");
               exit(-1);
            } else if (bytes > 0) {
               memcpy(&errHandle, buf, cHead.length - sizeof(struct chat_header));
            }

            printf("Flag 7 received. Destination handle <%.*s> does not exist\n", errHandle.hLen, errHandle.handle);            
            break;
         case FLAG_9:
            printf("Flag 9 received. Exiting program.\n");
            exit(0);
            break;
         case FLAG_11:
            recNumHandles();
            break;
         case FLAG_12:
            recHandleList();
            break;
      }
   }

}

void readUserInput() {
   char buffer[MAX_BUF_SIZE];
   char cmd[3];
   char handle[MAX_HANDLE_LEN + 1];
   char msg[MAX_MSG_LEN];
   char *ptr;

   struct chat_header ch;
   struct handle dHan;

   fgets(cmd, 3, stdin);
   cmd[2] = '\0';

   switch (cmd[1]) {
      case 'M':
      case 'm':
         fgets(buffer, MAX_HANDLE_LEN + MAX_MSG_LEN + 1, stdin);
         ptr = strchr(buffer, ' ');
         *ptr++ = '\0';
         strcpy(handle, buffer);
         memcpy(msg, ptr, MAX_MSG_LEN);
         msg[MAX_MSG_LEN - 1] = '\0';
         ptr += MAX_MSG_LEN;
         
         dHan.hLen = strlen(handle);
         memcpy(dHan.handle, handle, dHan.hLen);

         ch.length = sizeof(struct chat_header) + dHan.hLen + strlen(msg) + 2;
         ch.flag = FLAG_5;

         memcpy(buf, &ch, sizeof(struct chat_header));
         bp = buf + sizeof(struct chat_header);
         memcpy(bp, &dHan, dHan.hLen + 1);
         bp += dHan.hLen + 1;
         memcpy(bp, msg, strlen(msg) + 1);

         if (send(commSocket, buf, ch.length, 0) < 0) {
            perror("send call");
            exit(-1);
         }
         break;
      case 'B':
      case 'b':
         break;
   }

   
}

void loop() {
   while (1) {

      FD_ZERO(&sSet);
      FD_SET(commSocket, &sSet);
      FD_SET(STDIN_FILENO, &sSet);

      numSocks = select(FD_SETSIZE, &sSet, (fd_set *)0, (fd_set *)0, &time);

      if (numSocks < 0) {
         perror("Error in select call: ");
         exit(-1);
      } else if (FD_ISSET(commSocket, &sSet)) {
         printf("comm sock | %s\n", cHandle);
         cReadCommSocket();
      } else if (FD_ISSET(STDIN_FILENO, &sSet)) {
         printf("usr input\n");
         readUserInput();
      }
   }
}

void init(int argc, char *argv[]) {
   commSocket = parseClientArgs(argc, argv, &cHandle);
   sendFlag1();
   
   time.tv_sec = 0;
   time.tv_usec = 0;
}

int main(int argc, char *argv[]) {

   init(argc, argv);
   loop();

   return 0;
}
