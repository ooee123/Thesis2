/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
 *****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
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

struct clientInfo
{
	std::string name;
	int socket_fds;
	int socket_state; //holds the current state of the socket and how much has been read from it.
	char stdHeadBuffer[3];
	uint16_t packetLen;     //total packet length. Found in first 2 bytes of standard header
	uint16_t packetFilled;  //how much of the sub-packet has been read(does not include standard header)
	uint16_t packetToFill;  //how much of the sub-packet needs to be read(standard header not included)
	char * packetBuffer;
};

struct packetInfo
{
	uint8_t* packetBuffer; //buffer pointer for a packet to send through a socket
	uint16_t packetLen; //buffer length in host order
};

int tcp_listen(int);

std::vector<clientInfo> allHandles; //all accepted clients who have officially joined the server program
				//Clients with names of "NOTJOINED", have connected to the server, but not sent their request packet
				//all fds on which we need to be selecting
uint32_t handleListSize = 0;    //number of handles in handle list.
int server_socket = 0; //socket on which all perspective clients will attempt to connect

//state 4 method, Resets global variables to read and process new packet
void cleanUp(int clientIndex, int destroyClient)
{
	free(allHandles[clientIndex].packetBuffer);
	allHandles[clientIndex].socket_state = 1;
	allHandles[clientIndex].stdHeadBuffer[0] = 0;
	allHandles[clientIndex].stdHeadBuffer[1] = 0;
	allHandles[clientIndex].stdHeadBuffer[2] = 0;
	allHandles[clientIndex].packetLen = 0;
	allHandles[clientIndex].packetFilled = 0;
	allHandles[clientIndex].packetToFill = 0;
	if(destroyClient)
	{
		close(allHandles[clientIndex].socket_fds);
		allHandles.erase(allHandles.begin() + clientIndex);
	}
}

void printClientInfo(int clientIndex)
{
	std::cout << "client index: " << clientIndex << std::endl;
	std::cout << "---name: " << allHandles[clientIndex].name << std::endl;
	std::cout << "---socket_fds: " << allHandles[clientIndex].socket_fds << std::endl;
	std::cout << "---stdHeadBuff: " << "Not Yet" << std::endl;
	std::cout << "---packetLen: " << allHandles[clientIndex].packetLen << std::endl;
	std::cout << "---packetFilled: " << allHandles[clientIndex].packetFilled << std::endl;
	std::cout << "---packetToFill: " << allHandles[clientIndex].packetToFill << std::endl;
	std::cout << "---packetBuffer: " << "NOT YET" << std::endl;
}

//remember to free the pointer
packetInfo createPacketFLAG_2()
{
	packetInfo rtn;
	uint16_t length;
	rtn.packetBuffer = (uint8_t*)calloc(sizeof(void*), STANDARD_HEADER_LEN);
	rtn.packetLen = STANDARD_HEADER_LEN;
	length = htons(rtn.packetLen);
	memcpy(rtn.packetBuffer, &length, 2);
	((char*)rtn.packetBuffer)[2] = FLAG_2;
	return rtn;
}

//the returned pointer will need to be free called otherwise mem leak.
packetInfo createPacketFLAG_3()
{
	packetInfo rtn;
	uint16_t length;
	rtn.packetBuffer = (uint8_t*)calloc(sizeof(void*), STANDARD_HEADER_LEN);
	rtn.packetLen = STANDARD_HEADER_LEN;
	length = htons(rtn.packetLen);
	memcpy(rtn.packetBuffer, &length, 2);
	((char*)rtn.packetBuffer)[2] = FLAG_3;
	return rtn;
}

packetInfo createPacketFLAG_7(std::string missingName)
{
	packetInfo rtn;
	uint16_t length, tempSize;
	rtn.packetLen = STANDARD_HEADER_LEN + 1 + missingName.size();
	rtn.packetBuffer = (uint8_t*)calloc(sizeof(void*), rtn.packetLen);
	length = htons(rtn.packetLen);
	memcpy(rtn.packetBuffer, &length, 2); //inserting packetLen
	((char*)rtn.packetBuffer)[2] = FLAG_7; //inserting flag
	tempSize =  missingName.size();
	memcpy(rtn.packetBuffer + STANDARD_HEADER_LEN, &tempSize, 1); //getting missingname length in
	memcpy(rtn.packetBuffer + STANDARD_HEADER_LEN + 1, missingName.c_str(), tempSize); //inserting name
	return rtn;
}

