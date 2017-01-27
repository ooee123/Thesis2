#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>

#include "libChat.h"
#include "cclient.h"
#include "testing.h"

int client_socket;
fd_set active_set;
char client_handle_buf[MAX_HANDLE_LEN];
int client_handle_size = 0;
uint64_t clientCount;

int main(int argc, char *argv[])
{
   if (argc != 4)
   {
      fprintf(stderr, "Either not enough arguements, or too many\n");
      exit(-1);
   }
   client_socket = tcp_send_setup(argv[2], argv[3]);

   init_client(argv[1]);
   tcp_select();

   return 0;  
}

int tcp_send_setup(char *host_name, char *port)
{
   int socket_num;
   struct sockaddr_in remote;
   struct hostent *hp;

   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket call");
      exit(-1);
   }

   remote.sin_family = AF_INET;
   
   if ((hp = gethostbyname(host_name)) == NULL)
   {
      printf("Error getting host name: %s\n", host_name);
      exit(-1);
   }

   memcpy((char *)&remote.sin_addr, (char *)hp->h_addr, hp->h_length);
   remote.sin_port = htons(atoi(port));

   if (connect(socket_num, (struct sockaddr *)&remote, sizeof(struct sockaddr_in)) < 0)
   {
      perror("connect call");
      exit(-1);
   }
   return socket_num;
}

void init_client(char *handle)
{
   char *header;
   int getLen = strlen(handle);
   uint8_t handle_len;
   struct myHeader *myHead;

   if (getLen >= MAX_HANDLE_LEN)
      handle_len = MAX_HANDLE_LEN - 1;
   else
      handle_len = getLen;

   memcpy(client_handle_buf, &handle_len, 1);
   memcpy(client_handle_buf + 1, handle, handle_len);
   client_handle_size = handle_len + 1;
   
   header = createHeader(HEADER_SIZE + client_handle_size, CS_INIT);
   send_e(client_socket, header, HEADER_SIZE, 0);
   send_e(client_socket, client_handle_buf, handle_len + 1, 0);
  
   myHead = tcp_recieve_header(client_socket);
   if (!myHead)
   {
      fprintf(stderr, "Server Terminated\n");
      closeClient();
   }
   if(myHead->flag == SC_INIT_ERR)
   {
      fprintf(stderr, "Handle already in use: %s\n", handle);
      closeClient();
   }
   else if (myHead->flag != SC_INIT)
   {
      fprintf(stderr, "Sever first response not INIT Response\n");
      closeClient();
   }
}

void tcp_select()
{
   fd_set read_set;
   FD_ZERO(&active_set);
   FD_SET(client_socket, &active_set);
   FD_SET(STDIN_FILENO, &active_set);

   printf("$: ");
   fflush(stdout);
   while (1)
   {
      read_set = active_set;
      if (select(client_socket+ 1, &read_set, NULL, NULL, NULL) < 0)
      {
         perror("select call");
         exit(-1);
      }
      if (FD_ISSET(STDIN_FILENO, &read_set))
      {
         decode_input();
      }
      if (FD_ISSET(client_socket, &read_set))
      {
         decode_flags(tcp_recieve_header(client_socket)); 
      }
      fflush(stdout);
   }
}

void decode_input()
{
   char *command = calloc(sizeof(char), 3);
   int counter = 0;

   while ((command[counter++] = getchar()) != '\n' && counter < 2);
   command[counter] = '\0';

   if (command[0] == '%')
   {
      switch (command[1])
      {
         case 'M':
            send_message();
            break;
         case 'm':
            send_message();
            break;
         case 'B':
            send_broadcast();
            break;
         case 'b':
            send_broadcast();
            break;
         case 'L':
            req_list();
            break;
         case 'l':
            req_list();
            break;
         case 'E':
            req_exit();
            break;
         case 'e':
            req_exit();
            break;
         default:
            fprintf(stderr, "Invalid command\n");
            while (getchar() != '\n'); 
            break;
       }
   }
   else if (command[0] != '\n')
   {
      fprintf(stderr, "Invalid command\n");
      while (getchar() != '\n');
   }
   printf("$: ");
   fflush(stdin);
   free(command);
}
                   
void send_message()
{
   char *send_buf = calloc(sizeof(char), 1000);
   char *handle = calloc(sizeof(char), MAX_HANDLE_LEN);
   char *header;
   uint8_t handle_len = 0;
   int message_len = 1;
   char check = 1;

   if (getchar() == '\n')
   {
      fprintf(stderr, "Invalid command\n$: ");
      return;
   }
   while ((handle[handle_len + 1] = getchar()) != ' ' && handle_len < MAX_HANDLE_LEN - 1)
   {
      ++handle_len;
      if (handle[handle_len] == '\n')
      {
         fprintf(stderr, "Invalid command\n$: ");
         return;
      }
   }
   handle[0] = (char) handle_len; 

   while (check)
   {
      message_len = 0;
      while((send_buf[message_len] = getchar()) != '\n' && message_len < 999)
         ++message_len;
      if (send_buf[message_len] == '\n')
         check = 0;
      send_buf[message_len] = '\0';

      header = createHeader(HEADER_SIZE + handle_len + 1 + client_handle_size + message_len + 1, MESSAGE);
      send_e(client_socket, header, HEADER_SIZE, 0);
      send_e(client_socket, handle, handle_len + 1, 0);
      send_e(client_socket, client_handle_buf, client_handle_size, 0);
      send_e(client_socket, send_buf, message_len + 1, 0);
   }
   free(send_buf);
   free(handle);
}    
   
