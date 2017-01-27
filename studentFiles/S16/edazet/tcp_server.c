/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
 *****************************************************************************/

#include "networks.h"

void createClientList(client_list **cl) {
   if ((*cl = (client_list *)malloc(sizeof(client_list))) == NULL) {
      perror("client list malloc");
      exit(-1);
   }
   memset(*cl, 0, sizeof(client_list));

   (*cl)->numClients = 0;
   (*cl)->clients = NULL;
}

client *createClient(int socket, uint8_t handle_len, uint8_t *handle) {
   client *temp;

   if ((temp = (client *)malloc(sizeof(client))) == NULL) {
      perror("add client malloc");
	  exit(-1);
   }
   memset(temp, 0, sizeof(client));

   temp->socket_num = socket;
   temp->amountToRead = 0;
   temp->handle_len = handle_len;
   temp->handle = safeCharMalloc(handle_len);
   memcpy(temp->handle, handle, handle_len);
   temp->next = NULL;

   return temp;
}

int addClient(client_list **cl, uint8_t handle_len, uint8_t *handle, int c_socket) {
   client *iter = (*cl)->clients, *temp;
   int i;

   temp = createClient(c_socket, handle_len, handle);

   if (!(*cl)->clients)
      (*cl)->clients = temp;
   else {
      for (i = 0; i < (*cl)->numClients; i++) {
         if (!memcmp(iter->handle, handle, MAX(iter->handle_len, handle_len))) {
    	    free(temp);
            return F_HANDLE_EXIST;
         }
         if (!iter->next)
            break;
         iter = iter->next;
      }
      iter->next = temp;
   }
   (*cl)->numClients++;
   return F_HANDLE_CNFM;
}

void removeClient(client_list **cl, client *c) {
   client *iter, *next;
   uint8_t *handle, handle_len;
   int i;

   iter = (*cl)->clients;
   handle = c->handle;
   handle_len = c->handle_len;

   if (memcmp((*cl)->clients->handle, handle,
      MAX((*cl)->clients->handle_len, handle_len)) == 0) {

      iter = (*cl)->clients;
      (*cl)->clients = (*cl)->clients->next;
   }
   else {
	  for (i = 0; i < (*cl)->numClients; i++) {
		 next = iter->next;
		 if (!memcmp(next->handle, handle, MAX(next->handle_len, handle_len))) {
			iter->next = next->next;
			iter = next;
			break;
		 }
		 iter = iter->next;
	  }
   }
   close(iter->socket_num);
   free(iter);
   (*cl)->numClients--;
}

uint8_t readNewClientPacket(int client_socket, uint8_t **handle_name, int amountToRead) {
   int rcv = 0;
   uint8_t *buf, flag;
   uint8_t handle_len;

   buf = safeCharMalloc(amountToRead);

   rcv = recvPacket(client_socket, buf, amountToRead);
   memcpy(&flag, buf, sizeof(normal_header) - PKT_LEN_FIELD);
   memcpy((void *)(&(handle_len)), buf + (char)1, 1);

   *handle_name = safeCharMalloc(handle_len);
   memcpy(*handle_name, buf + (char)2, handle_len);

   free(buf);
   return handle_len;
}

int setClients(client_list *cl, fd_set *r, int s_sock, int l_sock) {
   client *iter;
   int largestSocket = l_sock;

   iter = cl->clients;

   FD_ZERO(r);
   FD_SET(s_sock, r);

   while (iter) {
	  FD_SET(iter->socket_num, r);
	  if (largestSocket <= iter->socket_num)
	     largestSocket = iter->socket_num + 1;
	  iter = iter->next;
   }
   return largestSocket;
}

