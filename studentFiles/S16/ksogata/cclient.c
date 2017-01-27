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

void create_init_packet(char *buffer, header h, init i) {
   uint16_t len_h = htons(h.len);

   memcpy(buffer + 0, &len_h, 2);
   memcpy(buffer + 2, &h.flag, 1);
   memcpy(buffer + 3, &i.handle_len, 1);
   memcpy(buffer + 4, i.handle, i.handle_len);
}

int confirm_connection(int socket_num, char *handle) {
   int total_len = 0;
   int flag = 1;
   int handle_len = 0;
   header h;
   init i;
   char *buffer;
   char *buf;
   int buf_size = 10;

   handle_len = strlen(handle);
   if (handle_len > 255) {
      printf("Handle cannot exceed 255 characters.\n");
      exit(-1);
   }
   total_len = handle_len + 4;
   
   buffer = (char *)malloc(total_len);

   h.len = total_len;
   h.flag = flag;
   
   i.handle_len = handle_len;
   i.handle = handle;

   create_init_packet(buffer, h, i);

   send(socket_num, buffer, total_len, 0);

   buf = (char *)malloc(buf_size);

   recv(socket_num, buf, buf_size, 0);

   return buf[2]; 
}

void exit_protocol(int socket_num) {
   char *buffer;
   int buf_size = 3;
   int buf_size_h = htons(buf_size);
   int flag = 8;

   buffer = (char *)malloc(buf_size);

   memcpy(buffer + 0, &buf_size_h, 2);
   memcpy(buffer + 2, &flag, 1);

   send(socket_num, buffer, buf_size, 0);

   memset(buffer, 0, buf_size);

   recv(socket_num, buffer, buf_size, 0);

   if (buffer[2] == 9) {
      exit(1);
   }
}

void get_handles(int socket_num) {
   int i;
   int buf_size = 255;
   char *buffer;

   printf("\n");
   printf("Number of clients: %d\n", clients_num);

   buffer = (char *)malloc(buf_size);

   memcpy(buffer, "yes", 3);
   send(socket_num, buffer, 3, 0);

   for (i = 0; i < clients_num; i++) {
      recv(socket_num, buffer, buf_size, 0);
      printf("%s\n", buffer + 1);
   }
}

void handle_packet(char *buffer, int socket_num) {
   int flag = buffer[2];
   int dest_len;
   char *src;
   int src_len;
   int str_pos;

   if (flag == 5) {
      dest_len = buffer[3];
      
      src_len = buffer[4 + dest_len];

      src = (char *)malloc(src_len + 1);
      memcpy(src, buffer + dest_len + 5, src_len);
      src[src_len] = '\0';

      str_pos = dest_len + src_len + 5;

      printf("\n");
      printf("%s: ", src);
      printf("%s\n", buffer + str_pos);
   } else if (flag == 4) {
      src_len = buffer[3];

      src = (char *)malloc(src_len + 1);
      memcpy(src, buffer + 4, src_len);

      src[src_len] = '\0';
      str_pos = src_len + 4;
      
      printf("\n");
      printf("%s: ", src);
      printf("%s\n", buffer + str_pos);
   } else if (flag == 7) {
      printf("Error. Destination handle %s does not exist\n", buffer + 3);
   } else if (flag == 11) {
      memcpy(&clients_num, buffer + 3, 4);
      clients_num = ntohl(clients_num);
   } else if (flag == 12) {
      get_handles(socket_num);
   }
}

void build_select_list(fd_set *socks, int sock) {
   FD_CLR(sock, socks);
   FD_SET(0, socks); //STDIN
   FD_SET(sock, socks);
}

void create_broad_packet(char *buffer, header h, msg_broad b, int send_len) {
   int len_h = htons(h.len);

   memset(buffer, 0, h.len);

   memcpy(buffer + 0, &len_h, 2);
   memcpy(buffer + 2, &h.flag, 1);
   memcpy(buffer + 3, &b.from_len, 1);
   memcpy(buffer + 4, b.handle_from, b.from_len);
   memcpy(buffer + 4 + b.from_len, b.message, send_len);  
}

