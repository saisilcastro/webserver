#include "Stream.h"
#include "Server.h"
#define MAX_CLIENT 65535

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
            continue;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &in_use, sizeof(int)) == -1) {
            perror("address yet in use");
            close(sock);
            freeaddrinfo(result);
            return -1;
        }
        if (!bind(sock, cur->ai_addr, cur->ai_addrlen))
            break;
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

vector<string> split(string str, string sep) {
	vector<string> result;
	size_t pos = 0;
	size_t found;

	while ((found = str.find(sep, pos)) != string::npos) {
		result.push_back(str.substr(pos, found - pos));
		pos = found + sep.length();
	}
	result.push_back(str.substr(pos));
	return result;
}

void Server::checkAcceptedMethod(Protocol &master) {
    static vector<string> methods;
    Location local = findLocationPath(master.getPath());

    methods = split(local.data["accepted_methods"], " ");
    if (methods.empty() || methods[0].empty()) {
        methods.clear();
        methods.push_back("GET");
        methods.push_back("POST");
        methods.push_back("DELETE");
    }

    string methodStr;
    switch (master.isMethod()) {
        case GET:    methodStr = "GET";    break;
        case POST:   methodStr = "POST";   break;
        case DELETE: methodStr = "DELETE"; break;
        default:     methodStr = "INVALID"; break;
    }

    if (find(methods.begin(), methods.end(), methodStr) == methods.end()) {
        master.setMethod("INVALID");
    }
}

string Server::createPacket(int client) {
    fd_set          read_fd;
    struct timeval  timeout;
    bool            packetCreated = false;
    bool            creating = true;
    char            buffer[65535];
    size_t          currentSize = 0;
    size_t          writtenByte = 0;
    size_t          dataLen;
    size_t          offset = 0;
    int             piece;
    string          path("");
    ofstream        out; 

    master.reset();
    transfer = false;
    
    while (creating) {
        FD_ZERO(&read_fd);
        FD_SET(client, &read_fd);

        if (!packetCreated || maxBodySize < master.getFileLen()) {
			timeout.tv_sec = 10;
			timeout.tv_usec = 0;
		}
		else {
            timeout.tv_sec = 0;
			timeout.tv_usec = master.getFileLen() / 1000;
		}
        int receiving = select(client + 1, &read_fd, NULL, NULL, &timeout);
        if (receiving < 0) {
            creating = false;
            transfer = false;
            cout << "Error on select\n";
        }
        else if (receiving == 0) {
            transfer = true;
            break;
        }
        else {
            if (FD_ISSET(client, &read_fd)) {
                while (true) {
                    piece = recv(client, buffer, 65535, 0);

                    if (piece > 0) {
                        currentSize += piece;
                        if (!packetCreated && !out.is_open()) {
                            master.extract(buffer);
                            checkAcceptedMethod(master);
                            packetCreated = true;

                            if (master.getFileLen() && master.getFileLen() <= maxBodySize) {
                                path = "upload/" + master.getFileName();
                                if (master.isMethod() == POST) {
                                    struct stat mStat;
                                    if (!stat(path.c_str(), &mStat) && mStat.st_size > 0) {
                                        remove(path.c_str());
                                    }
                                    out.open(path.c_str(), ios::out | ios::binary);
                                }
                                if (!out.is_open())
                                    continue;
                                offset = (size_t)master.getHeaderLen();
                            }
                        }
                        dataLen = piece - offset;
                        size_t remainingLen = size_t(master.getFileLen()) - writtenByte;

                        if (remainingLen <= dataLen)
                            dataLen = remainingLen;
                        if (dataLen > 0) {
                            char *sub = strstr(buffer + offset, master.getBoundary().c_str());
                            if (sub)
                                dataLen -= master.getBoundary().length() + 6;
                            if (master.getFileLen() <= maxBodySize)
                                out.write(buffer + offset, dataLen);
                            writtenByte += dataLen;
                            if (master.isMethod() == POST)
                                cout << "uploaded " << writtenByte << " of " << master.getFileLen() << endl;
                        }
                        if (writtenByte >= master.getFileLen()) {
                            cout << "transfer done\n";
                            transfer = true;
                            break;
                        }
                        offset = 0;
                    }
                    else if (piece == 0 || writtenByte >= master.getFileLen()) {
                        cout << "Received entire file\n";
                        transfer = true;
                        creating = true;
                        break;
                    }
                    else {
                        transfer = false;
                        cout << "*uploaded " << writtenByte << " of " << master.getFileLen() << endl;
                        break;
                    }
                }
            }
        }
    }
    
    out.close();
    if (!transfer) {
        remove(path.c_str());
        cout << "could not transfer " << path << endl;
    }
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
	if (len) {

	}
	char *content = new char[head_len + len];
	sprintf(content, "%s", head);
	memcpy(content + head_len, data, len);
	int ok = send(client, content, head_len + len, 0);
	if (ok == -1) {
		cerr << "could not send content\n";
	}
}

string getPageDefault(const string &errorCode) {
    static map<string, string> errorPages;
    if (errorPages.empty()) {
        errorPages["403"] = "www/defaultPages/403.html";
        errorPages["404"] = "www/defaultPages/404.html";
        errorPages["405"] = "www/defaultPages/405.html";
        errorPages["413"] = "www/defaultPages/413.html";
        errorPages["500"] = "www/defaultPages/500.html";
    }
    
    map<string, string>::iterator it = errorPages.find(errorCode);
    if (it != errorPages.end()) {
        return it->second;
    }
    return("");
}

void Server::loadErrorPage(Stream &stream, const string &errorCode) {
    string page = errorPages[errorCode];
    if(page.empty())
        stream.loadFile(getPageDefault(errorCode));
    else
        stream.loadFile(root + page);
}

