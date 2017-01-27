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

cclient: cclient.cpp ClientApp.h ClientApp.cpp ClientConnection.h ClientConnection.cpp ProcessMessage.h ProcessMessage.cpp ConstructMessage.h ConstructMessage.cpp PacketStructs.h testing.o
		$(CC) $(CFLAGS) -o cclient -Wall cclient.cpp ClientApp.h ClientApp.cpp ClientConnection.h ClientConnection.cpp ProcessMessage.h ProcessMessage.cpp ConstructMessage.h ConstructMessage.cpp PacketStructs.h testing.o $(LIBS)

server: server.cpp ServerApp.h ServerApp.cpp ServerConnection.h ServerConnection.cpp PacketStructs.h testing.o
		$(CC) $(CFLAGS) -o server -Wall server.cpp ServerApp.h ServerApp.cpp ServerConnection.h ServerConnection.cpp PacketStructs.h testing.o  $(LIBS)

testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c
clean:
	rm -f server cclient testing.o