void prepMessage(client_list *cl, uint8_t *buf, client *c, uint8_t f) {
   normal_header *h;
   uint8_t *packet;
   uint16_t pkt_len, buf_len;
   int offset = 0;

   h = safeNormalHeader();
   pkt_len = c->amountToRead + PKT_LEN_FIELD;
   buf_len = c->amountToRead - 1;	//the flag, first entry in buf, was previously read

   h->pkt_length = htons(pkt_len);
   h->flag = f;
   packet = safeCharMalloc(pkt_len);

   memcpy(packet, h, sizeof(normal_header));
   offset += sizeof(normal_header);
   memcpy(packet + (char)offset, buf, buf_len);

   if (f == F_MESSAGE)
      relayMessageToClient(cl, c, packet, pkt_len);
   else
	  broadcastMessage(cl, c, packet, pkt_len);

   free(h);
   free(packet);
}

void relayMessageToClient(client_list *cl, client *c, uint8_t *buf, uint16_t pkt_len) {
   client *iter;
   int i, sendSocket = 0;
   uint8_t dh_len, *dst_handle;
   uint16_t err_pkt_len;

   dst_handle = safeCharMalloc(MAX_HANDLE_LEN);
   memcpy(&dh_len, buf + (char)(sizeof(normal_header)), 1);
   memcpy(dst_handle, buf + (char)(sizeof(normal_header) + 1), dh_len);

   iter = cl->clients;
   for (i = 0; i < cl->numClients; i++, iter = iter->next) {
      if (!memcmp(iter->handle, dst_handle, MAX(iter->handle_len, dh_len)))
         sendSocket = iter->socket_num;
   }

   if (sendSocket)
      sendPacket(sendSocket, buf, pkt_len);
   else {	// send "handle doesn't exist" error packet
	  err_pkt_len = htons(sizeof(normal_header) + 1 + dh_len);
	  memcpy(buf, &err_pkt_len, PKT_LEN_FIELD);   //change packet length
	  memset(buf + (char)PKT_LEN_FIELD, F_HANDLE_NEXIST, 1);   //change flag
	  sendPacket(c->socket_num, buf, ntohs(err_pkt_len));
   }

   free(dst_handle);
}

void broadcastMessage(client_list *cl, client *c, uint8_t *buf, uint16_t pkt_len) {
   client *iter;
   int i;

   iter = cl->clients;
   for (i = 0; i < cl->numClients; i++, iter = iter->next) {
      if (memcmp(iter->handle, c->handle, MAX(iter->handle_len, c->handle_len)))
         sendPacket(iter->socket_num, buf, pkt_len);
   }
}

void sendHandles(client_list *cl, client *c, normal_header *h) {
   int pkt_len, i, offset = 0;
   client *iter;
   uint8_t *buf;

   h->pkt_length = 0;
   h->flag = F_HANDLES;

   iter = cl->clients;
   for (i = 0, pkt_len = 0; i < cl->numClients; i++, iter = iter->next)
      pkt_len += iter->handle_len;

   pkt_len += cl->numClients + sizeof(normal_header);

   buf = safeCharMalloc(pkt_len);
   memcpy(buf, h, sizeof(normal_header));
   offset += sizeof(normal_header);

   iter = cl->clients;
   for (i = 0; i < cl->numClients; i++, iter = iter->next) {
      memset(buf + (char)offset, iter->handle_len, 1);
      offset += 1;
      memcpy(buf + (char)offset, iter->handle, iter->handle_len);
      offset += iter->handle_len;
   }

   sendPacket(c->socket_num, buf, pkt_len);
   free(buf);
}

void sendNumHandles(client_list *cl, client *c) {
   uint8_t flag = F_NUM_HANDLES;
   normal_header *h;
   uint8_t *buf, offset = 0;

   h = safeNormalHeader();
   h->flag = flag;
   h->pkt_length = htons(sizeof(normal_header) + sizeof(int));

   buf = safeCharMalloc(ntohs(h->pkt_length));
   memcpy(buf, h, sizeof(normal_header));
   offset += sizeof(normal_header);
   memcpy(buf + (char)offset, &(cl->numClients), sizeof(int));

   sendPacket(c->socket_num, buf, ntohs(h->pkt_length));
   sendHandles(cl, c, h);
   free(h);
   free(buf);
}