void Server::loadIndexPage(Stream &stream, Location &location) {
    string index = location.data["index"];
	string tmpRoot = location.data["root"];

	if(index.empty())
	{
		index = findLocationPath("/ ").data["index"];
		if(index.empty())
			index = "index.html";
	}

	if(tmpRoot.empty())
		tmpRoot = root;
    cout << "|" << root << "|" << endl;
    stream.loadFile(tmpRoot + location.path + '/' + index);
}

void Server::loadDirectoryPage(Stream &stream, Location &location) {

    std::string html = "<html><head><title>Index of " + location.path + "</title></head><body><h1>Index of " + location.path + "</h1><hr><pre>";
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(location.path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fileName(ent->d_name);
            html += "<a href=\"" + fileName + "\">" + fileName + "</a><br>";
        }
        closedir(dir);
    } else {
        perror("could not open directory");
    }
    html += "</pre><hr></body></html>";

    char tempFile[] = "/tmp/tmpFileXXXXXX";
    int fd = mkstemp(tempFile);
    if (fd == -1) {
        perror("could not create temporary file");
        return;
    }

    std::ofstream ofs(tempFile);
    if (ofs.is_open()) {
        ofs << html;
        ofs.close();
    } else {
        perror("could not open temporary file for writing");
        close(fd);
        return;
    }

    stream.loadFile(tempFile);
    close(fd);
    std::remove(tempFile);
}

void Server::response(int client, string path, string protocol) {
	size_t  pos = path.rfind(".");
	Stream  stream("");
	string  status = " 200 OK";
	string url = extractURL(path);
    struct stat info;
    Location location = findLocationPath(url);
    if(url == "")
        location = findLocationPath("/");
    string fullPath = root + url;

    cout << "fullPath: " << fullPath << endl;
	int method = master.isMethod();
	
	if (transfer) {
		if (method != DELETE && method != INVALID_REQUEST) {
			mimeMaker(path);
			if (pos == string::npos) {
				if (maxBodySize < master.getFileLen() && master.getFileLen() > 0) {
					loadErrorPage(stream, "413");
					status = " 413 Content Too Large";
				}
				else if(stat(fullPath.c_str(), &info) == 0)
				{
					if(location.data.find("index") != location.data.end())
						loadIndexPage(stream, location);
					else
						loadDirectoryPage(stream, location);
				}
				else
				{
					status = " 404 Not Found";
					loadErrorPage(stream, "404");
				}
			}
			else {
				if (method == POST && maxBodySize > master.getFileLen()) {
					contentMaker(client, protocol + " 200 OK", "keep-alive", stream.getStream(), stream.streamSize());
					path = "/413.html";
					status = " 413 Content Too Large";
				}
				else if ((pos = path.find(".")) != string::npos) {
					if (path.find("?") != string::npos) {
						if ((path.find(".php") != string::npos && path[pos + 4] != '?') ||
							(path.find(".py") != string::npos && path[pos + 3])) {
							path = "/404.html";
							status = " 404 Not Found";
						}
						else {
							if (path.find(".php") != string::npos)
								path = path.substr(0, pos + 4);
							else if (path.find(".py") != string::npos)
								path = path.substr(0, pos + 3);
						}
					} else {
						if ((path.find(".php") != string::npos && path.length() > pos + 4) ||
							(path.find(".py") != string::npos && path.length() > pos + 3)) {
							path = "/404.html";
							status = " 404 Not Found";
						}
					}
				}
				stream.loadFile(root + path);
			}
		}
		// função pra delete
		else if (method == DELETE) {
			struct stat mStat;
			string file = root + path;
			if (!access(file.c_str(), F_OK) && !access(file.c_str(), R_OK | W_OK | X_OK)) {
				if (!stat(file.c_str(), &mStat) && mStat.st_size > 0){
					if (remove(file.c_str()) == -1) {
						status = " 405 Method Not Allowed";
						loadErrorPage(stream, "405");
					}
				}
			}
			else {
				status = " 403 Forbidden";
				loadErrorPage(stream, "403");
			}
		}
		// função pra erros
		else
		{
			status = " 405 Method Not Allowed";
			loadErrorPage(stream, "405");
		}
	}
	else {
		status = " 500 Internal Server Error";
		loadErrorPage(stream, "500");
	}
	contentMaker(client, protocol + status, "keep-alive", stream.getStream(), stream.streamSize());
}

void    Server::requestTreat(int client, string data) {
	(void)data;
	response(client, master.getPath(), master.getType());
	/* if (master.isMethod() == GET)
		response(client, master.getPath(), master.getType());
	else if (master.isMethod() == POST) {
		response(client, master.getPath(), master.getType());
	}
	else if (master.isMethod() == DELETE) {
		response(client, master.getPath(), master.getType());
	} */
}

void Server::execute(int socket) {
    int client = accept(socket, NULL, NULL);
    if (client == -1)
        return ;
    fcntl(client, F_SETFL, O_NONBLOCK);
    requestTreat(client, createPacket(client));
    close(client);
}

void Server::run(void) {
	int		sock;

	if ((sock = serverSocket(SOCK_STREAM)) == -1)
		exit(-1);
	// print();
	while (1)
        execute(sock);
}

void    Run(Server *server, int max) {
    int sock[max];
    for (int i = 0; i < max; i++) {
        if ((sock[i] = server[i].serverSocket(SOCK_STREAM)) == -1)
            exit(-1);
    }
    while (1) {
        for (int i = 0; i < max; i++)
            server[i].execute(sock[i]);
    }
}
