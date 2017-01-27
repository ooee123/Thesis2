#include <cstring>
#include <iostream>
using namespace std;

#include <arpa/inet.h>
#include <sys/socket.h>

#include "constants.hpp"
#include "message.hpp"

unsigned long Message::getPduLength(string sender, string reciever,
    string data) {
    unsigned long senderLen = sender.length() ? sender.length() + 1: 0;
    unsigned long recieverLen = reciever.length() ? reciever.length() + 1: 0;
    unsigned long dataLen = data.length() ? data.length() + 1: 0;
    return senderLen + recieverLen + dataLen;
}

Message::Message(string sender, string reciever, string data, n_char flag) {
    this->sender = sender;
    this->reciever = reciever;
    this->data = data;
    this->flag = flag;
    this->length = HEADER_SIZE + (int)getPduLength(sender, reciever, data);
}

Message::Message(n_short length, n_char flag) {
    this->length = length;
    this->flag = flag;
    this->sender = "";
    this->reciever = "";
    this->data = "";
}

Message::Message(n_long data, n_char flag) {
    this->length = 7;
    this->altData = data;
    this->flag = flag;
    this->sender = "";
    this->reciever = "";
    this->data = "";
}

n_short Message::getLength() {
    return length;
}

n_char Message::getFlag() {
    return flag;
}

string Message::getSender() {
    return sender;
}

string Message::getReciever() {
    return reciever;
}

string Message::getData() {
    return data;
}

n_long Message::getAltData() {
    return altData;
}

void Message::createMessage(n_char message[]) {
    int offset = 0;
    *(n_short *)(message + offset) = htons(length);
    offset += sizeof(n_short);
    *(message + offset) = flag;
    offset += sizeof(n_char);
    if (flag == 11) {
        *(n_long *)(message + offset) = htonl(altData);
        return;
    }
    switch(flag) {
        case 5:
        case 7:
        case 12:
            *(message + offset) = (n_char)reciever.length();
            offset += sizeof(n_char);
            memcpy(message + offset, reciever.data(), reciever.length());
            offset += reciever.length();
            break;
        default:
            break;
    }
    switch(flag) {
        case 1:
        case 4:
        case 5:
            *(message + offset) = (n_char)sender.length();
            offset += sizeof(n_char);
            memcpy(message + offset, sender.data(), sender.length());
            offset += sender.length();
            break;
        default:
            break;
    }
    if (!data.empty()) {
        memcpy(message + offset, data.data(), data.length() + 1);
    }
}

Message Message::unpackData(n_short length, n_char flag, n_char *data) {
    Message message (length, flag);
    n_char sendLen, recieveLen;
    int offset = 0;
    if (message.getFlag() == 11) {
        message.altData = ntohl(*(n_long *)(data + offset));
        return message;
    }
    switch (flag) {
        case 5:
        case 7:
        case 12:
            recieveLen = *(data + offset);
            offset += sizeof(n_char);
            message.reciever = string((char *)(data + offset), recieveLen);
            offset += recieveLen;
            break;
        default:
            message.reciever = "";
            break;
    }
    switch (flag) {
        case 1:
        case 4:
        case 5:
            sendLen = *(data + offset);
            offset += sizeof(n_char);
            message.sender = string((char *)(data + offset), sendLen);
            offset += sendLen;
            break;
        default:
            message.sender = "";
            break;
    }
    message.data = string((char *)(data + offset));
    return message;
}

void Message::sendMessage(int socket) {
    n_char packet[MAX_MESSAGE_LEN];
    memset(packet, 0, MAX_MESSAGE_LEN);
    createMessage(packet);
    if (send(socket, packet, (int)length, 0) < 0) {
        perror("error sending data");
    }
}
