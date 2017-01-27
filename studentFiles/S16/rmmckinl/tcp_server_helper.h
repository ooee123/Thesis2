//
//  tcp_server_helper.h
//  Chat_TCP_Final
//
//  Created by Rebecca McKinley on 4/25/16.
//  Copyright © 2016 Rebecca McKinley. All rights reserved.
//

#ifndef tcp_server_helper_h
#define tcp_server_helper_h

#include "networks.h"

typedef struct Handle {
   int socket;
   char name[257]; //has a null termination
}__attribute__((packed)) Handle;

void Make_Zero(Handle *clients);


#endif /* tcp_server_helper_h */

