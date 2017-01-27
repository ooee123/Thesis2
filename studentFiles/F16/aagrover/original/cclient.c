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

#include "networks.h"
#include "testing.h"

#define MAXBUF 1024
#define HEADER 3
#define BYTE 1
#define PACKET_LEN 2

void startUp(int socket_fd);
void printPrompt(void);
void initPacket(char *client_handle, int socket_fd);
void promptUser(int socket_fd);
void message(char *input, int socket_fd, int send_len);
void broadcast(char *input, int socket_fd, int send_len);
void handleList(char *input, int socket_fd, int send_len);
void exitClient(char *input, int socket_fd, int send_len);
void transmitCheck(int socket_fd);
void receivingMessage(char *packet);
void errorMessage(char *packet);
void receiveNumHandles(char *packet, int socket_fd);
void printHandles(char *packet, int socket_fd);
void terminateClient(char *packet);

char *global_handle;
int max_socket = 3;
int list_flag = 0;
int actual_clients = 0;
int count_clients = 0;

int main(int argc, char *argv[]) {
	int socket_fd = 0;

	if (argc != 4) {
		fprintf(stderr, "Error - must include IP address, port number, and client handle name\n");
		exit(EXIT_FAILURE);
	}
	
	/* Set up the TCP Client socket */
	socket_fd = tcpClientSetup(argv[2], argv[3]);
	initPacket(argv[1], socket_fd);
	
	startUp(socket_fd);
	close(socket_fd);

	return 0;
}

void startUp(int socket_fd) {
	int max_socket = socket_fd;
	fd_set fdvar;

        FD_ZERO(&fdvar);
        FD_SET(socket_fd, &fdvar);

	while (1) {	
		if (!list_flag) {
			printPrompt();
		}
		while (1) {
			FD_ZERO(&fdvar);
			FD_SET(socket_fd, &fdvar);
			FD_SET(STDIN_FILENO, &fdvar);

			if (select(max_socket + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, NULL) > 0) {
				if (FD_ISSET(socket_fd, &fdvar)) {
					transmitCheck(socket_fd);	
				}

				if (!list_flag) {
					if (FD_ISSET(STDIN_FILENO, &fdvar)) {
						promptUser(socket_fd);
					}
				}
				break;
			}

			break;
		}
	}
}

void transmitCheck(int socket_fd) {
	char *packet = NULL;
	int bytes_recv = -1;
	uint8_t flag;
	
	static struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	packet = malloc(MAXBUF);
	bytes_recv = recv(socket_fd, packet, MAXBUF, 0);

	if (bytes_recv != -1) {
		flag = *(packet + PACKET_LEN);

		switch (flag) {

			case 5:
				receivingMessage(packet);
				break;
			case 7:
				errorMessage(packet);
				break;
			case 9:
				terminateClient(packet);
				break;
			case 11:
				receiveNumHandles(packet, socket_fd);
				break;
			case 12:
				count_clients++;
				printHandles(packet, socket_fd);	

				if (count_clients == actual_clients) {
					list_flag = 0;
					count_clients = 0;
				}
				break;
			default:
				// invalid flag.
				break;	
		}
	}
}

void terminateClient(char *packet) {
	printf("\n");
	exit(EXIT_SUCCESS);
}

void printHandles(char *packet, int socket_fd) {
	uint16_t packet_len, gar_len = HEADER;
	uint8_t flag;
	uint8_t handle_len;	
	char *handle, *garbage = malloc(HEADER);

	packet_len = *(uint16_t*)packet;
	handle_len = *((uint8_t*)(packet + HEADER));
	handle = packet + HEADER + BYTE;

	printf("\t%s\n", handle);

	/* garbage signal to receive handles. */
        flag = 0;

        memcpy(garbage, &gar_len, PACKET_LEN);
        memcpy(garbage + PACKET_LEN, &flag, BYTE);

        send(socket_fd, garbage, gar_len, 0);
}

