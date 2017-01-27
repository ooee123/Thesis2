
/* 	Code originally give to Prof. Smith by his TA in 1994.
	No idea who wrote it.  Copy and use at your own Risk
*/


#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#define BACKLOG 5

struct chatheader {
      uint16_t length;
      uint8_t  flag;
}__attribute__((packed));

struct flagOnePacket {
      struct chatheader chatheader;
      uint8_t length;
      char * handle;
}__attribute__((packed));

struct genericPacket {
      struct chatheader chatheader;
}__attribute__((packed));

struct flagSevenPacket {
      struct chatheader chatheader;
      uint8_t length;
      char * handleDst;
}__attribute__((packed));

struct flagElevenPacket {
      struct chatheader chatheader;
      uint32_t numhandles;
}__attribute__((packed));

struct message_packet {
      struct chatheader chatheader;
      uint8_t dstLen;
      char * dstHandle;
      uint8_t sndLen;
      char* sndHandle;
      char* msg;
}__attribute__((packed));

struct broadcast_packet {
      struct chatheader chatheader;
      uint8_t dstLen;
      char * sndHandle;
      char * msg;
}__attribute__((packed));

struct handle_packet {
      uint8_t handlelen;
      char * handle;
}__attribute__((packed));


// for the server side
int tcpServerSetup();
int tcpAccept(int server_socket);

// for the client side
int tcpClientSetup(char *host_name, char *port);


#endif
