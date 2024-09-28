#include "Server.h"

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
: host(_host), port(_port), maxBodySize(_maxBodySize), root(_root), error(_error), location(_location){}

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


void trim(std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        str.clear();
        return;
    }
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    str = str.substr(start, end - start + 1);
}

void Server::setPort(std::string& port) {
    if (port.find(",") != std::string::npos) {
        size_t pos = 0;
        std::string token;
        while ((pos = port.find(",")) != std::string::npos) {
            token = port.substr(0, pos);
            trim(token);
            if (!token.empty()) {
                if (std::find(this->ports.begin(), this->ports.end(), token) == this->ports.end()) {
                    this->ports.push_back(token);
                }
            }
            port.erase(0, pos + 1);
        }
        trim(port);
        if (!port.empty()) {
            if (std::find(this->ports.begin(), this->ports.end(), port) == this->ports.end()) {
                this->ports.push_back(port);
            }
        }
    } else {
        trim(port);
        if (!port.empty()) {
            if (std::find(this->ports.begin(), this->ports.end(), port) == this->ports.end()) {
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

/* void Server::addLocation(const Location& location) {
    location.push_back(location);
} */

/* void Server::setMaxBodySize(const string& size)
{
    int bodySizeInBytes = atoi(size.c_str());

    if (bodySizeInBytes == 0 && size[0] != '0') {
        throw runtime_error("Invalid body size format, using default file .conf");
    }

    if (size.find_first_of("Gg") != string::npos)
        MaxBodySize = bodySizeInBytes * 1024 * 1024 * 1024;
    else if (size.find_first_of("Mm") != string::npos)
        MaxBodySize = bodySizeInBytes * 1024 * 1024;
    else if (size.find_first_of("Kk") != string::npos)
        MaxBodySize = bodySizeInBytes * 1024;
    else if (size.find_first_not_of("0123456789") == string::npos)
        MaxBodySize = bodySizeInBytes;
    else
	{

        throw runtime_error("Invalid body size, using default file .conf");
	}

} */

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

std::string ft_strip(const std::string& s) {
    std::string::size_type start = s.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos)
        return "";
    std::string::size_type end = s.find_last_not_of(" \t\n\r\f\v");
    return s.substr(start, end - start + 1);
}
