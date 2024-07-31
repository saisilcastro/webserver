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
    size_t          currentSize = 0;
    size_t          writtenByte = 0;
    size_t          offset;
    int             piece;
    ofstream        out; 
 
    master.reset();
    while (creating) {
        FD_ZERO(&read_fd);
        FD_SET(client, &read_fd);

        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        int receiving = select(client + 1, &read_fd, NULL, NULL, &timeout);
        if (receiving <= 0)
            creating = false;
        else {
            if (FD_ISSET(client, &read_fd)) {
                piece = recv(client, buffer, 65535, 0);

                if (piece > 0) {
                    currentSize += piece;
                    if (packetCreated == false && !out.is_open()) {
                        master.extract(buffer);
                        packetCreated = true;
                        if (master.getFileLen()) {
							cout << "entrou" << endl;
                            string path = "./" + root + "/upload/" + master.getFileName();
                            if (master.isMethod("POST")) {
                                struct stat mStat;
                                if (!stat(path.c_str(), &mStat) && mStat.st_size > 0) {
                                    remove(path.c_str());
                                }
                                out.open(path.c_str(), ios::out | ios::binary);
                            }
                            if (!out.is_open())
                                continue ;
                            offset = (size_t)master.getHeaderLen();
                        }
                    }

                    size_t  dataLen = piece - offset;
                    size_t  remainingLen = size_t(master.getFileLen()) - writtenByte;

                    if (remainingLen < dataLen)
                        dataLen = remainingLen;
                    if (dataLen > 0) {
                        out.write(buffer + offset, dataLen);
                        writtenByte += dataLen;
                    }
                    if (master.isMethod("POST"))
                        cout << "uploaded " << writtenByte << " of " << master.getFileLen() << endl;
                    if (writtenByte >= master.getFileLen())
                        break;
                    offset = 0;
                }
                else
                    creating = false;
            }
        }
    }
    out.close();
    return "";
}

string  Server::mimeMaker(string path) {
    size_t  pos;

    mime = "text/html";
    if ((pos = path.rfind(".")) != string::npos) {
        string  ext = path.substr(pos + 1, path.size() - pos);

        if (ext == "htm" || ext == "html")
            mime = "text/html";
        else if (ext == "css")
            mime = "text/css"; 
        else if (ext == "js")
            mime = "application/javascript";
        else if (ext == "json")
            mime = "application/json";
        else if (ext == "txt")
            mime = "text/plan";
        else if (ext == "gif")
            mime = "image/gif";
        else if (ext == "jpg" || ext == "jpeg")
            mime = "image/jpg";
        else
            mime = "text/html";
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

string Server::findLocation(const string& path)
{
	vector<Location>::const_iterator start = locations.begin();
	vector<Location>::const_iterator end = locations.end();

	while(start != end)
	{
		map<string, string>::const_iterator it = start->directives.begin();
		map<string, string>::const_iterator ite = start->directives.end();
		while(it != ite)
		{
			if(it->first == path)
				return it->second;
		}
	}
	return "";
}

void    Server::response(int client, string path, string protocol) {
    size_t  pos = path.rfind(".");
    Stream  stream("");
	
    mimeMaker(path);
    if (pos == string::npos)
	{
		string index = findLocation("index");
		if(!index.empty())
			stream.loadFile(root + '/' + index);
		else
			stream.loadFile(root + "/index2.html");
	}
    else {
        if ((pos = path.find(".")) != string::npos) {
            if (path.find("?") != string::npos) {
                if ((path.find(".php") != string::npos && path[pos + 4] != '?') ||
                    (path.find(".py") != string::npos && path[pos + 3]))
                    path = "/404.html";
                else {
                    if (path.find(".php") != string::npos)
                        path = path.substr(0, pos + 4);
                    else if (path.find(".py") != string::npos)
                        path = path.substr(0, pos + 3);
                }
            }
            else {
                if ((path.find(".php") != string::npos && path.length() > pos + 4) ||
                    (path.find(".py") != string::npos && path.length() > pos + 3))
                    path = "/404.html";
            }
        }
        stream.loadFile(root + path);
    }
    contentMaker(client, protocol + " 200 OK", "keep-alive", stream.getStream(), stream.streamSize());
}

void    Server::requestTreat(int client, string data) {
    (void)data;
    if (master.isMethod("GET"))
        response(client, master.getPath(), master.getType());
    else if (master.isMethod("POST")) {
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
		locations = pointer.locations;
    }
    return *this;
}

Server::~Server(void) { close(sock); }
