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

#include "networks.h"

#pragma pack(1)

#define MAXBUF 1000
#define HANDLE_MAX 255 

//Linked List 
typedef struct chat_client {
	char *handle_name; 
	int socket_num; 
	struct chat_client *next; 
} chat_client; 

chat_client *client_head, *client_tail;  
int total_clients; 
int new_client; 

/* Sets up new client by adding it to the linked list */ 
void parse_packet1(int socket_num, char *recv_buf) {
	
	chat_header response; 
	int err = 0; 

	//Check if client name exists already
	chat_client *cursor = client_head; 
	while (cursor != NULL) {
		if (!strcmp(recv_buf+1, cursor->handle_name)) {
			//Name already exists 
			err = 1; 
			break; 
		}
		cursor = cursor->next;
	}

	if (err == 0) {

		//Create NewClient 
		uint8_t handle_size = *(uint8_t *)recv_buf;

		if (total_clients == 0) {

			client_head = malloc(sizeof(chat_client)); 

			client_head->handle_name = malloc(handle_size+1); 
			strncpy(client_head->handle_name, recv_buf + 1, handle_size); 
			client_head->handle_name[handle_size] = '\0'; 
			client_head->socket_num = socket_num; 

			client_head->next = NULL; 
			client_tail = client_head; 

		}
		else {

			client_tail->next = malloc(sizeof(chat_client)); 

			client_tail->next->handle_name = malloc(handle_size+1);
			strncpy(client_tail->next->handle_name, recv_buf + 1, handle_size); 
			client_tail->next->handle_name[handle_size] = '\0'; 
			client_tail->next->socket_num = socket_num; 

			client_tail = client_tail->next; 
			client_tail->next = NULL; 

		}
		
		total_clients++; 

		response.packet_length = htons(3); 
		response.flag = 2; 
	}
	else {
		response.packet_length = htons(3); 
		response.flag = 3; 
	}

	//Send response 
	err =  send(socket_num, (char *) &response, 3, 0);
	if(err < 0)
	{
		perror("Response to Initialization Packet Send Error");
		exit(-1);
	}

	new_client = -1; 


}

/* Sends a message from one client to another. */ 
void parse_packet5(int socket_num, char *recv_buf) {
	int send_socket = 0, dest_socket = -1; 
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

	/*Check if client name exists already + 
	 *Grab the socket of the original sender in case we need 
	 *send error response back.
	 */ 
	chat_client *cursor = client_head; 
	while (cursor != NULL) {
		if (!strcmp(dest_handle, cursor->handle_name)) {
			dest_socket = cursor->socket_num; 
		}
		if (!strcmp(send_handle, cursor->handle_name)) {
			send_socket = cursor->socket_num; 
		}
		cursor = cursor->next;
	}

	if (dest_socket == -1) {
		//Error: Client with handle doesn't exist 
		//Send back to original send_socket
		chat_header errorResponse;
		errorResponse.packet_length = htons(3+1+dest_length); 
		errorResponse.flag = 7; 

		char *send_buf = malloc(3+1+dest_length); 
		memcpy(send_buf, &errorResponse, 3); 
		memcpy(send_buf + 3, &dest_length, 1); 
		memcpy(send_buf + 4, dest_handle, dest_length); 

		if(send(send_socket, send_buf, 3+1+dest_length, 0) < 0)
		{
			perror("Send Packet Error");
			close(socket_num); 
			exit(-1);
		} 

	}
	else {
		//Forward the packet to the destination
		if(send(dest_socket, recv_buf, MAXBUF, 0) < 0)
		{
			perror("Send Packet Error");
			close(socket_num); 
			exit(-1);
		} 
	}

	free(send_handle); 
	free(dest_handle); 
}

/* Services the client's request to exit. Removes it's node in 
 * the data structure and closes its socket file descriptor. 
 */ 
