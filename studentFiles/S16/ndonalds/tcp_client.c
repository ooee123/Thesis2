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
#include <ctype.h>

#include "networks.h"
#include "testing.h"


#define DATA_MAX 1000
#define NORMAL_HEADER 3

void print_handles(int size, int socket) {
   static int num_handles;
   char c, buf[256];
   int i, count, length, rec_bytes;

   if (size > 0)//if size, coming from flag 11
      num_handles = size;
   else if (size != -1)
      num_handles = 0;
   //printf("num_handles is %d\n", num_handles);

   if (size == -1) {//coming from recv
      printf("\b\b\bNumber of clients: %d\n", num_handles);
      rec_bytes = recv(socket, &c, 1, 0);//ignore flag
      if (!rec_bytes) {
         printf("Server Terminated\n");
         exit(-1);
      }
      
      for (i = 0; i < num_handles; i++) {
         rec_bytes = recv(socket, &length, 1, 0);//ignore flag
         if (!rec_bytes) {
            printf("Server Terminated\n");
            exit(-1);
         }
         count = 0;
         while (count < length) {//only read up to size bytes
            rec_bytes = recv(socket, buf + count, length - count, 0);
            if (!rec_bytes) {
               printf("Server Terminated\n");
               exit(-1);
            }
            count += rec_bytes;
         }
         buf[length] = '\0';
         printf("\t%s\n", buf);
      }
      
   }
}

int parse_flags(int flag, char *buf, int size, int socket) {//buf needs to be freed after use   

   char *freeThis = buf;
   char *handle;
   unsigned char handle_len = 0;
   //int message_len = 0;

   buf = buf + 1; //skip flag
   size--;//remove flag from size

   
   switch(flag) {
      /*case 2://good handle
         //printf("Good Handle\n");
         break;
      case 3://bad handle
         //printf("Bad Handle\n");
         break;*/
      case 4://broadcastrecv
         handle_len = buf[0];
         handle = calloc(handle_len + 1, 1);
         strncpy(handle, buf + 1, handle_len);
         handle[handle_len] = '\0';
         printf("\n%s: %s\n", handle, buf + 1 + handle_len);//needs bash prompt?
         free(handle);
         break;
      case 5://1-1 message
         //printf("Received 1-1 message\n\n");
         handle_len = *(buf + 1 + buf[0]);//get src handle len
         //printf("handle len recv %d\n", handle_len);
         handle = calloc(handle_len + 1, 1);
         strncpy(handle, buf + 2 + buf[0], handle_len);
         handle[handle_len] = '\0';
         printf("\n%s: %s\n", handle, buf + 2 + buf[0] + handle_len);//needs bash prompt?
         free(handle);
         break;
      case 7://dest handle invalid
         handle_len = buf[0];
         handle = calloc(handle_len + 1, 1);
         strncpy(handle, buf + 1, handle_len);
         handle[handle_len] = '\0';
         printf("\nClient with handle %s does not exist.\n", handle);
         break;
      case 9://server->client exit confirm
         exit(-1);
         break;
      case 11://num handles
         //printf("flag 11 received\n");
         print_handles(ntohl(*(int *)buf), socket);
         break;
      /*case 12://follows 11 with list of handles
         print_handles(0, socket);
         break;*/
   }
   free(freeThis);
   return flag;
}

int receive_data(int socket) {
   char *normal_buf;
   char *buf;
   int size, rec_bytes, count = 0;

   normal_buf = calloc(2, 1);

   if ((rec_bytes = recv(socket, normal_buf, 2, 0)) < 0)
   {
      perror("recv call");
      exit(-1);
   }
   if (!rec_bytes) {
      printf("Server Terminated\n");
      exit(-1);
   }
   else if (rec_bytes < 2) {
      if ((rec_bytes = recv(socket, normal_buf + 1, 1, 0)) < 0)
      {
         perror("recv call");
         exit(-1);
      }
   }
   size = ntohs(*(short *)normal_buf) - 2;//without size short
   buf = calloc(size, 1);
 
   if(size == -2) {
      print_handles(-1, socket);//needs special recv for stream of data
   }
   else {
      while (count < size) {//only read up to size bytes
         rec_bytes = recv(socket, buf + count, size - count, 0);
         if (!rec_bytes) {
            printf("Server Terminated\n");
            exit(-1);
         }
         count += rec_bytes;
      }
   }
   return size == -2 ? -1 : parse_flags(buf[0], buf, size, socket);
   
}

