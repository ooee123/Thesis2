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

#include "server.h"
#include "libChat.h"
#include "testing.h"

int server_socket = 0;
struct client *head = NULL;
uint64_t clientCount = 0;
int maxfd;
fd_set active_set;

int main (int argc, char *argv[])
{
   if (argc == 1) 
   {
      server_socket = tcp_recv_setup(0);
   }
   else if (argc == 2) 
   {
      server_socket = tcp_recv_setup((int16_t) atoi(argv[1]));
   }
   else
   {
      fprintf(stderr, "Too many arguements for server\n");
      exit(-1);
   }
   tcp_listen();

   tcp_select();

   return 0;  
}   

int tcp_recv_setup(int16_t portNum)
{
   int server_socket = 0;
   struct sockaddr_in local;
   socklen_t len = sizeof(local);

   server_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (server_socket < 0)
   {
      perror("socket call");
      exit(1);
   }

   local.sin_family = AF_INET;
   local.sin_addr.s_addr = INADDR_ANY;
   local.sin_port = htons(portNum);
   
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
   {
      perror("bind call");
      exit(-1);
   }
   
   if (getsockname(server_socket, (struct sockaddr *) &local, &len) < 0)
   {
      perror("getsockname call");
      exit(-1);
   }

   printf("Server is using port %d\n", ntohs(local.sin_port));
   maxfd = server_socket;
     
   return server_socket;
}

void tcp_listen()
{
   if (listen(server_socket, 5) < 0)
   {
      perror("listen call");
      exit(-1);
   }
}
int tcp_accept()
{
   int client_socket = 0;

   if ((client_socket = accept(server_socket, NULL, NULL)) < 0)
   {
      perror("accept call");
      exit(-1);
   }
      
   insertClient(client_socket);
   return client_socket;
}

void insertClient(int client_socket)
{
   struct client *insert = calloc(1, sizeof(client));
   
   insert->socket = client_socket;
   if (head == NULL)
   {
      head = insert;
      head->next = NULL;
   }
   else
   {
      insert->next = head;
      head = insert;
   }
   ++clientCount;
   if (client_socket > maxfd)
      maxfd = client_socket;
}

void deleteClient(int client_socket)
{
   struct client *temp = head;
   struct client *prev;
   
   while (temp)
   {
      if (temp->socket == client_socket)
      {
         if (temp == head)
            head = temp->next;
         else
            prev->next = temp->next;
         --clientCount;
         return;
      }
      prev = temp;
      temp = temp->next;
   }
}

struct client *findClientSocket(int socket)
{
   struct client *temp = head;
   
   while (temp)
   {
      if (temp->socket == socket)
         return temp;
      temp = temp->next;
   }
   return NULL;
}

struct client *findClientHandle(char *handle)
{
   struct client *temp = head;
   
   while (temp)
   {
      if (!strcmp(temp->handle, handle))
         return temp;
      temp = temp->next;
   }
   return NULL;
}

void updateMax()
{
   maxfd = server_socket;
   struct client *temp = head;

   while (temp)
   {
      if (temp->socket > maxfd)
         maxfd = temp->socket;
      temp = temp->next;
   }
}

void tcp_select()
{
   fd_set read_set;
   int counter;
   int socket_num;

   FD_ZERO(&active_set);
   FD_SET(server_socket, &active_set);   

   while (1)
   {
      read_set = active_set;
      if (select(maxfd + 1, &read_set, NULL, NULL, NULL) < 0)
      {
         perror ("select call");
         exit(-1);
      }
      for (counter = 0; counter <= maxfd;  ++counter)
      {
         if (FD_ISSET(counter, &read_set))
         {
            if(counter == server_socket)
            {
               socket_num = tcp_accept();
               FD_SET(socket_num, &active_set);
            }
            else
            {  
               decode_flags(counter, tcp_recieve_header(counter));
            }
         }
      }
   }
}

void decode_flags(int socketNum, struct myHeader *head)
{
   if (!head)
   {
      deleteClient(socketNum);
      return;
   }
   switch(head->flag)
   {
      case CS_INIT:
         init_client(socketNum, head->length - HEADER_SIZE);
         break;
      case BROADCAST:
         broadcast_message(socketNum, head->length - HEADER_SIZE); 
         break;
      case MESSAGE:
         message(socketNum, head->length - HEADER_SIZE); 
         break;
      case CS_EXIT:
         closeClient(socketNum);
         break;
      case CS_HANDLES:
         send_handles(socketNum);
         break;
   }
   free(head);
}

