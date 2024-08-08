#ifndef SERVER_H
#define SERVER_H

#include "Protocol.h"
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <ctime>
#include <vector>
#include <map>
#include <algorithm>
#include <dirent.h>
#include <sys/types.h>


class Stream;

typedef struct sockaddr_in SockAddrIn;
using namespace std;

struct Location
{
	string path;
	//map<name, value>
	map<string, string> directives;
    map<string, string> errorPages;
};

class Server {
public:
    Server(void);
    Server(char *);
    Server(Server const &);
    int     serverSocket(int, const string&);
    string  createPacket(int);
    void    requestTreat(int, string);
    string  mimeMaker(string);
    void    contentMaker(int, string, string, void *, size_t);
    void    response(int, string, string);
    void    postPrepare(string);
    void    run(void);
    Server & operator = (Server const &);
    ~Server(void);

	vector<Location> getLocations() const;
    vector<Location>::iterator getBegin();
    vector<Location>::iterator getEnd();
    vector<Location>::const_iterator getBegin() const;
    vector<Location>::const_iterator getEnd() const;
    
    void setPort(string&);  
    string getPort() const;

    void setRoot(const string& root);
    string getRoot() const;

    void setHost(const string& host);
    string getHost() const;

    void addLocation(const Location& location);

	Location findLocationPath(const string& name);
    string findDirectiveName(const string& name);
    string findDirectiveValue(const string& path);
    void setMaxBodySize(const string& size);
    void addErrorPage(const string& error, const string& path);
    void addErrorPage(const string& error, const string& path, struct Location& location);
    string getErrorPage(const string& error);
    string getErrorPage(const string& error, struct Location& location);
    void handleDeleteMethod(const string &path);
    string adjustScriptPath(const string &path);
    void loadIndexPage(Stream &stream, Location &location);
    void loadErrorPage(Stream &stream, const string &errorCode);
    void loadDirectoryPage(Stream &stream, Location &location);
private:
    string     host;
    string     port;
    size_t     MaxBodySize;
    int        sock;
    string     root;
    string     mime;
    Protocol   master;
    bool       transfer;
	vector<Location> locations;
    map<string, string> errorPages;
    vector<string> ports;
};

void parser(const char *file, Server& config);
void printLocations(const Server& config);
string extractURL(string &path);

#endif