void parse_packet8(int sending_socket) {
	//Remove client from linked list 
	chat_client *cursor = client_head; 
	chat_client *previous = NULL; 

	while (cursor != NULL) {
		if (cursor->socket_num == sending_socket) {
			//Remove the Node 
			if (cursor == client_head) {
				client_head = cursor->next; 
			}
			else {
				previous->next = cursor->next; 
				if (previous->next == NULL) {
					client_tail = previous; 
				}
			}
			free(cursor); 
			break; 

		}
		previous = cursor; 
		cursor = cursor->next; 
	}


	//Send ACK back to client (flag = 9)
	chat_header header_9; 
	header_9.packet_length = htons(3); 
	header_9.flag = 9; 

	if ((send(sending_socket, &header_9, 3, 0)) < 0) {
		perror("Send Packet Error");
		close(sending_socket); 
		exit(-1);
	}

	//Close the socket 
	close(sending_socket);
	total_clients--; 

}


/* Broadcasts this message to all clients */ 
void parse_packet4(int socket_num, char *recv_buf) {
	char *send_handle; 
	uint8_t send_length; 

	send_length = *(uint8_t *)(recv_buf+3); 
	send_handle = malloc(send_length+1); 
	strncpy(send_handle, recv_buf+4, send_length); 
	send_handle[send_length] = '\0'; 

	chat_client *cursor = client_head; 
	while (cursor != NULL) {
		//Don't send message back to itself 
		if (socket_num != cursor->socket_num) {

			//Forward the packet to the destination
			if(send(cursor->socket_num, recv_buf, MAXBUF, 0) < 0)
			{
				perror("Send Packet Error");
				close(socket_num); 
				exit(-1);
			} 
		}

		cursor = cursor->next;
	}
	
	free(send_handle); 
}

/* Services the client's request to list. Responds 
 * with Packet (flag 11) for total number of clients. 
 * and then continously sends packets (flag 12) to 
 * send names of clients to the requesters. 
 */ 
void parse_packet10(int sending_socket) {

	char *send_buf; 

	send_buf = malloc(7); 

	//Send Total Clients back (Flag 11)
	chat_header header_11; 
	header_11.packet_length = htons(7); 
	header_11.flag = 11; 

	uint32_t send_total = htonl(total_clients); 

	memcpy(send_buf, &header_11, 3); 
	memcpy(send_buf + 3, &send_total, 4); 

	if ((send(sending_socket, send_buf, 7, 0)) < 0) {
		perror("Send Packet Error");
		close(sending_socket); 
		exit(-1);
	}

	free(send_buf); 


	//Create Packet 12 Header 
	chat_header header_12; 
	header_12.packet_length = htons(0); 
	header_12.flag = 12; 

	//Loop through all handlenames and sum total lengths of 
	//names. in order to malloc. 
	chat_client *cursor = client_head; 
	int handle_sum = 0; 
	while (cursor != NULL) {
		handle_sum += strlen(cursor->handle_name); 
		cursor = cursor->next;
	}

	//Entire Packet will be header + 1 byte for len/client + total len of handles 
	send_buf = malloc(3 + total_clients + handle_sum); 

	memcpy(send_buf, &header_12, 3); 

	//Create Flag 12 Packets 
	cursor = client_head; 
	char *send_cursor = send_buf + 3; 
	while (cursor != NULL) {
		uint8_t handle_len = strlen(cursor->handle_name);
		memcpy(send_cursor, &handle_len, 1); 
		memcpy(send_cursor+1, cursor->handle_name, handle_len);  

		cursor = cursor->next;
		send_cursor += handle_len; 
		send_cursor += 1; 
	}

		if (send(sending_socket, send_buf, 3 + total_clients + handle_sum, 0) < 0) {
			perror("Send Packet Error");
			close(sending_socket); 
			exit(-1);
		}

			free(send_buf); 

}


