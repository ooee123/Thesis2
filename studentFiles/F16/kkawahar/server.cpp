#include <cstddef>
#include <iostream>
#include <list>
#include <map>
using namespace std;

#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>

extern "C"{
    #include "testing.h"
}
#include "constants.hpp"
#include "message.hpp"
#include "networks.hpp"
#include "selector.hpp"
#include "types.hpp"

static std::list<int> fds = std::list<int>();
static map<string, int> nameMap = map<string, int>();

void handleNewConnection(int socket) {
    int client_socket = tcpAccept(socket);
    setFd(client_socket);
    fds.push_back(client_socket);
}

void replyToMessage(int socket, Message message) {
    int replyFlag = 0;
    string reciever = "";
    switch (message.getFlag()) {
        case 1:
            if (nameMap.find(message.getSender()) == nameMap.end()) {
                nameMap.insert(make_pair(message.getSender(), socket));
                replyFlag = 2;
            } else {
                replyFlag = 3;
            }
            break;
        case 5:
            reciever = message.getReciever();
            replyFlag = 7;
        case 8:
            replyFlag = 9;
            break;
        default:
            break;
    }
    Message reply = Message("", reciever, "", replyFlag);
    reply.sendMessage(socket);
    if (replyFlag == 3 || replyFlag == 9) {
        removeFd(socket);
        fds.remove(socket);
        close(socket);
    }
}

void forwardMessage(int socket, Message message) {
    switch (message.getFlag()) {
        case 4:
            for (auto entry = nameMap.begin(); entry != nameMap.end();
                 entry++) {
                if (entry->first != message.getSender()) {
                    message.sendMessage(entry->second);
                }
            }
            break;
        case 5: {
            auto entry = nameMap.find(message.getReciever());
            if (entry != nameMap.end()) {
                message.sendMessage(entry->second);
            } else {
                replyToMessage(socket, message);
            }
        }
            break;
        default:
            break;
    }
}

string getUsernameFromSocket(int socket) {
    for (auto entry = nameMap.begin(); entry != nameMap.end(); entry++) {
        if (entry->second == socket) {
            return entry->first;
        }
    }
    return "";
}

void removeUser(string name) {
    nameMap.erase(nameMap.find(name));
}

void sendNameList(int socket) {
    Message countMessage = Message((n_long)nameMap.size(), 11);
    countMessage.sendMessage(socket);
    for (auto entry = nameMap.begin(); entry != nameMap.end(); entry++) {
        Message message ("", entry->first, "", 12);
        message.sendMessage(socket);
    }
}

void handleMessage(int socket, Message message) {
    switch(message.getFlag()) {
        case 1:
            replyToMessage(socket, message);
            break;
        case 10:
            sendNameList(socket);
            break;
        case 4:
        case 5:
            forwardMessage(socket, message);
            break;
        case 8:
            replyToMessage(socket, message);
            removeUser(getUsernameFromSocket(socket));
            break;
        default:
            break;
    }
}

void handleCurrentConnection(int socket) {
    char buf[MAX_MESSAGE_LEN];
    n_short length = readSocket(socket, buf);
    if (length == 0) {
        removeFd(socket);
        fds.remove(socket);
    }
    Message message = Message::unpackData(length, *(buf + sizeof(n_short)),
        (n_char *)buf + HEADER_SIZE);
    handleMessage(socket, message);
}

void handleConnections(int socket) {
    int idx;
    bool *statuses = getFileStatus();
    bool *iter = statuses;
    for(idx = 0; idx <= getHighestDescriptor(); idx++) {
        if (*(iter++)) {
            if (idx == socket) {
                handleNewConnection(socket);
            } else {
                handleCurrentConnection(idx);
            }
        }
    }
    delete statuses;
}

void SigintHandler(int signal) {
    for (auto fd = fds.begin(); fd != fds.end(); fd++) {
        close(*fd);
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    int socket;
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " PORT_NUMBER" << std::endl;
        return 1;
    }
    socket = tcpServerSetup(atoi(argv[1]));
    clearSet();
    setFd(socket);
    signal(SIGINT, SigintHandler);
    while(true) {
        handleConnections(socket);
    }
    return 0;
}