packetInfo createPacketFLAG_9()
{
	packetInfo rtn;
	uint16_t length;
	rtn.packetBuffer = (uint8_t*)calloc(sizeof(void*), STANDARD_HEADER_LEN);
	rtn.packetLen = STANDARD_HEADER_LEN;
	length = htons(rtn.packetLen);
	memcpy(rtn.packetBuffer, &length, 2);
	((char*)rtn.packetBuffer)[2] = FLAG_9;
	return rtn;
}

packetInfo createPacketFLAG_11()
{
	packetInfo rtn;
	rtn.packetLen = STANDARD_HEADER_LEN + 4;
	uint16_t length = htons(rtn.packetLen);
	uint32_t num_of_handles = htonl(allHandles.size() - 1);
	rtn.packetBuffer = (uint8_t*)calloc(sizeof(void*), rtn.packetLen);
	memcpy(rtn.packetBuffer, &length, 2);
	((uint8_t*)rtn.packetBuffer)[2] = FLAG_11;
	memcpy(rtn.packetBuffer + STANDARD_HEADER_LEN, &num_of_handles, 4);
	return rtn;
}

packetInfo createPacketFLAG_12()
{
	packetInfo rtn;
	uint16_t length, tracker = 0;
	uint8_t nameLen;
	uint32_t num_of_handles = allHandles.size() - 1;
	//determine total packet size
	rtn.packetLen = STANDARD_HEADER_LEN + num_of_handles; 
	for(uint32_t i = 1; i < allHandles.size(); i++)
		rtn.packetLen += allHandles[i].name.size();
	//allocate correct packet size memory
	rtn.packetBuffer = (uint8_t*)calloc(sizeof(void*), rtn.packetLen);
	length = htons(rtn.packetLen);
	memcpy(rtn.packetBuffer, &length, 2);
	((char*)rtn.packetBuffer)[2] = FLAG_12;
	//insert name length bytes and names
	for(uint32_t i = 1; i < allHandles.size(); i++)
	{
		nameLen = allHandles[i].name.size();
		memcpy(rtn.packetBuffer + STANDARD_HEADER_LEN + tracker, &nameLen, 1);
		tracker++;
		memcpy(rtn.packetBuffer + STANDARD_HEADER_LEN + tracker, allHandles[i].name.c_str(), allHandles[i].name.size());
		tracker += allHandles[i].name.size();
	}
	return rtn;
}

void processFLAG_10(int clientIndex)
{
	packetInfo packetToSend = createPacketFLAG_11();
	if(send(allHandles[clientIndex].socket_fds, packetToSend.packetBuffer, packetToSend.packetLen, 0) < 0)
	{
		perror("processFLAG_10#1");
		exit(-1);
	}
	free(packetToSend.packetBuffer);
	packetToSend = createPacketFLAG_12();
	if(send(allHandles[clientIndex].socket_fds, packetToSend.packetBuffer, packetToSend.packetLen, 0) < 0)
	{
		perror("processFLAG_10#2");
		exit(-1);
	}
	free(packetToSend.packetBuffer);
}

int processFLAG_8(int clientIndex)
{
	packetInfo packetToSend = createPacketFLAG_9();
	int test = send(allHandles[clientIndex].socket_fds, packetToSend.packetBuffer, packetToSend.packetLen, 0);
	if(test < 0)
	{
		perror("processFLAG_8: ");
		exit(-1);
	}
	free(packetToSend.packetBuffer);
	return 1;
}

void processFLAG_5(int clientIndex)
{
	//reassmble packet
	char* tempPacket = (char*)calloc(sizeof(void*), allHandles[clientIndex].packetLen);
	memcpy(tempPacket, allHandles[clientIndex].stdHeadBuffer, STANDARD_HEADER_LEN);
	memcpy(tempPacket + STANDARD_HEADER_LEN, allHandles[clientIndex].packetBuffer, allHandles[clientIndex].packetFilled);
	//constructing the name
	std::string tempname = "";
	uint8_t namesize = *(uint8_t*)allHandles[clientIndex].packetBuffer;
	for(uint8_t i = 0; i < namesize; i++)
		tempname += allHandles[clientIndex].packetBuffer[1 + i];
	//searching for the name
	for(uint32_t i = 0; i < allHandles.size(); i++)
	{
		if(tempname.compare(allHandles[i].name) == 0)
		{
			if(send(allHandles[i].socket_fds, tempPacket, allHandles[clientIndex].packetLen, 0) < 0) //name found, send it to socket fds
			{
				perror("processFLAG_5()#1: ");
				exit(-1);
			}
			free(tempPacket);
			return; //end function task completed
		}
	}
	//search failed. Send packet to sender
	packetInfo packetToSend = createPacketFLAG_7(tempname);
	if(send(allHandles[clientIndex].socket_fds, packetToSend.packetBuffer, packetToSend.packetLen, 0) < 0)
	{
		perror("processFLAG_5()#2");
		exit(-1);
	}
	free(packetToSend.packetBuffer);
}