void init_client(int socketNum, int packLength)
{
   char *buf;
   struct client *client = findClientSocket(socketNum); 
   char handleCheck[MAX_HANDLE_LEN];

   if (!client)
   {
      fprintf(stderr, "Client not found: init\n");
      exit(-1);
   }
   buf = getPacket(socketNum, packLength);
   if (!buf)
   {
      deleteClient(socketNum);
      return;
   }
  
   memcpy(&client->handle_len, buf, 1);
   memcpy(&handleCheck, buf + 1, client->handle_len);
   
   handleCheck[client->handle_len] = '\0';
   
   free(buf);
   if (findClientHandle(handleCheck))
   {
      buf = createHeader(HEADER_SIZE, SC_INIT_ERR);
      deleteClient(socketNum);
   }
   else
   {
      strcpy(client->handle, handleCheck);
      buf = createHeader(HEADER_SIZE, SC_INIT);
   }
   send_e(socketNum, buf, HEADER_SIZE, 0);
   free(buf);
}
    
void broadcast_message(int socketNum, int packLength)
{
   uint8_t handle_len;
   char handle[MAX_HANDLE_LEN];
   char *buf = getPacket(socketNum, packLength);
   char *header = createHeader(packLength + HEADER_SIZE, BROADCAST);
   struct client *temp = head;
 
   if (!buf)
   {
      deleteClient(socketNum);
      return;
   }  
   memcpy(&handle_len, buf, 1);
   memcpy(&handle, buf + 1, handle_len);
   handle[handle_len] = '\0';
   
   while (temp)
   {
      if (strcmp(temp->handle, handle))
      {
         send_e(temp->socket, header, HEADER_SIZE, 0);
         send_e(temp->socket, buf, packLength, 0);
      }
      temp = temp->next;
   }
   free(header);
   free(buf);
}
      
void message(int socketNum, int packLength)
{
   char *buf = getPacket(socketNum, packLength);
   char *header = createHeader(packLength + HEADER_SIZE, MESSAGE);
   char handle[MAX_HANDLE_LEN];
   uint8_t handle_len;
   struct client *to;

   if (!buf)
   {
      deleteClient(socketNum);
      return;
   }
   
   memcpy(&handle_len, buf, 1);
   memcpy(&handle, buf + 1, handle_len);
   handle[handle_len] = '\0';

   to = findClientHandle(handle);

   if (to)
   {
      send_e(to->socket, header, HEADER_SIZE, 0);
      send_e(to->socket, buf, packLength, 0);
   }
   else
   {
      free(header);
      header = createHeader(HEADER_SIZE + handle_len + 1, SC_DEST_ERR);
      send_e(socketNum, header, HEADER_SIZE, 0);
      send_e(socketNum, buf, handle_len + 1, 0);
   }
   free(header);
   free(buf);
}

void closeClient(int socketNum)
{
   char *header = createHeader(HEADER_SIZE, SC_EXIT);

   deleteClient(socketNum);
   send_e(socketNum, header, HEADER_SIZE, 0);

   free(header);
}

void send_handles(int socketNum)
{
   char *header = createHeader(HEADER_SIZE + 4, SC_HANDLES);
   char *buf = calloc(sizeof(char), 4);
   struct client *temp = head;
   
   memcpy(buf, &clientCount, 4);
   send_e(socketNum, header, HEADER_SIZE, 0);
   send_e(socketNum, buf, 4, 0);
   
   free(header);
   header = createHeader(0, SC_HANDLES_STREAM);
   send_e(socketNum, header, HEADER_SIZE, 0);
   buf = realloc(buf, MAX_HANDLE_LEN);
   while(temp)
   {
      memcpy(buf, &temp->handle_len, 1);
      memcpy(buf + 1, &temp->handle, temp->handle_len);
      send_e(socketNum, buf, temp->handle_len + 1, 0);
      temp = temp->next;
   }
   free(header);
   free(buf);
}
