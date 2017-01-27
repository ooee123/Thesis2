/******************************************************************************
* cclient.cpp
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
#include <limits.h>
#include <iostream>
using namespace std;

#include "networks.h"
#include "chat_structs.h"

#define hdr_len sizeof(chat_hdr)

static int socket_num = 0;
static int max_socket;
static string handle;
static fd_set all_fds;
static fd_set read_fds;

void clientConnect(char *argv[]) {
   chat_hdr send_hdr, recv_hdr;
   char *send_buf = new char[hdr_len + strlen(argv[1]) + 1]();

   socket_num = tcpClientSetup(argv[2], argv[3]);

   send_hdr.len = htons(hdr_len + strlen(argv[1]) + 1);
   send_hdr.flag = 1;
   memcpy(send_buf, &send_hdr, hdr_len);
   send_buf[hdr_len] = (char)strlen(argv[1]);
   memcpy(send_buf + hdr_len + 1, argv[1], strlen(argv[1]));
   send_pkt(socket_num, send_buf, ntohs(send_hdr.len));
   recv_pkt(socket_num, &recv_hdr, hdr_len);
   if (recv_hdr.flag == 3) {
      cerr << "Handle already in use: " << argv[1] << endl;
      delete[] send_buf;
      exit(1);
   }
   max_socket = socket_num + 1;
   FD_SET(socket_num, &all_fds);
   handle = string(argv[1]);
   delete[] send_buf;
}

void sendMessage() {
   string message, dest_handle;
   char *send_buf, *temp_buf;
   int pkt_len;
   chat_hdr send_hdr;
   send_hdr.flag = 5;
   cin >> dest_handle;
   getline(cin, message);
   message.erase(0, 1);
   while (message.length() > 1000) {
      string sub_msg = message.substr(0, 1000);
      string temp_msg = message.substr(1000);
      message = temp_msg;
      pkt_len = hdr_len + 1000 + handle.length() + dest_handle.length() + 2;
      send_buf = new char[pkt_len]();
      temp_buf = send_buf;
      send_hdr.len = htons(pkt_len);
      memcpy(temp_buf, &send_hdr, hdr_len);
      temp_buf += hdr_len;
      temp_buf[0] = (char) dest_handle.length();
      memcpy(temp_buf + 1, dest_handle.c_str(), dest_handle.length());
      temp_buf += (dest_handle.length() + 1);
      temp_buf[0] = (char) handle.length();
      memcpy(temp_buf + 1, handle.c_str(), handle.length());
      temp_buf += (handle.length() + 1);
      memcpy(temp_buf, sub_msg.c_str(), 1000);
      send_pkt(socket_num, send_buf, pkt_len);
      delete[] send_buf;
   }
   if (message.length() <= 1000) {
      pkt_len = hdr_len + message.length() + handle.length() +
         dest_handle.length() + 2;
      send_buf = new char[pkt_len]();
      temp_buf = send_buf;
      send_hdr.len = htons(pkt_len);
      memcpy(temp_buf, &send_hdr, hdr_len);
      temp_buf += hdr_len;
      temp_buf[0] = (char) dest_handle.length();
      memcpy(temp_buf + 1, dest_handle.c_str(), dest_handle.length());
      temp_buf += (dest_handle.length() + 1);
      temp_buf[0] = (char) handle.length();
      memcpy(temp_buf + 1, handle.c_str(), handle.length());
      temp_buf += (handle.length() + 1);
      memcpy(temp_buf, message.c_str(), message.length());
      send_pkt(socket_num, send_buf, pkt_len);
      delete[] send_buf;
   }
}

void sendBroadcast() {
   string message;
   char *send_buf;
   int pkt_len;
   chat_hdr send_hdr;
   send_hdr.flag = 4;
   getline(cin, message);
   message.erase(0, 1);
   while (message.length() > 1000) {
      string sub_msg = message.substr(0, 1000);
      string temp_msg = message.substr(1000);
      message = temp_msg;
      pkt_len = hdr_len + 1000 + handle.length() + 1;
      send_buf = new char[pkt_len]();
      send_hdr.len = htons(pkt_len);
      memcpy(send_buf, &send_hdr, hdr_len);
      send_buf[hdr_len] = (char) handle.length();
      memcpy(send_buf + hdr_len + 1, handle.c_str(), handle.length());
      memcpy(send_buf + pkt_len - 1000, sub_msg.c_str(), 1000);
      send_pkt(socket_num, send_buf, pkt_len);
      delete[] send_buf;
   }
   if (message.length() <= 1000) {
      pkt_len = hdr_len + message.length() + handle.length() + 1;
      send_buf = new char[pkt_len]();
      send_hdr.len = htons(pkt_len);
      memcpy(send_buf, &send_hdr, hdr_len);
      send_buf[hdr_len] = (char) handle.length();
      memcpy(send_buf + hdr_len + 1, handle.c_str(), handle.length());
      memcpy(send_buf + pkt_len - message.length(), message.c_str(),
         message.length());
      send_pkt(socket_num, send_buf, pkt_len);
      delete[] send_buf;
   }
}

void listHandles() {
   chat_hdr send_hdr;
   send_hdr.len = htons(hdr_len);
   send_hdr.flag = 10;
   send_pkt(socket_num, &send_hdr, hdr_len);
}

void requestExit() {
   chat_hdr send_hdr;
   send_hdr.len = htons(hdr_len);
   send_hdr.flag = 8;
   send_pkt(socket_num, &send_hdr, hdr_len);
}

void sendServer() {
   string op;
   if (cin >> op) {
      if (!op.compare("\%m") || !op.compare("\%M")) sendMessage();
      else if (!op.compare("\%b") || !op.compare("\%B")) sendBroadcast();
      else if (!op.compare("\%l") || !op.compare("\%L")) listHandles();
      else if (!op.compare("\%e") || !op.compare("\%E")) requestExit();
      else {
         cerr << "Invalid command: " << op << endl;
         cin.ignore(INT_MAX, '\n');
      }
   }
}

void printBroadcast(chat_hdr recv_hdr) {
   int pkt_len = recv_hdr.len - hdr_len;
   char *recv_buf = new char[pkt_len]();
   char *temp_buf = recv_buf;
   recv_pkt(socket_num, recv_buf, pkt_len);
   string src_handle = string(temp_buf + 1, temp_buf[0]);
   temp_buf += src_handle.length() + 1;
   pkt_len -= (src_handle.length() + 1);
   string message = string(temp_buf, pkt_len);
   cout << "\n" << src_handle << ": " << message << endl;
   delete[] recv_buf;
}

void printMsg(chat_hdr recv_hdr) {
   int pkt_len = recv_hdr.len - hdr_len;
   char *recv_buf = new char[pkt_len]();
   char *temp_buf = recv_buf;
   recv_pkt(socket_num, recv_buf, pkt_len);
   string dest_handle = string(temp_buf + 1, temp_buf[0]);
   temp_buf += dest_handle.length() + 1;
   pkt_len -= (dest_handle.length() + 1);
   string src_handle = string(temp_buf + 1, temp_buf[0]);
   temp_buf += src_handle.length() + 1;
   pkt_len -= (src_handle.length() + 1);
   string message = string(temp_buf, pkt_len);
   cout << "\n" << src_handle << ": " << message << endl;
   delete[] recv_buf;
}

void printMsgErr(chat_hdr recv_hdr) {
   int pkt_len = recv_hdr.len - hdr_len;
   char *recv_buf = new char[pkt_len]();
   recv_pkt(socket_num, recv_buf, pkt_len);
   string client_handle = string(recv_buf + 1, recv_buf[0]);
   cerr << "\nClient with handle " << client_handle << " does not exist." << endl;
   delete[] recv_buf;
}

void exitClient() {
   close(socket_num);
   exit(0);
}

void printHandle() {
   char handle_len;
   recv_pkt(socket_num, &handle_len, 1);
   char *recv_buf = new char[handle_len]();
   recv_pkt(socket_num, recv_buf, handle_len);
   string client_handle = string(recv_buf, handle_len);
   cout << "   " << client_handle << endl;
   delete[] recv_buf;
}

void handleNums() {
   int num_handles;
   recv_pkt(socket_num, &num_handles, sizeof(int));
   num_handles = ntohl(num_handles);
   cout << "\nNumber of clients: " << num_handles << endl;
   for (int i = 0; i < num_handles; i++) {
      chat_hdr temp;
      recv_pkt(socket_num, &temp, hdr_len);
      printHandle();
   }
}

void readServer() {
   chat_hdr recv_hdr;

   if (!recv_pkt(socket_num, &recv_hdr, hdr_len)) exitClient();
   recv_hdr.len = ntohs(recv_hdr.len);
   switch(recv_hdr.flag) {
      case 4: printBroadcast(recv_hdr); break;
      case 5: printMsg(recv_hdr); break;
      case 7: printMsgErr(recv_hdr); break;
      case 9: exitClient(); break;
      case 11: handleNums(); break;
   }
}

void selectSockets() {
   read_fds = all_fds;
   if (select(max_socket, &read_fds, NULL, NULL, NULL) < 0) {
      perror("select call");
      exit(-1);
   }
   if (FD_ISSET(STDIN_FILENO, &read_fds)) {
      sendServer();
   }
   if (FD_ISSET(socket_num, &read_fds)) {
      readServer();
   }
}

int main(int argc, char *argv[]) {
   if (argc != 4) {
      printf("usage: %s handle host-name port-number \n", argv[0]);
      exit(1);
   }

   FD_SET(STDIN_FILENO, &all_fds);

   clientConnect(argv);

   while (true) {
      cout << "$: " << flush;
      selectSockets();
   }
   
   return 0;
}