//process the broadcast packet. Send to everyone
void processFLAG_4(int clientIndex)
{
	//reassemble the packet
	char* tempPacket = (char*)calloc(sizeof(void*), allHandles[clientIndex].packetLen);
	memcpy(tempPacket, allHandles[clientIndex].stdHeadBuffer, STANDARD_HEADER_LEN);
	memcpy(tempPacket + STANDARD_HEADER_LEN, allHandles[clientIndex].packetBuffer, allHandles[clientIndex].packetFilled);
	for(int i = 1; i < allHandles.size(); i++)
	{
		if(i != clientIndex)
		{
			if(send(allHandles[i].socket_fds, tempPacket, allHandles[clientIndex].packetLen, 0) < 0)
			{
				perror("processFLAG_4(): ");
				exit(-1);
			}
		}
	}
	free(tempPacket);
}

//returns blank string if name is not valid(name already exists), returns with string if valid
std::string checkName(int clientIndex)
{
	std::string tempname = "";
	uint8_t namesize = *(uint8_t*)allHandles[clientIndex].packetBuffer;
	for(uint8_t i = 0; i < namesize; i++)
		tempname += allHandles[clientIndex].packetBuffer[1 + i];
	for(uint32_t i = 0; i < allHandles.size(); i++)
	{
		if(tempname.compare(allHandles[i].name) == 0)
			return "";
	}
	return tempname;
}

void processFLAG_1(int clientIndex)
{
	std::string testString = checkName(clientIndex);
	packetInfo packetToSend;
	if(testString.compare("") == 0)
	{
		//create and send packet with FLAG_3
		packetToSend = createPacketFLAG_3();
		if(send(allHandles[clientIndex].socket_fds, packetToSend.packetBuffer, packetToSend.packetLen, 0) < 0)
		{
			perror("processFLAG_1()#1: ");
			exit(-1);
		}
		cleanUp(clientIndex, 1);
	}
	else
	{
		//SEND PACket with FLAG_2
		packetToSend = createPacketFLAG_2();
		if(send(allHandles[clientIndex].socket_fds, packetToSend.packetBuffer, packetToSend.packetLen, 0) < 0)
		{
			perror("processFLAG_1()#2: ");
			exit(-1);
		}
		allHandles[clientIndex].name = testString;
	}
	free(packetToSend.packetBuffer); //VERY IMPORTANT STEP!!!
}

//state 4 method. Processes current packet in packetBuffer.
void processPacket(int clientIndex)
{
	std::string output = "";
	int destroyClient = 0; //flag to pass to cleanUp() function
	switch(allHandles[clientIndex].stdHeadBuffer[2]) 
	{
		case FLAG_1 : 
			processFLAG_1(clientIndex);
			break;
		case FLAG_4 :
	       		processFLAG_4(clientIndex);
	       		break;
		case FLAG_5 :
			processFLAG_5(clientIndex);
	       		break;
		case FLAG_8 :
			destroyClient = processFLAG_8(clientIndex);
	       		break;
		case FLAG_10 :
	       		processFLAG_10(clientIndex);
	       		break;
		default : 
			std::cerr << "PACKET FLAG IS INVALID: " << (uint8_t)allHandles[clientIndex].stdHeadBuffer[2] << " blank " << std::endl;
			break;
	}

	cleanUp(clientIndex, destroyClient);
}

