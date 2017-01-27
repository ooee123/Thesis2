#define MAX_BUFFER 1024
#define NORMAL_HEADER_LENGTH 2
#define MAX_HANDLE_LENGTH 255
#define BYTES_PER_HEADER 3

// for the server side
int tcp_recv_setup(int requested_port);
int tcp_listen(int server_socket, int back_log);

// for the client side
int tcp_send_setup(char *host_name, char *port);
