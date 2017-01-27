/******************************************************************************
 * tcp_server.c
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
#include "testing.h"
#include "networks.h"

int main(int argc, char *argv[]) {
   int server_socket;
   fd_set client_fds;

   server_socket = connect_and_fd_setup(NULL, &client_fds); 
   serve(server_socket, &client_fds);

   return 0;
}

void serve(int server_socket, fd_set* client_fds) {
   int client_socket, idx, code;
   char *buf = calloc(BUFFER_MAX_SIZE, sizeof(char)); 
   fd_set temp_fds;
   struct packet_t packet;
   struct handles_t h_list;

   init_packet(&packet);
   init_handle_list(&h_list, server_socket);

   while (1) {
      temp_fds = *client_fds;

      if (select(h_list.largest_fd + 1, &temp_fds, NULL, NULL, NULL) < 0) {
	 perror ("select");
	 exit (EXIT_FAILURE);
      }

      for (idx = 0; idx < h_list.largest_fd + 1; idx++) {
	 if (FD_ISSET(idx, &temp_fds)) {
	    if (idx == server_socket) {
	       if ((client_socket = accept(server_socket, (struct sockaddr*) 0,
	        (socklen_t *)0)) < 0) {
		  perror("accept call");
		  exit(-1);
	       }

	       if (client_socket > h_list.largest_fd) 
		  h_list.largest_fd = client_socket;

	       recv_pack(client_socket, &buf);
	       parse_flag(buf, &packet);

	       if (packet.flag == FLAG_HANDLE_REQUEST) {
		  parse_handle(buf, &packet);
		  code = add_client(client_socket, &h_list, packet.dest_handle);
		  send_pack(client_socket, build_handle_response(code));
		  FD_SET(client_socket, client_fds);
	       }
	    }
	    else {
	       if (recv_pack(idx, &buf) < 0) { 
		  remove_client(&h_list, idx);
		  FD_CLR(idx, client_fds);
	       }
	       else {
		  parse_flag(buf, &packet);
		  handle_flag(&packet, &h_list, buf, idx);
	       }
	       refresh_packet(&packet);
	    }
	 }
	 memset(buf, 0, BUFFER_MAX_SIZE);
      }
   }
   free_packet(&packet);
   free(buf);
}

/********* CONNECT AND FD SETUP *********/
int connect_and_fd_setup(char** argv,fd_set* set) {
   int server_socket;

   server_socket = tcp_recv_setup();

   if (listen(server_socket, 5) < 0) {
      perror("listen call");
      exit(-1);
   }
   FD_ZERO(set);
   FD_SET(server_socket, set);

   return server_socket;
}

/********* HANDLE FLAG *********/
void handle_flag(struct packet_t* packet, struct handles_t* h_list, char* buf,
 int curr_client) {
   int dest_fd, src_fd, ndx;

   switch (packet->flag ) {
      case FLAG_MESSAGE:
	 parse_message(buf, packet);

	 if ((dest_fd = find_client(h_list, packet->dest_handle)) < 0) {
	    send_message_failure(curr_client, packet);
	    break;
	 }
	 send_pack(dest_fd, buf);
	 break;
      case FLAG_BROADCAST:
	 parse_broadcast(buf, packet);

	 if ((src_fd = find_client(h_list, packet->src_handle)) < 0) {
	    send_message_failure(curr_client, packet); 
	    break;
	 }

	 for (ndx = STARTING_COUNT; ndx < h_list->count; ndx++) {
	    if (h_list->list[ndx] != NULL && ndx != src_fd)
	       send_pack(ndx, buf);
	 }
	 break;
      case FLAG_HANDLE_LIST_REQUEST:
	 build_list_response(curr_client, h_list);
	 break;
      case FLAG_EXIT_REQUEST:
	 send_pack(curr_client, build_exit_response());
	 break;
   }
}

/********* PARSE FLAG **********/
void parse_flag(char* buf, struct packet_t* packet) {
   memcpy(&(packet->total_len), buf, sizeof(short));
   buf += FIELD_LENGTH;
   memcpy(&(packet->flag), buf, sizeof(char));
}

/********* PARSE HANDLE **********/
void parse_handle(char *buf, struct packet_t* packet) {
   buf += NORMAL_HEADER_SIZE;
   memcpy(&(packet->dest_len), buf++, sizeof(char));
   memcpy(packet->dest_handle, buf, packet->dest_len);
}

