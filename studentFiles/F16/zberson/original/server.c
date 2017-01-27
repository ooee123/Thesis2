#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "testing.h"

#include "lib.h"
#include "packets.h"
#include "connection.h"
#include "handle.h"

static uint16_t port;
static int server_socket;
static c_handle* clients;
static uint16_t num_clients;

static void print_usage(const char** argv) {
   fprintf(stderr, "usage: %s [port]\n", *argv);
   exit(-1);
}

static void parse_args(int argc, const char** argv) {
   if(argc > 2) {
      print_usage(argv);
   }

   if(argc == 2) {
      port = strtol(*(argv + 1), NULL, 10);
      if(!port) {
         fprintf(stderr, "error: invalid port number\n");
         print_usage(argv);
      }
   }

   else {
      port = 0;
   }
}

static void init() {
   server_socket = tcpServerSetup(port);
   clients = NULL;
}

static int next_readfd(fd_set* set, c_handle* head, uint16_t index) {
   uint16_t i;
   int tmp_sock;

   for(i = index; ; i++) {
      tmp_sock = getHandleByIndex(head, i);
      if(FD_ISSET(tmp_sock, set)) {
         return tmp_sock;
      }
   }
}

static void reset_readfd(fd_set* set, c_handle* head) {
   uint16_t i;
   int tmp_sock;

   FD_ZERO(set);
   FD_SET(server_socket, set);
   for(i = 0; i < num_clients; i++) {
      tmp_sock = getHandleByIndex(head, i);
      FD_SET(tmp_sock, set);
   }
}

static void send_message(int socket, rdy_packet* packet, uint16_t len) {
   write(socket, packet, len);
}

static void broadcast_message(rdy_packet* packet, uint16_t len, int exclude) {
   int i;
   int send_sock;

   for(i = 0; i < num_clients; i++) {
      send_sock = getHandleByIndex(clients, i);
      if(send_sock != exclude) {
         send_message(send_sock, packet, len);
      }
   }   
}

static void send_state(int err_socket, int state, char* dest_name) {
   raw_packet* raw = create_raw_packet();
   rdy_packet* rdy = NULL;

   switch(state) {
      case JOIN_ACK:
         raw->packet_len = OFFSET_HANDLE_LEN;
         raw->type = FLAG2;
         break;
      case ALREADY_EXISTS:
         raw->packet_len = OFFSET_HANDLE_LEN;
         raw->type = FLAG3;
         break;
      case NO_RECIPIENT:
         raw->type = FLAG7;
         raw->dest_handle_len = strlen(dest_name);
         raw->dest_handle_name = calloc(strlen(dest_name) + 1, sizeof(char));
         memcpy(raw->dest_handle_name, dest_name, strlen(dest_name));
         raw->packet_len = OFFSET_HANDLE_NAME + strlen(dest_name);
         break;
      case EXIT_ACK:
         raw->packet_len = OFFSET_HANDLE_LEN;
         raw->type = FLAG9;
         break;
      case NUM_CLIENTS:
         raw->packet_len = OFFSET_HANDLE_LEN + sizeof(uint32_t);
         raw->type = FLAG11;
         raw->to_be_sent = num_clients;
         break;
      case LIST_CLIENTS:
         raw->type = FLAG12;
         raw->dest_handle_len = strlen(dest_name);
         raw->dest_handle_name = calloc(strlen(dest_name) + 1, sizeof(char));
         memcpy(raw->dest_handle_name, dest_name, strlen(dest_name));
         raw->packet_len = OFFSET_HANDLE_NAME + strlen(dest_name);
         break;
      default:
         break;
   }
   rdy = create_rdy_packet(raw->packet_len);
   construct_packet(rdy, raw);
   send_message(err_socket, rdy, raw->packet_len);

   destroy_raw_packet(raw);
   destroy_rdy_packet(rdy);
}

