# Makefile for CPE464 

CC= g++
CFLAGS=

# The  -lsocket -lnsl are sometimes needed for the sockets.
# The -L/usr/ucblib -lucb gives location for the Berkeley library needed for
# the bcopy, bzero, and bcmp.  The -R/usr/ucblib tells where to load
# the runtime library.

LIBS =

all:  cclient server

cclient: cclient.cpp networks.o testing.o
	$(CC) $(CFLAGS) -o cclient -Wall cclient.cpp testing.o networks.o $(LIBS)

server: server.cpp networks.o testing.o
	$(CC) $(CFLAGS) -o server -Wall server.cpp testing.o networks.o $(LIBS)

networks.o: networks.cpp networks.h
	$(CC) $(CFLAGS) -c networks.cpp
	
testing.o: testing.cpp testing.h
	   $(CC) $(CFLAGS) -c testing.cpp
clean:
	rm -f server cclient *.o



