# Makefile for CPE464

CC= gcc
CFLAGS= -g

# The  -lsocket -lnsl are needed for the sockets.
# The -L/usr/ucblib -lucb gives location for the Berkeley library needed for
# the bcopy, bzero, and bcmp.  The -R/usr/ucblib tells where to load
# the runtime library.

# The next line is needed on Sun boxes (so uncomment it if your on a
# sun box)
# LIBS =  -lsocket -lnsl

# For Linux/Mac boxes uncomment the next line - the socket and nsl
# libraries are already in the link path.
LIBS =

all:  cclient server

cclient: tcp_client.c helpers.c packet.c networks.h testing.o
	$(CC) $(CFLAGS) -o cclient -Wall tcp_client.c helpers.c packet.c testing.o $(LIBS)

server: tcp_server.c helpers.c client_linked_list.c packet.c networks.h testing.o
	$(CC) $(CFLAGS) -o server -Wall tcp_server.c client_linked_list.c packet.c helpers.c testing.o $(LIBS)

client_linked_list: client_linked_list.c testing.o
	$(CC) $(CFLAGS) -o client_linked_list -Wall client_linked_list.c testing.o $(LIBS)

testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c

clean:
	rm -f server cclient client_linked_list.o testing.o

handin:
	/usr/local/bin/handin csc-cpe464 prog2_chat tcp_client.c tcp_server.c client_linked_list.c client_linked_list.h helpers.c helpers.h Makefile networks.h packet.c packet.h README testing.c testing.h
