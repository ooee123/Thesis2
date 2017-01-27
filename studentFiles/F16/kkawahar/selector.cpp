#include <iostream>
#include <list>

extern "C"{
    #include "testing.h"
}
#include "selector.hpp"

static std::list<int> descList = std::list<int>();

void clearSet() {
    descList.clear();
}

void setFd(int fd) {
    descList.emplace_back(fd);
    descList.sort();
}

void removeFd(int fd) {
    descList.remove(fd);
}

long getNumDescriptors() {
    return descList.size();
}

int getHighestDescriptor() {
    return descList.back();
}

template <typename T> void fillList(T *list, long size, T with) {
    int idx;
    for (idx = 0; idx < size; idx++) {
        *list++ = with;
    }
}

bool *getFileStatus() {
    fd_set desc;
    FD_ZERO(&desc);
    bool *statuses = new bool[descList.back()];
    fillList<bool>(statuses, descList.back() + 1, false);
    
    for (auto fdPtr = descList.begin(); fdPtr != descList.end(); fdPtr++) {
        FD_SET(*fdPtr, &desc);
    }
    select(descList.back() + 1, &desc, NULL, NULL, NULL);
    for (auto fd = descList.begin(); fd != descList.end(); fd++) {
        if (FD_ISSET(*fd, &desc)) {
            *(statuses + *fd) = true;
        }
    }
    return statuses;
}
