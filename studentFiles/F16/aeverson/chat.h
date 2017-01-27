#define FAIL -1
#define PASS 0
#define FOUND 1
#define NOT_FOUND 0 

#define MAXBUF 1024
#define MAX_MSG 1000
#define MAX_NAME 255

#define INIT_SIZE_CONST 4
#define HEAD_SIZE_CONST 3
#define GIVE_HANDLE_CONST 4
#define MSG_SIZE_CONST 5
#define BCAST_SIZE_CONST 4
#define NUM_CLIENT_CONST 7

#define FLAG_OFF 2
#define MSG_DEST_LEN_OFF 3
#define MSG_DEST_OFF 4
#define BAD_DEST_LEN_OFF 3
#define REQ_HANDLE_LEN_OFF 3
#define MSG_SRC_LEN_OFF 4
#define MSG_TEXT_LEN_OFF 5
#define BCAST_SRC_LEN_OFF 3
#define BCAST_SRC_OFF 4


/* Flags: */
#define INIT_HANDLE 1
#define ACK_HANDLE 2
#define BAD_HANDLE 3
#define BROADCAST 4
#define MESSAGE 5
#define BAD_DEST 7
#define EXIT_REQ 8
#define ACK_EXIT 9
#define REQ_HANDLES 10
#define NUM_HANDLES 11
#define GIVE_HANDLE 12


typedef struct cclient {
	uint8_t len;
	char *name;
} cclient;

typedef struct chatHead {
	uint16_t packetLen;
	uint8_t flag;
} chatHead, handleACK, handleErr, listReq, exitReq;

typedef struct numClient {
	chatHead head;
	int32_t numClients;
} numClient;

typedef struct msgData {
	char *msg;
	char *remaining;
	uint32_t size;
} msgData;

typedef struct messagePacket {
	chatHead head;
	cclient *dest;
	cclient *src;
	msgData *text;
} messagePacket;

typedef struct broadcast {
	chatHead head;
	cclient *src;
	msgData *text;
} broadcast;

typedef struct initPacket {
	chatHead head;
	cclient *handle;
} initPacket, badDest, handleData;



