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

#pragma pack(1)

#define MAXBUF 1000
#define HANDLE_MAX 255 

int socket_num; 
char local_handle_name[HANDLE_MAX]; 
uint8_t local_handle_len; 

uint32_t total_clients; 


/* Function sets up the client-server connection. Sends 
 * initial packet to the server. Populate the value of 
 * socket number. 
 */ 
void init_client(char *host_name, char *port) {

	socket_num = 0; 
	char *data, *send_buf, *recv_buf; 

	//Check handle name length
	if (strlen(local_handle_name) >= HANDLE_MAX) {
		fprintf(stderr, "Invalid Command\n"); 
		exit(-1); 
	}

	/* set up the TCP Client socket  */
	socket_num = tcpClientSetup(host_name, port);

	//Create Flag 1 Packet (Handle Name + 1 byte for length)
	if ((data = (char *) malloc(1 + strlen(local_handle_name))) == NULL)
	{
		perror("Malloc Error When Initializing Data Packet");
		close(socket_num); 
		exit(-1);
	}

	local_handle_len = strlen(local_handle_name); 
	memcpy(data, &local_handle_len, 1); 
	memcpy(data + 1, local_handle_name, strlen(local_handle_name));

	//Create packet header 
	chat_header header_1; 
	int local_packet_len = sizeof(chat_header) + strlen(data); 
	header_1.packet_length = htons(local_packet_len); 
	header_1.flag = 1; 

	//Create Packet 
	if ((send_buf = (char *) malloc(local_packet_len)) == NULL)
	{
		perror("Malloc Error When Initializing Send Packet");
		exit(-1);
	}

	memcpy(send_buf, &header_1, 3); 
	memcpy(send_buf + 3, data, strlen(data)); 

	/* now send the initial packet */
	if(send(socket_num, send_buf, sizeof(header_1) + strlen(data), 0) < 0)
	{
		perror("Send Packet Error");
		close(socket_num); 
		exit(-1);
	}

	/* receive ack from server */ 
	recv_buf = malloc(3); 
	if (recv(socket_num, recv_buf, 3, 0) < 0)
	{
		perror("Receive Packet Error");
		exit(-1);
	}
	
	//Parse Response 
	uint8_t resp_flag = *((uint8_t *)(recv_buf+2)); 

	if (resp_flag == 3) {
		fprintf(stderr, "Handle already in use: %s\n", local_handle_name); 
		close(socket_num); 
		exit(-1); 
	}
	
	free(data); 
	free(send_buf); 
	free(recv_buf); 
}


/* Client sends a message to another/or all client(s) via the server. 
 * Function creates and sends the packet to the server. Takes in boolean
 * parameter. If set, packet flag is 4 (%B or %b) for broadcast. If clear 
 * packet flag is 5 (%M or %m) for direct message to another client. 
 */ 
