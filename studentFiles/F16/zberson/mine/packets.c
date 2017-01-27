

unsigned int __bswap_32(unsigned int a) /* = * ( pos ) ; if ( raw -> type */{
return __builtin_bswap32(a); /* return __builtin_bswap32(__bsx) */
}
__uint64_t __bswap_64(__uint64_t a) /* NULL6 || raw -> type > FLAG12 ) { */{
return __builtin_bswap64(a); /* return __builtin_bswap64(__bsx) */
}
void clear_raw_packet(raw_packet *a) /* static void clear_raw_packet ( raw_packet * packet ) { */{
a->type = NULL0; /* packet -> type = NULL0 ; */
a->packet_len = 0; /* packet -> packet_len = 0 ; */
a->src_handle_len = 0; /* packet -> src_handle_len = 0 ; */
a->src_handle_name = ((void *)0 ); /* packet -> src_handle_name = ( ( void * ) 0 ) ; */
a->dest_handle_len = 0; /* packet -> dest_handle_len = 0 ; */
a->dest_handle_name = ((void *)0 ); /* packet -> dest_handle_name = ( ( void * ) 0 ) ; */
a->message = ((void *)0 ); /* packet -> message = ( ( void * ) 0 ) ; */
a->to_be_sent = 0; /* packet -> to_be_sent = 0 ; */
}
raw_packet * create_raw_packet() /* raw_packet * create_raw_packet ( ) { */{
raw_packet *a = calloc(1, sizeof(raw_packet)); /* raw_packet * raw = calloc ( 1 , sizeof ( raw_packet ) ) ; */
clear_raw_packet(a); /* clear_raw_packet ( raw ) ; */
return a; /* return raw */
}
rdy_packet * create_rdy_packet(uint16_t a) /* rdy_packet * create_rdy_packet ( uint16_t size ) { */{
rdy_packet *b = calloc(a, sizeof(char)); /* rdy_packet * rdy = calloc ( size , sizeof ( char ) ) ; */
return b; /* return rdy */
}
void destroy_raw_packet(raw_packet *a) /* void destroy_raw_packet ( raw_packet * packet ) { */{
if (a->src_handle_name) /* if (packet->src_handle_name) */ {
free(a->src_handle_name); /* free ( packet -> src_handle_name ) ; */
a->src_handle_name = ((void *)0 ); /* packet -> src_handle_name = ( ( void * ) 0 ) ; */
}
if (a->dest_handle_name) /* if (packet->dest_handle_name) */ {
free(a->dest_handle_name); /* free ( packet -> dest_handle_name ) ; */
a->dest_handle_name = ((void *)0 ); /* packet -> dest_handle_name = ( ( void * ) 0 ) ; */
}
if (a->message) /* if (packet->message) */ {
free(a->message); /* free ( packet -> message ) ; */
a->message = ((void *)0 ); /* packet -> message = ( ( void * ) 0 ) ; */
}
free(a); /* free ( packet ) ; */
}
void destroy_rdy_packet(rdy_packet *a) /* void destroy_rdy_packet ( rdy_packet * packet ) { */{
if (a) /* if (packet) */ {
free(a); /* free ( packet ) ; */
}
}
raw_packet * recycle_raw_packet(raw_packet *a) /* raw_packet * recycle_raw_packet ( raw_packet * packet ) { */{
destroy_raw_packet(a); /* destroy_raw_packet ( packet ) ; */
return create_raw_packet(); /* return create_raw_packet() */
}
rdy_packet * recycle_rdy_packet(rdy_packet *a, uint16_t b) /* rdy_packet * recycle_rdy_packet ( rdy_packet * packet , uint16_t size ) { */{
destroy_rdy_packet(a); /* destroy_rdy_packet ( packet ) ; */
return create_rdy_packet(b); /* return create_rdy_packet(size) */
}
void construct_packet_header(rdy_packet *a, raw_packet *b) /* static void construct_packet_header ( rdy_packet * rdy , raw_packet * raw ) { */{
uint8_t *c; /* uint8_t * pos ; */
if (b->type == FLAG12) /* if (raw->type == FLAG12) */ {
*(c ) = 0; /* * ( pos ) = 0 ; */
*(c + 1 ) = 0; /* * ( pos + 1 ) = 0 ; */
} else {
*((uint16_t *)c ) = htons(b->packet_len); /* * ( ( uint16_t * ) pos ) = htons ( raw -> packet_len ) ; */
}
c = a + 2; /* pos = rdy + 0x02 ; */
*(c ) = b->type; /* * ( pos ) = raw -> type ; */
}
int construct_packet(rdy_packet *a, raw_packet *b) /* int construct_packet ( rdy_packet * rdy , raw_packet * raw ) { */{
uint16_t c; /* uint16_t message_len ; */
uint8_t *d; /* uint8_t * pos ; */
if (b->type<FLAG1 || b->type == NULL6 || b->type>FLAG12) /* if (raw->type<FLAG1 || raw->type == NULL6 || raw->type>FLAG12) */ {
fprintf(stderr, "Invalid flag type '%u'\n", b->type); /* fprintf ( stderr , "Invalid flag type '%u'\n" , raw -> type ) ; */
return -1; /* return -1 */
}
construct_packet_header(a, b); /* construct_packet_header ( rdy , raw ) ; */
if (b->dest_handle_len) /* if (raw->dest_handle_len) */ {
*(d++ ) = b->dest_handle_len; /* * ( pos ++ ) = raw -> dest_handle_len ; */
memcpy(d, b->dest_handle_name, b->dest_handle_len); /* memcpy ( pos , raw -> dest_handle_name , raw -> dest_handle_len ) ; */
}
if (b->src_handle_len) /* if (raw->src_handle_len) */ {
*(d++ ) = b->src_handle_len; /* * ( pos ++ ) = raw -> src_handle_len ; */
memcpy(d, b->src_handle_name, b->src_handle_len); /* memcpy ( pos , raw -> src_handle_name , raw -> src_handle_len ) ; */
} else if (b->to_be_sent) /* if (raw->to_be_sent) */ {
*((uint32_t *)d ) = htonl(b->to_be_sent); /* * ( ( uint32_t * ) pos ) = htonl ( raw -> to_be_sent ) ; */
}
if (b->type == FLAG4 || b->type == FLAG5) /* if (raw->type == FLAG4 || raw->type == FLAG5) */ {
c = b->packet_len - b->src_handle_len - b->dest_handle_len - b->type; /* message_len = raw -> packet_len - raw -> src_handle_len - raw -> dest_handle_len - raw -> type ; */
memcpy(d, b->message, c); /* memcpy ( pos , raw -> message , message_len ) ; */
}
return 0; /* return 0 */
}
int deconstruct_packet(raw_packet *a, rdy_packet *b) /* int deconstruct_packet ( raw_packet * raw , rdy_packet * rdy ) { */{
uint16_t c; /* uint16_t message_len ; */
uint8_t *d; /* uint8_t * pos ; */
d = b + 2; /* pos = rdy + 0x02 ; */
a->type = *(d ); /* raw -> type = * ( pos ) ; */
if (a->type<FLAG1 || a->type == NULL6 || a->type>FLAG12) /* if (raw->type<FLAG1 || raw->type == NULL6 || raw->type>FLAG12) */ {
fprintf(stderr, "Invalid flag type '%u'\n", a->type); /* fprintf ( stderr , "Invalid flag type '%u'\n" , raw -> type ) ; */
return -1; /* return -1 */
}
if (a->type == FLAG12) /* if (raw->type == FLAG12) */ {
d = b + 3; /* pos = rdy + 0x03 ; */
a->packet_len = *(d ) + 4; /* raw -> packet_len = * ( pos ) + 0x04 ; */
} else {
d = b + 0; /* pos = rdy + 0x00 ; */
a->packet_len = ntohs(*((uint16_t *)d )); /* raw -> packet_len = ntohs ( * ( ( uint16_t * ) pos ) ) ; */
}
d = b + 3; /* pos = rdy + 0x03 ; */
switch (a->type) /* switch (raw->type) */ {
case FLAG5: a->dest_handle_len = *(d++ ); /* raw -> dest_handle_len = * ( pos ++ ) ; */
memcpy(a->dest_handle_name, d, a->dest_handle_len); /* memcpy ( raw -> dest_handle_name , pos , raw -> dest_handle_len ) ; */
d += a->dest_handle_len; /* pos += raw -> dest_handle_len ; */
a->dest_handle_name = calloc(a->dest_handle_len + 1, sizeof(uint8_t)); /* raw -> dest_handle_name = calloc ( raw -> dest_handle_len + 1 , sizeof ( uint8_t ) ) ; */
case FLAG1: case FLAG4: case FLAG7: case FLAG12: a->src_handle_len = *(d++ ); /* raw -> src_handle_len = * ( pos ++ ) ; */
memcpy(a->src_handle_name, d, a->src_handle_len); /* memcpy ( raw -> src_handle_name , pos , raw -> src_handle_len ) ; */
d += a->src_handle_len; /* pos += raw -> src_handle_len ; */
a->src_handle_name = calloc(a->src_handle_len + 1, sizeof(uint8_t)); /* raw -> src_handle_name = calloc ( raw -> src_handle_len + 1 , sizeof ( uint8_t ) ) ; */
break;
default: break;
}
if (a->type == FLAG4 || a->type == FLAG5) /* if (raw->type == FLAG4 || raw->type == FLAG5) */ {
c = a->packet_len - a->src_handle_len - a->dest_handle_len - a->type; /* message_len = raw -> packet_len - raw -> src_handle_len - raw -> dest_handle_len - raw -> type ; */
a->message = calloc(c + 1, sizeof(uint8_t)); /* raw -> message = calloc ( message_len + 1 , sizeof ( uint8_t ) ) ; */
memcpy(a->message, d, c); /* memcpy ( raw -> message , pos , message_len ) ; */
} else if (a->type == FLAG11) /* if (raw->type == FLAG11) */ {
a->to_be_sent = ntohl(*((uint32_t *)d )); /* raw -> to_be_sent = ntohl ( * ( ( uint32_t * ) pos ) ) ; */
}
return 0; /* return 0 */
}