static void process_client(int client) {
   int status;
   raw_packet* rec_raw = create_raw_packet();
   rdy_packet* rec_rdy = create_rdy_packet(MAX_PACKET_LEN);
   rdy_packet* send_rdy = create_rdy_packet(MAX_PACKET_LEN);   
   int recipient = 0;
   int tmp;
   int i;
   char* name;

   status = read(client, rec_rdy, MAX_PACKET_LEN);
   if(status < 0) {
      perror("read call");
   }
   else if(!status) {
      shutdown(client, SHUT_RDWR);
      removeHandle(&clients, client);
      num_clients--;
   }
   else {
      status = deconstruct_packet(rec_raw, rec_rdy);
      if(status) {
         fprintf(stderr, "error: could not read incoming packet\n");
         exit(-12);
      }
      else {
         deconstruct_packet(rec_raw, rec_rdy);
         switch (rec_raw->type) {
            case FLAG4:
               tmp = getHandleSocket(clients, rec_raw->src_handle_name);
               broadcast_message(rec_rdy, rec_raw->packet_len, tmp);
               break;
            case FLAG5:
               recipient = getHandleSocket(clients, rec_raw->dest_handle_name);
               if(-1 != recipient) {
                  send_message(recipient, rec_rdy, rec_raw->packet_len);
               }
               else {
                  send_state(client, NO_RECIPIENT, rec_raw->dest_handle_name);
               }
               break;
            case FLAG8:
                  send_state(client, EXIT_ACK, NULL);
               break;
            case FLAG10:
                  send_state(client, NUM_CLIENTS, NULL);
                  for(i = 0; i < num_clients; i++) {
                     tmp = getHandleByIndex(clients, i);
                     name = getHandleName(clients, tmp);
                     send_state(client, LIST_CLIENTS, name);
                  }
               break;
            default:
               break;
         }
      }
   }
   destroy_rdy_packet(rec_rdy);
   destroy_raw_packet(rec_raw);
   destroy_rdy_packet(send_rdy);
}

static void accept_client(c_handle** clients) {
   int status;
   raw_packet* rec_raw = create_raw_packet();
   rdy_packet* rec_rdy = create_rdy_packet(MAX_PACKET_LEN);
   raw_packet* send_raw = create_raw_packet();
   rdy_packet* send_rdy = create_rdy_packet(MAX_PACKET_LEN);   
   int client_socket;

   client_socket = tcpServerAccept(server_socket);
   status = read(client_socket, rec_rdy, MAX_PACKET_LEN);
   if(status < 0) {
      perror("read call");
   }
   status = deconstruct_packet(rec_raw, rec_rdy);
   if(status) {
      fprintf(stderr, "error: could not read incoming packet\n");
   }
   else {
      status = addHandle(clients, client_socket, rec_raw->src_handle_name);
      if(-1 == status) {
         send_state(client_socket, ALREADY_EXISTS, NULL);
      }
      else if(!status) {
         num_clients++;
         send_state(client_socket, JOIN_ACK, NULL);
      }
   }

   destroy_rdy_packet(rec_rdy);
   destroy_raw_packet(rec_raw);
   destroy_rdy_packet(send_rdy);
   destroy_raw_packet(send_raw);
}

static void test_receive() {
   int status = 0;
   int i;
   int client_sock;
   fd_set readfd;

   while(1) {
      reset_readfd(&readfd, clients);
      status = select(FD_SETSIZE, &readfd, NULL, NULL, NULL);
      if(!status) {
         fprintf(stderr, "select returned prematurely\n");
      }
      else {
         if(FD_ISSET(server_socket, &readfd)) {
            accept_client(&clients);
            status--;
         }
         for(i = 0; i < status; i++) {
            client_sock = next_readfd(&readfd, clients, i);
            process_client(client_sock);
         }
      }
   }
}

int main(int argc, const char** argv) {
   parse_args(argc, argv);
   init();

   test_receive();
   
   return 0;
}
