#define MESSEGE_LEN 1000
#define STANDARD_HEADER_LEN 3

//Packet Flags
#define FLAG_1  1 //client connect packet
#define FLAG_2  2 // server ACK to connect packet
#define FLAG_3  3 // server RACK to connect packet
#define FLAG_4  4 // client/server broadcast packet
#define FLAG_5  5 // client/server messege packet
#define FLAG_6  6 // UNUSED
#define FLAG_7  7 // Server error packet to message packet
#define FLAG_8  8 // client exit packet
#define FLAG_9  9 // server ACK to client exit
#define FLAG_10  10 // client List packet
#define FLAG_11  11 //server ACK packet to list packet, contains number of client handles
#define FLAG_12  12 //server complete list of client handles

//Client State Flags
//limited use of state flags when reading input from STDIN. Only 0 and 1 are relevant
//0 init state
//1 = init finished, looping/selecting
//2 = Read the standard header from socket. (Using a blocking recv call to read 3 bytes)
//3 = currently reading from socket, total packet length not reached
//4 = finished reading packet length from socket, processed/executed packet
//5 = read input from STDIN. Processed/executed input.

//Server State flags
//0 = marked for destruction
//1 = init finished, looping/selecting
//2 = read the standard header from socket.
//3 = reading from a socket, total packet length not reached
//4 = finished reading packet length for only one socket, ready to process/execute that 1 packet
//5 = accepting on a connection from a client

// for the server side
int tcp_recv_setup();
void tcp_listen_setup(int server_socket, int back_log);

// for the client side
int tcp_send_setup(char *host_name, char *port);
