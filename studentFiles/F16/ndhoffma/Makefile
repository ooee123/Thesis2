# Makefile for CPE464 

CC = gcc
CFLAGS = -g

# The  -lsocket -lnsl are sometimes needed for the sockets.
# The -L/usr/ucblib -lucb gives location for the Berkeley library needed for
# the bcopy, bzero, and bcmp.  The -R/usr/ucblib tells where to load
# the runtime library.

LIBS =

all:  cclient server

cclient: cclient.c network.o testing.o
	$(CC) $(CFLAGS) -o cclient -Wall cclient.c testing.o network.o $(LIBS)

server: server.c network.o testing.o
	$(CC) $(CFLAGS) -o server -Wall server.c testing.o network.o $(LIBS)

network.o: network.c network.h
	$(CC) $(CFLAGS) -c network.c
	
testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c
clean:
	rm -f myServer myClient *.o



