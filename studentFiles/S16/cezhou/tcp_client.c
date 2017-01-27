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

#include "networks.h"
#include "testing.h"

int tcp_send_data(char *data, int socket_num, int send_len);
int send_packet_with_flag(char *data, int socket_num, int send_len, uint8_t flag);
int handshake(int socket_num, char* handle);



int read_data_from_socket(int socket, char* message, int* size) {

   int message_len= 0;
   char* buf =  malloc(1024);
   //printf("Processing headers...\n");
   if ((message_len = recv(socket, buf, 1024, 0)) < 0)
   {
         printf("Failed to receive the message\n");
         free(buf);
         return -1;
   }

   if (message_len == 0) {
         printf("Connection closed abnormally with server\n");
         free(buf);
         return -2;
   }

   buf[message_len] = 0;

   if (message_len < 3) {
      printf("Corrupted packet received, incomplete header\n");
      free(buf);
      return -1;
   }

   uint16_t length = ntohs(*((uint16_t*)buf));

   int flag = buf[2];
   if (flag <= 0) {
      printf("Corrupted Packet received, failed to parse flag\n");
      free(buf);
      return -1;
   }
   
   if (message_len != length) {
      if (flag == 11) {
         flag = 20;
      } else if (flag == 12) {
         flag = 21;
      } else {
         printf("Corrupted packet received, length mismatch\n");
         free(buf);
         return -1;
      }
   }
   
   *size = message_len -3;
   if (message_len > 3) {
      memcpy(message, buf + 3, message_len - 3);
   } else {
      message = NULL;
   }

   free(buf);
   return flag;
}


int tryReceiveMessage(int socket_num, char* message, int* size) {
   struct timeval tv;
   fd_set ports;
   
   tv.tv_sec = 3;
   tv.tv_usec = 0;
   
   FD_ZERO(&ports);
   FD_SET(socket_num, &ports);
   
   int returnval = select(socket_num + 1, &ports, NULL, NULL, &tv);
   
   if (returnval < 0) {
      printf("Error with select call\n");
      return -1;
   } else if (returnval == 0) {
      return 0;
   } else {
      if (FD_ISSET(socket_num, &ports)) {
         //printf("Packet pending from server\n");
         int flag = read_data_from_socket(socket_num, message, size);
         if (flag < 0) {
            printf("Failed to read data from socket\n");
            return flag;
         } else {
            return flag;
         }
      } else {
         return 0;
      }
   }
   
}

int handshake(int socket_num, char* handle) {
   printf("Trying to handshake with server...\n");
   int size = strlen(handle);
   char* buffer = malloc(size + 1);
   buffer[0] = (uint8_t)size;
   memcpy(buffer + 1, handle, size);
   printf("Sending handshake packet...\n");
   int returnval = send_packet_with_flag(buffer, socket_num, size + 1, 1);
   if (returnval < 0) {
      free(buffer);
      return -1;
   }
   free(buffer);
   printf("Waiting for server to respond...\n");
   
   int response = 0;
   int size2 = 0;
   char* message = malloc(1500);
   while (response == 0) {
      response = tryReceiveMessage(socket_num, message, &size2);
   }
   if (response == -2) {
      printf("Server exited\n");
      return -1;
   } else if (response == 2) {
      printf("Handle registered, handshake complete\n");
   } else {
      printf("Handle exists or communication error %d\n", response);
      free(message);
      return -1;
   }
   
   free(message);

   return 0;
}

int exitWithServer(int socket_num) {
   int returnval = send_packet_with_flag(NULL, socket_num, 0, 8);
   if (returnval < 0) {
      printf("Failed to send exit packet\n");
      return -1;
   }
   return 0;
}

int send_packet_with_flag(char *data, int socket_num, int send_len, uint8_t flag) {
   uint16_t newlength = send_len + 3;
   int returnval = -1;
   char* dataBuffer = malloc(newlength);;
   uint16_t netlength = htons(newlength);
   memcpy(dataBuffer, &netlength, 2);
   dataBuffer[2] = flag;
   if (data != NULL) {
      memcpy(dataBuffer + 3, data, send_len);
   }
   returnval = tcp_send_data(dataBuffer, socket_num, newlength);
   free(dataBuffer);
   return returnval;
}

