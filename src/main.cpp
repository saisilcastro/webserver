#include "Server.h"
#include "Stream.h"

void handleSignal(int signal) {
    if (signal == SIGINT) {
        cout << "SIGINT signal received. Quitting..." << endl;
        exit(0);
    }
}
void webserver(const char *file)
{
    Config config(file);
    size_t  max = config.infoGet().size();
    vector<ServerInfo> info = config.infoGet();
    Server server[max];
    max = 0;

    for (vector<ServerInfo>::iterator it = info.begin(); it != info.end(); ++it)
        server[max++] = Server(it->name, it->port, it->root, it->error, it->location, it->maxBodySize);

    for (size_t i = 0; i < max; i++)
        Run(server, max);
}

int main(int argc, char **argv) {
    signal(SIGINT, handleSignal);  // Ctrl+C Signal
    if(argc != 1 && argc != 2)
    {
        cout << "Usage: ./webserv [config_file]" << endl;
        return (1);
    }

    try{
        if(argc == 2)
            webserver(argv[1]);
        else
            webserver("Configs/default.conf");
    }
    catch(const exception &e) {
        cerr << e.what() << endl;
        webserver("Configs/default.conf");
    }
    return (0);
}