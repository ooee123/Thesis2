/******************************************************************************
 * tcp_client.c
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

#include "networks.h"
#include "testing.h"

#define MAX_PACKET_SIZE 1515
#define BUFFER_SIZE 1000
#define COMMAND_SIZE 3
#define HANDLE_SIZE 256

static uint32_t num_handles = 0; 

int send_data (char *send_buf, int socket_num, char type, char *src_handle, char *dest_handle) {
	int send_len = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent
	int more_data = 0;
	uint16_t packet_len = 0;
	uint8_t *packet;
	uint8_t *cur_loc;
	char first_char;

	if (type == 'm'|| type == 'b') {
		// Check for leading zeros
		if ((first_char = getchar()) != ' ') {
			ungetc(first_char, stdin);
		}

		// Retreive message
	   send_len = 0;
	   while ((send_buf[send_len] = getchar()) != '\n' && send_len < BUFFER_SIZE) {
		   send_len++;
		   if (send_len == BUFFER_SIZE) {
		   	more_data = 1;
		   	ungetc(send_buf[send_len], stdin);
		   }
	   }

	   if (!send_len) {
	   	send_buf[send_len++] = ' ';
	   }
	   send_buf[send_len] = '\0';
	}

   // Create packet to send
   switch (type) {
   	case 'm':
		   packet_len = 5 + strlen(src_handle) + strlen(dest_handle) + send_len;
		   packet = malloc(packet_len);
		   packet_len = htons(packet_len);
   		cur_loc = packet;
		   memcpy(cur_loc, &packet_len, 2);
		   memset(cur_loc += 2, 5, 1);
		   memset(++cur_loc, strlen(dest_handle), 1);
		   memcpy(++cur_loc, dest_handle, strlen(dest_handle));
		   memset(cur_loc += strlen(dest_handle), strlen(src_handle), 1);
		   memcpy(++cur_loc, src_handle, strlen(src_handle));
		   memcpy(cur_loc += strlen(src_handle), send_buf, send_len);
			break;
		case 'b':
		   packet_len = 4 + strlen(src_handle) + send_len;
		   packet = malloc(packet_len);
		   packet_len = htons(packet_len);
		   memcpy(packet, &packet_len, 2);
		   memset(packet + 2, 4, 1);
		   memset(packet + 3, strlen(src_handle), 1);
		   memcpy(packet + 4, src_handle, strlen(src_handle));
		   memcpy(packet + 4 + strlen(src_handle), send_buf, send_len);
			break;
		case 'e':
		   packet_len = 3;
		   packet = malloc(packet_len);
		   packet_len = htons(packet_len);
		   memcpy(packet, &packet_len, 2);
		   memset(packet + 2, 8, 1);
			break;
		case 'l':
		   packet_len = 3;
		   packet = malloc(packet_len);
		   packet_len = htons(packet_len);
		   memcpy(packet, &packet_len, 2);
		   memset(packet + 2, 10, 1);
			break;
		default:
			break;
	}

   /* now send the data */
   sent = send(socket_num, packet, ntohs(packet_len), 0);
   free(packet);

   if (sent < 0) {
 		perror("send call");
	 	exit(-1);
   }

   return more_data;
}

void process_message (char *header, int socket_num) {
	int flag = header[2];
	int i = 0;
	int message_len = 0;
	int buffer_size = ntohs(*(uint16_t *)header) - 2;
	uint8_t *buf = malloc(buffer_size + 1);
	message_len = recv(socket_num, buf, buffer_size, 0);
	buf[message_len] = '\0';

	switch (flag) {
		// Receiving a broadcast
		case 4:
			printf("\n");
			for (i = 0; i < buf[0]; i ++) {
		      printf("%c", *(buf + 1 + i));
		   }
			printf(": ");

			printf("%s\n", (buf + 1 + buf[0]));

			break;
		// Receive a direct message
		case 5:
			printf("\n");
			for (i = 0; i < buf[1 + buf[0]]; i ++) {
		      printf("%c", *(buf + 2 + buf[0] + i));
		   }
			printf(": ");

			printf("%s\n", buf + 2 + buf[0] + buf[1 + buf[0]]);

			break;
		// Client does not exist
		case 7:
			printf("Client with handle ");
			for (i = 0; i < buf[0]; i ++) {
		      printf("%c", *(buf + 1 + i));
		   }
			printf(" does not exist\n");
			break;
		default:
			break;
	}
   printf("$: ");
   fflush(stdout);
}

void receive_list (char *buf, int socket_num) {
	uint8_t *length = malloc(1);
	uint8_t *packet;
	int i = 0;
	int j = 0;

	printf("Number of clients: %d\n", num_handles);

	for (i = 0; i < num_handles; i++) {
		recv(socket_num, length, 1, 0);
		packet = malloc(*length);

		recv(socket_num, packet, *length, 0);

		printf("\t");
		for (j = 0; j < *length; j++) {
			putchar(packet[j]);
		}
		printf("\n");
		free(packet);
	}	

	free(length);
	printf("$: ");
	fflush(stdout);
}


