#include "networkFuncs.h"

/*
 * Initializes a new node and attach it to the linked list, does not
 *  initialize the handle of the node
 * Input: list - the list to add the node to
 *        socketNum - the socket number of the new node
 */
void initHandle(handleNode **list, int socketNum) {  
   handleNode *cur, *new;

   if (socketNum == 0) {
      return;
   }

   /*initialize node*/
   cur = *list;
   new = (handleNode *)malloc(sizeof(handleNode));
   new->socketNum = socketNum;
   new->handle = NULL;
   new->next = NULL;

   /*add the node to the linked list*/
   if (*list == NULL) {
      *list = new;
   }
   else {
      while (cur->next != NULL) {
         cur = cur->next;
      }
      cur->next = new;
   }
}

/* 
 * Removes a handle from the linked list
 * Input: list - linked list of nodes
 *        socketNum - socket number of the node to remove
 */
void freeHandle(handleNode **list, int socketNum) {
   int found = 0;
   handleNode *prev, *cur;

   prev = NULL;
   cur = *list;
   
   /*find the handle*/
   while (!found && cur != NULL) {
      if (cur->socketNum == socketNum) {
         found = 1;
      }
      else {
         prev = cur;
         cur = cur->next;
      }
   }

   /*free the handle*/
   if (found) {
      if (prev) {
         prev->next = cur->next;
      }
      else {
         *list = (*list)->next;
      }

      free(cur->handle);
      free(cur);
   }
}

/*
 * Attempts to find a handle in the linked list
 * Input: list - the linked list of handleNodes to search
 *        name - the name of the handle
 * Return: the socket number or the 0 if none found
 */
int findHandle(handleNode *list, char *name) {
   handleNode *cur = list;
   
   while (cur != NULL) {
      if (cur->handle && strcmp(cur->handle, name) == 0) {
         return cur->socketNum;
      }
      cur = cur->next;
   }
   return 0;
}

/*
 * Sets the handle of a node in the linked list
 * Input: list - the linked list of nodes
 *        socketNum - the socket number of the node whose handle will be set
 *        name - the handle of the node
 */
void setHandle(handleNode *list, int socketNum, char name[MAX_HANDLE_LEN]) {
   handleNode *cur = list;
   
   /*iterate through list until found*/
   while (cur != NULL) {
      if (cur->socketNum == socketNum) {
         cur->handle = (char *)malloc(sizeof(char) * strlen(name));
         memcpy(cur->handle, name, strlen(name));
         return;
      }
      cur = cur->next;
   }
}

/*
 * Finds the highest socket number in the linked list
 * Input: list - the linked list to search
 * Return: the highest socket number in the linked list
 */
int findHighestSocket(handleNode *list) {
   handleNode *cur = list;
   int highest = -1;
   while (cur != NULL) {
      if (highest < cur->socketNum) {
         highest = cur->socketNum;
      }
      cur = cur->next;
   }
   return highest;
}

/*
 * Adds a header to the beginning of a buffer
 * Input: buf -  the buffer to add the header to
 *        flag - the flag to add to the header
 *        len - the length of the packet
 */
void createHeader(uint8_t **buf, int flag, uint16_t len) {
   header h;
   h.flag = flag;
   h.len = htons(len);
   memcpy(*buf, (uint8_t *)&h, sizeof(header));
}

/*
 * Send the data to target socket
 * Input: buf - the buffer containing the data
 *        targetSocket - the socket to send the data to
 *        sendLen - the length of the message to send
 */
void sendMessage(uint8_t *buf, int targetSocket, int sendLen) {
   int res =  send(targetSocket, buf, sendLen, 0);
   if(res < 0)
   {
      perror("send call");
      exit(EXIT_FAILURE);
   }
}

/*
 * Receives a message but blocks until it arrives
 * Input: buf - the buffer to put the message into
 *        targetSocket - the socket to receive data from
 *        wantedLen - the length to request from recv
 *        offset - offset to save data into the data
 *        isServer - true if the caller is the server
 * Return: the response from the recv call
 */
