# Makefile for CPE464 

CC= gcc
CFLAGS= -g -Wall

# The  -lsocket -lnsl are sometimes needed for the sockets.
# The -L/usr/ucblib -lucb gives location for the Berkeley library needed for
# the bcopy, bzero, and bcmp.  The -R/usr/ucblib tells where to load
# the runtime library.

LIBS =

all:  cclient server

cclient: cclient.c networks.o testing.o
	$(CC) $(CFLAGS) -o cclient cclient.c networks.o testing.o $(LIBS)

server: server.c networks.o testing.o
	$(CC) $(CFLAGS) -o server server.c networks.o testing.o $(LIBS)

networks.o: networks.c networks.h
	$(CC) $(CFLAGS) -c networks.c

chat.o: chat.c chat.h
	$(CC) $(CFLAGS) -c chat.c

testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c

clean:
	rm -f server cclient *.o



