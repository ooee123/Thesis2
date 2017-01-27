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


typedef struct handleNode{
   char* handle;
   int port;
   struct handleNode* next;
} handleNode;

int read_data_from_socket(int socket, char* message, int* size);
int server_service_loop(int server_socket);
int process_message_flag(char* message, handleNode* root, int flag, int size, int numOpen, handleNode* thisNode);

int removeNode(handleNode** root, handleNode *subject) {
   handleNode* thisNode = *root;
   handleNode* previousNode = NULL;
   while(thisNode != NULL && thisNode != subject) {
       previousNode = thisNode;
       thisNode = thisNode->next;
   }
   if (thisNode == NULL) {
      return -1;
   }
   if (previousNode != NULL) {
      previousNode->next = thisNode->next;
      free(thisNode);
   } else {
      *root = thisNode->next;
      free(thisNode);
   }
   
   return 0;
}

int addNode(handleNode** root, handleNode* thisNode) {
   
   handleNode* preNode = *root;
   if (preNode == NULL) {
      *root = thisNode;
   } else {
      while (preNode->next != NULL) {
         preNode = preNode->next;
      }
      preNode->next = thisNode;
   }
   
   return 0;
}

int main(int argc, char *argv[])
{
    int requestedPort = 0;
    int server_socket= 0;
    if (argc > 1) {
       requestedPort = atoi(argv[1]);
       printf("Will try to register for port :%d\n", requestedPort);
    }
    //printf("sockaddr: %d sockaddr_in %d\n", sizeof(struct sockaddr), sizeof(struct sockaddr_in));
    
    server_socket= tcp_recv_setup((uint16_t)requestedPort);
    server_service_loop(server_socket);

    close(server_socket);
    return -1;
}

int server_service_loop(int server_socket) {

   int highFD = server_socket;
   fd_set client_sockets;
   int numOpen = 0;
   int flag = 0;
   handleNode* root = NULL;
   handleNode* thisNode = root;
   
   if (listen(server_socket, 10) < 0)
   {
      printf("Failed to listen\n");
      return -1;
   }

    while (1) {
      FD_ZERO(&client_sockets);
      FD_SET(server_socket, &client_sockets);
      
      thisNode = root;
      while (thisNode != NULL) {
         FD_SET(thisNode->port, &client_sockets);
         thisNode = thisNode->next;
      }

       if (select(highFD + 1, &client_sockets, NULL, NULL, NULL) < 0) {
          printf("Error with select call\n");
          return -1;
       }

      if (FD_ISSET(server_socket, &client_sockets)) {
             int client_socket = tcp_accept(server_socket);
             if (client_socket >= 0) {
                if (client_socket > highFD) {
                   highFD = client_socket;
                }
                numOpen++;
                handleNode* newNode = malloc(sizeof(handleNode));
                newNode->port = client_socket;
                newNode->next = NULL;
                newNode->handle = NULL;
                addNode(&root, newNode);
            }
      }
      //printf("%d\n", root);
       thisNode = root;
       while (thisNode != NULL) {
          handleNode* tempNode = thisNode->next;
          if (FD_ISSET(thisNode->port, &client_sockets)) {
                char* message = malloc(1500);
                int size = 0;
                flag = read_data_from_socket(thisNode->port, message, &size);
                if (flag < 0) {
                   close(thisNode->port);
                   numOpen--;
                   removeNode(&root, thisNode);
                } else {
                  int returnval = process_message_flag(message, root, flag, size, numOpen, thisNode);
                  if (returnval < 0) {
                     printf("Error with processing a message\n");
                  }
                  if (returnval == 1) {
                     printf("%s has exited\n", thisNode->handle);
                     close(thisNode->port);
                     numOpen--;
                     removeNode(&root, thisNode);
                  }
                }
                free(message);
          }
          thisNode = tempNode;
       }
    }
   return -1;
}

handleNode* check_handles(char *thisHandle, handleNode* root, int numOpen) {

   handleNode* thisNode = root;
   while (thisNode != NULL) {
      char* handle = thisNode->handle;
      if (handle != NULL) {
         if (!strcmp(handle, thisHandle)) {
            return thisNode;
         }
      }
      thisNode = thisNode->next;
   }
   
   return NULL;
}