int send_tcp_message(int socket_num, char* handle, char* destination, int length, char* message) {
   int message_length;
   int messagelength;
   uint8_t handleLength = (uint8_t)strlen(handle);
   if (message != NULL) {
      messagelength = strlen(message);
   } else {
      messagelength = 0;
   }
   if (message != NULL) {
      message_length = 1 + length + 1 + handleLength + messagelength +  1;
   } else {
       message_length = 1 + length + 1 + handleLength;
   }
   char *messageBuff = malloc(message_length);
   messageBuff[0] = (uint8_t)length ;
   memcpy(messageBuff + 1, destination, length);
   messageBuff[1 + length] = handleLength;
   memcpy(messageBuff + 2 + length, handle, handleLength);
   if (message != NULL) {
      memcpy(messageBuff + 2 + length + handleLength, message, messagelength + 1);
   }
   
   int returnval = send_packet_with_flag(messageBuff, socket_num, message_length, 5);
   
   if (returnval < 0) {
      printf("Error sending message\n");
   }
   
   free(messageBuff);
   return 0;
}

int sendMessage(int socket_num, char* handle, char* command, int command_len) {
   if (command_len < 4) {
      printf("Invalid command\n");
      return -1;
   }
   int i = 3;
   while ((i < command_len) && (command[i] != ' ')) {
      i++;
   }
   int handleLength = i - 3;
   char* handleBuf = malloc(handleLength);
   memcpy(handleBuf, command + 3, handleLength);
   int messageLength = command_len - handleLength - 4;
   char* message = NULL;
   if (messageLength > 0) {
      message = malloc(messageLength + 1);
      memcpy(message, command + 3 + handleLength + 1, messageLength);
      message[messageLength] = '\0';
   } else {
      messageLength = 0;
   }
   
   int returnval = send_tcp_message(socket_num, handle, handleBuf, handleLength, message);
   
   free(handleBuf);
   if (message != NULL) {
      free(message);
   }
   return returnval;
}

int sendBoardCast(int socket_num, char* handle, char* command, int command_len) {
   int messagelength = 0;
   int returnval = 0;
   uint8_t handleLength = (uint8_t)strlen(handle); 
   char *message = NULL;
   if (command_len > 4) {
      messagelength = command_len - 3;
      message = malloc(messagelength + 1);
      memcpy(message, command + 3, messagelength);
      message[messagelength] = '\0';
   }
   int message_length;
   if (message != NULL) {
      message_length = 2 + handleLength + messagelength;
   } else {
      message_length = 1 + handleLength;
   }
   char *packet = malloc(message_length);
   packet[0] = handleLength;
   memcpy(packet + 1, handle, handleLength);
   if (message != NULL) {
      memcpy(packet + 1 + handleLength, message, messagelength + 1);
   }
   
   returnval = send_packet_with_flag(packet, socket_num, message_length, 4);
   
   free(packet);
   if (message != NULL) {
      free(message);
   }
   
   if (returnval < 0) {
      printf("Failed to send the boardcast\n");
   }
   return returnval;
}

int sendListRequest(int socket_num) {
   int returnval = send_packet_with_flag(NULL, socket_num, 0, 10);
   if (returnval < 0) {
      printf("Failed to request list of handle\n");
   }
   return returnval;
}
 
int PromptForAction(int socket_num, char* handle) {

    char* command = malloc(1500);
    int command_len = 0;
    while ((command[command_len] = getchar()) != '\n' && command_len < 1024)
	   command_len++;
    command[command_len] = '\0';
    if (command_len >= 2) {
      if (command[0] != '%') {
         printf("Invalid command\n");
      } else if (command[1] == 'L' || command[1] == 'l') {
         sendListRequest(socket_num);
      }  else if (command[1] == 'B' || command[1] == 'b') {
         sendBoardCast(socket_num, handle, command, command_len);
      } else if (command[1] == 'M' || command[1] == 'm') {
         sendMessage(socket_num, handle, command, command_len);
      }
      else if (command[1] == 'E' || command[1] == 'e') {
         exitWithServer(socket_num);
      } else {
         printf("Invalid command\n");
      }
    } else {
       printf("Invalid command\n");
    }
    free(command);
    return 0;
}

