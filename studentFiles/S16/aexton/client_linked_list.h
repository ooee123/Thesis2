#ifndef CLIENT_LINKED_LIST
#define CLIENT_LINKED_LIST

  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <stdint.h>

  typedef struct client_type client;

  struct client_type {
    char *handle;
    int socket_num;
    client *next_client;
  };

  client * add_client(char *client_handle, int socket_num);
  client * search_clients(char *client_handle);
  client * search_clients_by_socket_num(int socket_num);
  client * get_root_client();
  uint32_t get_active_client_count();
  void remove_client(char *client_handle);
  void print_clients();

#endif
