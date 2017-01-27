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

#define MAXBUF 1024
#define BYTE 1
#define PACKET_LEN 2
#define HEADER 3
#define VAR_EL 1 

void receiveData(int client_socket);
void initialPacket(char *packet, int client_socket);
void sendMessage(char *packet, int client_socket);
void destinationError(int client_socket, char *invalid, char *send_to, 
	int send_to_len, char *send_from, int send_from_len);
int compareClients(char *send_to, int send_to_len);
void selectCall(int server_socket);
void broadcastMessage(char *packet, int client_socket);
void listAllHandles(char *packet, int client_socket);
void sendHandle(int sending_index);
void exitClient(char *packet, int client_socket);


int variable_elements = VAR_EL;
char **handle_list;
int *handle_list_fds;
int num_handles_name = 0;
int num_handles_fd = 0;
int max_socket = 3;
int global_socket;

int list_flag = 0;
int sending_index = 0;
int list_client;

int main(int argc, char *argv[]) {
	/* Socket descriptor for the server socket. */
	int server_socket = tcpServerSetup(); 

	handle_list = malloc(variable_elements * sizeof(char*));	
	handle_list_fds = malloc(variable_elements * sizeof(char));
	global_socket = server_socket;
	/* Socket descriptor for the client socket. */
	selectCall(server_socket);
	
	close(server_socket);

	return 0;
}

void selectCall(int server_socket) {
	int client_socket;
        fd_set fdvar;

	FD_ZERO(&fdvar);
        FD_SET(server_socket, &fdvar);
	handle_list_fds[num_handles_fd] = server_socket;
	num_handles_fd++;

	while (select(max_socket + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, NULL)) {
		int i;
		for (i = 0; i < num_handles_fd; i++) {
			int current_fd = handle_list_fds[i];
			if (FD_ISSET(current_fd, &fdvar)) {
				if (current_fd == server_socket) {
					client_socket = tcpAccept(server_socket);
        				FD_SET(client_socket, &fdvar);
				
					handle_list_fds[num_handles_fd] = client_socket;
					num_handles_fd++;
					
					if (client_socket > max_socket) {
						max_socket = client_socket;
					}
				}
				receiveData(current_fd);					
			}
		}

		FD_ZERO(&fdvar);
		for (i = 0; i < num_handles_fd; i++) {
			FD_SET(handle_list_fds[i], &fdvar);
		}
	}
}

void receiveData(int client_socket) {
	int message_len;
	char *packet = NULL;
	uint16_t packet_len;
	uint8_t flag;

	packet = malloc(MAXBUF);
	message_len = recv(client_socket, packet, MAXBUF, 0);

	if (!list_flag) {
		/* First, get the flag - parse chat header. */
		packet_len = *(uint16_t*)packet;
		flag = *((uint8_t*)(packet + PACKET_LEN));

		switch (flag) {
			case 1:
				/* Inital packet is sent from client. */
				initialPacket(packet, client_socket);
				break;
			case 5:
				/* Message is sent from client to another client. */
				sendMessage(packet, client_socket);
				break;
			case 4:
				broadcastMessage(packet, client_socket);
				break;
			case 8:
				exitClient(packet, client_socket);
				break;
			case 10:
				listAllHandles(packet, client_socket);
				break;
		}
	}
	else {
		if (sending_index == num_handles_name) {
			sending_index = 0;
			list_flag = 0;		
		}
		else {
			sendHandle(sending_index);		
		}
	}
}

void exitClient(char *packet, int client_socket) {
	char *ret_packet;
	uint16_t packet_len = HEADER;
	uint8_t flag = 9;
	int sent;

	ret_packet = malloc(HEADER);
	memcpy(ret_packet, &packet_len, PACKET_LEN);
	memcpy(ret_packet + PACKET_LEN, &flag, BYTE);

	/* Eliminate client_socket from dynamic storage */
	int i, index = -1;
	for (i = 0; i < num_handles_fd; i++) {
		if (handle_list_fds[i] == client_socket) {
			index = i;
		}
	}

	for (i = index; i < num_handles_fd - 1; i++) {
		handle_list_fds[i] = handle_list_fds[i + 1];
	}

	for (i = index - 1; i < num_handles_name - 1; i++) {
		handle_list[i] = handle_list[i + 1];
	}

	num_handles_name--;
	num_handles_fd--;

	sent = send(client_socket, ret_packet, packet_len, 0);
}

