# Makefile for CPE464 

CC= gcc
CFLAGS= -g -Wall

# The  -lsocket -lnsl are sometimes needed for the sockets.
# The -L/usr/ucblib -lucb gives location for the Berkeley library needed for
# the bcopy, bzero, and bcmp.  The -R/usr/ucblib tells where to load
# the runtime library.

CLIENT_LIBS =
SERVER_LIBS =

all:  cclient server

cclient: cclient.c packets.o connection.o testing.o
	$(CC) $(CFLAGS) -o cclient cclient.c packets.o connection.o testing.o $(CLIENT_LIBS)

server: server.c packets.o connection.o handle.o testing.o
	$(CC) $(CFLAGS) -o server server.c packets.o connection.o handle.o testing.o $(SERVER_LIBS)

handin: server.c lib.h cclient.c cclient.h connection.c connection.h packets.c packets.h handle.c handle.h testing.c testing.h README Makefile
	handin csc-cpe464 prog2_chat server.c lib.h cclient.c cclient.h connection.c connection.h packets.c packets.h handle.c handle.h testing.c testing.h README Makefile

connection.o: connection.c connection.h
	$(CC) $(CFLAGS) -c connection.c

packets.o: packets.c packets.h
	$(CC) $(CFLAGS) -c packets.c	

handle.o: handle.c handle.h
	$(CC) $(CFLAGS) -c handle.c	

testing.o: testing.c testing.h
	$(CC) $(CFLAGS) -c testing.c
clean:
	rm -vf server cclient *.o