//state 3 method. //returns 1 when packet reading is complete. 0 when not complete
int readPacketFromSocket(int clientIndex)
{
	int bytesRead = recv(allHandles[clientIndex].socket_fds, allHandles[clientIndex].packetBuffer + 
			allHandles[clientIndex].packetFilled, allHandles[clientIndex].packetToFill, MSG_DONTWAIT);
	if(bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
	{
		perror("readPacketFromSocket: ");
		exit(-1);
	}
	allHandles[clientIndex].packetFilled += bytesRead;
	allHandles[clientIndex].packetToFill -= bytesRead;

	if(allHandles[clientIndex].packetFilled == allHandles[clientIndex].packetLen - 3)
		return 1;
	else
		return 0;
}

//state 1, 2, 3 method. Read from socket appropriate amount of bytes. State dependent.
void readSocket(int clientIndex)
{
	int test = 0;
	if(allHandles[clientIndex].socket_state == 1)
	{
		allHandles[clientIndex].socket_state = 2;
		test = recv(allHandles[clientIndex].socket_fds, allHandles[clientIndex].stdHeadBuffer, STANDARD_HEADER_LEN, 0); //calling it with a block 3 bytes should be quick.
		allHandles[clientIndex].packetLen = ntohs(*(uint16_t*)allHandles[clientIndex].stdHeadBuffer);
		allHandles[clientIndex].packetFilled = 0;
		allHandles[clientIndex].packetToFill = allHandles[clientIndex].packetLen - 3; //for the 3 bytes that were just read from socket
		allHandles[clientIndex].packetBuffer = (char*)calloc(allHandles[clientIndex].packetToFill, sizeof(char));
		allHandles[clientIndex].socket_state = 3;

		if(allHandles[clientIndex].stdHeadBuffer[2] == FLAG_8 || allHandles[clientIndex].stdHeadBuffer[2] == FLAG_10)
			allHandles[clientIndex].socket_state = 4;
		if(test == 0)
			allHandles[clientIndex].socket_state = 0;
	}
	else if(allHandles[clientIndex].socket_state == 3)
	{
		if(readPacketFromSocket(clientIndex))
			allHandles[clientIndex].socket_state = 4;
	}
}

void acceptConnection()
{
	int client_socket = tcp_listen(server_socket);
	clientInfo new_client = {"NOTJOINED", client_socket, 1, {0, 0, 0}, 0, 0, 0};
	allHandles.push_back(new_client);
}

//determines which fds need reading. Calls functions to read them.
void loopOverFDS(fd_set* rfds)
{
	for(uint32_t i = 0; i < allHandles.size(); i++)
	{
		if(FD_ISSET(allHandles[i].socket_fds, rfds))
		{
			if(allHandles[i].socket_fds == server_socket)
				acceptConnection();
			else
			{
				readSocket(i);
				if(allHandles[i].socket_state == 4)
					processPacket(i);
				else if(allHandles[i].socket_state == 0)
					cleanUp(i, 1);
			}
		}
	}
}

void doSelect()
{
	fd_set rfds;
	int returnVal;
	// add all fds to fd_set
	FD_ZERO(&rfds);
	for(uint32_t i = 0; i < allHandles.size(); i++)
		FD_SET(allHandles[i].socket_fds, &rfds);

	returnVal = select(FD_SETSIZE, &rfds, NULL, NULL, NULL);

	if(returnVal == -1)
	{
		perror("doSelect(): ");
		exit(-1);
	}
	else
		loopOverFDS(&rfds);
}

int main(int argc, char *argv[])
{
    	//create the server socket
    	server_socket = tcp_recv_setup();

    	//setup server_socket as listener. Also server_socket is the first socket in the vector
	tcp_listen_setup(server_socket, 5);

	//select loop is inifinite. Can only kill server by control c signal
	while(1)
	{
		doSelect();
	}
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen. Prepares the server with a socket that can accept connections */
int tcp_recv_setup()
{
    	int server_socket= 0;
    	struct sockaddr_in local;      /* socket address for local side  */
    	socklen_t len= sizeof(local);  /* length of local address        */
    	/* create the socket  */
    	server_socket = socket(AF_INET, SOCK_STREAM, 0);
    	if(server_socket < 0)
    	{
    		perror("socket call");
      		exit(1);
    	}
    	local.sin_family = AF_INET;         //internet family
    	local.sin_addr.s_addr = INADDR_ANY; //wild card machine address
    	local.sin_port = htons(0);                 //let system choose the port
    	/* bind the name (address) to a port */
    	if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
      	{
		perror("bind call");
		exit(-1);
      	}
    	//get the port name and print it out
    	if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0)
      	{
		perror("getsockname call");
		exit(-1);
      	}
    	printf("socket has port %d \n", ntohs(local.sin_port));	        
	//creates entry for server_socket in the allHandles vector.
	clientInfo server = {"SERVER_SOCKET", server_socket, 1, {0, 0, 0}, 0, 0, 0};

	allHandles.push_back(server);

    	return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

void tcp_listen_setup(int server_socket, int back_log)
{
  	if (listen(server_socket, back_log) < 0)
    	{
      		perror("listen call");
      		exit(-1);
    	}
} 

//Accepts one pending connection, REturns the created socket fds
int tcp_listen(int server_socket)
{
	int client_socket = 0;
  	if ((client_socket = accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
    	{
      		perror("accept call");
      		exit(-1);
    	}
  	return(client_socket);
}