/********* PARSE MESSAGE **********/
void parse_message(char *buf, struct packet_t* packet) {
   buf += NORMAL_HEADER_SIZE;
   memcpy(&(packet->dest_len), buf++, sizeof(char));
   memcpy(packet->dest_handle, buf, packet->dest_len);
   buf += packet->dest_len;
   memcpy(&(packet->src_len), buf++, sizeof(char));
   memcpy(packet->src_handle, buf, packet->src_len);
   buf += packet->src_len;
   packet->data_len = packet->total_len - NORMAL_HEADER_SIZE -
    packet->dest_len - 1 - packet->src_len - 1;
   memcpy(packet->data, buf, packet->data_len);
}

/********* PARSE BROADCAST **********/
void parse_broadcast(char *buf, struct packet_t* packet) {
   buf += NORMAL_HEADER_SIZE;
   memcpy(&(packet->src_len), buf++, sizeof(char));
   memcpy(packet->src_handle, buf, packet->src_len);
   buf += packet->src_len;
   packet->data_len = packet->total_len - NORMAL_HEADER_SIZE -
    packet->src_len - 1;
   memcpy(packet->data, buf, packet->data_len);
}

/********* SEND MESSAGE FAILURE *********/
void send_message_failure(int socket_num, struct packet_t* packet) {
   short length = NORMAL_HEADER_SIZE + packet->dest_len +
    strlen(packet->dest_handle);
   char *temp_buf, flag = FLAG_BAD_CLIENT, *buf = malloc(length);

   temp_buf = buf;
   memcpy(temp_buf, &length, sizeof(short));
   temp_buf += 2;
   memcpy(temp_buf++, &flag, sizeof(char));
   memcpy(temp_buf++, &packet->dest_len, sizeof(char));
   temp_buf += strlen(packet->dest_handle);
   memcpy(temp_buf, packet->dest_handle, strlen(packet->dest_handle));
   send_pack(socket_num, buf);
   free(buf);
}

/********* BUILD HANDLE RESPONSE **********/
char *build_handle_response(int code) {
   char *temp_buf, flag_success = FLAG_SUCCESS, *buf = malloc(sizeof(char)
    * NORMAL_HEADER_SIZE), flag_failure = FLAG_FAILURE;
   short length = NORMAL_HEADER_SIZE;

   temp_buf = buf;
   memcpy(temp_buf, &length, sizeof(short));
   temp_buf += 2;
 
   if (!code)
      memcpy(temp_buf, &flag_success, sizeof(char));
   else
      memcpy(temp_buf, &flag_failure, sizeof(char));

   return buf;
}

/********* BUILD LIST RESPONSE **********/
void build_list_response(int socket_num, struct handles_t* h_list) {
   short length = LIST_RESPONSE_LENGTH;
   char *temp_buf, flag = FLAG_HANDLE_LIST_RESPONSE_ONE, temp_len,
    *buf = malloc(sizeof(char) * length);
   unsigned int idx, list_length = 0, count = 0, sent;

   temp_buf = buf;
   memcpy(temp_buf, &length, sizeof(short));
   temp_buf += 2;
   memcpy(temp_buf++, &flag, sizeof(char));

   for (idx = STARTING_COUNT; idx < h_list->largest_fd + 1; idx++) {
	 list_length = list_length + strlen(h_list->list[idx]) + 1;
	 count++;
   }  

   count = htonl(count);
   memcpy(temp_buf, &count, sizeof(unsigned int));
   send_pack(socket_num, buf);
   buf = realloc(buf, sizeof(char) * (list_length + NORMAL_HEADER_SIZE));
   length = 0;
   temp_buf = buf;
   memcpy(temp_buf, &length, sizeof(short));
   temp_buf += 2;
   memcpy(temp_buf++, &flag, sizeof(char));

   for (idx = STARTING_COUNT; idx < h_list->largest_fd + 1; idx++) {
      temp_len = strlen(h_list->list[idx]);
      memcpy(temp_buf++, &temp_len, sizeof(char));
      memcpy(temp_buf, h_list->list[idx], temp_len);
      temp_buf += temp_len;
   }  

   if ((sent = send(socket_num, buf, list_length + NORMAL_HEADER_SIZE, 0))
    < 0) {
      perror("send call");
      exit(-1);
   }
   free(buf);
}

/********* BUILD EXIT RESPOSNE **********/
char *build_exit_response() {
   char *temp_buf, flag_exit = FLAG_EXIT_RESPONSE,
    *buf = malloc(sizeof(char) * NORMAL_HEADER_SIZE);
   short length = NORMAL_HEADER_SIZE;

   temp_buf = buf;
   memcpy(temp_buf, &length, sizeof(short));
   temp_buf += 2;
   memcpy(temp_buf, &flag_exit, sizeof(char));

   return buf;
}