void listAllHandles(char *packet, int client_socket) {
	char *ret_packet;
	uint16_t packet_len;
	uint8_t flag = 11;
	int sent;

	packet_len = HEADER + 4;
	ret_packet = malloc(packet_len);
	
	memcpy(ret_packet, &packet_len, PACKET_LEN);
	memcpy(ret_packet + PACKET_LEN, &flag, BYTE);
	memcpy(ret_packet + HEADER, &num_handles_name, 4);

	sent = send(client_socket, ret_packet, packet_len, 0);
	free(ret_packet);

	/* Now start sending all handles to client */

	list_flag = 1;
	list_client = client_socket;
}

void sendHandle(int handle_index){
	char *ret_packet, *handle;
	uint8_t handle_len, flag = 12;
	uint16_t packet_len;
	int sent;

	handle = handle_list[handle_index];
	handle_len = strlen(handle_list[handle_index]);
	packet_len = HEADER + BYTE + handle_len;

	ret_packet = malloc(packet_len);

	memcpy(ret_packet, &packet_len, PACKET_LEN);
	memcpy(ret_packet + PACKET_LEN, &flag, BYTE);
	memcpy(ret_packet + HEADER, &handle_len, BYTE);
	memcpy(ret_packet + HEADER + BYTE, handle, handle_len);

	sent = send(list_client, ret_packet, packet_len, 0);
	free(ret_packet);
	
	sending_index++;
}

void initialPacket(char *packet, int client_socket) {
	char *ret_packet = malloc(3), *cmp2;
	char handle_length = *(packet + 3);
	char *handle = packet + 4;
	int packet_len, flag, sent;

	handle = strtok(handle, " ");

	packet_len = 3;
	flag = 2;

	int i;
	for (i = 0; i < num_handles_name; i++) {
		cmp2 = malloc(handle_length);
		memcpy(cmp2, handle, handle_length);

		if (strcmp(handle_list[i], cmp2) == 0) {
			flag = 3;
		}

		free(cmp2);
	}

	if (flag == 2) {
		if (num_handles_name == variable_elements) {
			variable_elements *= 2;
			char *tmp = realloc(handle_list, variable_elements);
			if (tmp == NULL)
				printf("Error reallocating memory!\n");

			/* now increase fd array size */
			int *tmp_arr =  malloc(variable_elements);
			int i;
			for (i = 0; i < num_handles_fd; i++) {
				tmp_arr[i] = handle_list_fds[i];
			}
			handle_list_fds = tmp_arr;
		}

		handle_list[num_handles_name] = malloc(handle_length);
		memcpy(handle_list[num_handles_name], handle, handle_length);
		num_handles_name++;
	}

	memcpy(ret_packet, &packet_len, PACKET_LEN);
	memcpy(ret_packet + PACKET_LEN, &flag, 1);
	
	sent = send(client_socket, ret_packet, 3, 0);
}

