
int tcp_send_setup(char *host_name, char *port);
void init_select();
void init_client(char *handle);
void tcp_select();

void decode_input();
void send_message();
void send_broadcast();
void req_list();
void req_exit();

void decode_flags(struct myHeader *head);
void handle_num();
void handle_stream();
void message_err(int packLength);
void rec_broadcast(int packLength);
void rec_message(int packLength);
void closeClient();