int process_message(char* message, int size, char* handle) {
    if (size < 2) {
      printf("Received Corrupted Packet\n");
      return -1;
   }
   uint8_t desHandleLength = message[0];
   if (size < (desHandleLength + 2)) {
      printf("Received Corrupted Packet\n");
      return -1;
   }
   char* desHandle = malloc(desHandleLength + 1);
   memcpy(desHandle, message + 1, desHandleLength);
   desHandle[desHandleLength] = '\0';
   
   uint8_t sourceHandleLength = message[1 + desHandleLength];
   if (size < (desHandleLength + sourceHandleLength + 2)) {
      printf("Received Corrupted Packet\n");
      free(desHandle);
      return -1;
   }
   
   char* sourceHandle = malloc(sourceHandleLength + 1);
   memcpy(sourceHandle, message + 2 + desHandleLength, sourceHandleLength);
   sourceHandle[sourceHandleLength] = '\0';
   
   if (strcmp(desHandle, handle)) {
      printf("Destination handle does not match\n");
      free(sourceHandle);
      free(desHandle);
      return -1;
   }
   
   if (size > (2 + desHandleLength + sourceHandleLength)) {
      printf("\n%s: %s\n", sourceHandle, (message + 2 + desHandleLength + sourceHandleLength));
   } else {
      printf("\n%s:\n\n", sourceHandle);
   }
   
   free(sourceHandle);
   free(desHandle);
   return 0;
}

int process_exist(char* message, int size) {
  
  if (size < 2) {
      printf("Received Corrupted Packet\n");
      return -1;
   }
   uint8_t desHandleLength = message[0];
   if (size < (desHandleLength + 1)) {
      printf("Received Corrupted Packet\n");
      return -1;
   }
   
   char* desHandle = malloc(desHandleLength + 1);
   memcpy(desHandle, message + 1, desHandleLength);
   desHandle[desHandleLength] = '\0';
   
   printf("Client with handle %s does not exist\n", desHandle);

   free(desHandle);
   return 0;
}

int process_boardcast(char* message, int size, char* handle) {
   
   if (size < 2) {
      printf("Corrupted Packet\n");
      return -1;
   }
   uint8_t sourceHandleLength = message[0];
   if (size < (sourceHandleLength + 1)) {
      printf("Corrupted Packet\n");
      return -1;
   }
   char* sourceHandle = malloc(sourceHandleLength + 1);
   memcpy(sourceHandle, message + 1, sourceHandleLength);
   sourceHandle[sourceHandleLength] = '\0';
   
   if (!strcmp(sourceHandle, handle)) {
      free(sourceHandle);
      return 0;
   }
   
   if (size > (1 + sourceHandleLength )) {
      printf("\n%s: %s\n", sourceHandle, (message + 1 + sourceHandleLength));
   } else {
      printf("\n%s:\n\n", sourceHandle);
   }
   
   return 0;

}

int process_handle(char* message, int size) {
   
   if (size < 2) {
      printf("Corrupted Packet\n");
      return -1;
   }
   uint8_t handleLength = message[0];
   if (size < (handleLength + 1)) {
      printf("Corrupted Packet\n");
      return -1;
   }
   
   char* handle = malloc(handleLength + 1);
   memcpy(handle, message + 1, handleLength);
   handle[handleLength] = '\0';
   printf(", %s", handle);
   free(handle);
   return handleLength;
}

int process_stream(char* message, int size, int count) {
   int countx = count;
   if (size < 3) {
      return countx;
   }
   
   uint16_t length = ntohs(*((uint16_t*)message));
   //printf("Packet Length :%d\n", length);
   int flag = message[2];
   if (flag <= 0) {
      return 0;
   } else if (flag == 12) {
      length = message[3] + 1 + 3;
      process_handle(message + 3, length - 3);
      countx++;
   }
   
   return process_stream(message + length, size - length, countx);
   
}

int process_size(char* message, int size, int* list_size) {
   if (size < 4) {
      printf("Corrupted Packet");
      return -1;
   }
   uint32_t* size2 = (uint32_t*) message;
   uint32_t size3 = ntohl(*size2);
   printf("\nNumber of handles %d", size3);
   int count = process_stream(message + 4, size - 4, 0);
   *list_size = size3 - count;
   if (*list_size == 0) {
      printf("\n");
   }
   if (count > 0) {
      return 1;
   } else {
      return 0;
   }
}

int process_handles(char* message, int size, int list_size, int socket_num, int flag) {
   int sizeofList = list_size;
   int temp = flag;
   int messageSize = size;
   
   while (sizeofList > 0) {      
      if (temp == 12) {
         process_handle(message, messageSize);
         fflush(stdout);
         sizeofList--;
      } else if (temp == 0) {
         printf("Timed out before receving complete list\n");
         break;
      } else if (temp == 21) {
         int handleLength = process_handle(message, messageSize);
         int count = process_stream(message + handleLength + 1, messageSize - handleLength - 1, 0);
         sizeofList = sizeofList - 1 - count;
      } else if (temp < 0) {
         break;
      }
      messageSize = 0;
      if (sizeofList > 0) {
         temp = tryReceiveMessage(socket_num, message, &messageSize);
      }
   }
   printf("\n");
   return 0;
}

