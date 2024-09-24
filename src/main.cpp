#include "Server.h"
#include "Stream.h"

void handleSignal(int signal) {
    if (signal == SIGINT) {
        std::cout << "SIGINT signal received. Quitting..." << std::endl;
        throw (string("Quit"));
    }
}

int main(int argc, char **argv) {
    std::signal(SIGINT, handleSignal);  // Ctrl+C Signal
    try
        {
        if (argc > 1) {
            Config config(argv[1]);
            size_t  max = config.infoGet().size();
            vector<ServerInfo> info = config.infoGet();
            Server server[max];
            max = 0;

            for (vector<ServerInfo>::iterator it = info.begin(); it != info.end(); ++it)
                server[max++] = Server(it->name, it->port, it->root, it->error, it->location, it->maxBodySize);

            for (size_t i = 0; i < max; i++)
                Run(server, max);
        }
        else {
            Server server;
        }
    }
    catch(string &e)
    {
        cout << e << endl;
        return(1);
    }
    return (0);
}