void loop (char *send_buf, int socket_num, char *src_handle) {
   char command[COMMAND_SIZE];
   char dest_handle[HANDLE_SIZE];
   
   static struct timeval timeout;
   fd_set readfds;
   timeout.tv_sec = 1;
   timeout.tv_usec = 0;

	char *buf;              //buffer for receiving from client
	buf = (char *) malloc(MAX_PACKET_SIZE);
	int message_len= 0;     //length of the received message

   printf("$: ");
   fflush(stdout);
   for (;;) {
	   FD_ZERO(&readfds);
	   FD_SET(fileno(stdin), &readfds);
	   FD_SET(socket_num, &readfds);

     	if (select(socket_num + 1, (fd_set *) &readfds, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
        	perror("select call");
        	exit(-1);
     	}

     	if (FD_ISSET(socket_num, &readfds)) {
     		message_len = recv(socket_num, buf, 3, 0);

		  	if (message_len == -1) {
		   	perror("recv call");
		   	exit(-1);
		  	} else if (message_len == 0) {
		  		printf("Server Terminated\n");
		  		return;
		  	} else if (message_len > 0) {
		      if (buf[2] == 12) {
		      	receive_list(buf, socket_num);
		      } else if (buf[2] == 11) {
		      	recv(socket_num, buf, 4, 0);
		      	num_handles = *(uint32_t *)buf;
		      } else if (buf[2] == 9) {
		      	return;
		      } else if (buf[2] != 0) {
					process_message(buf, socket_num);
				}
		   }

     	} else if (FD_ISSET(fileno(stdin), &readfds)) {
		   scanf("%s", command);
		   getchar();
		   if (!strcmp(command, "%m") || !strcmp(command, "%M")) {
		      scanf("%s", dest_handle);
		      while (send_data(send_buf, socket_num, 'm', src_handle, dest_handle)) {
		      	//Continue sending data until entire message is sent
		      }
		   } else if (!strcmp(command, "%b") || !strcmp(command, "%B")) {
		      while (send_data(send_buf, socket_num, 'b', src_handle, dest_handle)) {
		      	//Continue sending data until entire message is sent
		      }
		   } else if (!strcmp(command, "%l") || !strcmp(command, "%L")) {
		      while (send_data(send_buf, socket_num, 'l', src_handle, dest_handle)) {
		      	//Continue sending data until entire message is sent
		      }
		   } else if (!strcmp(command, "%e") || !strcmp(command, "%E")) {
		      while (send_data(send_buf, socket_num, 'e', src_handle, dest_handle)) {
		      	//Continue sending data until entire message is sent
		      }
		   } else {
		   	printf("Invalid command\n");
		   }
		   printf("$: ");
	      fflush(stdout);
		}
	}
}

void send_handle (char *handle, int socket_num) {
	uint16_t packet_len = 0;
	uint8_t *packet;
	uint8_t *cur_loc;
	int sent = 0;

	packet_len = 4 + strlen(handle);
	packet = malloc(packet_len);
	packet_len = htons(packet_len);
	cur_loc = packet;
	memcpy(cur_loc, &packet_len, 2);
	memset(cur_loc += 2, 1, 1);
	memset(++cur_loc, strlen(handle), 1);
	memcpy(++cur_loc, handle, strlen(handle));

	sent = send(socket_num, packet, ntohs(packet_len), 0);

   if (sent < 0) {
 		perror("send call");
	 	exit(-1);
   }

   packet_len = recv(socket_num, packet, 3, 0);

   if (packet_len < 0) {
   	perror("recvfrom");
   	exit(-1);
   }

   if (packet[2] != 2) {
   	printf("Handle already in use: %s\n", handle);
   	exit(-1);
   }

   free(packet);
}

int main (int argc, char * argv[]) {
   int socket_num;         //socket descriptor
   char *send_buf;         //data buffer

   /* check command line arguments  */
   if(argc!= 4) {
   	printf("usage: %s handle server-name server-port \n", argv[0]);
	  	exit(1);
   }

   if (strlen(argv[1]) > HANDLE_SIZE - 1) {
   	printf("Handle too long (%d). Max handle length: 255.\n", strlen(argv[1]));
   	exit(1);
   }

   /* set up the socket for TCP transmission  */
   socket_num= tcp_send_setup(argv[2], argv[3]);

   /* initialize data buffer for the packet */
   send_buf= (char *) malloc(BUFFER_SIZE);

   send_handle(argv[1], socket_num);
   loop(send_buf, socket_num, argv[1]);

   free(send_buf);
   close(socket_num);
   return 0;
    
}

int tcp_send_setup (char *host_name, char *port) {
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

   if (connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0) {
		perror("connect call");
		exit(-1);
   }

   return socket_num;
}

