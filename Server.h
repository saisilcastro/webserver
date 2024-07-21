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

typedef struct sockaddr_in SockAddrIn;
using namespace std;

class Server {
public:
    Server(void);
    Server(char *);
    Server(Server const &);
    int     serverSocket(int);
    
    void    run(void);
    void    sender(int, string);
    string  receiver(int, int);
    Server & operator = (Server const &);
    ~Server(void);
private:
    string     host;
    string     port;
    int        sock;
    string     root;
};

#endif
