#define HEADER_SIZE 3
#define MAX_MESSAGE 1000

typedef struct {
    uint16_t messageLength;
    uint8_t flag;
} message_header;

int process_arguments(int argc, char **argv);
void send_connection_request(int socket_number, char *handle);
int get_connection_response(int socket_number);
void init_client(int socket_number, fd_set *fds, fd_set *original);
void data_ready(int socket_number, fd_set *fds);
void get_server_data(int socket_number, fd_set *fds);
void quit_chat(int socket_number);
int get_handle_count(int socket_number);
void print_handles(int socket_number, unsigned int handle_count);
void handle_receive_broadcast(int socket_number, uint16_t message_length);
void handle_receive_message(int socket_number, uint16_t message_length);
void handle_receive_message_bad(int socket_number);
void get_user_input(int socket_number, char *handle, fd_set *fds, fd_set *original);
void handle_list_handles_command(int socket_number);
void handle_exit_command(int socket_number);
void handle_send_message(int socket_number, char *handle, char *buffer, int buffer_length);
void handle_send_broadcast(int socket_number, char *handle, char *buffer, int buffer_length);
void send_to_server(int socket_number, char *data, int data_size);
void recv_from_server(int socket_number, char *data, int data_size);