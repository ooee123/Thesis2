#ifndef message_hpp
#define message_hpp
#include <string>

extern "C"{
    #include "testing.h"
}
#include "types.hpp"

class Message {
private:
    std::string sender;
    std::string reciever;
    std::string data;
    n_long altData;
    n_short length;
    n_char flag;
    unsigned long getPduLength(std::string, std::string, std::string);
    void createMessage(n_char[]);

public:
    Message(std::string, std::string, std::string, n_char);
    Message(n_short, n_char);
    Message(n_long, n_char);
    static Message unpackData(n_short, n_char, n_char *);
    n_short getLength(void);
    n_char getFlag(void);
    n_long getAltData(void);
    std::string getSender(void);
    std::string getReciever(void);
    std::string getData(void);
    void sendMessage(int);
};
#endif
