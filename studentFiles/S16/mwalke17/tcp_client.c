/******************************************************************************
 * tcp_client.c
 *
 * CPE 464 - Program 1
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
#include <ctype.h>
#include "testing.h"
#include "networks.h"

int main(int argc, char* argv[]) {
   int socket_num;
   fd_set select_fds;

   if(argc!= 4) {
      printf("usage: %s [ handle ] [ host-name ] [ port-number ] \n", argv[0]);
      exit(1);
   }
   socket_num = connect_and_fd_setup(argv, &select_fds);
   chat(socket_num, &select_fds, argv[1]);

   return 0;   
}

/********* CONNET AND FD SETUP *********/
int connect_and_fd_setup(char** argv, fd_set* set) {
   int socket_num;

   socket_num = tcp_send_setup(argv[2], argv[3]);

   if (request_handle(socket_num, argv[1]) < 0) {
      fprintf(stderr, "Handle already in use: %s\n", argv[1]);
      exit(1);
   }
   FD_ZERO(set);
   FD_SET(socket_num, set);
   FD_SET(STD_IN, set);

   return socket_num;
}

/********* CHAT *********/
int chat(int socket_num, fd_set* select_fds, char* handle) {
   fd_set temp_fds;
   int list_count;
   struct packet_t packet;
   init_packet(&packet);
   char *message_buf = calloc(BUF_SIZE, sizeof(char));
   char *recv_buf = calloc(BUF_SIZE, sizeof(char));
   char *stream = calloc(BUF_SIZE, sizeof(char));

   while (1) {
      temp_fds = *select_fds;
      printf("$:");
      fflush(stdout);

      if (select(socket_num + 1, &temp_fds, NULL, NULL, NULL) < 0) {
	 perror ("select");
	 exit (EXIT_FAILURE);
      }

      if (FD_ISSET(STD_IN, &temp_fds))
	 get_input(&message_buf, &stream, handle, socket_num);
      else {
	 recv_pack(socket_num, &recv_buf);
	 parse_flag(recv_buf, &packet);

	 switch (packet.flag) {
	    case FLAG_MESSAGE:
	       parse_message(recv_buf, &packet);
	       print_message(&packet); 
	       break;
	    case FLAG_BROADCAST:
	       parse_broadcast(recv_buf, &packet);
	       print_message(&packet); 
	       break;
	    case FLAG_HANDLE_LIST_RESPONSE_ONE:
	       list_count = parse_list_response_one(recv_buf);
	       parse_list_response_two(socket_num, list_count);
	       break;
	    case FLAG_EXIT_RESPONSE:
	       free_packet(&packet);
	       exit(0);
	    case FLAG_BAD_CLIENT:
	       free_packet(&packet);
	       parse_message_failure(&packet); 
	 }
      }
      memset(message_buf, 0, BUF_SIZE);
      memset(recv_buf, 0, BUF_SIZE);
      memset(stream, 0, BUF_SIZE);
   }
   free(message_buf);
   free(recv_buf);
   free(stream);

   return 0;
}

/********* GET INPUT *********/
void get_input(char** buf, char** stream, char* handle, int socket_num) {
   int send_len, ret = -1;
   
   do {
      send_len = 0;

      while (((*buf)[send_len++] = getchar()) != '\n') { 
	 if (send_len >= BUF_SIZE - 1) { 
	    if ((ret = parse_buffer(socket_num, buf, handle, stream)) < 0) {
	       printf("$:");
	       fflush(stdout);
	    }
	    else {
	       send_pack(socket_num, *stream);
	       memset(buf, 0, sizeof(BUF_SIZE));
	       send_len = 0;
	    }
	 }
      }

      if ((ret = parse_buffer(socket_num, buf, handle, stream)) < 0) {
	 printf("$:");
	 fflush(stdout);
      }
      send_pack(socket_num, *stream);
      memset(*buf, 0, sizeof(BUF_SIZE));
      
   } while (ret == -1);
}