int process_handeshake(char* message, handleNode* root, int numOpen, handleNode* thisNode) {
      char* handleBuffer;

      int handleLength = (int)message[0];
      if (handleLength > 0) {
         handleBuffer = malloc(handleLength + 1);
         memcpy(handleBuffer, message+1, handleLength);
         handleBuffer[handleLength] = '\0';
      } else {
         printf("Invalid handshake packet\n");
         return -1;
      }
      handleNode* checker = check_handles(handleBuffer, root, numOpen);
      if (checker != NULL) {
         printf("Handle already exist\n");
         free(handleBuffer);
         return -1;
      } else {
         thisNode->handle = handleBuffer;
         return 0;
      }
}

int tcp_send_data(char *data, int socket_num, int send_len) {
   //printf("Message sent, length: %d\n", send_len);
   return send(socket_num, data, send_len, MSG_NOSIGNAL);
}

int send_packet_with_flag(char *data, int socket_num, int send_len, uint8_t flag) {
   uint16_t newlength = send_len + 3;
   int returnval = -1;
   char* dataBuffer = malloc(newlength);;
   uint16_t netlength = htons(newlength);
   if (flag == 12) {
      dataBuffer[0] = 0;
      dataBuffer[1] = 0;
   } else {
      memcpy(dataBuffer, &netlength, 2);
   }
   dataBuffer[2] = flag;
   if (data != NULL) {
      memcpy(dataBuffer + 3, data, send_len);
   }
   
   returnval = tcp_send_data(dataBuffer, socket_num, newlength);
   free(dataBuffer);
   return returnval;
}

int repeat_message(char* message, handleNode* root, int size, int numOpen, handleNode* thisNode) {
   
      int returnval = 0;
   if (size < 2) {
      printf("Corrupted Packet\n");
      return -1;
   }
   uint8_t desHandleLength = message[0];
   if (size < (desHandleLength + 2)) {
      printf("Corrupted Packet\n");
      return -1;
   }
   char* desHandle = malloc(desHandleLength + 1);
   memcpy(desHandle, message + 1, desHandleLength);
   desHandle[desHandleLength] = '\0';
   
   uint8_t sourceHandleLength = message[1 + desHandleLength];
   if (size < (desHandleLength + sourceHandleLength + 2)) {
      printf("Corrupted Packet\n");
      free(desHandle);
      return -1;
   }
   
   char* sourceHandle = malloc(sourceHandleLength + 1);
   memcpy(sourceHandle, message + 2 + desHandleLength, sourceHandleLength);
   sourceHandle[sourceHandleLength] = '\0';
   
   if (strcmp(sourceHandle, thisNode->handle)) {
      printf("Source handle does not match\n");
      free(sourceHandle);
      free(desHandle);
      return -1;
   }
   
   handleNode* findNode = check_handles(desHandle, root, numOpen);
   if (findNode == NULL) {
      char *errorMessage = malloc(1 + desHandleLength);
      errorMessage[0] = desHandleLength;
      memcpy(errorMessage + 1, desHandle, desHandleLength);
      returnval = send_packet_with_flag(errorMessage, thisNode->port, 1 + desHandleLength, 7);
      free(errorMessage);
   } else {
      if (size > (2 + desHandleLength + sourceHandleLength)) {
         returnval = send_packet_with_flag(message, findNode->port, size, 5);
      } else {
         returnval = send_packet_with_flag(message, findNode->port, size, 5);
      }
   }
   
   free(sourceHandle);
   free(desHandle);
   return returnval;
}

int boardcast_message(char* message, handleNode* root, int size, int numOpen, handleNode* thisNode) {
   
   int returnval = 0;
   
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
   
   if (strcmp(sourceHandle, thisNode->handle)) {
      printf("Source handle does not match\n");
      free(sourceHandle);
      return -1;
   }
   
   handleNode* current = root;
   
   while (current != NULL) {
      if (current != thisNode) {
         if (send_packet_with_flag(message, current->port, size, 4) < 0) {
            returnval = -1;
         }
      }
      current = current->next;
   }
   
   return returnval;
}

