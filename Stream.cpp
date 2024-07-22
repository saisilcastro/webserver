#include "Stream.h"

Stream::Stream(void) : buffer(NULL), size(0) {}

Stream::Stream(string file) : buffer(NULL), size(0) {
    if (!file.empty())
        loadFile(file);
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
        cerr << "can't manage file " << file << endl;
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
    ofstream out(file.c_str());

    if (!out.is_open() || out.bad() || out.fail()) {
        cerr << "can't manage file " << file << endl;
        return;
    }
    out.write(reinterpret_cast<char*>(buffer), size);
    if (!out) {
        cerr << "could not write file\n";
        return ;
    }
    out.close();
}

Stream::~Stream(void) {
    if (buffer)
        delete[] reinterpret_cast<char *>(buffer);
}
