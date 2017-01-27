#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <queue>
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

static int serverSocket = -1;

Message getData(int socket) {
    char *buf = new char[MAX_MESSAGE_LEN];
    int length = readSocket(socket, buf);
    if (length == 0) {
        cerr << "\nServer Terminated" << endl;
        exit(-1);
    }
    Message ret = Message::unpackData(length, *(buf + sizeof(n_short)),
        (n_char *)buf + 3);
    delete[] buf;
    return ret;
}

void doHandshake(int socket, string &name) {
    Message message (name, "", "", 1);
    message.sendMessage(socket);
    Message retValue = getData(socket);
    if (retValue.getFlag() != 2) {
        cerr << "Handle already in use: " << name << endl;
        exit(-1);
    }
}

void setupClient(int socket, string &name) {
    clearSet();
    setFd(STDIN);
    setFd(socket);
    doHandshake(socket, name);
}

int getFlag(char ctrlChar) {
    int ret;
    switch (ctrlChar) {
        case 'B':
        case 'b':
            ret = 4;
            break;
        case 'M':
        case 'm':
            ret = 5;
            break;
        case 'E':
        case 'e':
            ret = 8;
            break;
        case 'L':
        case 'l':
            ret = 10;
            break;
        default:
            ret = 0;
            break;
    }
    return ret;
}



void createMessage(string data, string &name, int flag,
    queue<Message> &messages) {
    string reciever = "", sender = "", message = "";
    char *dataCopy = new char[data.length() + 1];
    char *tok = NULL;
    bool validData = true;
    data.copy(dataCopy, data.length());
    dataCopy[data.length()] = '\0';
    strtok(dataCopy, " ");
    switch (flag) {
        case 4:
            sender = name;
            tok = strtok(NULL, "");
            if (tok != NULL) {
                message = string(tok);
            }
            break;
        case 5:
            sender = name;
            tok = strtok(NULL, " ");
            if (tok) {
                reciever = string(tok);
            } else {
                validData = false;
            }
            tok = strtok(NULL, "");
            if (tok) {
                message = string(tok);
            }
            break;
        default:
            break;
    }
    if (!validData) {
        flag = 13;
    }
    do {
        if (message.length() > 1000) {
            messages.push(Message(sender, reciever, message.substr(0, 1001), flag));
            message = message.substr(1001);
        } else {
            messages.push(Message(sender, reciever, message, flag));
            message = "";
        }
    }while (!message.empty());
}

void closeConnection(int socket) {
    Message("", "", "", 8).sendMessage(socket);
    int flag;
    do {
        Message message = getData(socket);
        flag = message.getFlag();
    } while (flag != 9);
    close(socket);
}

void getHandles(Message message, int socket) {
    cout << "\nNumber of clients: " << (int)message.getAltData() << endl;
    for (int i = 0; i < (int)message.getAltData(); i++) {
        cout << "\t" << getData(socket).getReciever() << endl;
    }
}

int runClient(int socket, string &name) {
    int idx, flag = 0;
    cout << "$: ";
    cout.flush();
    bool *statuses = getFileStatus();
    bool *iter = statuses;
    for(idx = 0; idx <= getHighestDescriptor(); idx++) {
        if (*(iter++)) {
            if (idx == socket) {
                Message message = getData(socket);
                if (message.getFlag() == 7) {
                    cerr << "\nClient with handle " << message.getReciever() <<
                        "does not exists" << endl;
                } else if (message.getFlag() == 11) {
                    getHandles(message, socket);
                } else {
                    cout << "\n" << message.getSender() << ": " <<
                        message.getData() << endl;
                }
                break;
            } else {
                string command;
                getline(cin, command);
                if (cin.eof()) {
                    cerr << "Unexpected EOF" << endl;
                    closeConnection(serverSocket);
                    exit(-1);
                }
                flag = getFlag(command[1]);
                if (command[0] != '%' || !flag) {
                    cerr << "Invalid Command" << (int)flag << endl;
                    continue;
                }
                queue<Message> messageQueue = queue<Message>();
                createMessage(command, name, flag, messageQueue);
                while (!messageQueue.empty()) {
                    Message data = messageQueue.front();
                    messageQueue.pop();
                    if (data.getFlag() != 13) {
                        data.sendMessage(socket);
                    } else {
                        cerr << "Invalid Command" << (int)data.getFlag() << endl;
                        continue;
                    }
                }
            }
        }
    }
    return flag;
}

void SigintHandler(int signal) {
    closeConnection(serverSocket);
    exit(3);
}

int main(int argc, char *argv[]) {
    string uname (argv[1]);
    int flag;
    if (argc != 4) {
        cerr << "usage: " << argv[0] <<
            " USERNAME HOST_NAME PORT_NUMBER" << endl;
        return 1;
    }
    serverSocket = tcpClientSetup(argv[2], argv[3]);
    setupClient(serverSocket, uname);
    signal(SIGINT, SigintHandler);
    do {
        flag = runClient(serverSocket, uname);
    } while (flag != 8);
    closeConnection(serverSocket);
    return 0;
}