/********* ADD CLIENT **********/
int add_client(int client_socket, struct handles_t* h_list, char* handle) {
   if (client_socket - STARTING_COUNT >= h_list->max) {
      h_list = realloc(h_list->list, h_list->max * 2);
      h_list->max *= 2;
   }

   if (!check_for_handle(h_list, handle)) {
      h_list->list[client_socket] = calloc(HANDLE_SIZE, sizeof(char));
      memcpy(h_list->list[client_socket], handle, strlen(handle));
      h_list->count++;
   }
   else 
      return -1;

   return 0;
}

/********* CHECK HANDLE **********/
int check_for_handle(struct handles_t* h_list, char* handle) {
   int idx;

   for (idx = 4; idx < h_list->largest_fd; idx++) {
	 if (!strcmp(handle, h_list->list[idx]))
	    return -1;
   }
   
   return 0;
}

/********* FIND CLIENT **********/
int find_client(struct handles_t* h_list, char* handle) {
   int idx;

   for (idx = 4; idx <= h_list->largest_fd; idx++) {
      if (!strcmp(h_list->list[idx], handle))
	 return idx; 
   }

   return -1;
}

/********* REMOVE CLIENT LIST **********/
void remove_client(struct handles_t* h_list, int fd) {
   memset(h_list->list[fd], 0, HANDLE_SIZE);
   h_list->count--;
}

/********* PRINT CLIENT LIST **********/
void print_client_list(struct handles_t* h_list) {
   int idx;

   for (idx = 0; idx < h_list->count; idx++) 
      printf("Handle %d: %s\n", idx, h_list->list[idx]);
}

/********* INIT PACKET **********/
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

/********* INIT HANDLES **********/
void init_handle_list(struct handles_t* h_list, int socket_num) {
   h_list->count = STARTING_COUNT;
   h_list->max = INITIAL_LIST_SIZE;
   h_list->list = calloc(h_list->max, sizeof(char *));
   h_list->largest_fd = socket_num;
}

/********* FREE PACKET **********/
void free_packet(struct packet_t* packet) {
   free(packet->dest_handle);
   free(packet->src_handle);
   free(packet->data);
}

/********* REFRESH PACKET **********/
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

/********* PRINT SRUCT **********/
void print_struct(struct packet_t* packet) {
   int idx;

   printf("total_len: %d\n", packet->total_len);
   printf("flag: %d\n", packet->flag);
   printf("dest_len: %d\n", packet->dest_len);
   printf("dest_handle:");

   for (idx = 0; idx < packet->dest_len; idx++) {
      printf(" %X", packet->dest_handle[idx]);
   }

   printf("\nsrc_len: %d\n", packet->src_len);
   printf("src_handle:");

   for (idx = 0; idx < packet->src_len; idx++) {
      printf(" %X", packet->src_handle[idx]);
   }

   printf("\ndata_len: %d\n", packet->data_len);
   printf("data:");

   for (idx = 0; idx < packet->data_len; idx++) {
      printf(" %X", packet->data[idx]);
   }
   
   printf("\n");
}

/********* PRINT BUFFER **********/
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

/********* SEND *********/
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

/********* RECV  **********/
int recv_pack(int socket_num, char** recv_buf){
   int message_len;
   short packet_length, temp;

   if ((message_len = recv(socket_num, (char *) &packet_length,
    FIELD_LENGTH, MSG_PEEK)) < 0) {
      perror("recv1 call1");
      exit(-1);
   }
   else if (!message_len) {
      close(socket_num);
      return -1;
   }      
   else {
      temp = ntohs(packet_length);
      if ((message_len = recv(socket_num, *recv_buf,
       temp, 0)) < 0) {
	 perror("recv2 call");
	 close(socket_num);
	 exit(-1);
      }
      else if (!message_len) {
	 close(socket_num);
	 return -1;
      }      
      memcpy(*recv_buf, &temp, sizeof(short));
   }

   return 0;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */

int tcp_recv_setup() {
   int server_socket= 0;
   struct sockaddr_in local;
   socklen_t len= sizeof(local);

   server_socket = socket(AF_INET, SOCK_STREAM, 0);
   if(server_socket < 0) {
      perror("socket call");
      exit(1);
   }

   local.sin_family = AF_INET;
   local.sin_addr.s_addr = INADDR_ANY;
   local.sin_port= htons(0);

   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
      exit(-1);
   }

   if (getsockname(server_socket, (struct sockaddr*) &local, &len) < 0) {
      perror("getsockname call");
      exit(-1);
   }

   printf("socket has port %d \n", ntohs(local.sin_port));

   return server_socket;
}