void serverResponse(client_list **cl, uint8_t *buf, client *c) {
   uint8_t flag;

   memcpy(&flag, buf, 1);

   if (flag == F_CLIENT_EXIT) {
      sendNormalPacket(c->socket_num, F_ACK_EXIT);
      removeClient(cl, c);
   }
   else if (flag == F_MESSAGE || flag == F_BROADCAST)
	  prepMessage(*cl, buf + (char)1, c, flag); //advances buf to account for reading flag
   else if (flag == F_HANDLE_LIST)
	  sendNumHandles(*cl, c);
}

int readFromClientSocket(client_list **cl, client **c) {
   int rcv, returnValue = 0;
   uint16_t csock_amountToRead = 0;
   uint8_t *buf;

   // checks if the client knows how much data to read from its socket
   if (!(*c)->amountToRead) {
      rcv = recvPacket((*c)->socket_num, (uint8_t *)&csock_amountToRead, PKT_LEN_FIELD);
      if (!rcv)	//client has disconnected
         return 1;
      (*c)->amountToRead = ntohs(csock_amountToRead) - PKT_LEN_FIELD;
   }
   else {
	  buf = safeCharMalloc((*c)->amountToRead);
      rcv = recvPacket((*c)->socket_num, buf, (*c)->amountToRead);
      if (!rcv)
         return 1;
      serverResponse(cl, buf, *c);
      (*c)->amountToRead = 0;
      free(buf);
   }

   return returnValue;
}

void serverWork(int server_socket) {
   int client_socket = 0, largestSocket, rcv;
   uint16_t ssock_amountToRead = 0;
   fd_set read;
   uint8_t *handle, handle_len, responseFlag, *buf;
   client_list *cl;
   client *iter;

   buf = safeCharMalloc(MAX_PKT_LEN);
   largestSocket = server_socket + 1;
   createClientList(&cl);

   while (1) {
	  largestSocket = setClients(cl, &read, server_socket, largestSocket);
	  memset(buf, MAX_PKT_LEN, 0);
	  iter = cl->clients;

	  select(largestSocket, (fd_set *)&read, (fd_set *)0, (fd_set *)0, NULL);

	  if (FD_ISSET(server_socket, &read)) {
		 client_socket = newClient(server_socket);	// look for client to serve
		 rcv = recvPacket(client_socket, (uint8_t *)&ssock_amountToRead, PKT_LEN_FIELD);

		 ssock_amountToRead = ntohs(ssock_amountToRead) - PKT_LEN_FIELD;
		 handle_len = readNewClientPacket(client_socket, &handle, ssock_amountToRead);
		 responseFlag = addClient(&cl, handle_len, handle, client_socket);

		 sendNormalPacket(client_socket, responseFlag);
		 if (responseFlag == F_HANDLE_EXIST)
	        close(client_socket);
	  }
	  // check all client sockets
	  while (iter) {
		 if (FD_ISSET(iter->socket_num, &read))
			if (readFromClientSocket(&cl, &iter))
			   removeClient(&cl, iter);
		 iter = iter->next;
	  }
   }
}

int main(int argc, char *argv[]) {
   int server_socket= 0;   //socket descriptor for the server socket

   server_socket = tcp_recv_setup();	//create the server socket
   tcp_listen(server_socket, 5);
   serverWork(server_socket);

   return 0;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */
int tcp_recv_setup() {
   int server_socket= 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len= sizeof(local);  /* length of local address        */

   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_STREAM, 0);    
   if(server_socket < 0) {
      perror("socket call");
      exit(1);
   }

   local.sin_family= AF_INET;         //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= htons(0);                 //let system choose the port

   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
	   perror("bind call");
	   exit(-1);
   }
    
   //get the port name and print it out
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
	   perror("getsockname call");
	   exit(-1);
   }

   printf("socket has port %d \n", ntohs(local.sin_port));
	        
   return server_socket;
}

/* This function waits for a client to ask for services. */
void tcp_listen(int server_socket, int back_log) {
   if (listen(server_socket, back_log) < 0) {
      perror("listen call");
      exit(-1);
   }
}

int newClient(int server_socket) {
   int client_socket= 0;

   if ((client_socket = accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0) {
	  perror("accept call");
	  exit(-1);
   }
   return(client_socket);
}