int process_list_request(handleNode* root, int numOpen, int thisSocket) {
   int returnval;
   
   char* messageBuf = malloc(4);
   *((uint32_t*)messageBuf) = htonl(numOpen);
   
   returnval = send_packet_with_flag(messageBuf, thisSocket, 4, 11);
   if (returnval < 0) {
      printf("Failed to send size packet\n");
      return -1;
   }
   free(messageBuf);
    
   handleNode* current = root;
   while (current != NULL) {
      char* thisHandle = current->handle;
      uint8_t handleLength = strlen(thisHandle);
      messageBuf = malloc(handleLength + 1);
      messageBuf[0] = handleLength;
      memcpy(messageBuf + 1, thisHandle, handleLength);
      int temp = send_packet_with_flag(messageBuf, thisSocket, handleLength + 1, 12);
      if (temp < 0) {
         printf("Failed to send a handle packet\n");
      }
      free(messageBuf);
      current = current->next;
   }
  
   return 0;
}

int process_message_flag(char* message, handleNode* root, int flag, int size, int numOpen, handleNode* thisNode) {
   
   if (flag == 1) {
      int returnval = process_handeshake(message, root, numOpen, thisNode);
      if (returnval < 0) {
         printf("Error with a handshake packet\n");
         int temp = send_packet_with_flag(NULL, thisNode->port, 0, 3);
         if (temp < 0) {
            printf("Failed to send a error handshake packet\n");
            return -1;
         }
      } else {
         printf("Handle registered: %s\n", thisNode->handle);
         int temp = send_packet_with_flag(NULL, thisNode->port, 0, 2);
         if (temp < 0) {
            printf("Failed to send a success handshake packet\n");
            return -1;
         }
      }
   } else if (flag == 8) {
      int temp = send_packet_with_flag(NULL, thisNode->port, 0, 9);
      if (temp < 0) {
         printf("Failed to send a error handshake packet\n");
         return -1;
      } else {
         return 1;
      }      
   } else if (flag == 5) {
      int temp = repeat_message(message, root, size, numOpen, thisNode);
      if (temp < 0) {
         printf("Failed to repeat a message\n");
      }
   } else if (flag == 4) {
      int temp = boardcast_message(message, root, size, numOpen, thisNode);
      if (temp < 0) {
         printf("Failed to boardcast a message\n");
      }
   } else if (flag == 10) {
      int temp = process_list_request(root, numOpen, thisNode->port);
      if (temp < 0) {
         printf("Failed to process list request\n");
      }
   } else {
      printf("Corrupted Packet Received\n");
   }
   return 0;
}

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
         printf("Connection closed abnormally with client\n");
         free(buf);
         return -1;
   }

   buf[message_len] = 0;
   //printf("Message length: %d\n", message_len);

   if (message_len < 3) {
      printf("Corrupted packet received, incomplete header\n");
      free(buf);
      return 0;
   }

   uint16_t length = ntohs(*((uint16_t*)buf));
   
   if (message_len != length) {
      printf("Corrupted packet received, length mismatch\n");
      free(buf);
      return 0;
   }

   int flag = buf[2];
   if (flag <= 0) {
      printf("Corrupted Packet received, failed to parse flag\n");
      free(buf);
      return 0;
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

int tcp_recv_setup(uint16_t port)
{
    int server_socket= 0;
    struct sockaddr_in local;
    socklen_t len= sizeof(local);

    server_socket= socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
    {
       printf("Failed to create server socket\n");
       return -1;
    }

    local.sin_family= AF_INET;
    local.sin_addr.s_addr= INADDR_ANY;
    local.sin_port= htons(port);

    if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
      {
         printf("Failed to bind\n");
         return -1;
      }

    if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0)
      {
         printf("Failed to get socket\n");
         return -1;
      }

    printf("Server has port %d \n", ntohs(local.sin_port));

    return server_socket;
}

int tcp_accept(int server_socket)
{
  int client_socket= 0;

  if ((client_socket= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
    {
      perror("Failed to accept connection\n");
      return -1;
    }

  return(client_socket);
}
