#include "Server.h"
#define MAX_CLIENT 10

int Server::serverSocket(int type) {
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *cur;
    int status;
    int in_use = 1;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = type;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    if ((status = getaddrinfo(NULL, port.c_str(), &hints, &result)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
        return -1;
    }
    for (cur = result; cur; cur = cur->ai_next) {
        if ((sock = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol)) == -1)
            continue ;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &in_use, sizeof(int)) == -1) {
            perror("address yet in use");
            close(sock);
            freeaddrinfo(result);
            return -1;
        }
        if (!bind(sock, cur->ai_addr, cur->ai_addrlen))
            break ;
        close(sock);
    }
    freeaddrinfo(result);
    if (cur == NULL) {
        perror("could not bind");
        return -1;
    }
    if (listen(sock, MAX_CLIENT) < 0) {
        perror("listen failure");
        close(sock);
        return -1;
    }
    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
        perror("non block set error");
        close(sock);
        return -1;
    }
    return sock;
}

Server::Server(void) : host("127.0.0.1"), port("80"), sock(-1), root("www") {
    if (serverSocket(SOCK_STREAM) == -1)
        exit(-1);
}

Server::Server(char *file) : host("127.0.0.1"), port("8080"), sock(-1), root("www") {
    ifstream in(file);
    if (!in.is_open() || in.bad() || in.fail()) {
        return;
    }
    cout << "ok" << endl;
    in.close();
}

Server::Server(Server const &pointer) { *this = pointer; }

void Server::run(void) {
    struct sockaddr_storage store;
    string method, path, protocol;

    while (1) {
        socklen_t len = sizeof(store);
        int client = accept(sock, (struct sockaddr *)&store, &len);
        if (client == -1)
            continue;
        istringstream parse(receiver(client, 65535));
        parse >> method >> path >> protocol;
        if (method == "GET") {

        }
        else if (method == "POST") {
        }
        else {
            cerr << "method " << method << " can't be interpreted\n";
        }
        close(client);
    }
}

void Server::sender(int client_socket, string buffer) {
    send(client_socket, buffer.c_str(), buffer.size(), 0);
}

std::string Server::receiver(int client_socket, int maxsize) {
    char buffer[maxsize];
    int len = recv(client_socket, buffer, maxsize, 0);
    buffer[len] = 0;
    std::stringstream ss;
    ss << buffer;
    return ss.str();
}

Server &Server::operator=(Server const &pointer) {
    if (this != &pointer) {
        host = pointer.host;
        port = pointer.port;
        sock = pointer.sock; 
    }
    return *this;
}

Server::~Server(void) { close(sock); }