void send_message(int broadcast) {
	char handle_name[HANDLE_MAX]; 
	uint8_t handle_len; 

	if (!broadcast) {
		if (!scanf("%s", handle_name)) {
			fprintf(stderr, "Invalid Command\n"); 
			close(socket_num); 
			exit(-1); 
		}
		handle_len = strlen(handle_name); 
	}
	
	//Get Text Message
	char **recv_buffers; 
	char *recv_buf; 
	int buf_num = 1, recv_len = 0, total_chars = 0; 

	if ((recv_buffers = (char **) malloc(sizeof(char*) * buf_num)) == NULL) {
		perror("Malloc Error When Initializing Send Packet Buffer");
		exit(-1);
	}
	if ((recv_buf = (char *) malloc(MAXBUF)) == NULL) {
		perror("Malloc Error When Initializing Send Packet");
		exit(-1);
	}

	*recv_buffers = recv_buf; 

	while ((recv_buf[recv_len] = getchar()) != '\n')
	{
		//Ignore iteration if the first char is a space 
		if (!(total_chars == 0 && recv_buf[recv_len] == ' ')) {
			
			recv_len++;

			if (recv_len == MAXBUF-1) {
				buf_num++; 
				recv_buf[recv_len] = '\0'; 
				if((recv_buffers = (char **) realloc(recv_buffers, sizeof(char*) * buf_num)) == NULL) {
					perror("Malloc Error When Initializing Send Packet Buffer");
					exit(-1);
				} 
				if ((recv_buf = (char *) malloc(MAXBUF)) == NULL) {
					perror("Malloc Error When Initializing Send Packet");
					exit(-1);
				}
				*(recv_buffers + (buf_num-1)) = recv_buf; 
				recv_len = 0; 
			}

			total_chars++; 
		}
	}

	if (total_chars > 0 && recv_len == 0) {
		buf_num--; 
	}


	recv_buf[total_chars] = '\0'; 


	//Create packets out of message
	int i; 
	for (i = 0; i < buf_num; i++) {
		
		//Create packet header - Flag 5 
		chat_header header_message; 

		//Add Null Character 
		int text_len = strlen(recv_buffers[i]) + 1; 

		int local_packet_len; 
		char *send_buf = NULL; 
		if (!broadcast) {

			 local_packet_len = sizeof(chat_header) +  
								1 + handle_len + 		//Length of and name of dest handle name 
							    1 + local_handle_len +	//Length of and name of send handle name  
								text_len; 
			header_message.packet_length = htons(local_packet_len); 
			header_message.flag = 5; 

			send_buf = malloc(local_packet_len); 
			memcpy(send_buf, (char *) &header_message, 3); 
			memcpy(send_buf + 3, &handle_len, 1); 
			memcpy(send_buf + 4, handle_name, handle_len); 
			memcpy(send_buf + 4 + handle_len, &local_handle_len, 1); 
			memcpy(send_buf + 5 + handle_len, local_handle_name, local_handle_len); 
			memcpy(send_buf + 5 + handle_len + local_handle_len, recv_buffers[i], text_len); 
		}
		else {
			 local_packet_len = sizeof(chat_header) +  
							    1 + local_handle_len +	//Length of and name of send handle name  
								text_len; 
			header_message.packet_length = htons(local_packet_len); 
			header_message.flag = 4; 

			send_buf = malloc(local_packet_len); 
			memcpy(send_buf, (char *) &header_message, 3); 
			memcpy(send_buf + 3, &local_handle_len, 1); 
			memcpy(send_buf + 4, local_handle_name, local_handle_len); 
			memcpy(send_buf + 4 + local_handle_len, recv_buffers[i], text_len);
		}
		
		//Send packet 
		if((send(socket_num, send_buf, local_packet_len, 0)) < 0)
		{
			perror("Send Packet Error");
			close(socket_num); 
			exit(-1);
		}
		free(send_buf); 
	}
	free(recv_buffers); 
	free(recv_buf); 
}

/* Client sends a message (%E or %e) to the server in order 
 * to exit. Function creates and sends the packet (flag 8) 
 * to that lets the server know it wants to exit. 
 */ 

void request_exit() {
	chat_header header_8; 
	header_8.packet_length = htons(3); 
	header_8.flag = 8; 

	if ((send(socket_num, &header_8, 3, 0)) < 0) {
		perror("Send Packet Error");
		close(socket_num); 
		exit(-1);
	}
}

/* Client sends a message (%L or %l) to the server in order 
 * to request list of all the other clients connected. 
 * Function creates and sends the packet (flag 10) to the server. 
 */ 

void request_list() {
	chat_header header_10; 
	header_10.packet_length = htons(3); 
	header_10.flag = 10; 

	if ((send(socket_num, &header_10, 3, 0)) < 0) {
		perror("Send Packet Error");
		close(socket_num); 
		exit(-1);
	}
}

/* Client receives a message from another client via the server. 
 * Function parses and outputs the packet conents. Takes in boolean
 * parameter. If set, packet flag is 4 (%B or %b) for broadcast. If clear 
 * packet flag is 5 (%M or %m) for direct message to another client. 
 */ 
void output_message(char *recv_buf, int broadcast) {
	
	if (!broadcast) {
		char *send_handle, *dest_handle; 
		uint8_t send_length, dest_length; 

		dest_length = *(uint8_t *)(recv_buf+3); 
		dest_handle = malloc(dest_length+1); 
		strncpy(dest_handle, recv_buf+4, dest_length); 
		dest_handle[dest_length] = '\0'; 

		send_length = *(uint8_t *)(recv_buf + 4 + dest_length); 
		send_handle = malloc(send_length+1); 
		strncpy(send_handle, recv_buf + 5 + dest_length, send_length); 
		send_handle[send_length] = '\0'; 

		printf("\n%s: %s\n", send_handle, recv_buf + 5 + dest_length + send_length); 

		free(dest_handle); 
		free(send_handle); 
	}
	else {
		char *send_handle; 
		uint8_t send_length; 

		send_length = *(uint8_t *)(recv_buf+3); 
		send_handle = malloc(send_length+1); 
		strncpy(send_handle, recv_buf+4, send_length); 
		send_handle[send_length] = '\0'; 

		printf("\n%s: %s\n", send_handle, recv_buf + 4 + send_length); 

		free(send_handle); 
	}
}

