#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>

#include "networks.h"

int state;
int getstdin;
string my_handle;
uint32_t num_handlelist;

int main(int argc, char * argv[]) {
	int socket_num;         //socket descriptor
	state = START;
	getstdin = 0;
	num_handlelist = 0 ;
	/* check command line arguments  */
	if(argc!= 4) {
		printf("usage: %s handle host-name port-number \n", argv[0]);
		exit(1);
	}
	my_handle = &argv[1][0u];
	if(my_handle.length() > 255) {
		printf("Handle is too long( > 255) \n");
		exit(-1);
	}

	/* set up the socket for TCP transmission  */
	socket_num= tcp_send_setup(argv[2], argv[3]);

	//does initial handle stuff....
	init_handle(socket_num,argv[1]);

	client_listen(socket_num);

	return 0;
}

int tcp_send_setup(char *host_name, char *port) {
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

    if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0) {
    	perror("connect call");
    	exit(-1);
    }

    return socket_num;
}


void init_handle(int socket, char *handle) {

	uint8_t *buf = (uint8_t *)malloc(3 + strlen(handle));
	uint8_t flag = 1;
	uint16_t size = htons(3+ 1 + strlen(handle));
	uint8_t handle_size = strlen(handle);

	memcpy(buf,&size,2);
	memcpy(buf+2,&flag,1);
	memcpy(buf+3,&handle_size,1);
	memcpy(buf+4,handle,handle_size);
	send(socket,buf,ntohs(size),0);
}

void client_listen(int socket) {
	fd_set fdvar;
	string input;
	while(1) {
		FD_ZERO(&fdvar);
		FD_SET(socket, &fdvar);
		if( getstdin == 1)
			FD_SET(1,&fdvar);
		select(socket+1,(fd_set *) &fdvar, (fd_set *) 0,
						(fd_set *) 0, NULL);

		if(FD_ISSET(socket,&fdvar))
			client_get_packet(socket);
		if (FD_ISSET (1,&fdvar)) {
			getline(cin,input);
			client_parse_input(input,socket);
		}
	}
}

void client_get_packet(int socket) {
	static uint8_t *buf=  (uint8_t *) malloc(1024);              //buffer for receiving from client
	static int recv_len= 0;     //length of the received message
	static uint16_t message_len = 0; //parsed mesage len
	int cur = 0;

	static uint8_t handle_length = 0;
	static uint8_t handle_processed = 0;
	static vector<uint8_t *> handle_list;
	static uint8_t *handle;
	switch(state) {
			case START:
				recv_len += client_recv(socket,buf,2,0 );
				if(recv_len == 2) {
					state=GET_DATA;
					memcpy(&message_len,buf,2);
					message_len = ntohs(message_len);
					if(message_len == 0) {
						// proceed with getting the flag out, then on to handles!
						client_recv(socket,buf+2,1,0);
						state = HANDLE_LENGTH;
						break;
					}

					message_len -= 2;
				}

				else if(recv_len == 1)
					state = PROCESS_LENGTH;
				break;

			case PROCESS_LENGTH:
				recv_len += client_recv(socket,buf+1,1,0);
				memcpy(&message_len,buf,2);
				message_len = ntohs(message_len);
				if(message_len == 0) {
					client_recv(socket,buf+2,1,0);
					state = HANDLE_LENGTH;
					break;
				}
				message_len -= 2;
				state = GET_DATA;
				break;

			case GET_DATA:
				if(message_len > 0) {
					cur = client_recv(socket,buf+recv_len,message_len,0);
					message_len -= cur;
					recv_len+=cur;
				}

				if(message_len == 0) {
					client_process_packet(buf,socket);
					recv_len = 0;
					message_len = 0;
					memset(buf,0,1024);
					state = START;
				}
				break;

			case HANDLE_LENGTH:
				client_recv(socket,&handle_length,1,0);
				handle = (uint8_t *) malloc(handle_length +1);
				memset(handle+handle_length,'\0',1);
				state = HANDLE_LIST;
				break;

			case HANDLE_LIST:

				if(handle_length > 0) {
					cur = client_recv(socket,handle+handle_processed,handle_length ,0);
					handle_processed += cur;
					handle_length -= cur;
				}

				if(handle_length == 0) {
					num_handlelist --;
					handle_list.push_back(handle);
				}

				if(handle_length == 0 && num_handlelist != 0) {
					//done with current handle, on to next one!
					handle_processed = 0;
					state = HANDLE_LENGTH;
				}
				else if(handle_length == 0 && num_handlelist == 0) {
					//done with cur handle and handle_list, print out
					print_handle_list(&handle_list);
					state = START;
					recv_len = 0;
					handle_processed = 0;
					message_len = 0;
				}
				break;

			default:
				perror("wrong state");
				exit(-1);
				break;
		}
}

