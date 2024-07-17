#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

typedef struct sockaddr_in SockAddressIn;

class Client {
	public:
		Client(void);
		Client(Client const &);
		void sender(char *);
		void receiver(void);
		Client & operator = (Client const &);
		~Client(void);
	private:
		int sock;
};

#endif