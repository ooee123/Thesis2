# Makefile for CPE464 

CC= gcc
CFLAGS= -g -Wall -Werror -Wno-unused-value

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

cclient: client.c packet.c testing.o socketWrapper.c
	$(CC) $(CFLAGS) -o cclient -Wall client.c packet.c socketWrapper.c testing.o $(LIBS)

server: server.c packet.c networks.h testing.o socketWrapper.c
	$(CC) $(CFLAGS) -o server -Wall server.c packet.c socketWrapper.c testing.o $(LIBS)

testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c
clean:
	rm -f server cclient *.o



