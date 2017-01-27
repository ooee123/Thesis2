#include <stdint.h>
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




#define HEADER_LEN 3





struct chat_frame
{

	uint16_t length;
	uint8_t flag;
	uint8_t *message;
};

struct handle
{
	uint8_t active;
	char name[255];
	uint32_t socket;
};


 enum states
    {
        START_STATE,
        PROCESS_LEN_STATE,
        GET_DATA

    };

enum Flags
    {
    	ZERO,
        FLAG1,
        FLAG2,
        FLAG3,
        FLAG4,
        FLAG5,
        FLAG6,
        FLAG7,
        FLAG8,
        FLAG9,
        FLAG10,
        FLAG11,
        FLAG12
    };







//char *receive_data(struct handle client);