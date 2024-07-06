#include "Server.h"

Server::Server(void) {
	client = NULL;
	socket = socket("127.0.0.1", SOCK_STREAM, SOCK_NONBLOCK);
}

Server::~Server(void) {
	close(net->sock);
}