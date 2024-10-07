#include "Server.h"
#include "Stream.h"

void handleSignal(int signal) {
    if (signal == SIGINT) {
        std::cout << "SIGINT signal received. Quitting..." << std::endl;
        exit(0);
    }
}

int main(int argc, char **argv) {
    std::signal(SIGINT, handleSignal);  // Ctrl+C Signal
    try{
        if (argc > 1) {
            Config config(argv[1]);
            if (!config.isPortRepeated()) {
                size_t  max = config.infoGet().size();
                vector<ServerInfo> info = config.infoGet();
                Server server[max];
                max = 0;

                for (vector<ServerInfo>::iterator it = info.begin(); it != info.end(); ++it)
                    server[max++] = Server(it->name, it->port, it->root, it->error, it->location, it->maxBodySize);

                Run(server, max);
            }
            else {
                cout << "Invalid .conf file\n";
                return -1;
            }
        }
        else {
            Server server;
            server.run();
        }
    }
    catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        Server server;
        server.run();
    }
    return (0);
}
