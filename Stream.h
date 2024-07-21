#ifndef STREAM_H
#define STREAM_H

#include <iostream>
#include <fstream>

using namespace std;

class Stream {
    void    *buffer;
    size_t  size;
  public:
    Stream(void);
    Stream(string file);
    void  *getStream(void);
    int   streamSize(void);
    ~Stream(void);
};

#endif
