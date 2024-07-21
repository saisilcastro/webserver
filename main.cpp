#include "Server.h"
#include "Stream.h"

int main(int c, char **v) {
    if (c == 1) {
        Server server;
        server.run();
    }
    else {
        Server server(v[1]);
    }
    return 0;
}

