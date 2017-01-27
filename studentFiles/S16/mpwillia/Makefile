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

cclient: client.c client.h packet.c packet.h testing.o
	$(CC) $(CFLAGS) -o cclient -Wall client.c packet.c testing.o $(LIBS)

server: server.c server.h client_handler.c client_handler.h packet.c packet.h testing.o
	$(CC) $(CFLAGS) -o server -Wall server.c client_handler.c packet.c testing.o$(LIBS)

testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c

clean:
	rm -f server cclient testing.o



