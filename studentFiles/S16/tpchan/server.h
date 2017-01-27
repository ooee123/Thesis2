int tcp_recv_setup(int16_t portNum);
void tcp_listen();
int tcp_accept();

void insertClient(int client_socket);
void deleteClient(int client_socket);
struct client *findClientSocket(int socket);
struct client *findClientHandle(char *handle);
void updateMax();

void tcp_select();
struct myHeader *tcp_recieve_header(int socketNum);
void decode_flags(int socketNum, struct myHeader *head);
char *createHeader(uint16_t length, uint8_t flag);

void init_client(int socketNum, int packLength);
void broadcast_message(int socketNum, int packLength);
void message(int socketNum, int packLength);
void closeClient(int client_socket);
void send_handles(int socketNum);
