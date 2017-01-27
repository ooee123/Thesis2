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

#include "testing.h"

void wait_for_acknowledgement(int socket_num) {
   int reading;
   unsigned char acknowledgement_packet[3];

   reading = read(socket_num, acknowledgement_packet, 3);
   if (reading < 0) {
      perror("read call");
      exit(-1);
   }

   if (acknowledgement_packet[2] == 3) { 
      printf("This handle is already taken. Please choose another.\n");
      exit(-1);
   }
}


int tcp_send_setup(char *host_name, char *handle, char *port) {
   int socket_num, sent;
   unsigned short len = 4 + strlen(handle), nlen = htons(len);
   struct sockaddr_in remote;
   struct hostent *hp;
   unsigned char setup_packet[len];

   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket call");
      exit(-1);
   }
   
   remote.sin_family= AF_INET;

   if ((hp = gethostbyname(host_name)) == NULL) {
      printf("Error getting hostname: %s\n", host_name);
      exit(-1);
   }
   
   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   remote.sin_port= htons(atoi(port));

   if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0) {
      perror("connect call");
      exit(-1);
   }

   memcpy(setup_packet, &nlen, 2);
   setup_packet[2] = 1;
   setup_packet[3] = strlen(handle);
   memcpy(setup_packet + 4, handle, len);

   sent = send(socket_num, setup_packet, len, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }

   wait_for_acknowledgement(socket_num);

   return socket_num;
}

void chat(int socket_num) {
   char *send_buf;
   int bufsize = 0;
   int msg_len = 0;
   int sent = 0;


   bufsize = 1024;
   send_buf = (char *) malloc(bufsize);

   while (1) {

      printf("$: ");
   
      msg_len = 0;
      while ((send_buf[msg_len] = getchar()) != '\n' && msg_len < 80)
            msg_len++;

      send_buf[msg_len] = '\0';
           
      sent = send(socket_num, send_buf, msg_len, 0);
      if (sent < 0) {
         perror("send call");
         exit(-1);
      }

      printf("String sent: %s \n", send_buf);
      printf("Amount of data sent is: %d\n", sent);
   }
       
   close(socket_num);
}

int main(int argc, char * argv[]) {
   if (argc!= 4) {
      printf("usage: %s host-name handle port-number \n", argv[0]);
      exit(1);
   }

   if (strlen(argv[2]) > 255) {
      printf("Handle must be less than 255 characters\n");
      exit(1);
   }

   chat(tcp_send_setup(argv[1], argv[2], argv[3]));
   return 0; 
}

