#include "Client.h"

int main(int argc, char **argv) {
	Client client;
	if (argc != 2)
		return -1;
	client.sender(argv[1]);
	client.receiver();
	return 0;
}