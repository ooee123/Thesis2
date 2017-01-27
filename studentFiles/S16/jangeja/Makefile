# Makefile for CPE464 

CC= gcc
CFLAGS= -g -ggdb 

# The  -lsocket -lnsl are needed for the sockets.
# The -L/usr/ucblib -lucb gives location for the Berkeley library needed for
# the bcopy, bzero, and bcmp.  The -R/usr/ucblib tells where to load
# the runtime library.

# The next line is needed on Sun boxes (so uncomment it if your on a
# sun box)
# LIBS =  -lsocket -lnsl

# For Linux/Mac boxes uncomment the next line - the socket and nsl
# libraries are already in the link path.
#LIBS

all:  cclient server tcp_read.o

cclient: tcp_client.c networks.h testing.o tcp_read.o
	$(CC) $(CFLAGS) -o cclient -Wall tcp_client.c testing.o tcp_read.o $(LIBS)

server: tcp_server.c networks.h testing.o tcp_read.o
	$(CC) $(CFLAGS) -o server -Wall tcp_server.c testing.o tcp_read.o $(LIBS)

tcp_read.o: tcp_read.c tcp_read.h
	   $(CC) $(CFLAGS) -c tcp_read.c

testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c
clean:
	rm -f server cclient testing.o tcp_read.o



