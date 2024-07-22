#include "Stream.h"
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

Server::Server(void) : host("127.0.0.1"), port("8080"), sock(-1), root("www"), mime("text/html") {
    if (serverSocket(SOCK_STREAM) == -1)
        exit(-1);
}

Server::Server(char *file) : host("127.0.0.1"), port("80"), sock(-1), root("www"), mime("text/html") {
    ifstream in(file);
    if (!in.is_open() || in.bad() || in.fail()) {
        return;
    }
    cout << "ok" << endl;
    in.close();
}

Server::Server(Server const &pointer) { *this = pointer; }

string  Server::mimeMaker(string path) {
    size_t  pos;

    if ((pos = path.rfind(".")) != string::npos) {
        string  ext = path.substr(pos + 1, path.size() - pos);

        if (ext == "htm" || ext == "html")
            mime = "text/html";
        if (ext == "css")
            mime = "text/css"; 
        if (ext == "js")
            mime = "application/javascript";
        if (ext == "json")
            mime = "application/json";
        if (ext == "txt")
            mime = "text/plan";
        if (ext == "gif")
            mime = "image/gif";
        if (ext == "jpg" || ext == "jpeg")
            mime = "image/jpg";

    }
    return mime;
}

void  Server::contentMaker(int client, string protocol, string connection, void *data, size_t len) {
    time_t  m_time;
    char    head[65536];
    m_time = time(NULL);

    int head_len = sprintf(head, "%s\n"
                                   "Date: %s"
                                   "Connection: %s\n"
                                   "Content-Type: %s\n"
                                   "Content-Lenght: %li\n\n",
                                   protocol.c_str(), ctime(&m_time), connection.c_str(), mime.c_str(), len);
    char *content = new char[head_len + len];
    sprintf(content, "%s", head);
    memcpy(content + head_len, data, len);
    int ok = send(client, content, head_len + len, 0);
    if (ok == -1) {
        cerr << "could not send content\n";
    }
    delete []content;
}

void    Server::response(int client, string path, string protocol) {
    size_t  pos = path.rfind(".");
    stringstream    file;
    Stream          stream("");

    mimeMaker(path);
    if (pos == string::npos) {
        file << root << "/index.html";
        stream.loadFile(file.str());
    }
    else {
        file << root << path;
        stream.loadFile(file.str());
    }
    contentMaker(client, protocol + " 200 OK", "alive", stream.getStream(), stream.streamSize());
}

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
            response(client, path, protocol);
        }
        else if (method == "POST") {
        }
        else {
            cerr << "method " << method << " can't be interpreted\n";
        }
        close(client);
    }
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
        root = pointer.root;
        mime = pointer.mime;
    }
    return *this;
}

Server::~Server(void) { close(sock); }