void broadcast_msg(int sock, char *handle, char *scan) {
   char *tokens[1024];
   int bufsize;
   char *p;
   char *msg_buffer;
   char *ptr;
   int n = 0;
   int i;
   uint8_t from_len;
   int total_len;
   int send_len;
   int send_lim = 1000;
   char *sub;
   char *buffer;
   char *strings[10];
   header h;
   msg_broad b;

   for (p = strtok(scan, " "); p; p = strtok(NULL, " ")) {
      tokens[n++] = p;
   }

   bufsize = 1024;

   msg_buffer = (char *)malloc(bufsize);
   ptr = msg_buffer;

   for (i = 1; i < n; i++) {
      memcpy(ptr, tokens[i], strlen(tokens[i]));
      if (i != n-1) {
         memcpy(ptr + strlen(tokens[i]), " ", 1);
      }
      ptr = ptr + strlen(tokens[i]) + 1;
   }

   from_len = strlen(handle);
   send_len = strlen(msg_buffer);
   msg_buffer[send_len - 1] = '\0';

   if (send_len > send_lim) {
      sub = (char *)malloc(send_lim);
      ptr = msg_buffer;
      n = 0;
      for (i = 0; i < 10; i++) {
         strings[i] = (char *)malloc(send_lim);
      }
      while (strlen(ptr) > send_lim) {
         strncpy(sub, ptr, send_lim);
         strncpy(strings[n], sub, strlen(sub));
         n++;
         memset(sub, 0, send_lim);
         ptr += send_lim;
      }
      strncpy(sub, ptr, send_lim);
      strncpy(strings[n], sub, strlen(sub));
      n++;
      for (i = 0; i < n; i++) {
         total_len = from_len + strlen(strings[i]) + 5;
         buffer = (char *)malloc(total_len);
         h.len = total_len;
         h.flag = 4;

         b.from_len = from_len;
         b.handle_from = handle;
         b.message = strings[i];
         create_broad_packet(buffer, h, b, strlen(strings[i]));
         if (send(sock, buffer, total_len, 0) < 0) {
            perror("send call");
            exit(-1);
         }
      }
   } else {
      total_len = from_len + send_len + 5;
      buffer = (char *)malloc(total_len);

      h.len = total_len;
      h.flag = 4;

      b.from_len = from_len;
      b.handle_from = handle;
      b.message = msg_buffer;

      //send data
      create_broad_packet(buffer, h, b, send_len);
      if (send(sock, buffer, total_len, 0) < 0) {
         perror("send call");
         exit(-1);
      }
   }
   return;
}

void get_handle_list(int socket_num) {
   header h;
   char *buffer;

   h.len = 3;
   h.flag = 10;
   
   buffer = (char *)malloc(h.len);
   memcpy(buffer, &h.len, 2);
   memcpy(buffer + 2, &h.flag, 1);

   if (send(socket_num, buffer, h.len, 0) < 0) {
      perror("send call");
      exit(-1);
   }
   return;
}

void scan_user_input(int socket_num, char *handle) {
   char *scan;
   int scan_size = 1024;
   char val;

   scan = (char *)malloc(scan_size);
   read(0, scan, scan_size);
   
   if (scan[0] == '%') {
      val = tolower(scan[1]);
      switch(val) {
         case 'e':
            exit_protocol(socket_num);
            break;
         case 'm':
            send_msg(socket_num, handle, scan);
            break;
         case 'b':
            broadcast_msg(socket_num, handle, scan);
            break;
         case 'l':
            get_handle_list(socket_num);
            break;
         default:
            printf("Invalid command.\n");
            break;
      }
   } else {
      printf("Invalid command.\n");
   }
}

void read_packet(int socket_num) {
   int msg_len = 0;
   char *buf;
   int buf_size = 1024;

   buf = (char *)malloc(buf_size);
   if ((msg_len = recv(socket_num, buf, buf_size, 0)) < 0) {
      perror("recv call");
      exit(-1);
   }
   if (msg_len == 0) {
      printf("Server Terminated.\n");
      exit(-1);
   }

   handle_packet(buf, socket_num);
   
   memset(buf, 0, buf_size);
}

void read_socks(fd_set *socks, int sock, char *handle) {
   if (FD_ISSET(0, socks)) {
      scan_user_input(sock, handle);
   } else if (FD_ISSET(sock, socks)) {
      read_packet(sock);   
   }

   printf("$: ");
   fflush(stdout);
}

void start_client(int socket_num, char* handle) {
   fd_set socks;
   struct timeval timeout;
   int readsocks = 0;

   if (confirm_connection(socket_num, handle) != 2) {
      printf("Handle already in use: <%s>\n", handle);
      exit(-1);
   }

   printf("$: ");
   fflush(stdout);

   while(1) {
      build_select_list(&socks, socket_num);
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      readsocks = select(socket_num + 1, &socks, NULL, NULL, &timeout);

      if (readsocks < 0) {
         perror("select");
         exit(-1);
      }

      if (readsocks != 0) {
         read_socks(&socks, socket_num, handle);
      }
   }
}

