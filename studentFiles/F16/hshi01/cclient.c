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

#define MAXMSG 1001
#define MAXHDL 256

#define MTYPE 5
#define BTYPE 4
#define LTYPE 10
#define ETYPE 8

typedef struct packet {
   int type;
   int sock;
   char *shdl;
   char *dhdl;
   char *msg;
   int numlist;
} packet;

packet *PacketInit(int sock, char *shdl) {
   static packet pack;
   int len = strlen(shdl);

   if (strlen(shdl) > 255)
      len = MAXHDL - 1;

   pack.shdl = calloc(MAXHDL, 1);
   memcpy(pack.shdl, shdl, len);
   pack.dhdl = malloc(MAXHDL);
   pack.msg = malloc(MAXMSG);
   pack.sock = sock;
   pack.numlist = 0;

   return &pack;
}

void SendMsgPack(packet *pack) {
   short len = 2 + 1 + 1 + strlen(pack->dhdl) + 1 + strlen(pack->shdl) + strlen(pack->msg);
   char buf[len];
   short nolen = htons(len);
   int lencnt = 4 + strlen(pack->dhdl);
   int sent;

   memcpy(buf, (char *)&nolen, 2);
   buf[2] = MTYPE;
   buf[3] = strlen(pack->dhdl);
   memcpy(&(buf[4]), pack->dhdl, strlen(pack->dhdl));
   buf[lencnt++] = strlen(pack->shdl);
   memcpy(&(buf[lencnt]), pack->shdl, strlen(pack->shdl));
   lencnt += strlen(pack->shdl);
   memcpy(&(buf[lencnt]), pack->msg, strlen(pack->msg));

   /* now send the data */
   sent = send(pack->sock, buf, len, 0);
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

int SendMsgParse(packet *pack) {
   char n;
   int hdl = 3;
   int ndx = 0;

   n = getchar();
   while (n != '\n') {
      if (hdl == 3 && n == ' ') {
         hdl--;
         n = getchar();
      }

      if (hdl == 3 && n != ' ') {
         while (n != '\n')
            n = getchar();
         return -1;
      }

      if (hdl == 2 && n == ' ')
         n = getchar();

      if (hdl == 2 && n != ' ' && n != '\n') {
         pack->dhdl[ndx] = n;
         n = getchar();
         ndx++;
         hdl--;
      }

      if (hdl == 1 && n != ' ' && n != '\n') {
         pack->dhdl[ndx] = n;
         n = getchar();
         ndx++;
      }

      if (hdl == 1 && n == ' ') {
         hdl--;
         pack->dhdl[ndx] = 0;
         n = getchar();
         ndx = 0;
      }

      if (!hdl && n != '\n') {
         pack->msg[ndx] = n;
         ndx++;
         n = getchar();
         if (ndx == 1000) {
            pack->msg[ndx] = 0;
            SendMsgPack(pack);
            ndx = 0;
         }
      }
   }
   pack->msg[ndx] = 0;

   if (hdl == 1) {
      pack->dhdl[ndx] = 0;
   }

   if (hdl > 1) {
      return -1;
   }

   SendMsgPack(pack);

   return 0;
}

void SendBrdcastPack(packet *pack) {
   short len = 2 + 1 + 1 + strlen(pack->shdl) + strlen(pack->msg);
   char buf[len];
   short nolen = htons(len);
   int sent;

   memcpy(buf, (char *)&nolen, 2);
   buf[2] = BTYPE;
   buf[3] = strlen(pack->shdl);
   memcpy(&(buf[4]), pack->shdl, strlen(pack->shdl));
   memcpy(&(buf[4 + strlen(pack->shdl)]), pack->msg, strlen(pack->msg));

   /* now send the data */
   sent = send(pack->sock, buf, len, 0);
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

int SendBrdcastParse(packet *pack) {
   char n;
   int ndx = 0;
   int txt = 1;

   n = getchar();
   while (n != '\n') {
      if (txt && n != ' ') {
         while (n != '\n')
            n = getchar();
         return -1;
      }
      if (txt == 1 && n == ' ') {
         txt--;
         n = getchar();
      }
      if (n != '\n') {
         pack->msg[ndx] = n;
         n = getchar();
         ndx++;
         if (ndx == 1000) {
            pack->msg[ndx] = 0;
            SendBrdcastPack(pack);
            ndx = 0;
         }
      }
   }
   pack->msg[ndx] = 0;
      
   SendBrdcastPack(pack);

   return 0;
}

void SendPackHdr(packet *pack, char flag) {
   short len = 3;
   char buf[len];
   short nolen = htons(len);
   int sent;

   memcpy(buf, (char *)&nolen, 2);
   buf[2] = flag;

   /* now send the data */
   sent = send(pack->sock, buf, len, 0);
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }
}

void Parse(packet *pack) {
   char n;
   int error = 0;

   n = getchar();
   if (n == '%') {
      n = getchar();
      if (n == 'M' || n == 'm') {
         pack->type = MTYPE;
         error = SendMsgParse(pack);
      }
      else if (n == 'B' || n == 'b') {
         pack->type = BTYPE;
         error = SendBrdcastParse(pack);
      }
      else if (n == 'L' || n == 'l') {
         n = getchar();
         if (n != ' ' && n != '\n')
            error = -2;
         else {
            pack->type = LTYPE;
            SendPackHdr(pack, LTYPE);
            while (n != '\n')
               n = getchar();
         }
      }
      else if (n == 'E' || n == 'e') {
         n = getchar();
         if (n != ' ' && n != '\n')
            error = -2;
         else {
            pack->type = ETYPE;
            SendPackHdr(pack, ETYPE);
            while (n != '\n')
               n = getchar();
         }
      }
      else {
         error = -2;
      }
   }
   else {
      error = -2;
   }

   if (error < 0) {
      printf("Error: Invalid Command\n");
   }

   if (error < -1) {
      while (n != '\n')
         n = getchar();
   }
}

void RecvConSetup(packet *pack) {
   char buf[3];

   recv(pack->sock, buf, 3, 0);

   if (buf[2] == 3) {
      printf("Handle already in use: %s\n", pack->shdl);
      close(pack->sock);
      exit(-1);
   }
}

void ConSetup(packet *pack) {
   short len = 2 + 1 + 1 + strlen(pack->shdl);
   char buf[len];
   short nolen = htons(len);
   int sent;

   memcpy(buf, (char *)&nolen, 2);
   buf[2] = 1;
   buf[3] = strlen(pack->shdl);
   memcpy(&buf[4], pack->shdl, strlen(pack->shdl));

   /* now send the data */
   sent = send(pack->sock, buf, len, 0);
   if(sent < 0)
   {
      perror("send call");
      exit(-1);
   }

   RecvConSetup(pack);

   printf("$: ");
   fflush(stdout);
}

void RecvBroadcast(short packlen, packet *pack) {
   char *buf = calloc(packlen, 1);
   unsigned char hdllen;
   char *shdl;
   char *text;

   recv(pack->sock, buf, packlen, 0);

   hdllen = buf[0];
   shdl = calloc(hdllen + 1, 1);
   memcpy(shdl, &buf[1], hdllen);

   text = calloc(packlen - hdllen, 1);
   memcpy(text, &buf[hdllen + 1], packlen - hdllen - 1);

   printf("\n%s: %s\n", shdl, text);
}

void RecvMessage(short packlen, packet *pack) {
   char *buf = calloc(packlen, 1);
   unsigned char dhdllen;
   char *dhdl;
   unsigned char shdllen;
   char *shdl;
   char *text;

   recv(pack->sock, buf, packlen, 0);

   dhdllen = buf[0];
   dhdl = calloc(dhdllen + 1, 1);
   memcpy(dhdl, &buf[1], dhdllen);

   shdllen = buf[dhdllen + 1];
   shdl = calloc(1 + shdllen, 1);
   memcpy(shdl, &buf[1 + dhdllen + 1], shdllen);

   text = calloc(packlen - dhdllen - 1 - shdllen, 1);
   memcpy(text, &buf[1 + dhdllen + 1 + shdllen], packlen - dhdllen - 2 - shdllen);

   printf("\n%s: %s\n", shdl, text);
}

void RecvMessageError(short packlen, packet *pack) {
   char *buf = calloc(packlen, 1);
   unsigned char hdllen;
   char *shdl;

   recv(pack->sock, buf, packlen, 0);

   hdllen = buf[0];
   shdl = calloc(hdllen + 1, 1);
   memcpy(shdl, &buf[1], hdllen);

   printf("\nClient with handle %s does not exist.\n", shdl);
}

void NumList(packet *pack) {
   int nonum;

   recv(pack->sock, (char *)&nonum, 4, 0);
   nonum = ntohl(nonum);

   pack->numlist = nonum;
}

void PrintList(short packlen, packet *pack) {
   char *buf = calloc(packlen, 1);
   char flag;
   unsigned char hdllen;
   char *shdl;

   printf("\nNumber of clients: %d\n", pack->numlist--);

   recv(pack->sock, buf, packlen, 0);

   hdllen = buf[0];
   shdl = calloc(hdllen + 1, 1);
   memcpy(shdl, &buf[1], hdllen);

   printf("%s\n", shdl);

   while (pack->numlist--) {
      free(shdl);
      free(buf);

      recv(pack->sock, (char *)&packlen, 2, 0);
      packlen = ntohs(packlen) - 3;
      recv(pack->sock, &flag, 1, 0);

      buf = calloc(packlen, 1);

      recv(pack->sock, buf, packlen, 0);

      hdllen = buf[0];
      shdl = calloc(hdllen + 1, 1);
      memcpy(shdl, &buf[1], hdllen);

      printf("%s\n", shdl); 
   }
}

int PacketParse(packet *pack) {
   short packlen = 0;
   char flag = 0;
   int exit = 1;
   int done;

   done = recv(pack->sock, (char *)&packlen, 2, 0);
   packlen = ntohs(packlen) - 3;
   recv(pack->sock, &flag, 1, 0);

   if (!done) {
      exit = 0;
      printf("\nServer Terminated\n");
      flag = 9;
   }

   if (flag == 4) {
      RecvBroadcast(packlen, pack);
   }

   if (flag == 5) {
      RecvMessage(packlen, pack);
   }

   if (flag == 7) {
      RecvMessageError(packlen, pack);
   }

   if (flag == 9) {
      exit = 0;
   }

   if (flag == 11) {
      NumList(pack);
   }

   if (flag == 12) {
      PrintList(packlen, pack);
   }

   if (flag != 11 && flag != 9) {
      printf("$: ");
      fflush(stdout);
   }

   return exit;
}

int CheckCon(int sock, fd_set *readfds, packet *pack) {
   int exit = 1;

   FD_ZERO(readfds);
   FD_SET(sock, readfds);
   FD_SET(0, readfds);

   select(sock + 1, readfds, NULL, NULL, NULL);

   if (FD_ISSET(0, readfds)) {
      Parse(pack);
      printf("$: ");
      fflush(stdout);
   }

   if (FD_ISSET(sock, readfds)) {
      exit = PacketParse(pack);
   }

   return exit;
}

int main(int argc, char *argv[]) {
   int sock = 0, end = 1;
   packet *pack;
   fd_set readfds;

   if (argc != 4) {
      printf("Error: Incorrect Input Parameters\n");
      exit(-1);
   }

   sock = tcpClientSetup(argv[2], argv[3]);
   pack = PacketInit(sock, argv[1]);

   ConSetup(pack);

   while (end) {
      end = CheckCon(sock, &readfds, pack);
   }

   close(sock);

   return 0;
}