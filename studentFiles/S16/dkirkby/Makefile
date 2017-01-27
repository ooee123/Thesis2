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

cclient: cclient.c networks.o testing.o handle.o
	$(CC) $(CFLAGS) -o cclient -Wall cclient.c testing.o networks.o handle.o $(LIBS)

server: server.c networks.o testing.o handle.o
	$(CC) $(CFLAGS) -o server -Wall server.c testing.o networks.o handle.o $(LIBS)

networks.o: networks.c networks.h
	$(CC) $(CFLAGS) -c networks.c

testing.o: testing.c testing.h
	$(CC) $(CFLAGS) -c testing.c

handle.o: handle.c handle.h
	$(CC) $(CFLAGS) -c handle.c

clean:
	rm -rf server cclient *.o *.dSYM

turnin:
	handin csc-cpe464 prog2_chat cclient.c handle.h networks.c README testing.c handle.c Makefile networks.h server.c testing.h