int tcp_send_setup(char *host_name, char *port)
{
    int socket_num;
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host

    // create the socket
    if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    perror("socket call");
	    exit(-1);
    }
    
    // designate the addressing family
    remote.sin_family= AF_INET;

    // get the address of the remote host and store
   if ((hp = gethostbyname(host_name)) == NULL) {
	   printf("Error getting hostname: %s\n", host_name);
	   exit(-1);
   }
    
    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

    // get the port used on the remote side and store
    remote.sin_port= htons(atoi(port));

    if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
    {
	perror("connect call");
	exit(-1);
    }

    return socket_num;
}

void create_msg_packet(char *buffer, header h, msg_data m, int msg_len) {
   uint16_t total_len_h = htons(h.len);
   memset(buffer, 0, h.len);

   memcpy(buffer + 0, &total_len_h, 2);
   memcpy(buffer + 2, &h.flag, 1);
   memcpy(buffer + 3, &m.to_len, 1);
   memcpy(buffer + 4, m.handle_to, m.to_len);
   memcpy(buffer + 4 + m.to_len, &m.from_len, 1);
   memcpy(buffer + 5 + m.to_len, m.handle_from, m.from_len);
   memcpy(buffer + 5 + m.to_len + m.from_len, m.message, msg_len);   
}

void send_msg(int socket_num, char* handle_from, char *scan) {
   char *send_buf;
   char *msg_buffer;
   char *ptr;
   int bufsize = 0;
   int send_len = 0;
   int sent = 0;
   char *handle_to;
   int name_len = 0;
   char *buffer;
   uint16_t total_len = 0;
   char *tokens[1024];
   int n = 0; 
   char *p;
   int i;
   int send_lim = 1000;
   header h;
   msg_data m;
   char *strings[10];
   char *sub;

   for (p = strtok(scan, " "); p; p = strtok(NULL, " ")) {
      tokens[n++] = p;
   }

   //initialize data buffer for packet
   bufsize = 1024;
   send_buf = (char *)malloc(bufsize);
   handle_to = (char *)malloc(255);

   handle_to = tokens[1];
   name_len = strlen(handle_to);

   if (n < 3) {
      name_len -= 1;
      handle_to[name_len] = '\0';
      tokens[2] = "\n";
   }

   msg_buffer = (char *)malloc(bufsize);
   ptr = msg_buffer;

   for (i = 2; i < n; i++) {
      memcpy(ptr, tokens[i], strlen(tokens[i]));
      if (i != n-1) {
         memcpy(ptr + strlen(tokens[i]), " ", 1);
      }
      ptr = ptr + strlen(tokens[i]) + 1;
   }

   send_len = strlen(msg_buffer);
   msg_buffer[send_len - 1] = '\0';
   uint8_t from_len = strlen(handle_from);

   if (send_len > send_lim) {
      sub = (char *)malloc(send_lim);
      ptr = msg_buffer;
      n = 0;
      for (i = 0; i < 10; i++) {
         strings[i] = (char *)malloc(send_lim);
      }
      while (strlen(ptr) > send_lim) {
         strncpy(sub, ptr, send_lim);
         strncpy(strings[n], sub, strlen(sub));
         n++;
         memset(sub, 0, send_lim);
         ptr += send_lim;
      }
      strncpy(sub, ptr, send_lim);
      strncpy(strings[n], sub, strlen(sub));
      n++;
      for (i = 0; i < n; i++) {
         total_len = from_len + name_len + strlen(strings[i]) + 5;
         buffer = (char *)malloc(total_len);
         h.len = total_len;
         h.flag = 5;
         m.to_len = name_len;
         m.handle_to = handle_to;
         m.from_len = from_len;
         m.handle_from = handle_from;
         m.message = strings[i];
         create_msg_packet(buffer, h, m, strlen(strings[i]));
         if (send(socket_num, buffer, total_len, 0) < 0) {
            perror("send call");
            exit(-1);
         }
      }
   } else {
      total_len = from_len + name_len + send_len + 5;
      buffer = (char *)malloc(total_len);
      h.len = total_len;
      h.flag = 5;

      m.to_len = name_len;
      m.handle_to = handle_to;
      m.from_len = from_len;
      m.handle_from = handle_from;
      m.message = msg_buffer;

      //now send the data
      create_msg_packet(buffer, h, m, send_len);
      sent = send(socket_num, buffer, total_len, 0);
      if (sent < 0) {
         perror("send call");
         exit(-1);
      }
   }
   return;
}

int main(int argc, char* argv[]) {
   int socket_num;
   char *handle;

   if (argc != 4) {
      printf("usage: %s host-name port-number handle-name\n", argv[0]);
      exit(1);
   }

   socket_num = tcp_send_setup(argv[2], argv[3]);
   handle = argv[1];

   start_client(socket_num, handle);

   close(socket_num);
   
   return 0;
}