void sendMessage(char *packet, int client_socket) {
	char *send_to, *send_from, *text, *invalid;
	uint8_t send_to_len, send_from_len, flag;
	int fd_index = -1, fd_send, sent, text_len;
	uint16_t packet_len;

	send_to_len = *(packet + HEADER);
	send_to = packet + HEADER + BYTE;
	send_from_len = *(packet + HEADER + BYTE + send_to_len);
	send_from = packet + HEADER + BYTE + send_to_len + BYTE;
	text = packet + HEADER + BYTE + send_to_len + BYTE + send_from_len;

	text_len = *(uint16_t*)packet - HEADER - BYTE - send_to_len - BYTE - send_from_len;

	if (strlen(text) == 0) {
		text = malloc(1);
		memcpy(text, " ", 1);
		text_len = 1;
	}

	invalid = malloc(send_to_len);
        memcpy(invalid, send_to, send_to_len);

	fd_index = compareClients(send_to, send_to_len);
	flag = 5;
	packet_len = HEADER + BYTE + send_from_len + text_len;

	packet = malloc(packet_len);
	memcpy(packet, &packet_len, PACKET_LEN);
	memcpy(packet + PACKET_LEN, &flag, BYTE);
	memcpy(packet + PACKET_LEN + BYTE, &send_from_len, BYTE);
	memcpy(packet + PACKET_LEN + BYTE + BYTE, send_from, send_from_len);
	memcpy(packet + PACKET_LEN + BYTE + BYTE + send_from_len, text, text_len);

	if (fd_index != -1) {
		fd_send = handle_list_fds[fd_index + 1];
		sent = send(fd_send, packet, packet_len, 0);
	}
	else {
		destinationError(client_socket, invalid, send_to, send_to_len, send_from, send_from_len);
	}
	
	free(invalid);
}

int compareClients(char *send_to, int send_to_len) {
	int i, fd_index = -1, cmp, handle_len;
	char *hand, *tmp;

	for (i = 0; i < num_handles_name; i++) {
		handle_len = strlen(handle_list[i]);
		hand = malloc(send_to_len);
		tmp = malloc(send_to_len);

		memcpy(hand, send_to, send_to_len);
		memcpy(tmp, handle_list[i], handle_len);

		cmp = strcmp(tmp, hand);

		if (cmp == 0) {
			return i;
		}
       	}

	return fd_index;
} 

void broadcastMessage(char *packet, int client_socket) {
	char *packet_new, *send_from, *text;
	uint16_t packet_len, text_len, packet_new_len;
	uint8_t send_from_len, flag = 5;

	packet_len = *((uint16_t*)packet);
	send_from_len = *((uint8_t*)(packet + HEADER));
	send_from = packet + HEADER + BYTE;
	text = packet + HEADER + BYTE + send_from_len;

	text_len = packet_len - HEADER - BYTE - send_from_len;

	if (strlen(text) == 0) {
                text = malloc(1);
                memcpy(text, " ", 1);
                text_len = 1;
        }

	packet_new_len = HEADER + BYTE + send_from_len + text_len;

	packet_new = malloc(packet_new_len);
	memcpy(packet_new, &packet_new_len, PACKET_LEN);
	memcpy(packet_new + PACKET_LEN, &flag, BYTE);
	memcpy(packet_new + HEADER, &send_from_len, BYTE);
	memcpy(packet_new + HEADER + BYTE, send_from, send_from_len);
	memcpy(packet_new + HEADER + BYTE + send_from_len, text, text_len);

	int i, cur_fd, sent;
	for (i = 0; i < num_handles_fd; i++) {
		cur_fd = handle_list_fds[i];

		if (cur_fd != global_socket && cur_fd != client_socket) {
			sent = send(cur_fd, packet_new, packet_len, 0);
		}
	}
}

void destinationError(int client_socket, char *invalid, char *send_to, 
 int send_to_len, char *send_from, int send_from_len) {
	/* Message packet for invalid clients that DNE */
	char *packet;
	uint16_t packet_len;
	uint8_t flag = 7, len = send_to_len;
	int sent;

	packet_len = HEADER + BYTE + send_to_len;	
	packet = malloc(packet_len);
	memcpy(packet, &packet_len, PACKET_LEN);
	memcpy(packet + PACKET_LEN, &flag, BYTE);
	memcpy(packet + PACKET_LEN + BYTE, &len, BYTE);
	memcpy(packet + PACKET_LEN + BYTE + BYTE, send_to, len);

	sent = send(client_socket, packet, packet_len, 0);	
}
