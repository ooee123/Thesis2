#define HEADER_LEN 3
#define HANDLE_SIZE 256

#define INIT_CLIENT 1
#define CONF_HANDLE 2
#define DENY_HANDLE 3
#define BROADCAST 4
#define MESSAGE 5
#define HANDLE_DNE 7
#define CLIENT_EXIT 8
#define ACK_EXIT 9
#define GET_HANDLES 10
#define NUM_HANDLES 11
#define SINGLE_HANDLE 12

typedef struct {
  uint16_t len;
  uint8_t flag;
} Header;