/********* REQUEST HANDLE *********/
int request_handle(int socket, char* handle) {
   char *buf, flag;

   buf = build_handle_request(handle);
   send_pack(socket, buf);
   memset(buf, 0, STARTING_COUNT + strlen(handle));
   recv_pack(socket, &buf);
   flag = buf[2];
   free(buf);

   if (flag == FLAG_HANDLE_SUCCESS)
      return 0;
   else if (flag == FLAG_HANDLE_FAILURE)	
      return -1;
   else
      fprintf(stderr, "ACK flag unknown\n");

   return -1; 
}

/********* BUILD HANDLE REQUEST *********/
char *build_handle_request(char* handle) {
   char *temp_buf, flag = FLAG_HANDLE_REQUEST, h_len = strlen(handle),
    *buf = calloc(NORMAL_HEADER_SIZE + strlen(handle) + 1, sizeof(char));
   short length = STARTING_COUNT + strlen(handle);

   temp_buf = buf;
   memcpy(temp_buf, &length, sizeof(short));
   temp_buf += 2;
   memcpy(temp_buf++, &flag, sizeof(char));   
   memcpy(temp_buf++, &h_len, sizeof(char));
   memcpy(temp_buf, handle, strlen(handle));  
   
   return buf;
}

/********* PARSE FLAG *********/
void parse_flag(char* buf, struct packet_t* packet) {
   memcpy(&(packet->total_len), buf, sizeof(unsigned short));
   buf += FIELD_LENGTH;
   memcpy(&(packet->flag), buf, sizeof(char));
}

/********* PARSE MESSAGE *********/
void parse_message(char *buf, struct packet_t* packet) {
   buf += FIELD_LENGTH;
   memcpy(&(packet->flag), buf++, sizeof(char));
   memcpy(&(packet->dest_len), buf++, sizeof(char));
   memcpy(packet->dest_handle, buf, packet->dest_len);
   buf += packet->dest_len;
   memcpy(&(packet->src_len), buf++, sizeof(char));
   memcpy(packet->src_handle, buf, packet->src_len);
   buf += packet->src_len;
   packet->data_len = packet->total_len - NORMAL_HEADER_SIZE - packet->dest_len - 1 -
    packet->src_len - 1;

   memcpy(packet->data, buf, packet->data_len);
}

/********* PARSE BROADCAST *********/
void parse_broadcast(char *buf, struct packet_t* packet) {
   buf += FIELD_LENGTH;
   memcpy(&(packet->flag), buf++, sizeof(char));
   memcpy(&(packet->src_len), buf++, sizeof(char));
   memcpy(packet->src_handle, buf, packet->src_len);
   buf += packet->src_len;
   packet->data_len = packet->total_len - NORMAL_HEADER_SIZE - packet->src_len - 1;

   memcpy(packet->data, buf, packet->data_len);
}

/********* PARSE MESSAGE FAILURE *********/
void parse_message_failure(struct packet_t* packet) {
   char *temp, *buf = malloc(NORMAL_HEADER_SIZE + 1 + packet->src_len);

   temp = buf;
   temp += FIELD_LENGTH;
   memcpy(&(packet->flag), temp++, sizeof(char));
   memcpy(&(packet->src_len), temp++, sizeof(char));
   memcpy(packet->src_handle, temp, packet->src_len);

   fprintf(stderr, "Client with handle %s does not exsist.\n", packet->src_handle);
}

/********* PARSE LIST RESPONSE ONE *********/
int parse_list_response_one(char* buf) {
   int list_length;

   buf += NORMAL_HEADER_SIZE;
   memcpy(&list_length, buf, sizeof(int));
   list_length = ntohl(list_length);

   return list_length;
}

/********* PARSE LIST RESPONSE TWO *********/
void parse_list_response_two(int socket_num, int list_length) {
   char temp_len, *buf = calloc(HANDLE_SIZE, sizeof(char));
   int message_len;
   
   if ((message_len = recv(socket_num, buf, NORMAL_HEADER_SIZE, 0)) < 0) {
      perror("C: recv1 call");
      exit(-1);
   }
   else if (!message_len)
      close_socket(socket_num);

   printf("\nNumber of clients: %d", list_length);

   while (list_length--) {
      if ((message_len = recv(socket_num, buf, 1, 0)) < 0) {
	 perror("C: recv2 call");
	 exit(-1);
      }
      else if (!message_len)
	 close_socket(socket_num);

      memcpy(&temp_len, buf, sizeof(char));

      if ((message_len = recv(socket_num, buf, temp_len, 0)) < 0) {
	 perror("C: recv2 call");
	 exit(-1);
      }
      else if (!message_len)
	 close_socket(socket_num);

      printf("\n%s", buf);
      memset(buf, 0, HANDLE_SIZE);
   }
   printf("\n");
   free(buf);
}

