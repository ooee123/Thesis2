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
#include <iostream>
#include <vector>
#include <unistd.h>
#include <algorithm>
using namespace std;

int state;
vector <int> cleanup_list;
vector <int> cleanup_socket_only;

int main(int argc, char *argv[]) {
	state = START;

	int server_socket= 0;   //socket descriptor for the server socket
	//create the server socket
	server_socket= tcp_recv_setup(argc, argv[1]);
	//look for a client to serve
	tcp_listen(server_socket, 5);
	return 0;
}


int tcp_recv_setup(int argc, char* port) {
	int server_socket= 0;
	struct sockaddr_in local;      /* socket address for local side  */
	socklen_t len= sizeof(local);  /* length of local address        */

	/* create the socket  */
	server_socket= socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket < 0) {
		perror("socket call");
		exit(1);
	}

	local.sin_family= AF_INET;         //internet family
	local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
	if (argc == 2)
		local.sin_port = htons(atoi(port));                 //let system choose the port
	else
		local.sin_port = htons(0);

	/* bind the name (address) to a port */
	if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
		perror("bind call");
		exit(-1);
	}

	//get the port name and print it out
	if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
		perror("getsockname call");
		exit(-1);
	}

	printf("socket has port %d \n", ntohs(local.sin_port));

	return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

void tcp_listen(int server_socket, int back_log) {
	vector<int> client_socket_list;
	vector<uint8_t *> handle_list;
	fd_set fdvar;


	if (listen(server_socket, back_log) < 0) {
		perror("listen call");
		exit(-1);
	}

	while(1) {
		select_setup(server_socket,&client_socket_list, &fdvar);
		select(max(max_list(&client_socket_list), server_socket)+1,(fd_set *) &fdvar, (fd_set *) 0,
				(fd_set *) 0, NULL);
		//checks server sockets for intial connections
		check_init_connections(server_socket,fdvar, &client_socket_list);
		//checks existing connections for data packets
		check_data_connections(fdvar, &client_socket_list, &handle_list);
		//clears out closed sockets out of socket list/handle list
		cleanup(&client_socket_list, &handle_list);

	}
}

void select_setup(int server_socket, vector<int> *client_socket_list, fd_set* fdvar) {
	vector<int>::iterator iter;
	//print_list(client_socket_list);

	FD_ZERO(fdvar); // reset variables
	FD_SET(server_socket,fdvar);

	for(iter = (*client_socket_list).begin();
	      iter != (*client_socket_list).end();
	        iter++) {
		FD_SET(*iter,fdvar);
	}

}


void check_init_connections(int server_socket, fd_set fdvar, vector<int> *client_socket_list) {
	int c_sock = 0;

	if (FD_ISSET(server_socket, &fdvar)) {
		if ((c_sock = (accept(server_socket, (struct sockaddr*)0, (socklen_t *)0))) < 0) {
			perror("accept call");
			exit(-1);
		}
		else {
			(*client_socket_list).push_back(c_sock);
			//printf("Accepted client on socket:%d\n",c_sock);
		}
	}
}

void check_data_connections(fd_set fdvar, vector<int> *client_socket_list, vector<uint8_t *> *handle_list) {
	vector<int>::iterator iter;

	for(iter = (*client_socket_list).begin(); iter != (*client_socket_list).end();
			iter++) {

		if( FD_ISSET(*iter, &fdvar)) {
			//printf("caught data from client socket: %d\n",*iter);
			get_packet(*iter,client_socket_list, handle_list);
		}
		while(state == PROCESS_LENGTH || state == GET_DATA)
			get_packet(*iter,client_socket_list, handle_list);
	}
}

void get_packet(int socket, vector<int> *client_socket_list, vector<uint8_t *> *handle_list) {
	static uint8_t *buf=  (uint8_t *) malloc(2048);              //buffer for receiving from client
	static int recv_len= 0;     //length of the received message
	static uint16_t message_len = 0; //parsed mesage len
	int cur = 0;
	switch(state) {
			case START:
				recv_len += recv(socket,buf,2,0 );
				if(recv_len == 2) {
					state=GET_DATA;
					memcpy(&message_len,buf,2);
					message_len = ntohs(message_len);
					message_len -= 2;
				}

				else if(recv_len == 1)
					state = PROCESS_LENGTH;
				else {
					cleanup_list.push_back(socket);
					close(socket);
				}
				break;

			case PROCESS_LENGTH:
				recv_len += recv(socket,buf+1,1,0);
				//alt-c check
				if (recv_len != 2) {
					close(socket);
					cleanup_list.push_back(socket);
					break;
				}
				memcpy(&message_len,buf,2);
				message_len = ntohs(message_len);
				message_len -= 2;

				state = GET_DATA;
				break;

			case GET_DATA:
				if(message_len > 0) {
					cur = recv(socket,buf+recv_len,message_len,0);
					//alt -c check
					if(cur == 0) {
						close(socket);
						cleanup_list.push_back(socket);
						break;
					}
					message_len -= cur;
					recv_len+=cur;
					//message_len -= recv_len;
				}
				//message_len == 0 when done parsing
				if(message_len == 0) {
					process_packet(buf,socket,client_socket_list,handle_list);
					recv_len = 0;
					message_len = 0;
					memset(buf,0,2048);
					state = START;
				}
				break;

			default:
				perror("wrong state");
				exit(-1);
				break;
		}
}

