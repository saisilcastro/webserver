#include "Server.h"

/**
 * @brief Returns the value of the directive
 * 
 * @param path It corresponds to the name of the directive in the `directives` map.
 * 
 * @return The value of the directive if found; otherwise, returns an empty string.
 */
string Server::findDirectiveValue(const string& path)
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
            ++it;
        }
        ++start;
    }
    return "";
}


string Server::findDirectiveName(const string& path)
{
    vector<Location>::const_iterator start = locations.begin();
    vector<Location>::const_iterator end = locations.end();
    while(start != end)
    {
        map<string, string>::const_iterator it = start->directives.begin();
        map<string, string>::const_iterator ite = start->directives.end();
        while(it != ite)
        {  
            cout << "path: " << path << " it->second: " << it->second << endl;
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
    vector<Location>::const_iterator start = locations.begin();
    vector<Location>::const_iterator end = locations.end();
    while(start != end)
    {
        if(start->path == path)
            return *start;
        ++start;
    }
    return Location();
}


// Orthodox Canonical Form

Server::Server(void) : host("127.0.0.1"), port("8080"), MaxBodySize(-1), sock(-1), root("www"), mime("text/html"), transfer(true) { 
}

Server::Server(char *file) : host("127.0.0.1"), port("80"), sock(-1), root("www"), mime("text/html"), transfer(true) {
    ifstream in(file);
    if (!in.is_open() || in.bad() || in.fail()) {
        return;
    }
    cout << "ok" << endl;
    in.close();
}

Server::Server(Server const &pointer) { *this = pointer; }

Server &Server::operator=(Server const &pointer) {
    if (this != &pointer) {
        host = pointer.host;
        port = pointer.port;
        sock = pointer.sock;
        root = pointer.root;
        mime = pointer.mime;
		locations = pointer.locations;
        MaxBodySize = pointer.MaxBodySize;
        transfer = pointer.transfer;
    }
    return *this;
}

Server::~Server(void) { close(sock); }

vector<Location> Server::getLocations() const {
    return locations;
}

vector<Location>::iterator Server::getBegin() {
    return locations.begin();
}

vector<Location>::iterator Server::getEnd() {
    return locations.end();
}

vector<Location>::const_iterator Server::getBegin() const {
    return locations.begin();
}

vector<Location>::const_iterator Server::getEnd() const {
    return locations.end();
}

void Server::setPort(const string& port) {
    this->port = port;
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

void Server::addLocation(const Location& location) {
    locations.push_back(location);
}

void Server::setMaxBodySize(const string& size)
{
    int bodySizeInBytes = std::atoi(size.c_str());

    if (bodySizeInBytes == 0 && size[0] != '0') {
        throw std::runtime_error("Invalid body size format, using default file .conf");
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
        throw std::runtime_error("Invalid body size, using default file .conf");
}

void Server::addErrorPage(const string& error, const string& path) {
    errorPages[error] = path;
}

void Server::addErrorPage(const string& error, const string& path, struct Location& location) {
    location.errorPages[error] = path;
}

string Server::getErrorPage(const string& error) {
    return errorPages[error];
}

string Server::getErrorPage(const string& error, struct Location& location) {
    return location.errorPages[error];
}

string extractURL(string &path)
{
    if(path.size() == 1)
        return("");
    for(size_t i = 1; i < path.length(); i++)
    {
        if(path[i] == '/')
            return(path.substr(0, i) + ' ');
    }
    return (path + " ");
}
