/******************************************************************************
 * chat_util.cpp
 *
 * Chris Clark
 * CPE 464
 * Program 2
 *****************************************************************************/

#include "chat_util.h"

void init_packet_read_state(struct packet_read_state *state) {
	state->state = STATE_START;
	state->len_read = 0;
	state->msg_len = 0;
}

int read_packet(int socket_fd, struct packet_read_state *state) {
	int rec_len;
	int data_wanted;
	switch (state->state) {
		case STATE_START:
			init_packet_read_state(state);
			rec_len = safe_recv(socket_fd, state->data, 2, 0);
			state->len_read += rec_len;

			switch (rec_len) {
				case 0:	// socket closed on remote end
					return -1;
				case 1:
					state->state = STATE_PROC_LEN;
					return 0;
				case 2:
					state->state = STATE_GET_DATA;
					state->msg_len = ntohs(*((uint16_t *)state->data));
					return 0;
				default:
					cerr << "Recv has a bug" << endl;
					exit(-1);
			}
			break;
		case STATE_PROC_LEN:
			rec_len = safe_recv(socket_fd, state->data + state->len_read, 1, 0);

			switch (rec_len) {
				case 0:	// socket closed on remote end
					return -1;
				case 1:
					state->state = STATE_GET_DATA;
					state->msg_len = ntohs(*((uint16_t *)state->data));
					state->len_read += rec_len;
					return 0;
				default:
					cerr << "Recv has a bug" << endl;
					exit(-1);
			}
			break;
		case STATE_GET_DATA:
			// special case: when msg_len = 0, just read up to the flag and return
			// this is for the handle stream case
			if (state->msg_len == 0) {
				state->msg_len = 3;
			}

			data_wanted = state->msg_len - state->len_read;

			rec_len = safe_recv(socket_fd, state->data + state->len_read, data_wanted, 0);

			// socket closed on remote end
			if (rec_len == 0) {
				return -1;
			}

			state->len_read += rec_len;

			if (rec_len == data_wanted) {
				state->state = STATE_START;
				return state->msg_len;
			}

			break;
		default:
			cerr << "Bad state (try initializing state before use?)" << endl;
			exit(-1);
	}
	return 0;
}

struct message_header read_header(char *data) {
	struct message_header hdr;
  hdr.msg_len = ((struct message_header*)data)->msg_len;
  hdr.flag = ((struct message_header*)data)->flag;

	return hdr;
}

int read_handle(char *dest, char *data) {
	int handle_length = *((uint8_t *)data);

	memcpy(dest, data + 1, handle_length);
	dest[handle_length] = '\0';

	return handle_length;
}

void parse_direct_message(struct direct_message *dm, char *data) {
  int msg_len = ntohs(read_header(data).msg_len) - sizeof(struct message_header);
  data += sizeof(struct message_header);

	int dest_handle_bytes_read = read_handle(dm->dest_handle, data) + 1;
	data += dest_handle_bytes_read;
	msg_len -= dest_handle_bytes_read;

	int src_handle_bytes_read = read_handle(dm->src_handle, data) + 1;
	data += src_handle_bytes_read;
	msg_len -= src_handle_bytes_read;

	// use memcpy instead of strcpy in case client doesn't null terminate
	memcpy(dm->message, data, msg_len);
}

void parse_broadcast_message(struct broadcast_message *bm, char *data) {
  int msg_len = ntohs(read_header(data).msg_len) - sizeof(struct message_header);
  data += sizeof(struct message_header);

	int src_handle_bytes_read = read_handle(bm->src_handle, data) + 1;
	data += src_handle_bytes_read;
	msg_len -= src_handle_bytes_read;

	// use memcpy instead of strcpy in case client doesn't null terminate
	memcpy(bm->message, data, msg_len);
}

int write_header(char *data, int msg_len, int flag) {
	((uint16_t *)data)[0] = htons((uint16_t)msg_len);
	((uint8_t *)data)[2] = (uint8_t)flag;

	return sizeof(struct message_header);
}

int write_handle(char *data, char *handle) {
	int handle_length = strlen(handle);
	((uint8_t *)data)[0] = (uint8_t)handle_length;
	memcpy(data + 1, handle, handle_length);

	return handle_length + 1;
}

void send_flag(int socket, int flag) {
	struct message_header header;
  header.msg_len = htons(sizeof(struct message_header));
  header.flag = flag;

  safe_send(socket, &header, sizeof(struct message_header), 0);
}

