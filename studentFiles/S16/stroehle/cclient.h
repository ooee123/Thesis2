#pragma once
#define _CCLIENT_
#ifndef _CCLIENT_

struct ChatHeader 
{
   char dest_handle_len;
   char dest_handle[];
   char src_handle_len;
   char src_handle[];
   char msg[]; //max 1000 bytes
} __attribute__((packed));

struct ChatData
{
   uint16_t length;
   char flags; //
};

#endif 