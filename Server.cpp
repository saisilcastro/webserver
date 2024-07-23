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

string  Server::createPacket(int client) {
    fd_set          read_fd;
    struct timeval  timeout;
    bool            packetCreated = false;
    bool            creating = true;
    char            buffer[65535];
    int             piece;
    string          data("");
 
    while (creating) {
        FD_ZERO(&read_fd);
        FD_SET(client, &read_fd);

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        int receiving = select(client + 1, &read_fd, NULL, NULL, &timeout);
        if (receiving <= 0)
            creating = false;
        else {
            if (FD_ISSET(client, &read_fd)) {
                piece = recv(client, buffer, 65535, 0);

                if (piece > 0) {
                    data.append(buffer, piece);
                    if (packetCreated == false) {
                        master.extract(buffer);
                        packetCreated = true;
                    }
                    if (packetCreated && master.getFileLen() > 0 && data.size() >= (size_t)(master.getFileLen() + master.getHeaderLen()))
                        break ;
                }
                else
                    creating = false;
            }
        }
    }
    return data;
}

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

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end, delimiter_length = delimiter.length();

    while ((end = str.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter_length;
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

void    Server::postPrepare(string data) {
    vector<string> part = split(data, "--" + master.getBoundary());
    string         content;

    for (size_t i = 0; i < part.size(); ++i) {
        if (part[i].find("Content-Disposition: form-data;") != std::string::npos) {
            size_t headersEnd = part[i].find("\r\n\r\n");
            if (headersEnd != std::string::npos) {
                content = part[i].substr(headersEnd + 4);
                content = content.substr(0, content.size() - 2);
            }
        }
    }
    Stream  file;
    file.createStream((void *)content.c_str(), content.size());
    file.saveFile(master.getFileName());
}

void    Server::requestTreat(int client, string data) {
    if (master.isMethod("GET"))
        response(client, master.getPath(), master.getType());
    else if (master.isMethod("POST")) {
        postPrepare(data);
        response(client, master.getPath(), master.getType());
    }
}

void Server::run(void) {
    int     client = -1;

    while (1) {
        client = accept(sock, NULL,NULL);
        if (client == -1)
            continue;
        fcntl(client, F_SETFL, O_NONBLOCK);
        requestTreat(client, createPacket(client));
        close(client);
    }
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
