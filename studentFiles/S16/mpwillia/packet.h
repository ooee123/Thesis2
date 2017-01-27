
#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

// maximum bytes a message can have, including null terminator
#define MAX_MSG_LEN 1000
// maximum bytes a message can have, not including null terminator
#define MAX_MSG_CHAR_LEN (MAX_MSG_LEN - 1)

// maximum bytes a handle can have
#define MAX_HANDLE_LEN 255

// standard packet header
#pragma pack(1)
struct header {
   uint16_t len;
   uint8_t flags;
};
#define HEADER_LEN 3

// header flag values
enum {
   TERM           = 0,  // when an empty header is returned signaling the socket closed
   C_CONNECT      = 1,  // client connects to server, requests handle
   S_ACCEPT       = 2,  // server accepts client, good handle
   S_ERR_HANDLE   = 3,  // server rejects client, bad handle
   C_BCAST        = 4,  // client broadcasting a message
   C_MSG          = 5,  // cient sending a message
   S_ERR_DST      = 7,  // server rejects message, destination doesn't exist
   C_EXIT         = 8,  // client disconnects from server
   S_EXIT_OK      = 9,  // server acknowledges client disconnecting
   C_LIST         = 10, // client requests list of all connected clients
   S_LIST_NUM     = 11, // server responds with number of connected clients
   S_LIST_WHO     = 12  // server response with list of all connected clients
};

// creates a header with the given flag to start a packet
int createPacket(int flag, uint8_t** packet);

// creates an empty header
#define EMPTY_HEADER (calloc(HEADER_LEN, sizeof(uint8_t)))


// unpacked address for source and destination addresses of packets
// simplifies creation of full packets
struct unpacked_address {
   int len;   // length of the handle (will be truncated to 255 if too long)
   char* handle;  // handle with no null terminator
};


// adds an unpacked_address to the given packet, reallocs space as needed and
// updates the packet length field in the header
// returns the new length of the packet
int addAddress(struct unpacked_address* addr, uint8_t** packet);

// adds the given message to the packet, reallocs space as need and updates
// the packet length field in the header.
// does not check the size of the message
// returns the new length of the packet
int addMessage(char* message, uint8_t** packet);





// functions like send but if it would return < 0 it throws an error
int safeSend(int socket, const void* buffer, size_t length, int flags);
// functions like recv but if it would return < 0 it throws an error
int safeRecv(int socket, void* buffer, size_t length, int flags);

// sends just a header to the given socket with the given flag
// useful for sending small, flag only messages such as C_EXIT or S_EXIT_OK
int sendHeader(int socket, int flag);
// recieves just a header from the given socket, returns the flag
int recvHeader(int socket);

/* Attempts to recieve a packet from the given socket.
 * Returns NULL if the full packet hasn't arrived yet.
 * Repeated calls with the same socket will continue to recieve data from the
 * socket until the full packet has arrived, in which case the full packet data 
 * is returned.
 * While wait to recieve a full packet from one socket any calls to this
 * function with another socket will return NULL without recieving any data.
 */
uint8_t* receive_packet(int socket);

/*
// represents a partial packet for
struct partial_packet {
   int who;       // who this partial packet is for
   int haveLen;   // how much data (bytes) we have so far
   int packetLen; // how much data (bytes) we are expecting to end up with
   uint8_t* data; // the data we've receieved
};
*/

#endif