int process_handles_block(int list_size, int socket_num) {

   int sizeofList = list_size;
   int temp = 1;
   int messageSize = 0;
   char* message = malloc(1500);
   
   while (sizeofList > 0) {
      messageSize = 0;
      temp = tryReceiveMessage(socket_num, message, &messageSize);    
      if (temp == 12) {
         process_handle(message, messageSize);
         fflush(stdout);
         sizeofList--;
      } else if (temp == 0) {
         printf("Timed out before receving complete list\n");
         break;
      } else if (temp == 21) {
         int handleLength = process_handle(message, messageSize);
         int count = process_stream(message + handleLength + 1, messageSize - handleLength - 1, 0);
         sizeofList = sizeofList - 1 - count;
      } else if (temp < 0) {
         break;
      }
   }
   
   printf("\n");
   free(message);
   return 0;
}

int process_response(int flag, int socket_num, char* handle, char* message, int size) {
   static int list_size = 0;
   if (flag == 9) {
      return 1;
   } else if (flag == 5) {
      process_message(message, size, handle);
   } else if (flag == 7) {
      process_exist(message, size);
   } else if (flag == 4) {
      process_boardcast(message, size, handle);
   } else if (flag == 11 || flag == 20) {
      int tempSize = 0;
      int temp = process_size(message, size, &tempSize);
      if (temp > 0) {
         list_size = tempSize;
         if (list_size > 0) {
            process_handles_block(list_size, socket_num);
            list_size = 0;
         }
      } else {
         list_size = tempSize;
      }
   } else if (flag == 12 || flag == 21) {
      if (list_size <= 0) {
         printf("Received Packet 12 before Packet 11\n");
      } else {
         process_handles(message, size, list_size, socket_num, flag);
         list_size = 0;
      }
   } else {
      printf("Corrupted message received\n");
      return -1;
   }
   return 0;
}

int passiveAction(int socket_num, char* handle) {
   
   struct timeval tv;
   fd_set ports;
   int size;
   
   char* message = malloc(1500);
   
   tv.tv_sec = 3;
   tv.tv_usec = 0;
   
   FD_ZERO(&ports);
   FD_SET(socket_num, &ports);
   FD_SET(0, &ports);
   
   int returnval = select(socket_num + 1, &ports, NULL, NULL, &tv);
   
   if (returnval < 0) {
      printf("Error with select call\n");
      return -1;
   } else if (returnval == 0) {
      free(message);
      return 0;
   } else {
      if (FD_ISSET(socket_num, &ports)) {
         //printf("Packet pending from server\n");
         fflush(stdout);
         int flag = read_data_from_socket(socket_num, message, &size);
         if (flag < 0) {
            if (flag == -2) {
               free(message);
               return -1;
            }
         } else {
            if (process_response(flag, socket_num, handle, message, size) == 1) {
               free(message);
               return 1;
            }
         }
      }
      if (FD_ISSET(0, &ports)) {
         PromptForAction(socket_num, handle);
      }
      printf("$");
      fflush(stdout);
   }
   
   free(message);
   return 0;
}


int tcp_send_data(char *data, int socket_num, int send_len) {
   //printf("Message sent, length: %d\n", send_len);
   return send(socket_num, data, send_len, MSG_NOSIGNAL);
}

int tcp_send_setup(char *host_name, char *port)
{
   int socket_num;
   struct sockaddr_in remote;
   struct hostent *hp;

   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	   printf("Error creating socket\n");
	   return(-1);
	}
   remote.sin_family= AF_INET;

    if ((hp = gethostbyname(host_name)) == NULL)
	{
      printf("Error getting hostname: %s\n", host_name);
      return(-1);
	}
   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);
   remote.sin_port= htons(atoi(port));
   if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
   {
      printf("Failed to connect\n");
      return(-1);
   }
   return socket_num;
}


int main(int argc, char* argv[])
{
   int socket_num;

   if(argc < 4)
   {
      printf("usage: %s handle server-name server-port\n", argv[0]);
      exit(-1);
   }

   socket_num = tcp_send_setup(argv[2], argv[3]);
   if (socket_num < 0) {
      printf("Error establishing connection\n");
      exit(-1);
   }

   if (handshake(socket_num, argv[1]) < 0) {
      printf("Failed to handshake with server\n");
      exit(-1);
   }
   
   printf("$");
   fflush(stdout);
    int commandAction = 0;
    while (commandAction == 0) {
       commandAction = passiveAction(socket_num, argv[1]);
    }
    
    close(socket_num);
    return 0;

}