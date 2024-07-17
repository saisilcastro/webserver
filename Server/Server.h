#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>

typedef struct sockaddr_in SockAddrIn;

class Server {
	public:
		Server(void);
		Server(Server const &);
		void run(void);
		void sender(int, char *);
		std::string receiver(int, int);
		Server & operator = (Server const &);
		~Server(void);
	private:
		SockAddrIn address;
		int sock;
};

#endif