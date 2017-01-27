/******************************************************************************
 * tcp_client.c
 *
 *****************************************************************************/

#include "networks.h"
#include "testing.h"

void checkArguments(int argc, char **argv) {
   if (argc != 4) {
      printf("usage: %s handle host-name port-number \n", argv[0]);
      exit(-1);
   }
}

uint8_t notifyServer(int socket_num, char *handle) {
   normal_header *h;
   uint8_t *data, handle_len;
   int offset = 0, sent = 0, rcv = 0;

   h = safeNormalHeader();
   h->flag = F_INIT;
   handle_len = findStringLength(handle);
   h->pkt_length = htons(sizeof(normal_header) + 1 + handle_len);

   data = safeCharMalloc(ntohs(h->pkt_length));
   memcpy(data, h, sizeof(normal_header));
   offset += sizeof(normal_header);
   memset(data + (char)offset, (char)(handle_len), 1);
   offset += 1;
   memcpy(data + (char)offset, handle, handle_len);

   sent = sendPacket(socket_num, data, ntohs(h->pkt_length));
   rcv = recvPacketClient(socket_num, data, sizeof(normal_header));
   memcpy(h, data, sizeof(normal_header));

   if (h->flag == F_HANDLE_EXIST) {
      printf("Handle already in use: %.*s\n", handle_len, handle);
      exit(-1);
   }

   free(h);
   free(data);
   return handle_len;
}

void buildMessage(int socket_num, uint8_t *com, char *handle, uint8_t h_len) {
   uint8_t *d_handle, *msg, flag, dh_len;
   uint16_t msg_len;
   int bufEmpty = 0;

   d_handle = safeCharMalloc(MAX_HANDLE_LEN);
   flag = !memcmp(com, MESSAGE, CMD_LEN) ? F_MESSAGE : F_BROADCAST;

   if (flag == F_MESSAGE) {
	  getchar(); // removes white space between command and handle from stdin
      bufEmpty = getChars(&d_handle, (uint16_t *)&dh_len, ' ', MAX_HANDLE_LEN, 0);
   }

   msg = safeCharMalloc(MAX_MSG_LEN);

   if (!bufEmpty) {   // if the message or broadcast has text
	  // loop until new line, accounts for messages > 1000 bytes
      while (!getChars(&msg, &msg_len, '\n', MAX_MSG_LEN - 1, 1)) {
	     sendMessage(socket_num, flag, msg, msg_len, handle, h_len, d_handle, dh_len);
	     memset(msg, 0, MAX_MSG_LEN);
      }
   }
   else {	// build empty message
	  memset(msg, '\0', 1);
	  msg_len = 1;
   }
   sendMessage(socket_num, flag, msg, msg_len, handle, h_len, d_handle, dh_len);

   free(d_handle);
   free(msg);
}

void sendMessage(int socket_num, uint8_t flag, uint8_t *msg, uint16_t msg_len,
   char *handle, uint8_t handle_len, uint8_t *d_handle, uint8_t dh_len) {

   normal_header *h;
   int offset = 0;
   uint8_t *buf;
   uint16_t pkt_len;

   if (flag == F_BROADCAST) {
      msg++;   // truncates preceding white space on broadcast message
      msg_len--;
   }

   h = safeNormalHeader();
   h->flag = flag;
   pkt_len = sizeof(normal_header) + 1 + handle_len + msg_len;
   if (flag == F_MESSAGE)
      pkt_len += 1 + dh_len;
   h->pkt_length = htons(pkt_len);

   buf = safeCharMalloc(pkt_len);
   memcpy(buf, h, sizeof(normal_header));
   offset += sizeof(normal_header);

   if (flag == F_MESSAGE) {
      memset(buf + (char)offset, (char)dh_len, 1);
      offset += 1;
      memcpy(buf + (char)offset, d_handle, dh_len);
      offset += dh_len;
   }
   memset(buf + (char)offset, (char)handle_len, 1);
   offset += 1;
   memcpy(buf + (char)offset, handle, handle_len);
   offset += handle_len;
   memcpy(buf + (char)offset, msg, msg_len);

   sendPacket(socket_num, buf, pkt_len);
   free(buf);
   free(h);
}

void printMessage(uint8_t *buf, uint8_t flag, uint16_t pkt_len) {
   uint8_t *s_handle, *msg, dh_len, sh_len;
   int offset = 0;

   s_handle = safeCharMalloc(MAX_HANDLE_LEN);
   msg = safeCharMalloc(MAX_MSG_LEN);

   if (flag == F_MESSAGE) {		// skips over destination handle information
      memcpy(&dh_len, buf, 1);
      offset += 1 + dh_len;
   }

   memcpy(&sh_len, buf + (char)offset, 1);
   offset += 1;
   memcpy(s_handle, buf + (char)offset, sh_len);
   offset += sh_len;
   memcpy(msg, buf + (char)offset, pkt_len - offset);

   printf("\n%.*s: %.*s\n", sh_len, s_handle, pkt_len - offset, msg);

   free(s_handle);
   free(msg);
}

void printHandles(int socket_num, int numHandles) {
   uint8_t *handle, handle_len;

   handle = safeCharMalloc(MAX_HANDLE_LEN);

   printf("Number of clients: %d\n", numHandles);
   while (numHandles--) {
      recvPacketClient(socket_num, &handle_len, 1);
      recvPacketClient(socket_num, handle, handle_len);
      printf("   %.*s\n", handle_len, handle);
      memset(handle, 0, MAX_HANDLE_LEN);
   }
   free(handle);
}