void send_direct_message(int socket, struct direct_message dm) {
	static char msg_buf[MAX_PACKET_SIZE];
	char *ptr = &(msg_buf[0]);
	int msg_len = sizeof(struct message_header) + 1 + strlen(dm.dest_handle) + 1 + strlen(dm.src_handle) + strlen(dm.message) + 1;

	ptr += write_header(ptr, msg_len, FLAG_CLIENT_MESSAGE);
	ptr += write_handle(ptr, dm.dest_handle);
	ptr += write_handle(ptr, dm.src_handle);
	strcpy(ptr, dm.message);

	safe_send(socket, msg_buf, msg_len, 0);
}

void send_broadcast_message(int socket, struct broadcast_message bm) {
	static char msg_buf[MAX_PACKET_SIZE];
	char *ptr = msg_buf;
	int msg_len = sizeof(struct message_header) + 1 + strlen(bm.src_handle) + strlen(bm.message) + 1;

	ptr += write_header(ptr, msg_len, FLAG_BROADCAST);
	ptr += write_handle(ptr, bm.src_handle);
	strcpy(ptr, bm.message);

	safe_send(socket, msg_buf, msg_len, 0);
}

void reset_timer(struct timeval *timer) {
  timer->tv_sec = 1;
  timer->tv_usec = 0;
}

void reset_sockets(list<int> socket_fds, fd_set *sockets) {
  FD_ZERO(sockets);
  list<int>::iterator iter = socket_fds.begin();
  while (iter != socket_fds.end()) {
  	FD_SET(*iter, sockets);
  	++iter;
  }
}

int get_highest_fd(list<int> socket_fds) {
  int highest = 0;
  list<int>::iterator iter = socket_fds.begin();
  while (iter != socket_fds.end()) {
    int cur = (*iter);
    highest = max(highest, cur);
    ++iter;
  }

  return highest;
}

int safe_socket(int domain, int type, int protocol) {
	int res = socket(domain, type, protocol);
	if (res < 0) {
		cerr << "Error on creating socket" << endl;
		exit(-1);
	}
	return res;
}

int safe_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	int res = bind(sockfd, addr, addrlen);
	if (res < 0) {
		cerr << "Error on binding socket (possible invalid port number)" << endl;
		exit(-1);
	}
	return res;
}

int safe_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	int res = getsockname(sockfd, addr, addrlen);
	if (res < 0) {
		cerr << "Error on getting socket name" << endl;
		exit(-1);
	}
	return res;
}

int safe_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
	int res = select(nfds, readfds, writefds, exceptfds, timeout);
	if (res < 0) {
		cerr << "Error on select" << endl;
		exit(-1);
	}
	return res;
}

ssize_t safe_send(int sockfd, const void *buf, size_t len, int flags) {
	//print_bytes(buf, len);

	ssize_t res = send(sockfd, buf, len, flags);
	if (res < 0) {
		cerr << "Error on sending data" << endl;
		exit(-1);
	}
	return res;
}

ssize_t safe_recv(int sockfd, void *buf, size_t len, int flags) {
	ssize_t res = recv(sockfd, buf, len, flags);
	if (res < 0) {
		cerr << "Error on receiving data" << endl;
		exit(-1);
	}
	return res;
}

int safe_listen(int sockfd, int backlog) {
	int res = listen(sockfd, backlog);
	if (res < 0) {
		cerr << "Error on listen" << endl;
		exit(-1);
	}
	return res;
}

int safe_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	int res = accept(sockfd, addr, addrlen);
	if (res < 0) {
		cerr << "Error on accept" << endl;
		exit(-1);
	}
	return res;
}

struct hostent* safe_gethostbyname(const char *name) {
	struct hostent* res = gethostbyname(name);
	if (res == NULL) {
		cerr << "Error on getting host by name" << endl;
		exit(-1);
	}
	return res;
}

int safe_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	int res = connect(sockfd, addr, addrlen);
	if (res < 0) {
		cerr << "Error on connect" << endl;
		exit(-1);
	}
	return res;
}

void print_bytes(const void *data, int len) {
	char *ptr = (char *)data;
	for (int i = 0; i < len; ++i) {
		printf("%hX ", (unsigned short)(*ptr));
		++ptr;
	}
	printf("\n");
}