# Makefile for CPE464 

CC= g++
CFLAGS= 

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

cclient: chat_client.cpp chat_client_funcs.h testing.o
	$(CC) $(CFLAGS) -o cclient -Wall chat_client.cpp chat_client_funcs.h  testing.o $(LIBS)

server: chat_server.cpp chat_server_funcs.cpp chat_server_funcs.h testing.o
	$(CC) $(CFLAGS) -o server -Wall chat_server.cpp chat_server_funcs.cpp chat_server_funcs.h testing.o $(LIBS)

testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c
clean:
	rm -f server cclient testing.o



