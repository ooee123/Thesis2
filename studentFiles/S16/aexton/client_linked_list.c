#include "client_linked_list.h"

static client *root_client = NULL;

//       ---- TESTS ----
// int main(int argc, char **argv) {
//
//   printf("--VISUAL TESTS--\n");
//
//   add_client("johnny");
//   print_clients();
//
//   add_client("robert");
//   add_client("jake");
//   print_clients();
//
//   add_client("melissa");
//   add_client("lauren");
//   print_clients();
//
//   remove_client("robert");
//   remove_client("jake");
//   print_clients();
//
//   remove_client("johnny");
//   print_clients();
//
//   remove_client("lauren");
//   print_clients();
//
//   return 0;
// }

client * add_client(char *client_handle, int socket_num) {

  client *new_client = (client *) calloc(1, sizeof(client));
  char *handle = (char *) calloc(strlen(client_handle) + 1, sizeof(char));
  memcpy(handle, client_handle, strlen(client_handle));

  new_client->handle = handle;
  new_client->socket_num = socket_num;
  new_client->next_client = NULL;

  //printf("ADDING: %s, with socket: %u\n", new_client->handle, new_client->socket_num);

  if (root_client == NULL) {
    root_client = new_client;
  } else {

    client *current = root_client;

    while (current != NULL  && current->next_client != NULL) {
      current = current->next_client;
    }

    current->next_client = new_client;

  }

  return new_client;
}

client * search_clients(char *client_handle) {

  client *current = root_client;
  client *found_client = NULL;

  while (current != NULL) {

      if (strcmp(client_handle, current->handle) == 0) {
          found_client = current;
      }

      current = current->next_client;

  }

  return found_client;
}

client * search_clients_by_socket_num(int socket_num) {

  client *current = root_client;
  client *found_client = NULL;

  while (current != NULL) {

      if (socket_num == current->socket_num) {
          found_client = current;
      }

      current = current->next_client;

  }

  return found_client;
}


client * get_root_client() {
  return root_client;
}

uint32_t get_active_client_count() {

  client *current = root_client;
  uint32_t client_count = 0;

  while (current) {
    client_count += 1;
    current = current->next_client;
  }

  return client_count;

}

void remove_client(char *client_handle) {
  client *current = root_client;
  client *previous = NULL;
  client *found_client = NULL;

  while (current != NULL) {

    if (strcmp(client_handle, current->handle) == 0) {
        found_client = current;

        if (previous != NULL) {
          previous->next_client = found_client->next_client;
        } else {
          root_client = found_client->next_client;
        }

        free(found_client->handle);
        free(found_client);
    }

    previous = current;
    current = current->next_client;

  }

  return;
}

void print_clients() {

  //printf("CLIENT LINKED LIST:\n");

  client *current = root_client;
  int nodeNum = 0;

  while (current != NULL) {

    //printf("[%d] HANDLE: %s, with socket: %u\n", nodeNum, current->handle, current->socket_num);
    current = current->next_client;
    nodeNum++;

  }

  return;
}
