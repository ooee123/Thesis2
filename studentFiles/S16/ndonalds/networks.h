// for the server side
int tcp_recv_setup();
int tcp_listen(int, int);
void close_all();
void init_connections();
void accept_connections(int);
int server_parse_flags(int, char *, int, int);
int verify_handle(char *, int, int);
void handle_reply(int, int);
void send_handles();
void remove_client(int);
int get_socket(char *);
void forward_message(char *, int, int);
void forward_broadcast(char *, int, int);

// for the client side
int tcp_send_setup(char *, char *);
void start_connection(int, char *);
void parse_input(int, char *);
void send_message(int, char *, char *, int);
void send_broadcast(int, char *, char *, int);
void send_handle_request(int);
void send_exit(int);
void check_handle(int, char *);
int receive_data(int);
int parse_flags(int, char *, int, int);
void print_handles(int, int);