/* Output list of clients after receiving the first packet 12 */ 
void output_list(char *recv_buf, int recv_len) {

	char *cursor = recv_buf+3; 
	while (cursor < recv_buf + recv_len) {
		uint8_t handle_len = *((uint8_t *)cursor); 
		char *handle = malloc(handle_len+1); 
		strncpy(handle, cursor+1, handle_len); 
		handle[handle_len] = '\0'; 

		printf("\t%s\n", handle); 
		cursor+= handle_len;
		cursor++;  
	}

/*
	char *cursor = recv_buf; 

	while (cursor < recv_buf + recv_len) {
		uint16_t pack_len = ntohs(*((uint16_t *)cursor)); 
		uint8_t handle_len = *((uint8_t *)(cursor+3)); 
		char *handle = malloc(handle_len+1); 
		strncpy(handle, cursor+4, handle_len); 
		handle[handle_len] = '\0'; 

		printf("\t%s\n", handle); 
		cursor += pack_len; 
	}*/ 
}

/* Parse socket input. 
 */ 
void parse_sockt_input() {

	char *recv_buf; 
	int recv_len = 0; 

	recv_buf = malloc(MAXBUF+5+HANDLE_MAX*2);

	if ((recv_len = recv(socket_num, recv_buf, MAXBUF+5+HANDLE_MAX*2, 0)) < 0) {
		perror("Recieve Error\n");
		exit(-1);
	}

	if (recv_len == 0) {
		//Server has most likely closed 
		fprintf(stderr, "Server Terminated\n"); 
		exit(-1); 
	}
	recv_buf[recv_len] = '\0'; 


	uint16_t packet_length; 
	uint8_t flag; 			

	memcpy(&packet_length, recv_buf, 2); 
	memcpy(&flag, recv_buf + 2, 1); 

	switch(flag) {
    	case 5 :
    		output_message(recv_buf, 0); //%M message
    		break; 
    	case 7 : 
    		fprintf(stdout, "\nClient with handle %s does not exist.\n", recv_buf+4);
    		break; 
    	case 9 : 
    		close(socket_num); //Exit the Program
    		printf("\n"); 
    		exit(0); 
    		break; 
    	case 4 : 	
    		output_message(recv_buf, 1); //%B message
    		break; 
    	case 11 : 
    		total_clients = ntohl(*((uint32_t*)(recv_buf+3)));
    		printf("Number of clients: %u\n", total_clients);  
    		break; 
    	case 12 : 
    		output_list(recv_buf, recv_len); 
    		break; 
 
      	default :
         	printf("Invalid flag\n" );
    }
}

/* Parse Input */ 
void parse_std_input() {
	char cmd[2]; 

	if (!scanf("%s", cmd)) {
		fprintf(stderr, "Invalid Command\n"); 
		close(socket_num); 
		exit(-1); 
	}

	if (!strcmp("%M", cmd) || !strcmp("%m", cmd)) {
		send_message(0); 
	}
	else if (!strcmp("%E", cmd) || !strcmp("%e", cmd)) {
		request_exit(); 
	}
	else if (!strcmp("%L", cmd) || !strcmp("%l", cmd)) {
		request_list(); 
	}
		else if (!strcmp("%B", cmd) || !strcmp("%b", cmd)) {
		send_message(1); 
	}
	else {
		fprintf(stderr, "Invalid Command\n"); 
		close(socket_num); 
		exit(-1); 
	}
}

void checkArgs(int argc, char *argv[]) {
	if (argc != 4)
	{
		printf("Usage: %s handle-name host-name port-number \n", argv[0]);
		exit(-1);
	}

	if (strlen(argv[1]) >= HANDLE_MAX) {
		fprintf(stderr, "Invalid Command\n");
		exit(-1); 
	}


	strcpy(local_handle_name, argv[1]); 
	local_handle_len = strlen(local_handle_name); 
}

void select_code(fd_set *socket_list) {
	FD_ZERO(socket_list); 
	FD_SET(socket_num, socket_list); 
	FD_SET(STDIN_FILENO, socket_list); 

	select(socket_num+1, socket_list , (fd_set *) 0, (fd_set *) 0 , NULL); 

	if (FD_ISSET(STDIN_FILENO, socket_list)) {
		parse_std_input(); 
	}

	else if (FD_ISSET(socket_num, socket_list)) {
		parse_sockt_input(); 
	}

	total_clients = -1; 
}

int main(int argc, char * argv[])
{
	fd_set socket_list;	


	checkArgs(argc, argv); 
	init_client(argv[2], argv[3]); 

	while (1) {

		printf("$: "); 
		fflush(stdout); 

		select_code(&socket_list); 
		
		
	}
	return 0;
}


