#include <Stream.h>
#include <Server.h>

Server::Server(void) : host("localhost"), port("80"), root("./www"){}

Server::Server(string _host, string _port, string _root, map<string, string> _error, vector<Location> _location)
: host(_host), port(_port), root(_root), error(_error), location(_location){}

#define MAX_CLIENT 10

int Server::serverSocket(int type) {
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *cur;
	int status;
	int in_use = 1;
	int	sock = -1;
	
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

string  Server::createPacket(int client) {
	fd_set          read_fd;
	struct timeval  timeout;
	bool            packetCreated = false;
	bool            creating = true;
	char            buffer[65535];
	size_t          currentSize = 0;
	size_t          writtenByte = 0;
	size_t          offset = 0;
	int             piece;
	string			path("");
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
			timeout.tv_usec = 100000;
		}
		int receiving = select(client + 1, &read_fd, NULL, NULL, &timeout);
		if (receiving <= 0) {
			creating = false;
			transfer = false;
			cout << "not receiving\n";
		}
		else {
			if (FD_ISSET(client, &read_fd)) {
				piece = recv(client, buffer, 65535, 0);

				if (piece > 0) {
					currentSize += piece;
					if (packetCreated == false && !out.is_open()) {
						master.extract(buffer);
						packetCreated = true;
						if (master.getFileLen() && master.getFileLen() < maxBodySize) {
							path = "./" + root + "/upload/" + master.getFileName();
							if (master.isMethod() == POST) {
								struct stat mStat;
								if (!stat(path.c_str(), &mStat) && mStat.st_size > 0) {
									remove(path.c_str());
								}
								out.open(path.c_str(), ios::out | ios::binary);
							}
							if (!out.is_open())
								continue ;
							if (master.getFileLen() < maxBodySize)
								offset = (size_t)master.getHeaderLen();
						}
					}
					size_t  dataLen = piece - offset;
					size_t  remainingLen = size_t(master.getFileLen()) - writtenByte;

					if (remainingLen < dataLen)
						dataLen = remainingLen;
					if (dataLen > 0) {
						char *sub = strstr(buffer + offset, master.getBoundary().c_str());
						if (sub)
							dataLen -= master.getBoundary().length() + 6;
						if (master.getFileLen() < maxBodySize)
							out.write(buffer + offset, dataLen);
						writtenByte += dataLen;
					}
					if (master.isMethod() == POST)
						cout << "uploaded " << writtenByte << " of " << master.getFileLen() << endl;
					if(writtenByte >= master.getFileLen()) {
						cout << "transfer done\n";
						transfer = true;
						break;
					}
					offset = 0;
				}
				else if (writtenByte < master.getFileLen()) {
					transfer = false;
					break ;
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

void Server::setHost(string value) {
	host = value;
}

void Server::setPort(string value) {
	port = value;
}

void Server::setRoot(string value) {
	root = value;
}

void Server::setError(map<string, string> value) {
	error = value;
}

void Server::setLocation(vector<Location> value) {
	location = value;
}

string Server::getHost(void) {
	return host;
}

string Server::getRoot(void) {
	return root;
}

string Server::getPort(void) {
	return port;
}

map<string, string> Server::getError(void) {
	return error;
}

vector<Location> Server::getLocation(void) {
	return location;
}

void Server::print(void) {
	cout << "----------------------------------------------------------------------------------------------\n";
	cout << "host: " << host << endl;
	cout << "root: " << root << endl;
	cout << "port: " << port << endl;
	for (map<string, string>::iterator iError = error.begin(); iError != error.end(); ++iError)
		cout << "ErrorName: " << iError->first << "ErrorPage: " << iError->second << endl;
	for (vector<Location>::iterator iLocation = location.begin(); iLocation != location.end(); ++iLocation) {
		cout << "Location: \n\n";
		cout << "path: " << iLocation->path << endl;
		for (map<string, string>::iterator data = iLocation->data.begin(); data != iLocation->data.end(); ++data)
			cout << "name: " << data->first << " value: " << data->second << endl;
	}
	cout << "----------------------------------------------------------------------------------------------\n";
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

string Server::locationValueByName(string name) {
  for (vector<Location>::iterator cur = location.begin(); cur != location.end(); ++cur) {
    for (map<string, string>::iterator field = cur->data.begin(); field != cur->data.end(); ++field) {
      cout << field->first << " and " << name << "\n";
      if (field->first == name)
        return field->second;
    }
  }
  return "";
}

void Server::response(int client, string path, string protocol) {
	size_t  pos = path.rfind(".");
	Stream  stream("");
	string  status = " 200 OK";

	if (transfer) {
		if (master.isMethod() != DELETE) {
			mimeMaker(path);
			if (pos == string::npos) {
				string index = locationValueByName("index");
        cout << index << endl;
				if (maxBodySize < master.getFileLen() && master.getFileLen() > 0) {
					stream.loadFile(root + '/' + "413.html");
					status = " 413 Content Too Large";
				}
				else if (!index.empty())
					stream.loadFile(root + '/' + index);
				else {
                    struct stat s;
                    if (stat((root + path).c_str(), &s) == -1)
                        stream.loadFile(root + "/404.html");
                    else
					    stream.loadFile(root + "/index2.html");
                }
			} else {
				if (master.isMethod() == POST && maxBodySize > master.getFileLen()) {
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
		else {
			struct stat mStat;
			string file = root + path;
			if (!access(file.c_str(), F_OK) && !access(file.c_str(), R_OK | W_OK | X_OK)) {
				if (!stat(file.c_str(), &mStat) && mStat.st_size > 0){
					if (remove(file.c_str()) == -1) {
						status = " 405 Method Not Allowed";
						stream.loadFile(root + "/405.html");
					}
				}
			}
			else {
				status = " 403 Forbidden";
				stream.loadFile(root + "/403.html");
			}
		}
	}
	else {
		status = " 500 Internal Server Error";
		stream.loadFile(root + "/500.html");
	}
	contentMaker(client, protocol + status, "keep-alive", stream.getStream(), stream.streamSize());
}

void	Server::requestTreat(int client, string data) {
	(void)data;
	if (master.isMethod() == GET) {
		response(client, master.getPath(), master.getType());
	}
	else if (master.isMethod() == POST) {
		response(client, master.getPath(), master.getType());
	}
	else if (master.isMethod() == DELETE) {
		response(client, master.getPath(), master.getType());
	}
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
	print();
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

Server::~Server(){}
