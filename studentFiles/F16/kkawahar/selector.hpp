#ifndef SELECTOR_H
#define SELECTOR_H

#include <sys/select.h>

void clearSet(void);
void setFd(int);
void removeFd(int);
long getNumDescriptors(void);
int getHighestDescriptor(void);
bool *getFileStatus(void);
#endif