int recvCall(uint8_t **buf, int targetSocket, int wantedLen, int offset, int isServer) {
   int res = recv(targetSocket, *buf + offset, wantedLen, 0);
   if (res <= 0) {
      if (isServer) {
         return 0;
      }
      else {
         printf("Server Terminated\n");
         exit(EXIT_FAILURE);  
      }
   }
   return res;
}

/* 
 * Receives a certain amount of bytes from a socket depending on the state and
 *  flags passed in
 * Input: buf - the buffer to read into
 *        state - describes how many byte to read in
 *        targetSocket - the socket to read from
 *        flags - different behavior: blocking or nonblocking, getting the list of handles
 * Return: number of bytes in the buffer (sum of current bytes + newly read bytes)
 */
int receiveMessage(uint8_t **buf, int state, int targetSocket, int flags) {
   uint16_t wantedLen;
   int res;
   int recLen = 0;

   if (flags == HANDLE_LIST) {
      wantedLen = state;
   }

   /*determine how much data to get from recv*/
   switch (state) {
      case ONLY_ONE:
         wantedLen = 1;
         break;
      case INIT_RECV:
         wantedLen = 2;
         break;
      default:
         if (flags == HANDLE_LIST) {
            wantedLen = state;
         }
         else {
            wantedLen = getLen(*buf) - state;
            recLen = state;  
         }
   }

   /*if is blocking, loop until get length of msg*/
   do {
      res = recvCall(buf, targetSocket, wantedLen, recLen, (flags & BLOCKING_SERVER));
      recLen += res;
   } while (res && flags != NONBLOCKING && recLen < wantedLen);

   /*if is blocking, get the data of the message*/
   if (res && flags != NONBLOCKING && state == INIT_RECV) {
      res = receiveMessage(buf, recLen, targetSocket, flags);
      recLen = res;
   }
   else if (state == INIT_RECV && res == 1) {
      res = ONLY_ONE;
   }
   
   return res == 0 ? 0 : recLen;
}

//may not need this if it just... works
//state: ONLY_ONE - getting length
//       some number > ONLY_ONE
/*gets exactly 1 handle, used only for 12*/
void receiveHandleList(uint8_t **buf, int state, int targetSocket) {
   if (state == ONLY_ONE) {
      receiveMessage(buf, state, targetSocket, HANDLE_LIST);
   }
   else {
      receiveMessage(buf, state, targetSocket, HANDLE_LIST);
   }
}

/*
 * Does a select call to see if the sockets have data to receive
 * Input: fds - the fd_set struct to pass to the select call
 *        highSock - the highest socket
 * Return: the result of the select call
 */
int selectCall(fd_set *fds, int highSock) {
   int res;
   static struct timeval timeout;
   timeout.tv_sec = 1;
   timeout.tv_usec = 0;

   res = select(highSock + 1, (fd_set *)&(*fds), (fd_set *)0, (fd_set *)0, &timeout);
   // res = select(highSock + 1, (fd_set *)&(*fds), (fd_set *)0, (fd_set *)0, &timeout);

   if (res < 0) {
      perror("select call");
      exit(EXIT_FAILURE);
   }
   return res;
}

/*
 * Extracts a client's handle from a buffer
 * Input: handle - the array to move the handle to
 *        buf - the buffer that contains the handle
 *        offset - offset into the buffer
 *        lwn - the length of the handle in question
 */
void extractHandle(char handle[MAX_HANDLE_LEN], uint8_t *buf, int offset, int len) {
   memcpy(handle, buf + offset, len);
   handle[len] = '\0';
}

/*
 * Assumes that there is at least 2 bytes in the buffer
 * Input: a buffer with the length stored in the first 2 bytes
 * Return: the length in host order
 */
uint16_t getLen(uint8_t *buf) {
   return ntohs(*((uint16_t *)buf));
}