void process_packet(uint8_t *buf, int client_socket, vector<int> *client_socket_list, vector<uint8_t *> *handle_list) {

	 switch (check_flag(buf)) {
			case 1:
				process_handle_packet(buf,client_socket,handle_list);
				break;
			case 4:
				send_broadcast_packet(buf,client_socket,client_socket_list);
				break;

			case 5:
				process_message_packet(buf,client_socket,client_socket_list,handle_list);
				break;
			case 8:
				send_okay_exit(client_socket);
				cleanup_list.push_back(client_socket);
				close(client_socket);
				break;
			case 10:
				send_num_handle(client_socket,handle_list);
				send_handle_list(client_socket, handle_list);
				break;
		}
}


int check_flag(uint8_t *packet) {
	uint8_t flag;
	memcpy(&flag,packet+2,1);
	return flag;
}

void process_handle_packet( uint8_t *buf, int client_socket, vector<uint8_t *> *handle_list) {
	uint8_t handle_length;
	uint8_t *handle;

	memcpy(&handle_length,buf+3,1);
	handle = (uint8_t*) malloc((unsigned int)handle_length+1);
	memcpy(handle,buf+4,handle_length);
	memset(handle+handle_length,'\0',1);

	if(handle_dup_check(handle,handle_list) == -1) {
		(*handle_list).push_back(handle);
		send_okay_handle(client_socket);
	}
	else {
		free(handle);
		//SEND BAD HANDLE AND KILL.
		send_bad_handle(client_socket);
		cleanup_socket_only.push_back(client_socket);
		close(client_socket);
	}
}

void send_okay_handle(int client_socket) {
	char buf[3];
	uint16_t packetlength = htons(3);
	uint8_t flag = 2;

	memcpy(buf,&packetlength,2);
	memcpy(buf+2,&flag,1);
	send(client_socket,buf,3,0);
}

void send_bad_handle(int client_socket) {
	char buf[3];
	uint16_t packetlength = htons(3);
	uint8_t flag = 3;

	memcpy(buf,&packetlength,2);
	memcpy(buf+2,&flag,1);
	send(client_socket,buf,3,0);
}

void send_broadcast_packet(uint8_t *buf, int src_socket, vector<int> *client_socket_list) {
	uint16_t total_packet_length;

	memcpy(&total_packet_length,buf,2);
	total_packet_length = ntohs(total_packet_length);
	for(unsigned int i = 0; i < (*client_socket_list).size();i++) {
		if((*client_socket_list)[i]!=src_socket)
			send( (*client_socket_list)[i],buf,total_packet_length,0);
	}
}

void process_message_packet(uint8_t *buf, int client_socket, vector<int> *client_socket_list, vector<uint8_t *> *handle_list ) {
	uint8_t dest_handle_length;
	uint8_t *dest_handle;
	uint8_t src_handle_length;
	uint8_t *src_handle;
	int dup_check = 0;

	memcpy( &dest_handle_length,buf+3, 1);
	dest_handle = (uint8_t *)malloc(dest_handle_length+1);
	memcpy(dest_handle,buf+4, dest_handle_length );
	memset(dest_handle+dest_handle_length,'\0',1);

	memcpy(&src_handle_length,buf+4+dest_handle_length,1);
	src_handle = (uint8_t *) malloc(src_handle_length+1);
	memcpy(src_handle, buf+4+dest_handle_length+1,src_handle_length);
	memset(src_handle+src_handle_length,'\0',1);

	//now its a good thing theres a duplicate!
	dup_check = handle_dup_check(dest_handle,handle_list);

	if(dup_check != -1)
		send_good_message(buf, dup_check, client_socket_list);
	else
		send_bad_message(client_socket,dest_handle,dest_handle_length);

	free(dest_handle);
	free(src_handle);


}

void send_good_message(uint8_t *buf, int dup_pos,vector<int> *client_socket_list  ) {
	vector<int>::iterator socket_iter;
	uint16_t total_packet_length;

	socket_iter = (*client_socket_list).begin();
	for(int i = 0 ; i < dup_pos; i ++)
		socket_iter ++;

	memcpy(&total_packet_length,buf,2);
	total_packet_length = ntohs(total_packet_length);
	send(*socket_iter,buf,total_packet_length,0);
}