/********* PARSE BUFFER *********/
int parse_buffer(int socket_num, char** buffer, char* src, char** stream) {
   char delim_space[2] = " ", delim_null[2] = "\0", delim_return[2] = "\n";;
   char *token, *dest, *message, flag = 0;

   token = strtok(*buffer, delim_space);

   if(token[0] == '%') {
      switch (toupper(token[1])) { 
	 case 'M':
	    flag = FLAG_MESSAGE;
	    dest = strtok(NULL, delim_space);
	    message = strtok(NULL, delim_null);

	    if (message == NULL) 
	       message = delim_return;

	    build_message_packet(src, dest, flag, message, stream); 
	    break;
	 case 'B':
	    flag = FLAG_BROADCAST;
	    message = strtok(NULL, delim_null);

	    if (message == NULL) 
	       message = delim_return;

	    build_broadcast_packet(src, flag, message, stream); 
	    break;
	 case 'L':
	    flag = FLAG_HANDLE_LIST_REQUEST;
	    build_list_packet(flag, stream); 
	    break;
	 case 'E':
	    flag = FLAG_EXIT_REQUEST;
	    build_exit_packet(flag, stream); 
	    break;
	 default : 
	    return -1;
      }
   }
   else
      return -1;

   return 0;
}

/********* BUILD MESSAGE PACKET ********/
void build_message_packet(char* src_handle, char* dest_handle, char flag,
 char* data, char** packet) {
   char *temp, dest_h_len, src_h_len;
   unsigned short total_length;

   dest_h_len = strlen((char *)dest_handle);
   src_h_len = strlen((char *)src_handle);
   total_length = 2 + sizeof(flag) + 1 + strlen(dest_handle) + 1 +
    strlen(src_handle) + strlen(data) + 1;         
   temp = *packet;
   memcpy(temp, &total_length, sizeof(unsigned short));
   temp += 2;
   memcpy(temp++, &flag, sizeof(char));
   memcpy(temp++, &dest_h_len, sizeof(char));
   memcpy(temp, dest_handle, strlen((char *)dest_handle));
   temp += strlen((char *)dest_handle);
   memcpy(temp++, &src_h_len, sizeof(char));
   memcpy(temp, src_handle, strlen((char *)src_handle));
   temp += strlen((char *)src_handle);
   memcpy(temp, data, strlen((char *)data) + 1);
}

/********* BUILD BROADCAST PACKET ********/
void build_broadcast_packet(char* src_handle, char flag, char* data,
 char** packet) {
   char *temp, src_h_len;
   unsigned short total_length;

   src_h_len = strlen((char *)src_handle);
   total_length = 2 + sizeof(flag) + 1 + strlen((char *)src_handle) +
    strlen((char *)data) + 1;  
   temp = *packet;
   memcpy(temp, &total_length, sizeof(unsigned short));
   temp += 2;
   memcpy(temp++, &flag, sizeof(char));
   memcpy(temp++, &src_h_len, sizeof(char));
   memcpy(temp, src_handle, strlen((char *)src_handle));
   temp += strlen((char *)src_handle);
   memcpy(temp, data, strlen((char *)data) + 1);
}

/********* BUILD LIST PACKET ********/
void build_list_packet(char flag, char** packet) {
   char *temp;
   unsigned short total_length;

   total_length = 2 + sizeof(flag) + 1;         
   temp = *packet;
   memcpy(temp, &total_length, sizeof(unsigned short));
   temp += 2;
   memcpy(temp++, &flag, sizeof(char));
}

/********* BUILD EXIT PACKET ********/
void build_exit_packet(char flag, char** packet) {
   char *temp;
   unsigned short total_length;

   total_length = 2 + sizeof(flag) + 1;         
   temp = *packet;
   memcpy(temp, &total_length, sizeof(unsigned short));
   temp += 2;
   memcpy(temp++, &flag, sizeof(char));
}

