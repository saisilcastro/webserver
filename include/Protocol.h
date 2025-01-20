#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <sstream>
#include <iostream>
#include <cstdlib>
#include <map>

class Server;

// colors
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define RESET "\033[0m"

using namespace std;

typedef enum {
    INVALID_REQUEST,
    ENTITY_TOO_LARGE,
    GET,
    POST,
    DELETE,
    INVALID_HOST,
    CONFLICT,
    EMPTY
} method_e;

class Protocol {
    string method;
    string path;
    string type;
    string connection;
    string boundary;
    string tmpHost;
    string file;
    size_t length;
    size_t header;
    string contentBody;
    bool invalidMethod;
    bool invalidHost;

public:
    Protocol(void);
    Protocol(char *);
    void reset(void);
    bool extract(const char *, Server *);
    method_e isMethod(void);
    void setMethod(string);
    string getPath(void);
    string getType(void);
    string getConnection(void);
    string getBoundary(void);
    string getFileName(void);
    size_t getFileLen(void);
    size_t getHeaderLen(void);
    string getHost(void);
    ~Protocol(void);
    string getContentBody(void) const { return contentBody; }
    string getMethod(void) const { return method; }
    friend ostream &operator<<(ostream &os, const Protocol &protocol);
};

#endif
