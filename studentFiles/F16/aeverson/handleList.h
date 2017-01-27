typedef struct Handle {
	char *name;
	int socket;
	int length;
	struct Handle *next;
} Handle;

int findHandle(char *name);

int findSocket(int socket);

void addHandle(char *name, int hSize, int socket);

void removeHandle(char *name);

void removeSocket(int socket);

int getSocket(char *name);

int32_t getNumHandles();

Handle *getClientData(int ndx);

