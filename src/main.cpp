#include <Server.h>

int main(int argc, char **argv) {
    if (argc > 1) {
        Config config(argv[1]);
        size_t  max = config.infoGet().size();
        vector<ServerInfo> info = config.infoGet();
        Server server[max];
        max = 0;
        for (vector<ServerInfo>::iterator it = info.begin(); it != info.end(); ++it)
            server[max++] = Server(it->name, it->port, it->root, it->error, it->location);
        for (int i = 0; i < max; i++)
            Run(server, max);
    }
    else {
        Server server;
    }
    return (0);
}
