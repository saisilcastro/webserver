#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <sstream>
#include <iostream>
#include <cstdlib>

using namespace std;
typedef enum{
    INVALID_REQUEST,
    GET,
    POST,
    DELETE
}method_e;

class Protocol {
    string  method;
    string  path;
    string  type;
    string  connection;
    string  boundary;
    string  tmpHost;
    string  file;
    size_t  length;
    size_t  header;
public:
    Protocol(void);
    Protocol(char *);
    void        reset(void);
    void        extract(char *);
    method_e    isMethod(void);
    void        setMethod(string);
    string      getPath(void);
    string      getType(void);
    string      getConnection(void);
    string      getBoundary(void);
    string      getFileName(void);
    size_t      getFileLen(void);
    size_t      getHeaderLen(void);
    string      getHost(void);
    ~Protocol(void);
};

#endif
