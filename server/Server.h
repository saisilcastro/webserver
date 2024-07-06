#ifndef SERVER_H
#define SERVER_H

#include <sys/socket>
#include "../client/Client.h"

class Server {
	public:
		Server(void);
		~Server();
	private:
		int socket;
		Client *client;
};

#endif