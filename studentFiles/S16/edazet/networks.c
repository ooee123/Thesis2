#include "networks.h"

int sendPacket(int socket_num, uint8_t *send_buf, uint16_t send_len) {
   int sent = 0;

   if ((sent = send(socket_num, send_buf, send_len, 0)) < 0) {
      perror("send error\n");
      exit(-1);
   }
   return sent;
}

int recvPacket(int socket_num, uint8_t *rcv_buf, uint16_t rcv_len) {
   int rcv = 0;

   if ((rcv = recv(socket_num, rcv_buf, rcv_len, 0)) < 0) {
      perror("recv error\n");
      exit(-1);
   }
   return rcv;
}

int recvPacketClient(int socket_num, uint8_t *rcv_buf, uint16_t rcv_len) {
   int rcv = 0;

   if ((rcv = recv(socket_num, rcv_buf, rcv_len, 0)) < 0) {
      perror("recv error\n");
      exit(-1);
   }
   if (!rcv) {
	  fprintf(stderr, "\nServer Terminated\n");
	  exit(-1);
   }
   return rcv;
}

normal_header *safeNormalHeader() {
   normal_header *h;

   if ((h = (normal_header *)malloc(sizeof(normal_header))) == NULL) {
      perror("normal header malloc");
      exit(-1);
   }
   return h;
}

uint8_t *safeCharMalloc(int size) {
   char *c;

   if ((c = malloc(size)) == NULL) {
      perror("safe malloc");
      exit(-1);
   }
   memset(c, 0, size);
   return (uint8_t *)c;
}

uint8_t findStringLength(char *string) {
   int i, length;

   i = length = 0;
   while (string[i] != '\0') {
      length++;
      i++;
   }
   if (length > MAX_HANDLE_LEN) {
      fprintf(stderr, "Handle length is greater than 255 bytes\n");
      exit(-1);
   }

   return length;
}

int getChars(uint8_t **buf, uint16_t *len, uint8_t stop, uint16_t max_len, int null_term) {
   int c;

   *len = 0;
   while (*len < max_len && (c = (uint8_t)getchar()) != stop && c != '\n') {
	  (*buf)[*len] = c;
      (*len)++;
   }
   if (null_term) {
      (*buf)[*len] = '\0';
      (*len)++;
   }

   return c == '\n';   // indicates if max_len was reached
}

void sendNormalPacket(int socket_num, uint8_t flag) {
   normal_header *h;
   int sent;

   h = safeNormalHeader();
   h->pkt_length = htons(sizeof(normal_header));
   h->flag = flag;

   sent = sendPacket(socket_num, (uint8_t *)h, ntohs(h->pkt_length));
   free(h);
}

void clearStdin() {
   int c;
   while ((c = getchar()) != '\n');
}
