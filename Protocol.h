#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <sstream>
#include <iostream>
#include <cstdlib>

using namespace std;

class Protocol {
    string  method;
    string  path;
    string  type;
    string  connection;
    string  boundary;
    string  file;
    ssize_t  length;
    ssize_t  header;
public:
    Protocol(void);
    Protocol(char *);
    void    reset(void);
    void    extract(char *);
    bool    isMethod(string);
    void    setMethod(string);
    string  getPath(void);
    string  getType(void);
    string  getConnection(void);
    string  getBoundary(void);
    string  getFileName(void);
    ssize_t  getFileLen(void);
    ssize_t  getHeaderLen(void);
    ~Protocol(void);
};

#endif
