#include "Protocol.h"

Protocol::Protocol(void) : method("GET"), path("/"), type("HTTP/1.1"), connection("keep-alive"), boundary(""), file(""), length(0), header(0){}

Protocol::Protocol(char *data) : method("GET"), path("/"), type("HTTP/1.1"), connection("keep-alive"), boundary(""), file(""), length(0), header(0){
    extract(data);
}

string inside(string text, string sub, string stop) {
    size_t  pos;
    if ((pos = text.find(sub)) != string::npos) {
        size_t  start = pos + sub.length();
        size_t  end = text.find(stop, start);
        return text.substr(start, end - start);
    }
    return "";
}

void    Protocol::extract(char *data) {
    istringstream parse(data);
    size_t  pos;

    parse >> method >> path >> type;
    if ((pos = parse.str().find("\r\n\r\n")) != string::npos)
        header = parse.str().substr(0, pos + 4).size();
    connection = inside(parse.str(), "Connection: ", "\n");
    boundary = inside(parse.str(), "boundary=", "\n");
    file = inside(parse.str(), "filename=\"","\"");
    length = atoi(inside(parse.str(), "Content-Length: ", "\n").c_str());
}

bool    Protocol::isMethod(string value) {
    if (method == value)
        return true;
    return false;
}

string  Protocol::getPath(void) {
    return path;
}

string  Protocol::getType(void) {
    return type;
}

string  Protocol::getConnection(void) {
    return connection;
}

string  Protocol::getBoundary(void) {
    return boundary;
}

string  Protocol::getFileName(void) {
    return file;
}

size_t  Protocol::getFileLen(void) {
    return length;
}

size_t  Protocol::getHeaderLen(void) {
    return header;
}

Protocol::~Protocol(){}
