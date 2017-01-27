/******************************************************************************
* server.cpp
*
*****************************************************************************/

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
#include <sys/select.h>
#include <map>
#include <string>
#include <iostream>
using namespace std;

#include "networks.h"
#include "chat_structs.h"

#define hdr_len sizeof(chat_hdr)

static int server_socket = 0;
static int max_socket = 0;
static map<string,int> clients;
static fd_set all_fds;
static fd_set read_fds;

void clientSetup() {
   chat_hdr send_hdr, recv_hdr;

   int client_socket = tcpAccept(server_socket);

   recv_pkt(client_socket, &recv_hdr, hdr_len);
   recv_hdr.len = ntohs(recv_hdr.len);
   
   if (recv_hdr.flag == 1) {
      char *recv_buf = new char[recv_hdr.len - hdr_len]();
      recv_pkt(client_socket, recv_buf, recv_hdr.len - hdr_len);
      string handle = string(recv_buf + 1, recv_buf[0]);
      send_hdr.len = htons(hdr_len);
      if (clients.count(handle)) {
         send_hdr.flag = 3;
         send_pkt(client_socket, &send_hdr, hdr_len);
         delete[] recv_buf;
         return;
      }
      send_hdr.flag = 2;
      send_pkt(client_socket, &send_hdr, hdr_len);
      clients[handle] = client_socket;
      if (max_socket < client_socket + 1) max_socket = client_socket + 1;
      FD_SET(client_socket, &all_fds);
      delete[] recv_buf;
   }
}

void broadcast(int client_socket, chat_hdr send_hdr) {
   int pkt_len = ntohs(send_hdr.len);
   char *send_buf = new char[pkt_len]();

   memcpy(send_buf, &send_hdr, hdr_len);
   recv_pkt(client_socket, send_buf + hdr_len, pkt_len - hdr_len);
   map<string,int>::iterator it;
   for (it = clients.begin(); it != clients.end(); it++) {
      if (it->second != client_socket) {
         send_pkt(it->second, send_buf, pkt_len);
      }
   }
   delete[] send_buf;
}

void message(int client_socket, chat_hdr send_hdr) {
   int pkt_len = ntohs(send_hdr.len);
   char *send_buf = new char[pkt_len]();

   memcpy(send_buf, &send_hdr, hdr_len);
   recv_pkt(client_socket, send_buf + hdr_len, pkt_len - hdr_len);
   string dest_handle = string(send_buf + hdr_len + 1, send_buf[hdr_len]);
   if (!clients.count(dest_handle)) {
      delete[] send_buf;
      pkt_len = hdr_len + 1 + dest_handle.length();
      send_buf = new char[pkt_len];
      send_hdr.len = htons(pkt_len);
      send_hdr.flag = 7;
      memcpy(send_buf, &send_hdr, hdr_len);
      send_buf[hdr_len] = (char) dest_handle.length();
      memcpy(send_buf + hdr_len + 1, dest_handle.c_str(), dest_handle.length());
      send_pkt(client_socket, send_buf, pkt_len);
      delete[] send_buf;
      return;
   }
   send_pkt(clients[dest_handle], send_buf, pkt_len);
   delete[] send_buf;
}

void clientCleanup(int client_socket) {
   close(client_socket);
   FD_CLR(client_socket, &all_fds);
   map<string,int>::iterator it;
   for (it = clients.begin(); it != clients.end() &&
      it->second != client_socket; it++);
   clients.erase(it);
}

void clientExit(int client_socket) {
   chat_hdr send_hdr;

   send_hdr.len = htons(hdr_len);
   send_hdr.flag = 9;
   send_pkt(client_socket, &send_hdr, hdr_len);
   clientCleanup(client_socket);
}

void listHandles(int client_socket) {
   chat_hdr send_hdr;
   char *send_buf = new char[hdr_len + sizeof(int)];

   send_hdr.len = htons(hdr_len + sizeof(int));
   send_hdr.flag = 11;
   int num_hdrs = htonl(clients.size());
   memcpy(send_buf, &send_hdr, hdr_len);
   memcpy(send_buf + hdr_len, &num_hdrs, sizeof(int));
   send_pkt(client_socket, send_buf, hdr_len + sizeof(int));
   delete[] send_buf;

   map<string,int>::iterator it;
   for (it = clients.begin(); it != clients.end(); it++) {
      int pkt_len = hdr_len + it->first.length() + 1;
      send_buf = new char[pkt_len];
      send_hdr.len = 0;
      send_hdr.flag = 12;
      memcpy(send_buf, &send_hdr, hdr_len);
      send_buf[hdr_len] = (char) it->first.length();
      memcpy(send_buf + hdr_len + 1, it->first.c_str(), pkt_len - hdr_len);
      send_pkt(client_socket, send_buf, pkt_len);
      delete[] send_buf;
   }
}

int readClient(int client_socket) {
   chat_hdr recv_hdr;
   if (!recv_pkt(client_socket, &recv_hdr, hdr_len)) {
      clientCleanup(client_socket);
      return 1;
   }
   switch(recv_hdr.flag) {
      case 4: broadcast(client_socket, recv_hdr); break;
      case 5: message(client_socket, recv_hdr); break;
      case 8: clientExit(client_socket); break;
      case 10: listHandles(client_socket); break;
   }
   return 0;
}

void selectSockets() {
   read_fds = all_fds;
   if (select(max_socket, &read_fds, NULL, NULL, NULL) < 0) {
      perror("select call");
      exit(-1);
   }
   if (FD_ISSET(server_socket, &read_fds)) {
      clientSetup();
   }
   map<string,int>::iterator it;
   for (it = clients.begin(); it != clients.end(); it++) {
      if (FD_ISSET(it->second, &read_fds)) {
         if (readClient(it->second))
            break;
      }
   }
}

int main(int argc, char *argv[]) {
   int serv_port = 0;
   
   if (argc == 2) serv_port = atoi(argv[1]);
   server_socket = tcpServerSetup(serv_port);
   max_socket = server_socket + 1;
   FD_SET(server_socket, &all_fds);

   while (true) {
      selectSockets();
   }
   
   close(server_socket);
   
   return 0;
}