void check_handle(int socket, char *handle) {
   char *buf;
   int size = NORMAL_HEADER + 1 + strlen(handle);
   int strsize = strlen(handle);

   if (strsize > 255) {
      printf("Handle length exceeds 255 characters\n");
      exit(-1);
   }
   buf = calloc(size, 1);
   *(short *)buf = htons(size);//2 byte packet length IN NETWORK ORDER
   *(char *)(buf + sizeof(short)) = 1;//1 byte client to server flag
   *(char *)(buf + sizeof(short) + 1) = strsize;//1 byte handle size
   strncpy(buf + 2 * sizeof(short), handle, strsize);//put in str no null
   if(send(socket, buf, size, 0) < 0)
   {
      free(buf);
      perror("send call");
      exit(-1);
   }
   else
      free(buf);
   //printf("Sent Handle Request for handle: %s\n", handle);
   if (receive_data(socket) == 3) {//if handle failed, print and exit
      printf("Handle already in use: %s\n", handle);
      exit(-1);
   }
   
}

void send_message(int socket, char *handle, char *data, int data_size) {
   char *buf;//, data;
   int header_size = 0, i = 0, invalid = 0;
   int target_handle_size = 0, local_handle_size = strlen(handle);
   char target_handle[256];

   while (data[i] != ' ' && !invalid) {
      if (data[i] == '\n' || data[i] == '\0')
         invalid = 1;
      target_handle[i] = data[i];
      i++;
      target_handle_size++;
      data_size--;
   }
   if (target_handle_size > 255) {
      printf("Handle length exceeds 255 characters\n");
      exit(-1);
   }
   else if (!invalid) {
      data_size--;
      data += target_handle_size + 1;
      //printf("|%c|\n", data[0]);


      //scanf("%s%*c", target_handle);//consume extra space
      //target_handle_size = strlen(target_handle);
      header_size = sizeof(short) + 3 + target_handle_size + local_handle_size;

      //printf("handle %s and handlesize %d target %s and target size %d\n", handle, local_handle_size, target_handle, target_handle_size);
      buf = calloc(header_size + DATA_MAX, 1);

      buf[2] = 5;
      buf[3] = target_handle_size;
      strncpy(&buf[4], target_handle, target_handle_size);
      *(&buf[4] + target_handle_size) = local_handle_size;
      strncpy(&buf[4] + target_handle_size + 1, handle, local_handle_size);


      while(data_size - 1 >= DATA_MAX - 1) {
         memcpy(buf + header_size, data, DATA_MAX - 1);
         buf[header_size + DATA_MAX] = '\0';
         data_size -= (DATA_MAX - 1);
         data += (DATA_MAX - 1);

         *(short *)buf = htons(header_size + DATA_MAX);
         if(send(socket, buf, header_size + DATA_MAX, 0) < 0) {
            free(buf);
            perror("send call");
            exit(-1);
         }
      }

      if (data_size) {
         memcpy(buf + header_size, data, data_size);
         buf[header_size + data_size] = '\0';

         *(short *)buf = htons(header_size + data_size + 1);
         if(send(socket, buf, header_size + data_size + 1, 0) < 0) {
            free(buf);
            perror("send call");
            exit(-1);
         }
      }
      free(buf);
   }
}

void send_broadcast(int socket, char *handle, char *data, int data_size) {
   char *buf;
   int header_size = 0;
   int local_handle_size = strlen(handle);

   header_size = sizeof(short) + 2 + local_handle_size;

   buf = calloc(header_size + DATA_MAX, 1);

   buf[2] = 4;
   buf[3] = local_handle_size;
   strncpy(&buf[4], handle, local_handle_size);


   while(data_size - 1 >= DATA_MAX - 1) {
      memcpy(buf + header_size, data, DATA_MAX - 1);
      buf[header_size + DATA_MAX] = '\0';
      data_size -= (DATA_MAX - 1);
      data += (DATA_MAX - 1);

      *(short *)buf = htons(header_size + DATA_MAX);
      if(send(socket, buf, header_size + DATA_MAX, 0) < 0) {
         free(buf);
         perror("send call");
         exit(-1);
      }
   }

   if (data_size) {
      memcpy(buf + header_size, data, data_size);
      buf[header_size + data_size] = '\0';

      *(short *)buf = htons(header_size + data_size + 1);
      if(send(socket, buf, header_size + data_size + 1, 0) < 0) {
         free(buf);
         perror("send call");
         exit(-1);
      }
   }
   free(buf);
}

