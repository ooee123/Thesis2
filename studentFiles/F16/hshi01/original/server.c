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

#define MAXBUF 1516
#define MAXHDL 256

typedef struct handle {
   char name[MAXHDL];
   int sock;
   struct handle *next;
} handle; 

void SendPackHdr(handle *csock, char flag) {
   short len = 3;
   char buf[len];
   short nolen = htons(len);
   int sent;

   memcpy(buf, (char *)&nolen, 2);
   buf[2] = flag;

   /* now send the data */
   sent = send(csock->sock, buf, len, 0);
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

handle *AddHandle(int newsock, handle *csock) {
   short packlen = 0;
   char flag = 0;
   char *buf;
   unsigned char hdllen;
   handle *temp = calloc(1, sizeof(handle));
   handle *head = csock;
   int hdlexists = 0;
   handle *ndx;

   recv(newsock, (char *)&packlen, 2, 0);
   packlen = ntohs(packlen) - 3;
   buf = malloc(packlen);
   recv(newsock, &flag, 1, 0);
   recv(newsock, buf, packlen, 0);

   // check flag = 1?
   hdllen = buf[0];
   temp->sock = newsock;
   memcpy(temp->name, &buf[1], hdllen);

   if (!csock) {
      temp->next = NULL;
      head = temp;
      SendPackHdr(temp, 2);
   }
   else {
      temp->next = head;
      ndx = head;
      while (ndx && !hdlexists) {
         if (!strcmp(ndx->name, temp->name)) {
            hdlexists = 1;
            SendPackHdr(temp, 3);
            free(temp);
         }
         ndx = ndx->next;
      }

      if (!hdlexists) {
         head = temp;
         SendPackHdr(temp, 2);
      }
   }

   temp = head;

   return head;
}

void Broadcast(short packlen, handle *client, handle *head) {
   char *buf = calloc(packlen, 1);
   unsigned char hdllen;
   char *shdl;
   handle *temp;
   char sendbuf[packlen + 3];
   short nolen = htons(packlen + 3);

   recv(client->sock, buf, packlen, 0);

   hdllen = buf[0];
   shdl = calloc(hdllen + 1, 1);
   memcpy(shdl, &buf[1], hdllen);

   memcpy(sendbuf, (char *)&nolen, 2);
   sendbuf[2] = 4;
   memcpy(&sendbuf[3], buf, packlen);

   temp = head;

   while (temp) {
      if (strcmp(temp->name, shdl)) {
         send(temp->sock, sendbuf, packlen + 3, 0);
      }
      temp = temp->next;
   }
}

void MessageError(handle *client, char hdllen, char *dhdl) {
   short len = 3 + hdllen + 1;
   char buf[len];
   short nolen = htons(len);
   int sent;

   memcpy(buf, (char *)&nolen, 2);
   buf[2] = 7;
   buf[3] = hdllen;
   memcpy(&buf[4], dhdl, hdllen);

   /* now send the data */
   sent = send(client->sock, buf, len, 0);
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

void Message(short packlen, handle *client, handle *head) {
   char *buf = calloc(packlen, 1);
   unsigned char hdllen;
   char *dhdl;
   handle *temp;
   char sendbuf[packlen + 3];
   short nolen = htons(packlen + 3);
   int found = 0;

   recv(client->sock, buf, packlen, 0);

   hdllen = buf[0];
   dhdl = calloc(hdllen + 1, 1);
   memcpy(dhdl, &buf[1], hdllen);

   memcpy(sendbuf, (char *)&nolen, 2);
   sendbuf[2] = 5;
   memcpy(&sendbuf[3], buf, packlen);

   temp = head;
   while (temp && !found) {
      if (!strcmp(temp->name, dhdl)) {
         found = 1;
         send(temp->sock, sendbuf, packlen + 3, 0);
      }
      temp = temp->next;
   }

   if (!found) {
      MessageError(client, hdllen, dhdl);
   }
}

void Flag11(handle *client, int num) {
   char buf[7];
   short len = htons(7);
   int nonum = htonl(num);
   int sent;

   memcpy(buf, (char *)&len, 2);
   buf[2] = 11;
   memcpy(&buf[3], (char *)&nonum, 4);

   /* now send the data */
   sent = send(client->sock, buf, 7, 0);
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

void List(handle *client, handle *head) {
   handle *temp = head;
   int num = 0;
   char *buf;
   short len;
   unsigned char hdllen;

   while (temp) {
      num++;
      temp = temp->next;
   }
   temp = head;

   Flag11(client, num);

   while (temp) {
      hdllen = strlen(temp->name);
      len = 2 + 1 + 1 + hdllen;
      len = htons(len);
      buf = calloc(len, 1);

      memcpy(buf, (char *)&len, 2);
      buf[2] = 12;
      buf[3] = hdllen;
      memcpy(&buf[4], temp->name, hdllen);

      send(client->sock, buf, ntohs(len), 0);

      free(buf);
      temp = temp->next;
   }
}

handle *RmvClient(handle *client, handle *head) {
   handle *prev;
   handle *cur = head;
   handle *temphead = head;
   int found = 0;

   close(client->sock);

   if (!strcmp(cur->name, client->name)) {
      temphead = head->next;
      free(head);
      return temphead;
   }

   prev = cur;
   cur = cur->next;
   while (cur && !found) {
      if (!strcmp(cur->name, client->name)) {
         prev->next = cur->next;
         free(cur);
         found = 1;
      }
      else {
         prev = cur;
         cur = cur->next;
      }
   }

   return temphead;
}

handle *PacketParse(handle *client, handle *head) {
   short packlen = 0;
   char flag = 0;
   handle *temp = head;
   int done;

   done = recv(client->sock, (char *)&packlen, 2, 0);
   if (!done)
      temp = RmvClient(client, head);

   packlen = ntohs(packlen) - 3;
   recv(client->sock, &flag, 1, 0);

   if (flag == 4) {
      Broadcast(packlen, client, head);
   }

   if (flag == 5) {
      Message(packlen, client, head);
   }

   if (flag == 8) {
      SendPackHdr(client, 9);
      temp = RmvClient(client, head);
   }

   if (flag == 10) {
      List(client, head);
   }

   return temp;
}

handle *CheckCon(fd_set *readfds, int ssock, handle *csock) {
   handle *temp = csock;
   handle *head = csock;
   int maxfds = ssock, newsock;

   FD_ZERO(readfds);
   FD_SET(ssock, readfds);

   while (temp) {
      FD_SET(temp->sock, readfds);
      if (temp->sock > maxfds)
         maxfds = temp->sock;
      temp = temp->next;
   }

   select(maxfds + 1, readfds, NULL, NULL, NULL);

   if (FD_ISSET(ssock, readfds)) {
      newsock = tcpAccept(ssock);
      head = AddHandle(newsock, csock);
   }

   temp = head;
   while (temp) {
      if (FD_ISSET(temp->sock, readfds)) {
         head = PacketParse(temp, head);
      }
      temp = temp->next;
   }

   return head;
}

int main(int argc, char *argv[]) {
   int ssock = 0;
   handle *csock = NULL;
   fd_set readfds;   

   if (argc == 2)
      ssock = tcpServerSetup(atoi(argv[1]));
   else if (argc == 1)
      ssock = tcpServerSetup(0);
   else {
      printf("Error: Incorrect number of Input Parameters\n");
      exit(-1);
   }

   while(1) {
      csock = CheckCon(&readfds, ssock, csock);
   }

   close(ssock);

   return 0;
}