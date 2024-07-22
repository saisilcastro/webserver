#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
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

typedef struct sockaddr_in SockAddrIn;
using namespace std;

class Server {
public:
    Server(void);
    Server(char *);
    Server(Server const &);
    int     serverSocket(int);
    string  mimeMaker(string);
    void    contentMaker(int, string, string, void *, size_t);
    void    response(int, string, string);
    void    run(void);
    string  receiver(int, int);
    Server & operator = (Server const &);
    ~Server(void);
private:
    string     host;
    string     port;
    int        sock;
    string     root;
    string     mime;
};

#endif