void receiveNumHandles(char *packet, int socket_fd) {
	uint32_t num_clients;
	char *garbage = malloc(HEADER);
	uint16_t gar_len = HEADER;
	/* garbage signal to receive handles. */
	int flag = 0;
	
	num_clients = *((uint32_t*)(packet + HEADER));
	actual_clients = num_clients;

	printf("\nNumber of clients: %d\n", num_clients);
	list_flag = 1;

	memcpy(garbage, &gar_len, PACKET_LEN);
	memcpy(garbage + PACKET_LEN, &flag, BYTE);

	send(socket_fd, garbage, gar_len, 0);
}

void receivingMessage(char *packet) {
	char *sender, *text, *buf;
	uint16_t packet_len;
	uint8_t sender_len;

	packet_len = *((uint16_t*)packet);
	sender_len = *(packet + HEADER);
	sender = packet + HEADER + BYTE;
	text = packet + HEADER + BYTE + sender_len;	

	buf = malloc(sender_len);
	memcpy(buf, sender, sender_len);

	printf("\n%s: %s\n", buf, text);
	free(buf);
}

void initPacket(char *client_handle, int socket_fd) {
	// This fn needs to start the inital client setup.
	// flag = 1 and format is: chat header + 1 byte handle length + handle
	uint16_t packet_len = HEADER + BYTE + strlen(client_handle);
	char *packet = malloc(packet_len);
	uint8_t flag = 1, handle_len = strlen(client_handle);
	int sent;

	memcpy(packet, &packet_len, PACKET_LEN);
	memcpy(packet + PACKET_LEN, &flag, 1);
	memcpy(packet + PACKET_LEN + 1, &handle_len, 1);
	memcpy(packet + PACKET_LEN + 2, client_handle, handle_len);

	sent = send(socket_fd, packet, packet_len, 0);

	packet = NULL;
	packet = malloc(3);

	fd_set fdvar;
	int nfds, msg_len;

	FD_ZERO(&fdvar);
	FD_SET(socket_fd, &fdvar);
	
	nfds = select(socket_fd + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, NULL);

	if (FD_ISSET(socket_fd, &fdvar)) {
	
		msg_len = recv(socket_fd, packet, MAXBUF, 0);
		packet_len = *(uint16_t*)packet;
       		flag = *((uint8_t*)packet + PACKET_LEN);

		if (flag == 3) {
			fprintf(stderr, "Handle already in use: <%s>\n", client_handle);
			exit(EXIT_FAILURE);
		}

		global_handle = client_handle;
		free(packet);
	}
}

void promptUser(int socket_fd) {
	char *send_buf = NULL, command;
	int send_len = 0;

	send_buf = (char *) malloc(MAXBUF);
	
	send_len = 0;
	while ((send_buf[send_len] = getchar()) != '\n' && send_len < MAXBUF-1)
	{
		send_len++;
	}

	send_buf[send_len] = '\0';

	if (send_buf[0] != '%') {
		printf("Invalid command\n");	
	}
	else {
		// Get second char.
		command = send_buf[1];
		switch (tolower(command)) {
			case 'm':
				message(send_buf, socket_fd, send_len);
				break;
			case 'b':
				broadcast(send_buf, socket_fd, send_len);
				break;
			case 'l':
				handleList(send_buf, socket_fd, send_len);
				break;
			case 'e':
				exitClient(send_buf, socket_fd, send_len);
				break;
			default:
				/* Error. */
				printf("Invalid command\n");
				break;
		}
	}
}

