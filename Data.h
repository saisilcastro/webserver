#include "Server.h"
#define MAX_CLIENT 10

static inline int server_socket_search(int type, char * port) {
  int s;
  int sock;
  int in_use = 1;
  struct addrinfo hints;
  struct addrinfo *result;
  struct addrinfo *cur;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = type; 
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  if ((s = getaddrinfo(NULL, port, &hints, &result)) != 0) {
    cerr << "getaddrinfo: " << gai_strerror(s) << "\n";
    return -1;
  }
  for (cur = result; cur; cur = cur->ai_next) {
    if((sock = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol)) == -1)
      continue;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &in_use, sizeof(int)) == -1) {
      perror("address in use yet");
      close(sock);
      freeaddrinfo(result);
      return -1;
    }
    if (!bind(sock, cur->ai_addr, cur->ai_addrlen))
      break;
    close(sock);
  }

  freeaddrinfo(result);
  if (cur == NULL)
  {
    cerr << "Could not bind\n";
    return -1;
  }
  if (listen(sock, MAX_CLIENT) < 0) {
		perror("listen failed");
    close(sock);
    return -1;
	}
  if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
    perror("fcntl failed");
    close(sock);
    return -1;
  }
  return sock;
}

Server::Server(void) : sock("8080") {
	sock = server_socket_search(SOCK_STREAM, (char *)port.c_str());
  if (sock == -1)
    exit(-1);
}

Server::Server(char *file) : sock("8080") {
  ifstream in(file);
  if (!in.is_open() || in.bad() || in.fail()) {
    return ;
  }
  cout << "ok" << endl;
  in.close();
}

Server::Server(Server const & pointer) {
	*this = pointer;
}

void Server::addressType(struct sockaddr *address, string message) {
  switch (address->sa_family) {
    case AF_INET:
      inet_ntop(address->sa_famiy, &((struct sockaddr_in *)address)->sin_addr, message.c_str(), sizeof(message));
    break;
    case AF_INET6:
      inet_ntop(address->sa_family, &((struct sockaddr_in6 *)address)->sin_addr, message.c_str(), sizeof(message));
    break;
    default:
      message = "Not handleable";
    break;
  }
}

string stream_maker(string file) {
  ifstream in(file.c_str());
  stringstream stream;

  if (!in.is_open() || in.bad() || in.fail())
  {
    cout << "error reading: " << file << endl;
    return NULL;
  }
  while (!in.eof()) {
    string line;
    getline(in, line);
    stream << line;
  }
  in.close();
  return stream.str();
}

string make_content(const char *header, const char *connection, const char *type, const char *content) {
  stringstream stream;

  stream << header << "\n"
         << "Connection: " << connection << "\n"
         << "Content-Type: " << type << "\n"
         << "Content-Lenght: " << strlen((const char *)content) << "\n\n"
         << content;
  return stream.str();
}

void Server::run(void) {
	while (1) {
		int client = accept(sock, (struct sockaddr*)NULL, NULL);
		if (client != -1) {

      string file = "index.html";
    	string rcv = receiver(client, 1024);
      string page = stream_maker(file);
      string content = make_content("HTTP/1.0 200 OK", "close", "text/html", page.c_str());
			sender(client, content);
			close(client);
		}
	}
}

void Server::sender(int client_socket, string buffer) {
	send(client_socket, buffer.c_str(), buffer.size(), 0);
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
