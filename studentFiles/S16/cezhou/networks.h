// for the server side
int tcp_recv_setup(uint16_t port);
int tcp_accept(int server_socket);

// for the client side
int tcp_send_setup(char *host_name, char *port);
