#include <stdio.h>
#include "handle.h"

// Handle Utility
int handleEquals(char *handle1, int handleLen1, char *handle2, int handleLen2) {
	if (handleLen2 != handleLen1) {
		return 0;
	}
	while (handleLen2--) {
		if (*handle1++ != *handle2++) {
			return 0;
		}
	}

	return 1;
}

// Handle Utility
void printHandle(char *handle, int len) {
	printHandleWithoutNewline(handle, len);
	printf("\n");
}

void printHandleWithoutNewline(char *handle, int len) {
	while (len--){
		printf("%c", *handle++);
	}
}
