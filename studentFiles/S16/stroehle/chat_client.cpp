/******************************************************************************
* tcp_client.c
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "testing.h"
#include "chat_client_funcs.h"

char buffer[1100]; 
char input_buffer[1100];
char temp_handle[255];
int server_socket;
char *handle;



int get_dst_len(char *data, int len) {

  int i = 0;
  while(data[i] != ' ') {
    i++;
    if(data[i] == 0)
      return -1;
  }
  int j = i+1;
  while(data[j] != ' ') {
    j++;
    if(data[j] == 0)
      return j - (i+1);
  }
  return j - (i+1);
}


void process_user_input(char *input_buffer) {

  if(input_buffer[0] != '%') {  
    printf("Incorrect Command\n");
  } else if(input_buffer[1] == 'B' || input_buffer[1] == 'b' ) {
    send_broadcast_packet(input_buffer + 3);
  } else if(input_buffer[1] == 'M' || input_buffer[1] == 'm' ) {

    int dest_handle_len = get_dst_len(input_buffer, strlen(input_buffer));
    if(dest_handle_len != -1) {
      
      if(dest_handle_len > 255) {
        printf("\nHandle cannot be more than 255 characters\n");
      } else {
        send_msg_packet(input_buffer + 3, dest_handle_len);
      }
    } else {
      printf("Bad input. Usage: %%M <destination handle> <message>\n");
    }
  } else if(input_buffer[1] == 'L' || input_buffer[1] == 'l' ) { 
    send_handle_request_packet();
  } else if(input_buffer[1] == 'E' || input_buffer[1] == 'e' ) {
    send_exit_ack_packet();
  } else {
    printf("Invalid Command\n");
  }
}

void get_user_input() {

  int send_len = 0;
  while ((input_buffer[send_len] = getchar()) != '\n') {
    send_len++;
    if(send_len == 1000) {
      input_buffer[send_len] = '\0';
      process_user_input(input_buffer);
      if(input_buffer[1] == 'M' || input_buffer[1] == 'm' ) {
        int dest_handle_len = get_dst_len(input_buffer, strlen(input_buffer));
        send_len = 3 + dest_handle_len + 1;
      }
      else {
        send_len = 3;
      }
    }
  }

  input_buffer[send_len] = '\0';
  process_user_input(input_buffer);
  printf("$: ");
  fflush(stdout);
}

void chat_select() {

  fd_set srvr_and_stdout,temp;
  FD_ZERO(&srvr_and_stdout);
  FD_ZERO(&temp);

  FD_SET(server_socket, &srvr_and_stdout);
  FD_SET(2, &srvr_and_stdout);

  while(1) {

    temp = srvr_and_stdout;
    if(select(server_socket + 1, &temp, NULL, NULL, NULL)  < 0) {
      perror("select call");
      exit(-1);
    }

    if(FD_ISSET(2, &temp)) {
      get_user_input();
    }

    if(FD_ISSET(server_socket, &temp)) {
      get_packet();
      printf("$: ");
      fflush(stdout);
    }
  }
}

void send_msg_packet(char *data, int dst_handle_len) {

  uint8_t src_handle_len = strlen(handle); 
  uint32_t msg_len = strlen(data + dst_handle_len + 1); 

  // sizeofpacket + flag + len of dst handle + dst handle
  // + len of src handle + src handle + msg + null terminator
  uint16_t packet_length = 2 + 1 + 1 + dst_handle_len +
  1 + src_handle_len + msg_len + 1;
  uint16_t net_ord_pl = htons(packet_length);
  memcpy(buffer, &net_ord_pl, 2);
  buffer[2] = PRIVATE_MESSAGE;
  buffer[3] = dst_handle_len;
  memcpy(buffer + 4, data, dst_handle_len);
  buffer[4+dst_handle_len] = src_handle_len;
  memcpy(buffer + 4 + dst_handle_len + 1, handle, src_handle_len);
  memcpy(buffer + 4 + dst_handle_len + 1 + src_handle_len, 
  &data[dst_handle_len+1], msg_len+1);

  send_packet(buffer, packet_length);   
}

void send_broadcast_packet(char *data) {

  uint8_t handle_len = strlen(handle); 
  uint32_t msg_len = strlen(data); 

  // sizeofpacket + flag + length of handle + handle
  // + msg + null terminator
  uint16_t packet_length = 2 + 1 + 1 + handle_len + msg_len + 1;
  uint16_t net_ord_pl = htons(packet_length);

  memcpy(buffer, &net_ord_pl, 2);
  buffer[2] = BROADCAST;
  buffer[3] = handle_len;
  memcpy(buffer + 4, handle, handle_len);
  memcpy(buffer + 4 + handle_len, data, msg_len+1);

  send_packet(buffer, packet_length);    
}

void send_handle_request_packet() {

  uint16_t net_ord_pl = htons(3);
  memcpy(buffer, &net_ord_pl, 2);
  buffer[2] = PRINT_HANDLES;

  send_packet(buffer, 3);    
}
  
void send_exit_ack_packet() {

  uint16_t net_ord_pl = htons(3);
  memcpy(buffer, &net_ord_pl, 2);
  buffer[2] = CLIENT_EXITING;

  send_packet(buffer, 3);    
}

void read_broadcast_packet(char *data) {

  char *sender_handle;
  uint8_t handle_len = data[3];
  sender_handle = temp_handle;
  memcpy(sender_handle, data + 4, handle_len);
  sender_handle[handle_len] = 0;

  printf("\n%s: %s\n", sender_handle, &(data[4 + handle_len]));

}

void read_bad_dest_handle_packet(char *data) {

  char *bad_handle;
  uint8_t handle_len = data[3];
  bad_handle = temp_handle;
  memcpy(bad_handle, data + 4, handle_len);
  bad_handle[handle_len] = 0;

  printf("\nClient with handle %s does not exist.\n", bad_handle);
}    

void read_message_packet(char *data) {

  char *src_handle;
  uint8_t dst_handle_len = data[3];
  uint8_t src_handle_len = data[3+dst_handle_len+1];

  src_handle = temp_handle;

  memcpy(src_handle, data + 4 + dst_handle_len + 1, src_handle_len);

  src_handle[src_handle_len] = 0;

  printf("\n%s: %s\n", src_handle,
  &(data[3 + dst_handle_len + 1 + src_handle_len + 1]));
}

void read_handle_packets(int handles_to_read)
{
  if (recv(server_socket, buffer, 3, 0) < 0) {
    perror("get_packet: recv call");
    exit(-1);
  }

  if(buffer[2] != ALL_HANDLES) {
    printf("Did not receive a handles packet\n");
    exit(-1);
  }
  
  uint8_t handle_len;
  for(int i = 0; i < handles_to_read; i++) {
    if (recv(server_socket, &handle_len, 1, 0) < 0) {
      perror("get_packet: recv call");
      exit(-1);
    }
    if (recv(server_socket, buffer, handle_len, 0) < 0) {
      perror("get_packet: recv call");
      exit(-1);
    }

    buffer[handle_len] = 0;

    printf("\t%s\n", buffer);
  }
}

void read_num_handles_packet(char *data) {
  uint32_t num_handles;
  memcpy(&num_handles, data + 3, 4);
  num_handles = ntohl(num_handles);

  printf("\nThere are %d users, here are their handles:\n", num_handles);

  read_handle_packets(num_handles);
}

void read_exit_ack_packet()
{
  close(server_socket);
  exit(0);
}

void get_packet() {

  uint16_t pkt_len;
  int msg_len;
  if ((msg_len = recv(server_socket, &pkt_len, 2, 0)) < 0) {
    perror("get_packet: recv call");
    exit(-1);
  }
  else if(msg_len == 0) {
    printf("Server Terminated\n");
    exit(1);
  }

  pkt_len = ntohs(pkt_len);

  if (recv(server_socket, buffer + 2, pkt_len - 2, 0) < 0) {
    perror("get_packet: recv call");
    exit(-1);
  }

  switch(buffer[2]) {
    case GOOD_HANDLE:
      break;
    case BAD_HANDLE:
      printf("Handle is already in use: %s\n",handle);
      exit(-1);
      break;
    case BROADCAST:
      read_broadcast_packet(buffer);
      break;
    case PRIVATE_MESSAGE:
      read_message_packet(buffer);
      break;
    case BAD_DEST_HANDLE:
      read_bad_dest_handle_packet(buffer);
      break;
    case EXIT_ACK:
      read_exit_ack_packet();       
      break;
    case NUM_HANDLES:
      read_num_handles_packet(buffer);           
      break;
  }
}


void send_intial_packet() {   

  uint8_t handle_len = strlen(handle); 
  // sizeofpacket + flag + length of string + string
  uint16_t packet_length = 2 + 1 + 1 + handle_len;
  uint16_t net_ord_pl = htons(packet_length);

  memcpy(buffer, &net_ord_pl, 2);
  buffer[2] = SEND_HANDLE;
  buffer[3] = handle_len;
  memcpy(buffer + 4, handle, handle_len);

  send_packet(buffer, packet_length);    
  get_packet();
  printf("$: ");
  fflush(stdout);
}

void send_packet(char *packet, int length) {

  int sent = send(server_socket, packet, length, 0);
  if(sent < 0) {
    perror("send call");
    exit(-1);
  }
}

int main(int argc, char * argv[]) {

/* check command line arguments  */
  if(argc!= 4) {
   printf("usage: %s handle host-name port-number \n", argv[0]);
   exit(1);
  }

  if(strlen(argv[1]) > 255) {
    printf("handle name is too long, must 255 characters or less\n");
    exit(1);
  }
  handle = argv[1];
  /* set up the socket for TCP transmission  */
  server_socket= tcp_send_setup(argv[2], argv[3]);
  send_intial_packet();
  chat_select();
  return 0;    
}



int tcp_send_setup(char *host_name, char *port) {

  int socket_num;
  struct sockaddr_in remote;       // socket address for remote side
  struct hostent *hp;              // address of remote host

  // create the socket
  if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket call");
    exit(-1);
  }


  // designate the addressing family
  remote.sin_family= AF_INET;

  // get the address of the remote host and store
  if ((hp = gethostbyname(host_name)) == NULL) {
    printf("Error getting hostname: %s\n", host_name);
    exit(-1);
  }

  memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

  // get the port used on the remote side and store
  remote.sin_port= htons(atoi(port));

  if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0) {
    perror("connect call");
    exit(-1);
  }

  return socket_num;
}

