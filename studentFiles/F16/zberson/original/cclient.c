#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <netdb.h>

#include "testing.h"

#include "cclient.h"
#include "lib.h"
#include "packets.h"
#include "connection.h"

static char name[MAX_NAME_SIZE + 1];
static int sock = 0;
static struct termios old;
static struct termios cur;
   
static void process();

static void print_usage(const char** argv) {
   printf("usage: %s <client handle name> <Host IP> <port>\n", *argv);
   exit(-1);
}

static void parse_args(int argc, const char** argv) {
   if(argc != 4) {
      print_usage(argv);
   }

   sock = dns_lookup(*(argv + CMD_OFFSET_IP), *(argv + CMD_OFFSET_PORT));

   memset(name, '\0', MAX_NAME_SIZE + 1);
   strcpy(name, *(argv + CMD_OFFSET_NAME));
}

static void print_commands() {
   printf("usage: <command> [option]...\ncommands:\n");
   printf("\t%%M <handle> [text]\n\t%%B [text]\n");
   printf("\t%%L\n\t%%E\n");
}

static void init() {
   raw_packet* raw = create_raw_packet();
   rdy_packet* rdy_send = create_rdy_packet(MAX_PACKET_LEN);
   rdy_packet* rdy_rec = create_rdy_packet(MAX_PACKET_LEN);
   fd_set readfd;

   raw->type = FLAG1;
   raw->packet_len = OFFSET_HANDLE_NAME + strlen(name);
   raw->src_handle_len = strlen(name);
   raw->src_handle_name = calloc(strlen(name) + 1, sizeof(char));
   memcpy(raw->src_handle_name, name, strlen(name));
   
   construct_packet(rdy_send, raw);

   write(sock, rdy_send, raw->packet_len);
   
   destroy_raw_packet(raw);
   destroy_rdy_packet(rdy_send);
   destroy_rdy_packet(rdy_rec);

   tcgetattr(STDIN_FILENO, &old);
   cur = old;
   cur.c_lflag &= ~ICANON;
   tcsetattr(STDIN_FILENO, TCSANOW, &cur);

   FD_ZERO(&readfd);
   FD_SET(sock, &readfd);
   select(sock + 1, &readfd, NULL, NULL, NULL);
   process();
}

static void exit_prog() {
   printf("\n");
   fflush(stdout);
   tcsetattr(STDIN_FILENO, TCSANOW, &old);
   exit(0);
}

static void request_exit() {
   raw_packet* raw = create_raw_packet();
   rdy_packet* rdy = create_rdy_packet(OFFSET_HANDLE_LEN);

   raw->packet_len = OFFSET_HANDLE_LEN;
   raw->type = FLAG8;
   construct_packet(rdy, raw);

   write(sock, rdy, OFFSET_HANDLE_LEN);

   destroy_raw_packet(raw);
   destroy_rdy_packet(rdy);
}

static void request_list() {
   raw_packet* raw = create_raw_packet();
   rdy_packet* rdy = create_rdy_packet(OFFSET_HANDLE_LEN);

   raw->packet_len = OFFSET_HANDLE_LEN;
   raw->type = FLAG10;
   construct_packet(rdy, raw);

   write(sock, rdy, OFFSET_HANDLE_LEN);

   destroy_raw_packet(raw);
   destroy_rdy_packet(rdy);
}

static BOOLEAN get_message(char* buf) {
   int i = 0;
   BOOLEAN result = TRUE;

   buf[i++] = fgetc(stdin);
   while('\n' != buf[i-1] && i < MAX_INPUT_SIZE) {
      buf[i++] = fgetc(stdin);
   }
   if('\n' == buf[i-1]) {
     i--;
     result = FALSE;
   }
   buf[i] = '\0';

   return result;
}

static void send_message(char* dest_name, BOOLEAN isEmpty) {
//loop whole function until all of message is parsed from input
   raw_packet* raw = create_raw_packet();
   rdy_packet* rdy;
   BOOLEAN cont;

//loop until cont FALSE: no more message to parse
   while(cont) {
      raw->message = calloc(MAX_INPUT_SIZE + 1, sizeof(char));
      if(!isEmpty) {
         cont = get_message(raw->message);
      }
      if(dest_name) {
         raw->type = FLAG5;
         raw->packet_len = OFFSET_HANDLE_NAME + strlen(dest_name) + 1
                       + strlen(name) + strlen(raw->message);
         raw->dest_handle_len = strlen(dest_name);
         raw->dest_handle_name = calloc(strlen(dest_name) + 1, sizeof(char));
         strcpy(raw->dest_handle_name, dest_name);
      }
      else {
         raw->type = FLAG4;
         raw->packet_len = OFFSET_HANDLE_NAME + strlen(name)
                            + strlen(raw->message);
      }
    
      raw->src_handle_len = strlen(name);
      raw->src_handle_name = calloc(strlen(name) + 1, sizeof(char));
      strcpy(raw->src_handle_name, name);
         
      rdy = create_rdy_packet(raw->packet_len);   
      construct_packet(rdy, raw);
    
      write(sock, rdy, raw->packet_len);
      raw = recycle_raw_packet(raw);
      destroy_rdy_packet(rdy);
      rdy = NULL;
   }
   destroy_raw_packet(raw);
   destroy_rdy_packet(rdy);
}

