#include "Server.h"
#include "Stream.h"

#define MAX_CLIENT 65535

/**
 * @brief Returns the value of the directive
 * 
 * @param path It corresponds to the name of the directive in the `data` map.
 * 
 * @return The value of the directive if found; otherwise, returns an empty string.
 */
string Server::findDirectiveValue(const string& path)
{
    vector<Location>::const_iterator start = location.begin();
    vector<Location>::const_iterator end = location.end();

    while(start != end)
    {
        map<string, string>::const_iterator it = start->data.begin();
        map<string, string>::const_iterator ite = start->data.end();
        while(it != ite)
        {
            if(it->first == path)
                return it->second;
            ++it;
        }
        ++start;
    }
    return "";
}


string Server::findDirectiveName(const string& path)
{
    vector<Location>::const_iterator start = location.begin();
    vector<Location>::const_iterator end = location.end();
    while(start != end)
    {
        map<string, string>::const_iterator it = start->data.begin();
        map<string, string>::const_iterator ite = start->data.end();
        while(it != ite)
        {  
            if(it->second == path)
                return it->first;
            ++it;
        }
        ++start;
    }
    return "";
}


Location Server::findLocationPath(const string& path)
{
    vector<Location>::const_iterator start = location.begin();
    vector<Location>::const_iterator end = location.end();
    while(start != end)
    {
        if(start->path == path)
            return *start;
        ++start;
    }
    return Location();
}


// Orthodox Canonical Form

Server::Server(void) : host("127.0.0.1"), port("8080"), maxBodySize(-1), sock(-1), root("www"), mime("text/html"), transfer(true) { 
    this->location.push_back(Location());
    this->location[0].path = "/";
    this->location[0].data["root"] = "www/";
    this->location[0].data["index"] = "index.html";
}

Server::Server(char *file) : host("127.0.0.1"), port("80"), sock(-1), root("www"), mime("text/html"), transfer(true) {
    ifstream in(file);
    if (!in.is_open() || in.bad() || in.fail()) {
        return;
    }
    in.close();
}

Server::Server(string _host, string _port, string _root, map<string, string> _error, vector<Location> _location, size_t _maxBodySize)
: host(_host), port(_port), maxBodySize(_maxBodySize), root(_root), error(_error), location(_location) {
	errorPages["403"] = "default/defaultErrorPages/403.html";
	errorPages["404"] = "default/defaultErrorPages/404.html";
	errorPages["405"] = "default/defaultErrorPages/405.html";
	errorPages["413"] = "default/defaultErrorPages/413.html";
	errorPages["500"] = "default/defaultErrorPages/500.html";
	errorPages["504"] = "default/defaultErrorPages/504.html";
}


Server &Server::operator=(Server const &pointer) {
    if (this != &pointer) {
        host = pointer.host;
        port = pointer.port;
        sock = pointer.sock;
        root = pointer.root;
        mime = pointer.mime;
		location = pointer.location;
        maxBodySize = pointer.maxBodySize;
        transfer = pointer.transfer;
        error = pointer.error;
    }
    return *this;
}

Server::~Server(void) { close(sock); }

vector<Location> Server::getLocations() const {
    return location;
}

vector<Location>::iterator Server::getBegin() {
    return location.begin();
}

vector<Location>::iterator Server::getEnd() {
    return location.end();
}

vector<Location>::const_iterator Server::getBegin() const {
    return location.begin();
}

vector<Location>::const_iterator Server::getEnd() const {
    return location.end();
}


void trim(string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == string::npos) {
        str.clear();
        return;
    }
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    str = str.substr(start, end - start + 1);
}

void Server::setPort(string& port) {
    if (port.find(",") != string::npos) {
        size_t pos = 0;
        string token;
        while ((pos = port.find(",")) != string::npos) {
            token = port.substr(0, pos);
            trim(token);
            if (!token.empty()) {
                if (find(this->ports.begin(), this->ports.end(), token) == this->ports.end()) {
                    this->ports.push_back(token);
                }
            }
            port.erase(0, pos + 1);
        }
        trim(port);
        if (!port.empty()) {
            if (find(this->ports.begin(), this->ports.end(), port) == this->ports.end()) {
                this->ports.push_back(port);
            }
        }
    } else {
        trim(port);
        if (!port.empty()) {
            if (find(this->ports.begin(), this->ports.end(), port) == this->ports.end()) {
                this->ports.push_back(port);
            }
        }
    }
}



string Server::getPort() const {
    return port;
}

void Server::setRoot(const string& root) {
    this->root = root;
}