void send_handle_request(int socket) {
   char *buf = calloc(3, 1);

   *(short *)buf = htons(3);//2 byte packet length IN NETWORK ORDER
   *(char *)(buf + sizeof(short)) = 10;//1 byte client to server flag
   if(send(socket, buf, 3, 0) < 0)
   {
      free(buf);
      perror("send call");
      exit(-1);
   }
   //printf("sent list request with flag 10\n");
}

void send_exit(int socket) {
   char *buf = calloc(3, 1);

   *(short *)buf = htons(3);//2 byte packet length IN NETWORK ORDER
   *(char *)(buf + sizeof(short)) = 8;//1 byte client to server flag
   if(send(socket, buf, 3, 0) < 0)
   {
      free(buf);
      perror("send call");
      exit(-1);
   }
   //printf("sent exit request with flag 8\n");
}


void parse_input(int socket, char *handle) {
   char *free_data, *data;
   size_t size;
   int bytes_read;
   free_data = data = calloc(1024, 1);
   bytes_read = getline(&data, &size, stdin);

   if (data[0] == '%') {
      data[1] = toupper(data[1]);
      switch (data[1]) {
         case 'M':
            if (data[2] == ' ')
               send_message(socket, handle, data + 3, bytes_read - 4);
            else
               printf("Invalid command\n");
            break;
         case 'B':
            if (data[2] == ' ')
               send_broadcast(socket, handle, data + 3, bytes_read - 4);
            else
               printf("Invalid command\n");
            break;
         case 'L':
            if (data[2] == '\n')
               send_handle_request(socket);
            else
               printf("Invalid command\n");
            break;
         case 'E':
            if (data[2] == '\n')
               send_exit(socket);
            else
               printf("Invalid command\n");

            break;
         default:
            printf("Invalid command\n");
            break;
      }
   }
   else
      printf("Invalid command\n");

   free(free_data);
}

void start_connection(int socket, char *handle) {

   fd_set active_set, read_set;
   //int i;
   //char c;

   FD_ZERO(&active_set);//init/zero out fd set
   FD_SET(fileno(stdin), &active_set);//add stdin to list
   FD_SET(socket, &active_set);
   //printf("stdin num %d is set? %d socket num %d is set? %d\n",fileno(stdin), FD_ISSET(fileno(stdin), &active_set), socket, FD_ISSET(socket, &active_set));
   

   while(1) {
      printf("$: ");
      fflush(stdout);
      read_set = active_set;
      if (select(FD_SETSIZE, &read_set, NULL, NULL, NULL) < 0) {
         perror("select");
         exit(-1);
      }
      if (FD_ISSET(fileno(stdin), &read_set)) {//either connecting or already connected
         parse_input(socket, handle);//readinput
      }
      if (FD_ISSET(socket, &read_set)) {
         receive_data(socket);//recv from server
      }
   }
   //close(socket_num);
}

int main(int argc, char * argv[])
{
   int socket_num;         //socket descriptor


   /* check command line arguments  */
   if(argc!= 4)
   {
      printf("usage: %s client-name host-name port-number \n", argv[0]);
      exit(1);
   }

   /* set up the socket for TCP transmission  */
   socket_num = tcp_send_setup(argv[2], argv[3]);
   check_handle(socket_num, argv[1]);
   start_connection(socket_num, argv[1]);

   return 0;

}



int tcp_send_setup(char *host_name, char *port)
{
   int socket_num;
   struct sockaddr_in remote;       // socket address for remote side
   struct hostent *hp;              // address of remote host

   // create the socket
   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket call");
      exit(-1);
   }


   // designate the addressing family
   remote.sin_family= AF_INET;

   // get the address of the remote host and store
   if ((hp = gethostbyname(host_name)) == NULL)
   {
      printf("Error getting hostname: %s\n", host_name);
      exit(-1);
   }

   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   // get the port used on the remote side and store
   remote.sin_port= htons(atoi(port));

   if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
   {
      perror("connect call");
      exit(-1);
   }

   return socket_num;
}

