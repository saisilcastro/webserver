#include "Client.h"

SockAddressIn serverAddress(const char *host, int port) {
	SockAddressIn address;

	std::memset(&address, '\0', sizeof(address));

	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if(inet_pton(AF_INET, host, &address.sin_addr) <= 0)
		std::cout << "\n inet_pton error occured\n";
	return address;
}

Client::Client(void) {
	SockAddressIn server;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server = serverAddress("127.0.0.1", 8080);
	if( connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
		std::cout << "\n Error : Connect Failed \n";
}

Client::Client(Client const & pointer) {
	*this = pointer;
}

void Client::sender(char * message) {
	send(sock, (const char *)message, strlen(message), 0);
}

void Client::receiver(void) {
	char buffer[1024];
	int len = recv(sock, buffer, 1024, 0);
	buffer[len] = '\0';
	std::cout << buffer << std::endl;
}

Client & Client::operator = (Client const & pointer) {
	if (this == &pointer) {
		sock = pointer.sock;
	}
	return *this;
}

Client::~Client(void) {
}