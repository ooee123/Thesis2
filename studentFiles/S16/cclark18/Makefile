# Chris Clark
# CPE 464
# Program 2

CC= g++
CFLAGS= -g -std=c++98

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

CLIENT_SRC = tcp_client.cpp chat_util.cpp
SERVER_SRC = tcp_server.cpp chat_util.cpp
TEST_SRC = testing.c
HEADERS = chat_util.h testing.h

all:  cclient server

cclient: $(CLIENT_SRC) $(HEADERS) tcp_client.h testing.o
	$(CC) $(CFLAGS) -o cclient -Wall $(CLIENT_SRC) testing.o $(LIBS)

server: $(SERVER_SRC) $(HEADERS) tcp_server.h testing.o
	$(CC) $(CFLAGS) -o server -Wall $(SERVER_SRC) testing.o $(LIBS)

testing.o: $(TEST_SRC) testing.h
	   $(CC) $(CFLAGS) -c $(TEST_SRC)
clean:
	rm -f server cclient testing.o