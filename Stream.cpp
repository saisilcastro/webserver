#include "Stream.h"
#define BUFFER_SIZE 65536

Stream::Stream(void) : buffer(NULL), size(0) {}

Stream::Stream(string file) : buffer(NULL), size(0) {
    if (!file.empty())
        loadFile(file);
    else
        saveFile(file);
}

void    Stream::createStream(void *data, size_t len) {
    size = len;
    buffer = new char[size];
    if (!buffer)
        return ;
    memcpy(buffer, data, size);
    ((char *)buffer)[size - 1] = '\0';
}

void	*Stream::getStream(void) {
    return buffer;
}

int		Stream::streamSize(void) {
    return size;
}

void    Stream::loadFile(string file) {
    ifstream in(file.c_str(), std::ios::binary | std::ios::ate);

    if (!in.is_open() || in.bad() || in.fail()) {
        return;
    }
    size = in.tellg();
    in.seekg(0, std::ios::beg);
    buffer = new char[size];
    if (!buffer)
        return ;
    in.read(reinterpret_cast<char *>(buffer), size);
    in.close();
}

void	Stream::saveFile(string file) {
    if (file.empty())
        return;
    ofstream out(file.c_str(), std::ofstream::out | std::ofstream::binary);

    if (!out.is_open() || out.bad() || out.fail())
        return;
    out.write(reinterpret_cast<char*>(buffer), size);
    if (!out) {
        cerr << "could not write file\n";
        return ;
    }
    out.close();
}

Stream & Stream::operator = (Stream & pointer) {
    if (this != &pointer) {
        buffer = reinterpret_cast<char*>(pointer.buffer);
        size = pointer.size;
    }
    return *this;
}

Stream::~Stream(void) {
    if (buffer)
        delete[] reinterpret_cast<char *>(buffer);
}
