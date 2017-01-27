# Makefile for CPE464 

CC= g++
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

cclient: tcp_client.cpp networks.h testing.o
	$(CC) $(CFLAGS) -o cclient -Wall tcp_client.cpp testing.o $(LIBS)

server: tcp_server.cpp networks.h testing.o
	$(CC) $(CFLAGS) -o server -Wall tcp_server.cpp testing.o $(LIBS)

testing.o: testing.cpp testing.h
	   $(CC) $(CFLAGS) -c testing.cpp
clean:
	rm -f server cclient testing.o