void client_process_packet(uint8_t *buf, int socket) {
	switch( check_flag(buf) ) {
		case 2:
			cout << "\n$:";
			fflush(stdout);
			//good handle flag
			getstdin = 1;
			break;
		case 3:
			cout << "Handle already in use: " << my_handle << endl;
			close(socket);
			exit(-1);
			break;
		case 4:
			print_broadcast_packet(buf);
			cout << "\n$:";
			fflush(stdout);
			break;
		case 5:
			print_msg_packet(buf);
			cout << "\n$:";
			fflush(stdout);
			break;
		case 7:
			print_bad_msg_handle(buf);
			cout << "\n$:";
			fflush(stdout);
			break;
		case 9:
			close(socket);
			exit(1);
			break;
		case 11:
			memcpy(&num_handlelist,buf+3,4);
			num_handlelist = ntohl(num_handlelist);
			break;
	}
}

int check_flag(uint8_t *packet) {
	uint8_t flag;
	memcpy(&flag,packet+2,1);
	return flag;
}

void print_broadcast_packet(uint8_t *buf) {
	uint8_t *handle;
	uint8_t handle_length;
	uint16_t total_packet_length;

	memcpy(&total_packet_length,buf,2);
	memcpy(&handle_length, buf+3, 1);
	handle = (uint8_t *) malloc(handle_length +1);
	memcpy(handle,buf+4,handle_length);
	memset(handle+handle_length,'\0',1);

	printf("\n%s: %s",handle,buf+4+handle_length);
	free(handle);
}

void print_msg_packet(uint8_t *buf) {
	uint8_t dest_handle_length;
	uint8_t *dest_handle;
	uint8_t src_handle_length;
	uint8_t *src_handle;

	memcpy( &dest_handle_length,buf+3, 1);
	dest_handle = (uint8_t *)malloc(dest_handle_length+1);
	memcpy(dest_handle,buf+4, dest_handle_length );
	memset(dest_handle+dest_handle_length,'\0',1);

	memcpy(&src_handle_length,buf+4+dest_handle_length,1);
	src_handle = (uint8_t *) malloc(src_handle_length+1);
	memcpy(src_handle, buf+4+dest_handle_length+1,src_handle_length);
	memset(src_handle+src_handle_length,'\0',1);

	printf("\n%s: %s",src_handle,  buf+3 + 1 +dest_handle_length+1 + src_handle_length);

	free(dest_handle);
	free(src_handle);
}

void print_bad_msg_handle(uint8_t *buf) {
	uint8_t dest_handle_length;
	uint8_t *dest_handle;
	memcpy( &dest_handle_length,buf+3, 1);
	dest_handle = (uint8_t *)malloc(dest_handle_length+1);
	memcpy(dest_handle,buf+4, dest_handle_length );
	memset(dest_handle+dest_handle_length,'\0',1);

	cout << "Client with handle " << dest_handle << " does not exist." << endl ;

	free(dest_handle);
}

void print_handle_list( vector<uint8_t *> *handle_list) {

	cout << "Number of clients: " << (*handle_list).size() << endl;
	for(uint32_t i = 0; i < (*handle_list).size();i++) {
		printf("%s\n",(*handle_list)[i]);
		free((*handle_list)[i]);
	}
	(*handle_list).clear();
	cout << "\n$:";
	fflush(stdout);
}

void client_parse_input(string input, int socket) {
	string command;
	stringstream ss(input);
	vector<string> tokens;

	while (ss >> command)
		tokens.push_back(command);

	if(tokens[0].compare(string("%e")) == 0 || tokens[0].compare(string("%E")) == 0  ) {
		client_send_exit(socket);
	}
	else if(tokens[0].compare(string("%M")) == 0 || tokens[0].compare(string("%m")) == 0  ) {
		if(tokens.size() >= 2 && tokens[1].length() <= 255) {
			client_create_message(socket, tokens);
		}
		else {
			cout << "Invalid Command." << endl;
			cout << "\n$:";
			fflush(stdout);
		}

	}
	else if(tokens[0].compare(string("%b")) == 0 || tokens[0].compare(string("%B")) == 0  ) {
		if(tokens.size()>=1)
			client_create_broadcast(socket, tokens);
		else {
			cout << "Invalid Command." << endl;
			cout << "\n$:";
		}

	}
	else if(tokens[0].compare(string("%l")) == 0 || tokens[0].compare(string("%L")) == 0  ) {
		if(tokens.size() == 1)
			client_request_list(socket);
	}
	else {
		cout << "Invalid Command." << endl ;
		cout << "\n$:";
		fflush(stdout);
	}
	tokens.clear();

}

void client_send_exit(int socket) {
	char buf[3];
	uint16_t packetlength = htons(3);
	uint8_t flag = 8;

	memcpy(buf,&packetlength,2);
	memcpy(buf+2,&flag,1);
	send(socket,buf,3,0);
	cout << "\n$:";
	fflush(stdout);
}

