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

mine: cclient server

all:  cclient server

cclient: cclient.c testing.o networkFuncs.o
	$(CC) $(CFLAGS) -o cclient -Wall cclient.c networkFuncs.o testing.o $(LIBS)

server: server.c testing.o networkFuncs.o
	$(CC) $(CFLAGS) -o server -Wall server.c networkFuncs.o testing.o $(LIBS)

testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c

networkFuncs.o: networkFuncs.c networkFuncs.h
	$(CC) $(CFLAGS) -c networkFuncs.c

clean:
	rm -f server cclient testing.o networkFuncs.o
