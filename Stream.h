#ifndef STREAM_H
#define STREAM_H

#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

class Stream {
    void	*buffer;
    size_t	size;
public:
    Stream(void);
    Stream(string file);
    void    createStream(void *, size_t);
    void    setStream(void *, size_t);
    void	*getStream(void);
    int		streamSize(void);
    void    loadFile(string file);
    void	saveFile(string file);
    Stream & operator = (Stream &);
    ~Stream(void);
};

#endif
