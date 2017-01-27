CPP=g++
CC=gcc
CFLAGS=-Wall -Werror -g
CPPFLAGS=-Wall -Werror -g -std=c++0x

all: cclient server

cclient: cclient.o message.o networks.o selector.o testing.o
	$(CPP) -o cclient cclient.o message.o networks.o selector.o testing.o

server: server.o message.o networks.o selector.o
	$(CPP) -o server server.o message.o networks.o selector.o testing.o

cclient.o: cclient.cpp constants.hpp message.hpp networks.hpp selector.hpp testing.h types.hpp
	$(CPP) $(CPPFLAGS) -c cclient.cpp

server.o: server.cpp constants.hpp message.hpp networks.hpp selector.hpp testing.h types.hpp
	$(CPP) $(CPPFLAGS) -c server.cpp

message.o: message.cpp message.hpp constants.hpp testing.h
	$(CPP) $(CPPFLAGS) -c message.cpp

networks.o: networks.cpp networks.hpp testing.h
	$(CPP) $(CPPFLAGS) -c networks.cpp

selector.o: selector.cpp selector.hpp testing.h
	$(CPP) $(CPPFLAGS) -c selector.cpp

testing.o: testing.c testing.h
	$(CC) $(CFLAGS) -c testing.c

clean:
	rm *.o server cclient
