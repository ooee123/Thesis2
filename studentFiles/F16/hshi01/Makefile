# Makefile for CPE464 

CC= gcc
CFLAGS= -g

# The  -lsocket -lnsl are sometimes needed for the sockets.
# The -L/usr/ucblib -lucb gives location for the Berkeley library needed for
# the bcopy, bzero, and bcmp.  The -R/usr/ucblib tells where to load
# the runtime library.

LIBS =

all:  myClient myServer

myClient: networks.o testing.o cclient.c
	$(CC) $(CFLAGS) -o cclient -Wall cclient.c testing.o networks.o $(LIBS)

myServer: networks.o testing.o server.c
	$(CC) $(CFLAGS) -o server -Wall server.c testing.o networks.o $(LIBS)

networks.o: networks.c networks.h
	$(CC) $(CFLAGS) -c networks.c
	
testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c
clean:
	rm -f cclient server *.o



