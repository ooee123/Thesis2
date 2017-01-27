# Makefile for CPE464 

CC= gcc
CFLAGS= -g
pp= g++

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

all: testing.o cclient server

cclient: tcp_client.cpp networks.h testing.o
	$(pp) $(CFLAGS) -o cclient -Wall tcp_client.cpp testing.o $(LIBS)

server: tcp_server.cpp networks.h testing.o
	$(pp) $(CFLAGS) -o server -Wall tcp_server.cpp testing.o $(LIBS)

testing.o: testing.c testing.h
	   $(pp) $(CFLAGS) -c testing.c
clean:
	rm -f server cclient testing.o