void parse_header(fd_set *set) {

	char *recv_buf;      				//buffer for receiving from client
	recv_buf = (char *) malloc(MAXBUF);
	int recv_len; 


	if (new_client > 0) {
		if (recv(new_client, recv_buf, MAXBUF, 0) < 0){
				perror("Recieve Error\n");
				exit(-1);
		}

		uint16_t packet_length; 
		uint8_t flag; 			

		memcpy(&packet_length, recv_buf, 2); 
		memcpy(&flag, recv_buf + 2, 1); 

		if (flag == 1) {
			parse_packet1(new_client, recv_buf+3); 
		}
	}
	else {

		int largest = 0; 
		if (total_clients == 0) {
			largest = 3;  
		}
		else {

			chat_client *cursor = client_head;

			while (cursor != NULL) {
				if (cursor->socket_num > largest) { 
					largest = cursor->socket_num; 
				}
				cursor = cursor->next;
			}
		}	


		int i = 0; 
		for (i = 4; i < largest + 1; i++) {
			if (FD_ISSET(i, set)) {
				if ((recv_len = recv(i, recv_buf, MAXBUF, 0)) < 0){
					perror("Recieve Error\n");
					exit(-1);
				}

				if (recv_len == 0) {
					//Socket has closed 
					parse_packet8(i); 
				}

				uint16_t packet_length; 
				uint8_t flag; 			

				memcpy(&packet_length, recv_buf, 2); 
				memcpy(&flag, recv_buf + 2, 1); 

				switch(flag) {
				    case 5 :	//Send message to another client
				    	parse_packet5(i, recv_buf); 
				    	break; 
				    case 8 : 	//Client requests exit 
				    	parse_packet8(i); 
				    	break; 
				    case 4 : 	//Broadcast
				    	parse_packet4(i, recv_buf); 
				    	break; 
				    case 10 : 	//Client requests list 
				    	parse_packet10(i); 
				    	break;
				}
			}
		}
	}
	
	free(recv_buf); 
}

/* This function blocks until it finds a client to service. 
 * Returns the file descriptor of the client. 
 */ 
void select_socket(int server_socket, fd_set *socket_list) {
	//Look for the largest Socket in the list to get the size
	int largest = 0; 
	if (total_clients == 0) {
		largest = server_socket; 
	}
	else {
		chat_client *cursor = client_head;

		while (cursor != NULL) {
			if (cursor->socket_num > largest) { 
				largest = cursor->socket_num; 
			}
			
			FD_SET(cursor->socket_num, socket_list); 
			cursor = cursor->next;
		}	
	}
		
	//Allows the program to continue while waiting for a new socket to arrive 
	select(largest+1, socket_list , (fd_set *) 0, (fd_set *) 0 , NULL); 

	if (FD_ISSET(server_socket, socket_list)) {
		//Initializing the New Client 
		new_client = tcpAccept(server_socket);
		FD_SET(new_client, socket_list);
	}
}

int init_server(int argc, char *argv[]) {

	total_clients = 0; 				 		//Initialize Linked List of Clients 
	client_head = client_tail = NULL; 
	new_client = -1; 
	if (argc == 2) {
		return tcpServerSetup_SpecifiedPort((uint16_t)atoi(argv[1])); 
	}
	else if (argc == 1) {
		return tcpServerSetup(); //create the server socket
	}

	fprintf(stderr, "usage: %s [port-number]\n", argv[0]); 
	exit(-1); 
					
}

void select_code(fd_set *socket_list, int server_socket) {
	FD_ZERO(socket_list); 
	FD_SET(server_socket, socket_list); 
		
  	select_socket(server_socket, socket_list);

	parse_header(socket_list); 
}


int main(int argc, char *argv[])
{

	int server_socket = 0;	//socket descriptor for the client/server socket	 	
	fd_set socket_list;							//Set of socket descriptors 

	server_socket = init_server(argc, argv); 

	while (1) {
	
		 select_code(&socket_list, server_socket); 
	}
	
	/* close the sockets */
	close(server_socket);
	
	return 0;
}
