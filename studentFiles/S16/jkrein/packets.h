#define INITIAL_CLIENT 1    //Initial packet to server
                            //Has 1 byte handle length, then handle
                            //Client blocks until 2 or 3 flag recieved
                            //Server casts to client_handle_info
#define HANDLE_CONFIRM 2    //Server confirms good flag. No extra struct/NH
#define HANDLE_EXISTS 3     //Server response that the handle exists. NH
#define CLIENT_BCAST 4      //Flag to say that this packet should be broadcast
                            //Both client and server use bcast_packet
#define DIRECT_MESSAGE 5    //Client is going to send to another client
                            //Sending client doesn't block
                            //Use struct direct_message
#define HANDLE_DNE 7        //Handle from a DM doesn't exist. Make sure
                            //to include original handle in this
                            //Use struct handle_dne_error
#define CLIENT_EXIT 8       //Client sends to server while terminating.
                            //Doesn't terminate until a 9 is received
                            //NH
#define ACK_EXIT 9          //Server acknowledgement that client is exiting
                            //NH
#define GET_HANDLES 10      //Client wants to get the handles. %L
                            //NH
#define SEND_NUM_HANDLES 11 //Server tells clients how many handles will send
                            //Use struct num_handles
#define SEND_HANDLES 12     //After 11,sends all handles preceded by handle len
                            //See docs for description
                            //use struct handle_list
#define NORMLEN 3


typedef struct normal_header
{
    u_short packetLen;  //Length of the packet in network order
    u_char flag;        //Flag. Possible flags defined at top
}__attribute__((packed)) normal_header;

/*Used when receiving a flag 4. Server gets client handle, sends back
 * to all clients besides the one that sent. Clients read the message*/
typedef struct bcast_packet
{
    struct normal_header header;
    char senderHandleLen;
    char *beginHandle; //Use this + senderHandleLen to get to message
}__attribute__((packed)) bcast_packet;

/*Used to receive flag 5, by server and receiving client*/
typedef struct direct_message
{
    struct normal_header header;
    char destHandleLen;
    char *destHandleStart; //To access message, this + destHandleLen
}__attribute__((packed)) direct_message;

/*Below this are used by the client to read headers. Server should create
 * headers of the same format using malloc
 **/
/*CLIENT WILL USE THESE HEADERS ON RECEIVED PACKETS*/
/*
 * Sent with Flag 11 in header, informing the client how many handles are
 * coming
 **/
typedef struct num_handles
{
    struct normal_header header;
    uint32_t numHandles;
}__attribute__((packed)) num_handles;

/*
 * Send immediately following the above, with flag 12. Contains
 * the number of handles specified by flag 11, each handle
 * preceded by how long it is.
 **/
typedef struct handle_list
{
    struct normal_header header;
    char *handles;
}__attribute__((packed)) handle_list;

/*Used when flag is = 7*/
typedef struct handle_dne_error
{
    struct normal_header header;
    char handleLen;
    char *attemptedDestination; //Name of the person who was attempted sent to
}__attribute__((packed)) handle_dne_error;

/* SERVER HEADERS TO APPLY TO RECEIVED PACKETS */

/*Apply to packet with flag 1*/
typedef struct client_handle_info
{
    struct normal_header header;
    char handleLen;
    char handle; //Points to the number of chars specified by handleLen
}__attribute__((packed)) client_handle_info;


