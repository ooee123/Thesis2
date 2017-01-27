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

cclient: cclient.c networks.h testing.o libChat.o
	$(CC) $(CFLAGS) -o cclient -Wall cclient.c testing.o libChat.o $(LIBS)

server: server.c networks.h testing.o libChat.o
	$(CC) $(CFLAGS) -o server -Wall server.c testing.o libChat.o $(LIBS)

testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c

libChat.o: libChat.c libChat.h
	$(CC) $(CFLAGS) -c libChat.c

clean:
	rm -f server cclient testing.o libChat.o