void message(char *send_buf, int socket_fd, int send_len) {
	char *token = send_buf, *handle, *packet, *text;
	const char *delim = " ", flag = 5;
	int sent, txt_len = 0;
	uint16_t total_len = 0;
	uint8_t handle_len, sending_len;
	
	/* the command */
	token = strtok(token, delim);

	/* the handle/client */
	handle = strtok(NULL, delim);

	/* the handle length and name */
	handle_len = strlen(handle);
	sending_len = strlen(global_handle);

	txt_len = send_len - 4 - handle_len;
	text = malloc(txt_len + 1);

	while ((token = strtok(NULL, delim)) != NULL) {
		strcat(text, token);
		strcat(text, " ");
	}
	strcat(text, "\0");

	total_len = 3 + 1 + handle_len + 1 + sending_len + txt_len;
 
	/* Add header size in bytes. */
	packet = malloc(total_len);
	/* Copy packet length into packet. */
	memcpy(packet, &total_len, 2);
	/* Copy flag into packet. */
	memcpy(packet + 2, &flag, 1);
	/* Copy handle length into packet. */
	memcpy(packet + 2 + 1, &handle_len, 1);
	/* Copy handle name into packet. */
	memcpy(packet + 2 + 1 + 1, handle, handle_len);
	/* Copy sending client handle length */
	memcpy(packet + 2 + 1 + 1 + handle_len, &sending_len, 1);
	/* Copy sending client handle name */
	memcpy(packet + 2 + 1 + 1 + handle_len + 1, global_handle, sending_len);
	/* Copy text message into packet. */
	memcpy(packet + 2 + 1 + 1 + handle_len + 1 + sending_len, text, txt_len + 1);

	sent = send(socket_fd, packet, total_len, 0);
	//printf("message sent %d bytes\n", sent);
}

  
void broadcast(char *input, int socket_fd, int send_len) {
	char *token = input, *handle, *packet, *text;
	uint8_t sending_len, flag = 4;
	int total_len, txt_len, sent;

	/* the command */
        token = strtok(token, " ");

	/* sending client handle length */
        sending_len = strlen(global_handle);

	/* counted chars - %b - 1 space */
        txt_len = send_len - 3;

	/* take account for ending null char */
        text = malloc(txt_len + 1);

        while ((token = strtok(NULL, " ")) != NULL) {
                strcat(text, token);
                strcat(text, " ");
        }
	strcat(text, "\0");

	handle = global_handle;

        strcat(text, "\0");
        total_len = 3 + 1 + sending_len + txt_len;

	/* Add header size in bytes. */
        packet = malloc(total_len);
        /* Copy packet length into packet. */
        memcpy(packet, &total_len, 2);
        /* Copy flag into packet. */
        memcpy(packet + 2, &flag, 1);
        /* Copy handle length into packet. */
        memcpy(packet + 2 + 1, &sending_len, 1);
        /* Copy handle name into packet. */
        memcpy(packet + 2 + 1 + 1, handle, sending_len);
	/* Text message */
	memcpy(packet + 2 + 1 + 1 + sending_len, text, txt_len + 1);

	// ssize_t send(int socket, const void *buffer, size_t length, int flags);
	sent = send(socket_fd, packet, total_len, 0);	
}

void handleList(char *input, int socket_fd, int send_len) {
	/* Print out a list of the handles currently known by the server. */
	char *packet;
	uint16_t packet_len = HEADER;
	uint8_t flag = 10;
	int sent;

	packet = malloc(packet_len);
	memcpy(packet, &packet_len, PACKET_LEN);
	memcpy(packet + PACKET_LEN, &flag, BYTE);

	sent = send(socket_fd, packet, packet_len, 0);
}


void exitClient(char *input, int socket_fd, int send_len) {
	uint16_t packet_len;
	uint8_t flag = 8;
	char *packet;
	int sent;

	packet_len = HEADER;
	packet = malloc(packet_len);
	memcpy(packet, &packet_len, PACKET_LEN);
	memcpy(packet + PACKET_LEN, &flag, BYTE);

	sent = send(socket_fd, packet, packet_len, 0);
}

// FLAG = 7
void errorMessage(char *packet) {
	char *invalid, *client;
	int invalid_len = *(packet + HEADER);

	client = packet + HEADER + BYTE;
	invalid = malloc(invalid_len);
        memcpy(invalid, client, invalid_len);

	fprintf(stderr, "\nClient with handle %s does not exist.\n", invalid);
	free(invalid);
}

void printPrompt() {
	/* Print user prompt to view. */
	printf("$: ");
	fflush(stdout);
}
