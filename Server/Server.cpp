#include "Server.h"
#define MAX_CLIENT 1

Server::Server(void) : sock(-1) {
	sock = socket(AF_INET, SOCK_STREAM, 0);
	std::memset(&address, '\0', sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(8080);
	if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("bind failed");
        exit(EXIT_FAILURE);
	}
	if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl failed");
        exit(EXIT_FAILURE);
    }
	if (listen(sock, MAX_CLIENT) < 0) {
		perror("listen failed");
        exit(EXIT_FAILURE);
	}
}

Server::Server(Server const & pointer) {
	*this = pointer;
}

void Server::run(void) {
	while (1) {
		int client = accept(sock, (struct sockaddr*)NULL, NULL);
		if (client != -1) {
			char buffer[] = "hello world";
			std::string rcv = receiver(client, 1024);
			std::stringstream ss;
			ss << rcv << " " << buffer;
			sender(client, (char *)ss.str().c_str());
			close(client);
		}
	}
}

void Server::sender(int client_socket, char *buffer) {
	send(client_socket, buffer, strlen(buffer), 0);
}

std::string Server::receiver(int client_socket,int maxsize) {
	char buffer[maxsize];
	int len = recv(client_socket, buffer, maxsize, 0);
	buffer[len] = 0;
	std::stringstream ss;
	ss << buffer;
	return ss.str();
}

Server & Server::operator = (Server const & pointer) {
	if (this != &pointer) {
		sock = pointer.sock;
		address.sin_family = pointer.address.sin_family;
		address.sin_port = pointer.address.sin_port;
		address.sin_addr = pointer.address.sin_addr;
		*address.sin_zero = *pointer.address.sin_zero;
	}
	return *this;
}

Server::~Server(void) {
	close(sock);
}