void client_create_message(int socket, vector <string> tokens) {
	int total_msg_length = 0;
	int counter = 0;
	uint8_t *msg;

	int num_full_packets=0;
	int remainder=0;
	uint8_t *msg_split;

	for(unsigned int i = 2; i < tokens.size(); i ++) {
		total_msg_length += tokens[i].length();
		total_msg_length ++;
	}
	msg = (uint8_t *) malloc(total_msg_length);
	for(unsigned int i = 2; i < tokens.size(); i ++) {
			memcpy(msg + counter,&tokens[i][0u],tokens[i].length() );
			counter += tokens[i].length();
			if(i == tokens.size()-1)
				memset(msg + counter,'\0', 1 );
			else
				memset(msg + counter,' ', 1 );
			counter ++;
		}
	if(total_msg_length > 1000) {

		remainder = total_msg_length % 1000;
		num_full_packets = (total_msg_length - remainder) / 1000 ;

		for(int i = 0; i < num_full_packets; i++) {
			msg_split = (uint8_t*) malloc(1000);
			memcpy(msg_split,msg+i*1000,1000);
			client_send_message_packet(socket,1000,tokens[1],msg_split);
			free(msg_split);
		}
		msg_split = (uint8_t *) malloc(remainder);
		memcpy(msg_split,msg+num_full_packets*1000,remainder);
		client_send_message_packet(socket,remainder,tokens[1],msg_split);
		free(msg_split);
	}

	else
		client_send_message_packet(socket,total_msg_length, tokens[1], msg);
	free(msg);
	cout << "\n$:";
	fflush(stdout);

}

void client_send_message_packet(int socket,int msg_length, string dest_handle, uint8_t *msg) {
	uint16_t total_packet_length = htons(3 + 1 + dest_handle.length() + 1 + my_handle.length() + msg_length);
	uint8_t buf[ntohs(total_packet_length)];

	uint8_t flag = 5;
	uint8_t dest_length = dest_handle.length();
	uint8_t my_length = my_handle.length();

	memcpy(buf, &total_packet_length, 2);
	memcpy(buf+2, &flag, 1);
	memcpy(buf+3, &dest_length, 1);
	memcpy(buf+4, &dest_handle[0u],dest_length );
	memcpy(buf+4+dest_length,&my_length,1);
	memcpy(buf+4+dest_length+1,&my_handle[0u],my_length);
	memcpy(buf+4+dest_length+1+my_length,msg,msg_length);

	send(socket,buf,ntohs(total_packet_length),0);
}

void client_create_broadcast(int socket, vector<string> tokens) {
	int total_msg_length = 0;
	int counter = 0;
	uint8_t *msg;

	int num_full_packets=0;
	int remainder=0;
	uint8_t *msg_split;

	for(unsigned int i = 1; i < tokens.size(); i ++) {
		total_msg_length += tokens[i].length();
		total_msg_length ++;
	}
	msg = (uint8_t *) malloc(total_msg_length);
	for(unsigned int i = 1; i < tokens.size(); i ++) {
			memcpy(msg + counter,&tokens[i][0u],tokens[i].length() );
			counter += tokens[i].length();
			if(i == tokens.size()-1)
				memset(msg + counter,'\0', 1 );
			else
				memset(msg + counter,' ', 1 );
			counter ++;
		}

	if(total_msg_length > 1000) {
		cout << "msg length TRIGGERED" << endl;
		remainder = total_msg_length % 1000;
		num_full_packets = (total_msg_length - remainder) / 1000 ;

		for(int i = 0; i < num_full_packets; i++) {
			msg_split = (uint8_t*) malloc(1000);
			memcpy(msg_split,msg+i*1000,1000);
			client_send_broadcast(socket,1000,msg_split);
			free(msg_split);
		}
		msg_split = (uint8_t *) malloc(remainder);
		memcpy(msg_split,msg+num_full_packets*1000,remainder);
		client_send_broadcast(socket,1000,msg_split);
		free(msg_split);
	}
	else
		client_send_broadcast(socket,total_msg_length, msg);
	free(msg);
	cout << "\n$:";
	fflush(stdout);
}

void client_send_broadcast(int socket, int msg_length, uint8_t *msg) {
	uint16_t total_packet_length = htons(3 + 1 + my_handle.length() + msg_length);
	uint8_t buf[ntohs(total_packet_length)];

	uint8_t flag = 4;

	uint8_t my_length = my_handle.length();

	memcpy(buf, &total_packet_length, 2);
	memcpy(buf+2, &flag, 1);
	memcpy(buf+3, &my_length, 1);
	memcpy(buf+4, &my_handle[0u], my_length );
	memcpy(buf+4+my_length,msg,msg_length);
	send(socket,buf,ntohs(total_packet_length),0);
}


void client_request_list(int socket) {
	char buf[3];
	uint16_t packetlength = htons(3);
	uint8_t flag = 10;

	memcpy(buf,&packetlength,2);
	memcpy(buf+2,&flag,1);
	send(socket,buf,3,0);
	cout << "\n$:";
	fflush(stdout);
}