void send_broadcast()
{
   char *send_buf = calloc(sizeof(char), 1000);
   char *header;
   int message_len = 0;
   char check = 1;
   
   if(getchar() == '\n')
   {
      check = 0;
      send_buf[0] = '\0';
      header = createHeader(HEADER_SIZE + client_handle_size + message_len + 1, BROADCAST);
      send_e(client_socket, header, HEADER_SIZE, 0);
      send_e(client_socket, client_handle_buf, client_handle_size, 0);
      send_e(client_socket, send_buf, 1, 0);
   }  
   while (check)
   {
      message_len = 0;
      while((send_buf[message_len] = getchar()) != '\n' && message_len < 999)
         ++message_len;
      if (send_buf[message_len] == '\n')
         check = 0;
      send_buf[message_len] = '\0';

      header = createHeader(HEADER_SIZE + client_handle_size + message_len + 1, BROADCAST);
      send_e(client_socket, header, HEADER_SIZE, 0);
      send_e(client_socket, client_handle_buf, client_handle_size, 0);
      send_e(client_socket, send_buf, message_len + 1, 0);
   }
   free(send_buf);
}      

void req_list()
{
   char *header = createHeader(HEADER_SIZE, CS_HANDLES);
   
   send_e(client_socket, header, HEADER_SIZE, 0);
}

void req_exit()
{
   char *header = createHeader(HEADER_SIZE, CS_EXIT);
   
   send_e(client_socket, header, HEADER_SIZE, 0);
   FD_CLR(STDIN_FILENO, &active_set);
}

void decode_flags(struct myHeader *head)
{
   if (!head)
   {
      fprintf(stderr, "Server Terminated\n");
      closeClient();
   }
   switch(head->flag)
   {
      case BROADCAST:
         rec_broadcast(head->length - HEADER_SIZE);
         break;
      case MESSAGE:
         rec_message(head->length - HEADER_SIZE);
         break;
      case SC_DEST_ERR:
         message_err(head->length - HEADER_SIZE);
         break;
      case SC_EXIT:
         closeClient();
         break;
      case SC_HANDLES:
         handle_num();
         break;
      case SC_HANDLES_STREAM:
         handle_stream();
         break;
   }
}

void handle_num()
{
   char *buf = getPacket(client_socket, 4);

   memcpy(&clientCount, buf, 4);
  
   printf("\nNumber of clients: %u\n", (unsigned int) clientCount);
}

void handle_stream()
{
   char *buf;
   uint8_t handle_len;
   char *handle;
   int counter;
   
   for (counter = 0; counter < clientCount; ++counter)
   {
      buf = getPacket(client_socket, 1);
      handle = calloc(sizeof(char), MAX_HANDLE_LEN);
      memcpy(&handle_len, buf, 1);
      free(buf);
      buf = getPacket(client_socket, handle_len);
      memcpy(handle, buf, handle_len);
      handle[handle_len] = '\0';
      printf("  %s\n", handle);
      free(buf);
      free(handle);
   }
   printf("$: ");
}

void message_err(int packLength)
{
   char *buf = getPacket(client_socket, packLength);
   uint8_t handle_len;
   uint8_t handle_count = 0;

   if (!buf)
   {
      fprintf(stderr, "Server Terminated\n");
      closeClient();
   }
   
   memcpy(&handle_len, buf, 1);
    
   fprintf(stderr, "\nClient with handle ");
   while (handle_count < handle_len)
      fprintf(stderr, "%c", buf[++handle_count]);
   fprintf(stderr, " does not exist\n$: ");
   free(buf);
}
   
void rec_broadcast(int packLength)
{
   char *buf = getPacket(client_socket, packLength);
   uint8_t handle_len;
   uint8_t handle_count = 0;

   if (!buf)
   {
      fprintf(stderr, "\nServer Terminated\n");
      closeClient();
   }
   
   memcpy(&handle_len, buf, 1);
   printf("\n");
   while (handle_count < handle_len)
      printf("%c", buf[++handle_count]);
   printf(": ");
   
   printf("%s\n$: ", buf + handle_len + 1);
   free(buf);
}

void rec_message(int packLength)
{
   char *buf = getPacket(client_socket, packLength);
   uint8_t handle_len;
   uint8_t handle_count = 0;

   if (!buf)
   {
      fprintf(stderr, "\nServer Terminated\n");
      closeClient();
   }
   
   memcpy(&handle_len, buf + client_handle_size, 1);
   printf("\n");
   while (handle_count < handle_len)
      printf("%c", buf[++handle_count + client_handle_size]);
   printf(": ");

   printf("%s\n$: ", buf + client_handle_size + handle_len + 1);
   free(buf);
}

void closeClient()
{
   close(client_socket);
   exit(0);
}
