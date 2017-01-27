#include <vector>

#define HEADER_SIZE 3

// Used for buffer for receiving packets
#define MAX_SIZE 1600

struct chat_client {
   char *handle;  // Dynamically allocated
   int fd;
};

struct chat_header {
   uint16_t message_length;
   uint8_t flag;
} __attribute__((packed));

int process_arguments(int argc, char **argv);
void init_server(int server_socket, fd_set *fds, fd_set *original, int *max_fd);
void look_for_clients(int sk, int *max_fd, fd_set *read_fds, fd_set *original);
void check_sockets(int sk, int *max_fd, fd_set *fds, fd_set *original, std::vector<struct chat_client> *clients);
void get_header(char *buffer, struct chat_header *header);
int process_packet(int client_socket, struct chat_header *header, std::vector<chat_client> *clients);
int handle_new_client(int client_socket, std::vector<chat_client> *clients);
void send_new_client_response(int client_socket, int response);
void client_connected(int sk, std::vector<struct chat_client> *clients, char *buffer);
int check_handles(std::vector<struct chat_client> *clients, char *handle_name);
void handle_list_handles_request(int client_socket, std::vector<struct chat_client> *clients);
int handle_exit_request(int client_socket);
void handle_sent_message(int client_socket, struct chat_header *header, std::vector<struct chat_client> *clients);
int get_handle_index(std::vector<struct chat_client> *clients, char *handle);
void send_message(int client_socket, struct chat_header *header, char *src_handle, char *dest_handle, char *message, int response);
void handle_sent_broadcast(int client_socket, struct chat_header *header, std::vector<struct chat_client> *clients);
void send_broadcast(int client_socket, struct chat_header *header, char *handle, char *message);
void send_to_client(int socket_number, char *data, int data_size);
void recv_from_client(int socket_number, char *data, int data_size);
void remove_fd(int socket_number, std::vector<struct chat_client> *clients, fd_set *fds, fd_set *original, int *max_fd);