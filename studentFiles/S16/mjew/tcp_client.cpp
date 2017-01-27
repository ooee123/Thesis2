/******************************************************************************
 * tcp_client.c
 *
 *****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <errno.h>

#include "networks.h"
#include "testing.h"

std::vector<int> allrfds; //list of all file descriptors
std::string myName;
char stdHeadBuffer[3];
char * packetBuffer;	//where read packet bytes are stored
uint16_t packetLen;	//total packet length. Found in first 2 bytes of standard header
uint16_t packetFilled;	//how much of the sub-packet has been read(does not include standard header)
uint16_t packetToFill; 	//how much of the sub-packet needs to be read(standard header not included)
uint32_t handleListSize = 0;	//number of handles in the incoming handle list.
int stateFlag = 0;

void grabSTDIN();
void loopOverFDS(std::vector<int>, fd_set*);

//state 4 method, Resets global variables to read and process new packet
void cleanUp(int exit)
{
	free(packetBuffer);
	packetLen = 0;
	packetFilled = 0;
	packetToFill = 0;
	stateFlag = 1;
	if(exit)
	{
		for(uint32_t i = 0; i < allrfds.size(); i++)
			close(allrfds[i]);
		std::exit(0);
	}
}

//creates a packet. Allocates mem for specific size, then loads the standard header into the pointer including flag. Returns the pointer
//REMEMBER TO FREE THE POINTER LATER!!!!
uint8_t* createPacket(uint16_t length, uint8_t flag)
{
	uint8_t* rtn = (uint8_t*)calloc(sizeof(uint8_t*), length); //entire packet length allocated
	uint16_t length2 = htons(length);
	memcpy(rtn, &length2, 2);
	rtn[2] = flag;
	return rtn;
}

void sendPacketFLAG_1()
{
	//calc the total packet length
	uint16_t packetLength = STANDARD_HEADER_LEN + 1 + myName.length();
	//create the packet pointer
	uint8_t * packetToSend = createPacket(packetLength, FLAG_1);
	//load payload, begin with my name
	packetToSend[3] = (uint8_t)myName.length();
	strncpy((char*)packetToSend + STANDARD_HEADER_LEN + 1, myName.c_str(), myName.size()); //does not null terminate the string
	//send
	if(send(allrfds[1], packetToSend, packetLength, 0) < 0)
	{
		perror("sendPacketFLAG_1(): ");
		exit(-1);
	}
	free(packetToSend);
	//std::cout << "Client application packet sent! PacketLEngth: " << packetLength << std::endl;

}

//creates a broadcast packet and sends it to the server
void sendPacketFLAG_4(std::string msg)
{
	//calc the total packet length
	uint16_t packetLength = STANDARD_HEADER_LEN + 1 + myName.length() + msg.length();
	//create the packet pointer
	uint8_t * packetToSend = createPacket(packetLength, FLAG_4);
	//load payload, begin with my name
	packetToSend[3] = (uint8_t)myName.length();
	strncpy((char*)packetToSend + STANDARD_HEADER_LEN + 1, myName.c_str(), myName.size()); //does not null terminate the string
	strcpy((char*)packetToSend + STANDARD_HEADER_LEN + 1 + myName.size(), msg.c_str()); //messege of the packet
	//send
	if(send(allrfds[1], packetToSend, packetLength, 0) < 0)
	{
		perror("sendPacketFLAG_4(): ");
		exit(-1);
	}
	free(packetToSend);
	//std::cout << "Broadcast messege has been sent!" << " PacketLEngth: " << packetLength << std::endl;
}

//creates a messege packet and sends it to the server
void sendPacketFLAG_5(std::string recipient, std::string msg)
{
	//calc the total packet length
	uint16_t packetLength = STANDARD_HEADER_LEN + 1 + recipient.length() + 1 + myName.length() + msg.length();
	//create the packet pointer
	uint8_t * packetToSend = createPacket(packetLength, FLAG_5);
	//load payload, begin with recipient name
	packetToSend[3] = (uint8_t)recipient.length();
	strncpy((char*)packetToSend + STANDARD_HEADER_LEN + 1, recipient.c_str(), recipient.length()); //does not null terminate the string
	uint8_t mylength = myName.length();
	memcpy(packetToSend + STANDARD_HEADER_LEN + 1 + recipient.length(), &mylength, 1);
	strncpy((char*)packetToSend + STANDARD_HEADER_LEN + 1 + recipient.length() + 1, myName.c_str(), myName.length()); //does not null terminate the string
	strcpy((char*)packetToSend + STANDARD_HEADER_LEN + 1 + recipient.length() + 1 + myName.length(), msg.c_str()); //messege of the packet
	//send
	if(send(allrfds[1], packetToSend, packetLength, 0) < 0)
	{
		perror("sendPacketFLAG_5(): ");
		exit(-1);
	}
	free(packetToSend);
}

void sendPacketFLAG_8()
{
	//calc the total packet length
	uint16_t packetLength = STANDARD_HEADER_LEN;
	//create the packet pointer
	uint8_t * packetToSend = createPacket(packetLength, FLAG_8);
	//send
	if(send(allrfds[1], packetToSend, packetLength, 0) < 0)
	{
		perror("sendPacketFLAG_8(): ");
		exit(-1);
	}
	free(packetToSend);
	//std::cout << "Exit request messege has been sent!" << " PacketLEngth: " << packetLength << std::endl;
}

void sendPacketFLAG_10()
{
	//calc the total packet length
	uint16_t packetLength = STANDARD_HEADER_LEN;
	//create the packet pointer
	uint8_t * packetToSend = createPacket(packetLength, FLAG_10);
	//send
	if(send(allrfds[1], packetToSend, packetLength, 0) < 0)
	{
		perror("sendPacketFLAG_10(): ");
		exit(-1);
	}
	free(packetToSend);
}


//state 4 method, Reads the correct number of handles, out of the packet.
std::string processFLAG_12()
{
	std::string rtn = "";
	char * bufferPtr = packetBuffer;
	for(uint32_t handlesRead = 0; handlesRead < handleListSize; handlesRead++)
	{
		uint8_t h = *bufferPtr; //represents the length of the handle
		bufferPtr++;
		for(uint8_t i = 0; i < h; i++)
		{
			rtn += *bufferPtr;
			bufferPtr++;
		}
		rtn += " ";
	}
	return rtn;
}

//state 4 method. preps for incoming handles
std::string processFLAG_11()
{
	std::string rtn = "";
	uint32_t listSize = ntohl(*(uint32_t*)packetBuffer);
	handleListSize = listSize;
	{
	std::ostringstream oss;
	oss << listSize;
	rtn = oss.str();
	}
	return rtn;
}

//state 4 method. Returns string of bad recipient handle out of packet
std::string processFLAG_7()
{
	std::string rtn = "";
	uint8_t handleLen = packetBuffer[0];
	for(uint8_t i = 0; i < handleLen ; i++)
		rtn += packetBuffer[i + 1];
	return rtn;
}

std::string processFLAG_5()
{
	std::string rtn = "", handle = "";
	uint8_t destHandleLen = ((uint8_t*)packetBuffer)[0];
	uint8_t sendHandleLen = ((uint8_t*)packetBuffer)[1 + destHandleLen];
	for(uint8_t i = 0; i < sendHandleLen; i++)
		handle += packetBuffer[1 + destHandleLen + 1 + i];
	rtn += handle + ": ";
	rtn.append(packetBuffer + 1 + destHandleLen + 1 + sendHandleLen);
	return rtn;
}

std::string processFLAG_4()
{
	std::string rtn = "", handle = "";
	uint8_t handleLen = ((uint8_t*)packetBuffer)[0];
	for(uint8_t i = 0; i < handleLen; i++)
		handle += packetBuffer[i + 1];
	rtn += handle + ": ";
	rtn.append(packetBuffer + 1 + handleLen);
	return rtn;
}

//state 4 method
void processPacket()
{
	std::string output = "";
	int exit = 0;
	switch(stdHeadBuffer[2]) 
	{
		case FLAG_2 :
			std::cout << std::endl << "Handle is valid. Successfully joined server" << std::endl;
	       		break;
		case FLAG_3 :
	       		std::cout << std::endl << "Given handle name is invalid. Terminating client program." << std::endl;
			std::exit(0);
	       		break;
		case FLAG_4 :
	       		output = processFLAG_4();
	       		std::cout << std::endl << output << std::endl;
	       		break;
		case FLAG_5 :
			output = processFLAG_5();
			std::cout << std::endl << output << std::endl;
	       		break;
		case FLAG_7 :
	       		output = processFLAG_7();
	       		std::cout << std::endl << "The handle \"" << output <<"\" does not exist." << std::endl;
		       	break;
		case FLAG_9 :
	       		std::cout << std::endl << "Server has sent an ACK. Terminating client program." << std::endl;
	       		exit = 1;
	       		break;
		case FLAG_11 :
	       		output = processFLAG_11();
	       		std::cout << std::endl << "List of handles incoming. Size: " << output << std::endl;
	       		break;
		case FLAG_12 :
	       		output = processFLAG_12();
	       		std::cout << std::endl << "All Handles: " << output << std::endl;
	       		break;
		default : 
			std::cerr << "PACKET FLAG IS INVALID: " << stdHeadBuffer[2] << std::endl;
			std::exit(-1);
			break;
	}

	cleanUp(exit);
}

//state 3 method.
void readPacketFromSocket(int fds)
{
	int bytesRead = recv(fds, packetBuffer + packetFilled, packetToFill, MSG_DONTWAIT);
	if(bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
	{
		perror("readPacketFromSocket(): ");
		exit(-1);
	}
	packetFilled += bytesRead;
	packetToFill -= bytesRead;

	if(packetFilled == packetLen - 3)
		stateFlag = 4;
}

//state 1, 2, 3 method. Read from socket appropriate amount of bytes. State dependent.
void readSocket(int fds)
{
	if(stateFlag == 1)
	{
		stateFlag = 2;
		recv(fds, stdHeadBuffer, STANDARD_HEADER_LEN, 0);
		packetLen = ntohs(*(uint16_t*)stdHeadBuffer);
		packetFilled = 0;
		packetToFill = packetLen - 3; //for the 3 bytes that were just read from socket
		packetBuffer = (char*)calloc(packetToFill, sizeof(char));
		stateFlag = 3;

		if(stdHeadBuffer[2] == FLAG_2 || stdHeadBuffer[2] == FLAG_3 || stdHeadBuffer[2] == FLAG_9)
			stateFlag = 4;
	}
	else if(stateFlag == 3)
		readPacketFromSocket(fds);
	else
	{
		std::cerr << "fds: " << fds << " is not in a correct state for readSocket()!" << std::endl;
		exit(-1);
	}
}

//state 1, 2, 3 method. If fds is STDIN then read from it. Else read from fds with other method
void readFDS(int fds)
{
	if(fds == STDIN_FILENO)
		grabSTDIN();
	else
		readSocket(fds);
}

//Used after the select function returns in doSelect(). Checks which file descriptors are ready to read. state 1, 2, 3 method
void loopOverFDS(std::vector<int> allrfds, fd_set* rfds)
{
	for(uint32_t i = 0; i < allrfds.size(); i++)
	{
		if(FD_ISSET(allrfds[i], rfds))
			readFDS(allrfds[i]);
	}
}

//state 1 method
void doSelect(std::vector<int> allrfds)
{
	fd_set rfds;
	int returnVal;
	
	// add all fds to fd_set
	FD_ZERO(&rfds);
	for(uint32_t i = 0; i < allrfds.size(); i++)
		FD_SET(allrfds[i], &rfds);
	std::cout << "$: " << std::ends << std::flush;
	returnVal = select(FD_SETSIZE, &rfds, NULL, NULL, NULL);

	if(returnVal == -1)
	{
		perror("doSelect: ");
		exit(-1);
	}
	else
		loopOverFDS(allrfds, &rfds);

	//process state check
	if(stateFlag == 4)
		processPacket();
}

//method breaks packets in to 1000 byte messeges. Only messeges are 1000 bytes.
void breakPackets(int flagType, std::string recipient, std::string input)
{
	std::string msg, submsg; //holds just the messege string, no meta data
	if(flagType == FLAG_4 && recipient.size() == 0)
	{
		msg = input.substr(3);
		while(msg.size() > 1000)
		{
			submsg = msg.substr(0, 999);
			submsg += '\0';
			msg = msg.substr(1000);
			sendPacketFLAG_4(submsg);
		}
		sendPacketFLAG_4(msg);
	}
	else if(flagType == FLAG_5 && recipient.length() > 0)
	{
		msg = input.substr(3 + recipient.size());
		while(msg.size() > 1000)
		{
			submsg = msg.substr(0, 999);
			submsg += '\0';
			msg = msg.substr(1000);
			sendPacketFLAG_5(recipient, submsg);
		}
		sendPacketFLAG_5(recipient, msg);
	}
	else
	{
		std::cerr << "breakPackets() is not being passed the correct args" << std::endl;
	}
}

//gets the name of the intended recipient from a messege, input is entire string from stdin
std::string* getName(std::string input, std::string* name)
{
	*name = "";
	for(int i = 3; input[i] != ' '; i++)
		(*name).append(1, input[i]);
	return name;	
}

//To be called when the select returns with a positive on STDIN, state 5 method
void grabSTDIN()
{
	stateFlag = 5;
	//grab the command first
	std::string input, recipient;
	std::getline(std::cin, input);
	if(input[0] == '%')
	{
		if((input[1] == 'M' || input[1] == 'm') && input[2] == ' ')
		{
			getName(input, &recipient);
			breakPackets(FLAG_5, recipient, input);
			//sendPacketFLAG_5(re.lcipient, input);
		}
		else if((input[1] == 'B' || input[1] == 'b') && input[2] == ' ')
		{
			breakPackets(FLAG_4, "", input);
			//sendPacketFLAG_4(input);
		}
		else if(input[1] == 'L' || input[1] == 'l')
		{
			//TODO CALL FUNCTION THAT CREATES LIST PACKET AND SENDS IT
			sendPacketFLAG_10();
		}
		else if(input[1] == 'E' || input[1] == 'e')
			sendPacketFLAG_8();
		else
			std::cout << "Invalid command" << std::endl;
	}
	else
		std::cout << "Invalid command" << std::endl;
	stateFlag = 1;
}

void limitNameLen(std::string name)
{
	if(name.length() > 255)
	{
		std::cout << "Given name is longer than 255 characters. Terminating program.";
		exit(0);
	}
}

int main(int argc, char * argv[])
{
    	int socket_num;         //socket descriptor
	//std::vector<int> allrfds; //list of all file descriptors

    	/* check command line arguments  */
    	if(argc!= 4)
    	{
    		printf("usage: %s client-name host-name port-number \n", argv[0]);
		exit(1);
    	}

	//get my name
	myName = argv[1];
	limitNameLen(myName);

    	/* set up the socket for TCP transmission  */
    	socket_num = tcp_send_setup(argv[2], argv[3]);

	stateFlag = 1;
	allrfds.push_back(STDIN_FILENO);
	allrfds.push_back(socket_num);
	sendPacketFLAG_1();
    	//infinite looping to select. Can only exit by ctrl c or by giving exit command
    	while(1)
	{
		doSelect(allrfds);
	}
    	return 0;
    
}


int tcp_send_setup(char *host_name, char *port)
{
    	int socket_num;
    	struct sockaddr_in remote;       // socket address for remote side
    	struct hostent *hp;              // address of remote host

    	// create the socket
    	if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket call");
	    	exit(-1);
	}
    	// designate the addressing family
    	remote.sin_family= AF_INET;
    	// get the address of the remote host and store
    	if ((hp = gethostbyname(host_name)) == NULL)
	{
	  	printf("Error getting hostname: %s\n", host_name);
	  	exit(-1);
	}
    
    	memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);
    	// get the port used on the remote side and store
    	remote.sin_port= htons(atoi(port));
    	std::cout << "trying to connect to port: " << atoi(port) << " at host: " << hp->h_name << std::endl;
    	if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
    	{
		perror("connect call");
		exit(-1);
    	}
	std::cout << "success!!!" << std::endl;
    	return socket_num;
}
