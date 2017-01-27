# Makefile for CPE464 

CC= gcc
CFLAGS= -g

# The  -lsocket -lnsl are sometimes needed for the sockets.
# The -L/usr/ucblib -lucb gives location for the Berkeley library needed for
# the bcopy, bzero, and bcmp.  The -R/usr/ucblib tells where to load
# the runtime library.

LIBS =

all:  cclient server

cclient: cclient.c networks.o testing.o node.o
	$(CC) $(CFLAGS) -o cclient -Wall cclient.c testing.o networks.o node.o $(LIBS)

server: server.c networks.o testing.o node.o
	$(CC) $(CFLAGS) -o server -Wall server.c testing.o networks.o node.o $(LIBS)

node.o: node.c node.h
	$(CC) $(CFLAGS) -c node.c

networks.o: networks.c networks.h
	$(CC) $(CFLAGS) -c networks.c
	
testing.o: testing.c testing.h
	$(CC) $(CFLAGS) -c testing.c

clean:
	rm -f server cclient *.o



