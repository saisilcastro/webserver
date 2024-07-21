#include "Stream.h"

Stream::Stream(void) : buffer(NULL), size(0) {}

Stream::Stream(string file) : buffer(NULL), size(0) {
  ifstream in(file.c_str(), std::ios::binary | std::ios::ate);

  if (!in.is_open() || in.bad() || in.fail()) {
    cerr << "can't manager file" << file << endl;
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

void *Stream::getStream(void) {
  return buffer;
}

int Stream::streamSize(void) {
  return size;
}

Stream::~Stream(void) {
  if (buffer)
    delete[] reinterpret_cast<char *>(buffer);
}
