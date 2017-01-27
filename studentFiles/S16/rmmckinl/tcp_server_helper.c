//
//  tcp_server_helper.c
//  Chat_TCP_Final
//
//  Created by Rebecca McKinley on 4/25/16.
//  Copyright © 2016 Rebecca McKinley. All rights reserved.
//

#include "tcp_server_helper.h"

//initiates all stuff to zero
void Make_Zero(Handle *clients) {
   int i = 0;
   
   while (i < 10) {
      (clients + i)->socket = 0;
      memset(clients + i, '\0', 257);
      i++; 
   }
}