static void wait_for_input() {
   char tmp = '\0';
   
   while('\n' != tmp) {
      tmp = fgetc(stdin);
   }
}

static void prompt() {
   char input[IN_BUF] = {0};
   char cmd = '\0';
   int chars_read = 0;

   input[chars_read++] = fgetc(stdin);
   if('%' != input[0]) {
      cmd = '\0';
      if('\n' != input[0]) {
         wait_for_input();
      }
   }
   else {
      input[chars_read++] = fgetc(stdin);
      if('\n' != input[1]) {
         cmd = input[1];
         input[chars_read++] = fgetc(stdin);
      }
   }
   switch (cmd) {
      case 'm':
      case 'M':
         if(input[2] != ' ') {
            if(input[2] != '\n') {
               wait_for_input();
            }
            print_commands();
            break;
         }
         memset(input, '\0', 3);
         chars_read = 0;
         input[chars_read] = fgetc(stdin);
         while('\n' != input[chars_read] && ' ' != input[chars_read]) {
            input[++chars_read] = fgetc(stdin);
         }
         if(chars_read == 0 && '\n' == input[chars_read]) {
            print_commands();
         }
         else {
            if('\n' == input[chars_read]) {
               input[chars_read] = '\0';
               send_message(input, TRUE);
            }
            else {
               input[chars_read] = '\0';
               send_message(input, FALSE);
            }
         }
         break;
      case 'b':
      case 'B':
         if(input[2] == ' ') {
            memset(input, '\0', 3);
            send_message(NULL, FALSE);
         }
         else if(input[2] == '\n') {
            memset(input, '\0', 3);
            send_message(NULL, TRUE);
         }
         else {
            print_commands();
         }
         break;
      case 'l':
      case 'L':
         if(input[2] != '\n') {
            wait_for_input();
            print_commands();
         }
         else {
            request_list();
         }
         break;
      case 'e':
      case 'E':
         if('\n' != input[2]) {
            wait_for_input();
            print_commands();
         }
         else {
            request_exit();
         }
         break;
      default:
         print_commands();
         break;
   }
}

static void talk() {
   fd_set readfd;
   int status = 0;

   printf("$: ");
   fflush(stdout);
   fflush(stdin);
   
   while(TRUE) {
      FD_ZERO(&readfd);
      FD_SET(sock, &readfd);
      FD_SET(0, &readfd);

      status = select(sock + 1, &readfd, NULL, NULL, NULL);
      if(-1 == status) {
         perror("select call");
      }
      if(FD_ISSET(sock, &readfd)) {
         process();
      }
      if(FD_ISSET(0, &readfd)) {
         tcsetattr(STDIN_FILENO, TCSANOW, &old);
         prompt();
         tcsetattr(STDIN_FILENO, TCSANOW, &cur);
      }
      status = 0;
      printf("$: ");
      fflush(stdout);
      fflush(stdin);
   }
}

static void process() {
   rdy_packet* rdy = create_rdy_packet(MAX_PACKET_LEN);
   raw_packet* raw = create_raw_packet();
   int status;
   int to_process = 0;
   int i;
   int size = 0;

   status = read(sock, rdy, OFFSET_HANDLE_LEN);
   if(!status) {
      exit_prog();
   }
   if(*(rdy + OFFSET_FLAG) != FLAG12) {
      size = ntohs(*((uint16_t*)rdy)) - OFFSET_HANDLE_LEN;
   }
   else {
      status += read(sock, rdy + OFFSET_HANDLE_LEN, 1);
      size = *(rdy + OFFSET_HANDLE_LEN);
   }
   status = read(sock, rdy + status, size);
   deconstruct_packet(raw, rdy);

   switch(raw->type) {
      case FLAG2:
         talk();
         break;
      case FLAG3:
         printf("Handle already in use: %s", name);
         fflush(stdout);
         exit_prog();
         break;
      case FLAG4:
      case FLAG5:
         printf("\n%s: ", raw->src_handle_name);
         printf("%s\n", raw->message);
         fflush(stdout);
         break;
      case FLAG7:
         printf("\nClient with handle %s does not exist.\n", raw->src_handle_name);
         fflush(stdout);
         break;
      case FLAG9:
         exit_prog();
         break;
      case FLAG11:
         to_process = raw->to_be_sent;
         printf("\nNumber of clients: %u\n", to_process);
         for(i = 0; i < to_process; i++) {
            process();
         }
         fflush(stdout);
         break;
      case FLAG12:
         printf("\t%s\n", raw->src_handle_name);
         break;
      default:
         break;
   }
}

int main(int argc, const char** argv) {
   parse_args(argc, argv);
   init();

   return 0;
}