string Server::getRoot() const {
    return root;
}

void Server::setHost(const string& host) {
    this->host = host;
}

string Server::getHost() const {
    return host;
}

void Server::addErrorPage(const string& errorCode, const string& path) {
    error[errorCode] = path;
}

string Server::getErrorPage(const string& errorCode) {
    return error[errorCode];
}

string extractURL(string &path)
{
    if(path.size() == 1)
        return("");
    for(size_t i = 1; i < path.length(); i++)
    {
        if(path[i] == '/')
            return(path.substr(0, i));
    }
    return (path);
}

string ft_strip(const string& s) {
    string::size_type start = s.find_first_not_of(" \t\n\r\f\v");
    if (start == string::npos)
        return "";
    string::size_type end = s.find_last_not_of(" \t\n\r\f\v");
    return s.substr(start, end - start + 1);
}

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

void Server::contentMaker(ContentMaker& content)
{
    int client = content.getClient();
    string protocol = content.getProtocol() + content.getStatus();
    string connection = content.getConnection();
    void *data = content.getData();
    size_t len = content.getLen();

    contentMaker(client, protocol, connection, data, len);
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
    delete[] content;
}

string Server::getPageDefault(const string &errorCode) {
    string page = error[errorCode];
    if(!page.empty() && access(page.c_str(), F_OK))
        return page;
    
    map<string, string>::iterator it = this->errorPages.find(errorCode);
    if (it != this->errorPages.end()) {
        return it->second;
    }
    return("");
}

void Server::loadErrorPage(Stream &stream, const string &errorCode) {
    string page = this->errorPages[errorCode];
    if(page.empty() || !access(page.c_str(), F_OK))
        stream.loadFile(getPageDefault(errorCode));
    else
        stream.loadFile(root + page);
}

void Server::loadIndexPage(Stream &stream, Location &location) {
    string index = location.data["index"];
	string tmpRoot = location.data["root"];

	if(index.empty())
		index = findLocationPath("/").data["index"];

    if(tmpRoot.empty())
        stream.loadFile(root + location.path + '/' + index);
    else
        stream.loadFile(tmpRoot + '/' + index);
    
}

#include <iostream>
#include <sstream>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>

void Server::loadDirectoryPage(int client, Stream &stream, const string &fullPath) {

    string html = "<html><head><title>Index of " + fullPath + "</title></head>"
                       "<body><h1>Index of " + fullPath + "</h1><hr><pre>";
    
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(fullPath.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string fileName(ent->d_name);
            html += fileName + "<br>";
        }
        closedir(dir);
    } else {
        loadError(client, getPageDefault("500"), "500 Internal Server Error");
        return;
    }

    html += "</pre><hr></body></html>";

    char tempFile[] = "/tmp/tmpFileXXXXXX";
    int fd = mkstemp(tempFile);
    if (fd == -1) {
        loadError(client, getPageDefault("500"), "500 Internal Server Error");
        return;
    }

    ofstream ofs(tempFile);
    if (ofs.is_open()) {
        ofs << html;
        ofs.close();
    } else {
        loadError(client, getPageDefault("500"), "500 Internal Server Error");
        close(fd);
        return;
    }

    stream.loadFile(tempFile);
    close(fd);
    remove(tempFile);
}



void Server::defineFullPath(string &fullPath, Location &location, string url) {
    if(location.data.find("root") != location.data.end())
        fullPath = location.data["root"];
    else
        fullPath = root + url;
}

void Server::defineLocationPath(Location &location, string path, string &LocationRoot) {
    string url = extractURL(path);
    if (url == "")
        location = findLocationPath("/");
    else
        location = findLocationPath(url);

    if(location.data.find("root") != location.data.end())
        LocationRoot = location.data["root"];
    else if(!location.path.empty() && location.data.find("root") == location.data.end())
        LocationRoot = "";
}

bool Server::HandleErrors(int client, string path, string protocol) {
	// 405
	// 404
	// 413
	// 504
	Stream stream(this);
	bool result = false;
	(void)path;
	if(master.isMethod() == INVALID_REQUEST)
	{
		result = true;
		_contentMaker.setStatus(" 405 Method Not Allowed");
		// loadErrorPage(stream, "405");
		stream.loadFile("default/defaultErrorPages/405.html");
	}
	if(result == true)
	{
		_contentMaker = ContentMaker(client, protocol, "keep-alive", _contentMaker.getStatus(), stream.getStream(), stream.streamSize());
		contentMaker(_contentMaker);
	}
	return result;
}