void printHandleError(int socket_num, uint16_t pkt_len) {
   uint8_t handle_len, *handle;
   int rcv;

   handle = safeCharMalloc(MAX_HANDLE_LEN);
   rcv = recvPacketClient(socket_num, &handle_len, 1);
   rcv = recvPacketClient(socket_num, handle, handle_len);

   printf("\nClient with handle %.*s does not exist.\n", handle_len, handle);
   free(handle);
}

int checkCommand(int socket_num, char *handle, uint8_t handle_len) {
   uint8_t *command;
   uint16_t c_len;
   int ret = 1;

   command = safeCharMalloc(CMD_LEN + 1);
   getChars(&command, &c_len, ' ', CMD_LEN, 0);
   command[1] = tolower(command[1]);

   if (c_len != CMD_LEN) {   // checks for newline or 1-character inputs
      printf("Invalid command\n");
      if (c_len > 1)
         clearStdin();
   }
   else if (memcmp(command, EXIT, CMD_LEN) != 0) {
      if (!memcmp(command, MESSAGE, CMD_LEN) || !memcmp(command, BROADCAST, CMD_LEN))
         buildMessage(socket_num, command, handle, handle_len);
      else if (!memcmp(command, LIST, CMD_LEN)) {
         sendNormalPacket(socket_num, F_HANDLE_LIST);
         clearStdin();
         ret = 0;
      }
      else {
         printf("Invalid command\n");
         clearStdin();
      }
   }
   else {
      sendNormalPacket(socket_num, F_CLIENT_EXIT);
      ret = 0;
   }

   free(command);
   return ret;
}

int readSocket(int socket_num) {
   int rcv, retValue = 0;
   static uint16_t pkt_len = 0;
   static int state = START, offset = 0, numHandles = 0;
   static uint8_t *buf, flag = 0;

   if (state == START)
      buf = safeCharMalloc(MAX_PKT_LEN);

   switch (state) {
      case START:
         rcv = recvPacketClient(socket_num, (uint8_t *)&pkt_len, PKT_LEN_FIELD);
         state = (rcv < PKT_LEN_FIELD) ? GET_LEN : GET_FLAG;
         break;
      case GET_LEN:
    	 rcv = recvPacketClient(socket_num, (uint8_t *)(&pkt_len + (char)1), 1);
    	 state = GET_FLAG;
    	 break;
      case GET_FLAG:
    	 rcv = recvPacketClient(socket_num, &flag, 1);
    	 pkt_len = ntohs(pkt_len) - PKT_LEN_FIELD - 1;
    	 state = setState(flag);
    	 break;
      case GET_MESSAGE:
    	 rcv = recvPacketClient(socket_num, buf + (char)offset, pkt_len - offset);
    	 if (rcv < (pkt_len - offset))
    	    offset = pkt_len - offset - rcv;
    	 else {
    		printMessage(buf, flag, pkt_len);
       	    resetValues(&buf, &flag, &pkt_len, &offset, &state, &retValue);
    	 }
    	 break;
      case HANDLE_NEXIST:
    	 printHandleError(socket_num, pkt_len);
    	 resetValues(&buf, &flag, &pkt_len, &offset, &state, &retValue);
    	 break;
      case GET_NUM_HANDLES:
    	 rcv = recvPacketClient(socket_num, (uint8_t *)&numHandles, sizeof(int));
    	 resetValues(&buf, &flag, &pkt_len, &offset, &state, &retValue);
    	 retValue = 0;
    	 break;
      case GET_HANDLES:
    	 printHandles(socket_num, numHandles);
    	 resetValues(&buf, &flag, &pkt_len, &offset, &state, &retValue);
    	 break;
   }
   return retValue;
}

void userPrompt(int socket_num, char *handle, uint8_t handle_len) {
   fd_set read;
   int print = 1;

   while (1) {
      if (print)
	     printf("$: ");

      FD_ZERO(&read);
      FD_SET(STDIN_FILENO, &read);
      FD_SET(socket_num, &read);
	  select(socket_num + 1, (fd_set *)&read, (fd_set *)0, (fd_set *)0, NULL);

	  if (FD_ISSET(STDIN_FILENO, &read))
	     print = checkCommand(socket_num, handle, handle_len);
	  if (FD_ISSET(socket_num, &read))
	     print = readSocket(socket_num);
   }
}

int main(int argc, char *argv[]) {
   int socket_num;
   uint8_t h_len;

   setbuf(stdout, NULL);

   checkArguments(argc, argv);	/* check command line arguments  */
   socket_num = tcp_send_setup(argv[2], argv[3]); /* set up TCP socket */
   h_len = notifyServer(socket_num, argv[1]); /* notify server of new handle */
   userPrompt(socket_num, argv[1], h_len); /* start user command prompt */

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

void resetValues(uint8_t **buf, uint8_t *flag, uint16_t *pkt_len, int *offset,
   int *state, int *retValue) {
   free(*buf);
   *flag = 0;
   *pkt_len = 0;
   *offset = 0;
   *state = START;
   *retValue = 1;
}

int setState(uint8_t flag) {
   int state;

   if (flag == F_BROADCAST || flag == F_MESSAGE)
      state = GET_MESSAGE;
   else if (flag == F_NUM_HANDLES)
	  state = GET_NUM_HANDLES;
   else if (flag == F_HANDLES)
	  state = GET_HANDLES;
   else if (flag == F_HANDLE_NEXIST)
	  state = HANDLE_NEXIST;
   else if (flag == F_ACK_EXIT)
	  exit(0);
   return state;
}