/********* SEND ********/
void send_pack(int socket_num, char* buf) {
   int sent;
   short length, temp;

   memcpy(&length, buf, sizeof(short));
   temp = htons(length);
   memcpy(buf, &temp, sizeof(short));

   if ((sent = send(socket_num, buf, length, 0)) < 0) {
      perror("send call");
      exit(-1);
   }
}

/********* RECV *********/
int recv_pack(int socket_num, char** recv_buf) {
   int message_len;
   short packet_length, temp;

   if ((message_len = recv(socket_num, (char *) &packet_length,
    FIELD_LENGTH, MSG_PEEK)) < 0) {
      perror("recv1 call");
      exit(-1);
   }
   else if (!message_len) 
      close(socket_num);
   else {
      temp = ntohs(packet_length);
      if ((message_len = recv(socket_num, *recv_buf,
       temp, 0)) < 0) {
	 perror("C: recv2 call");
	 exit(-1);
      }
      else if (!message_len)
	 close(socket_num);

      memcpy(*recv_buf, &temp, sizeof(short));
   }

   return 0;
}

/********* INIT PACKET *********/
void init_packet(struct packet_t* packet) {
   packet->total_len = 0;
   packet->flag = 0;
   packet->dest_len = 0;
   packet->dest_handle = calloc(HANDLE_SIZE, sizeof(char));
   packet->src_len = 0;
   packet->src_handle = calloc(HANDLE_SIZE, sizeof(char));
   packet->data_len = 0;
   packet->data = calloc(MESSAGE_SIZE, sizeof(char));
}

/********* FREE PACKET *********/
void free_packet(struct packet_t* packet) {
   free(packet->dest_handle);
   free(packet->src_handle);
   free(packet->data);
}

/********* REFRESH PACKET *********/
void refresh_packet(struct packet_t* packet) {
   packet->total_len = 0;
   packet->flag = 0;
   packet->dest_len = 0;
   memset(packet->dest_handle, 0, HANDLE_SIZE);
   packet->src_len = 0;
   memset(packet->src_handle, 0, HANDLE_SIZE);
   packet->data_len = 0;
   memset(packet->data, 0, MESSAGE_SIZE);
}

/********* PRINT STUCT *********/
void print_struct(struct packet_t* packet) {
   printf("total_len: %d\n", packet->total_len);
   printf("flag: %d\n", packet->flag);
   printf("dest_len: %d\n", packet->dest_len);
   printf("dest_handle: %s\n", packet->dest_handle);
   printf("src_len: %d\n", packet->src_len);
   printf("src_handle: %s\n", packet->src_handle);
   printf("data_len: %d\n", packet->data_len);
   printf("data: %s\n", packet->data);
}

/********* PRINT BUFFER *********/
void print_buffer(char* buf, int len) {
   int idx;

   for (idx = 0; idx < len; idx++) {
      printf(" %d", buf[idx]);
   }

   puts("");
}

/********* PRINT DATA **********/
void print_data(char* buf, int len) {
   int idx;

   for (idx = 0; idx < len; idx++) {
      printf(" %c", buf[idx]);
   }

   puts("");
}

/********* PRINT MESSAGE **********/
void print_message(struct packet_t* packet) {
   printf("\n%s: %s", packet->src_handle, packet->data);
}

/********* CLOSE SOCKET **********/
void close_socket(int socket_num) {
   close(socket_num);
   exit(-1);
}

/********* TCP SEND SETUP *********/
int tcp_send_setup(char *host_name, char *port) {
   int socket_num;
   struct sockaddr_in remote;
   struct hostent *hp;

   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket call");
      exit(-1);
   }
   remote.sin_family = AF_INET;

   if ((hp = gethostbyname(host_name)) == NULL) {
      printf("Error getting hostname: %s\n", host_name);
      exit(-1);
   }
   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);
   remote.sin_port= htons(atoi(port));

   if(connect(socket_num, (struct sockaddr*)&remote,
    sizeof(struct sockaddr_in)) < 0) {
      perror("connect call");
      exit(-1);
   }

   return socket_num;
}