void send_bad_message(int client_socket, uint8_t *handle, uint8_t handle_length ) {
	uint16_t total_packet_length = htons(3 + 1 + handle_length);
	uint8_t *buf = (uint8_t *) malloc(ntohs(total_packet_length));
	uint8_t flag = 7;

	memcpy(buf,&total_packet_length,2 );
	memcpy(buf+2, &flag,1);
	memcpy(buf+3, &handle_length,1);
	memcpy(buf+4,handle,handle_length);

	send(client_socket,buf,ntohs(total_packet_length),0);

	free(buf);
}

void send_okay_exit(int client_socket) {

	char buf[3];
	uint16_t packetlength = htons(3);
	uint8_t flag = 9;

	memcpy(buf,&packetlength,2);
	memcpy(buf+2,&flag,1);
	send(client_socket,buf,3,0);
}

void send_num_handle(int socket, vector<uint8_t *> *handle_list ) {
	char buf[7];

	uint16_t packetlength = htons(7);
	uint8_t flag = 11;
	uint32_t num_handle = htonl((*handle_list).size());
	memcpy(buf,&packetlength,2);
	memcpy(buf+2,&flag,1);
	memcpy(buf+3,&num_handle,4);


	send(socket,buf,7,0);
}

void send_handle_list(int socket, vector<uint8_t *> *handle_list) {
	uint8_t *buf;
	int buf_size = 0;
	int counter = 3;
	uint8_t handle_length = 0;

	uint8_t flag = 12;
	uint16_t packetlength = 0;

	for(unsigned int i = 0; i < (*handle_list).size();i++) {
		buf_size += string_length((*handle_list)[i]);
		buf_size ++;
	}
	buf = (uint8_t *) malloc(buf_size+3);
	memcpy(buf,&packetlength,2);
	memcpy(buf+2,&flag,1);
	for(unsigned int i = 0; i < (*handle_list).size();i++) {
		handle_length = string_length((*handle_list)[i]);
		memcpy(buf+counter,&handle_length, 1);
		counter++;
		memcpy(buf+counter,(*handle_list)[i],handle_length);
		counter += handle_length;
	}

	send(socket,buf,buf_size+3,0);
	free(buf);
}

int handle_dup_check(uint8_t *handle, vector<uint8_t *> *handle_list  ) {
	vector<uint8_t *> :: iterator handle_iter;
	int counter = 0;

	for(handle_iter = (*handle_list).begin(); handle_iter != (*handle_list).end();
				handle_iter++) {
		if(string_compare(handle,*handle_iter) == 0)
			return counter;
		counter++;
	}
	return -1;
}

int string_compare(uint8_t *s1, uint8_t *s2) {
	int ctr=0;

	while(s1[ctr]==s2[ctr]) {
		if(s1[ctr]=='\0'||s2[ctr]=='\0')
			break;
		ctr++;
	}
	if(s1[ctr]=='\0' && s2[ctr]=='\0')
		return 0;
	else
		return -1;
}

int string_length(uint8_t *s) {
   int c = 0;
   while (s[c] != '\0')
      c++;
   return c;
}

int max_list(vector<int> *client_socket_list) {
	vector<int>::iterator iter;
	int max = -1;
	for(iter = (*client_socket_list).begin(); iter != (*client_socket_list).end();
			iter++) {
					if(*iter > max)
						max = *iter;

			}
	return max;
}

//cleanup, cleanup, everybody  cleanup
void cleanup(vector<int> *client_socket_list, vector<uint8_t *> *handle_list) {
	vector<int>::iterator clean_iter;
	vector<int>::iterator socket_iter;
	vector<uint8_t *>::iterator handle_iter;

	for(unsigned int i = 0; i < cleanup_socket_only.size(); i ++) {
		socket_iter = find( (*client_socket_list).begin() , (*client_socket_list).end(),cleanup_socket_only[i]);
		(*client_socket_list).erase(socket_iter);
	}

	for(clean_iter = cleanup_list.begin(); clean_iter != cleanup_list.end();
				clean_iter++) {

		socket_iter = find( (*client_socket_list).begin() , (*client_socket_list).end(),*clean_iter);
		(*client_socket_list).erase(socket_iter);

			handle_iter = (*handle_list).begin();
			for (int i = 0; i < socket_iter - (*client_socket_list).begin();i++) {
				handle_iter++;
			}
			free(*handle_iter);
			(*handle_list).erase(handle_iter  );

	}
	cleanup_list.clear();
	cleanup_socket_only.clear();
}

