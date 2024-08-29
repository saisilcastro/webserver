#include "Server.h"
#include "Stream.h"

/* void printLocations(const Server& config) {
    cout << "--------------------------------" << endl;
    cout << "Server settings:" << endl;
    cout << "Host: " << config.getHost() << endl;
    cout << "Port: " << config.getPort() << endl;
    cout << "Root: " << config.getRoot() << endl;
    cout << "Error Pages:" << endl;

    cout << "Locations:" << endl;

    vector<Location>::const_iterator start = config.getBegin();
    vector<Location>::const_iterator end = config.getEnd();

    while (start != end) {
        cout << "Location: " << start->path << endl;
        map<string, string>::const_iterator it = start->directives.begin();
        map<string, string>::const_iterator ite = start->directives.end();
        while (it != ite) {
            cout << "  " << it->first << ": " << it->second << endl;
            it++;
        }

        cout << "  Error Pages:" << endl;
        map<string, string>::const_iterator err_it = start->errorPages.begin();
        map<string, string>::const_iterator err_ite = start->errorPages.end();
        while (err_it != err_ite) {
            cout << "    Error: " << err_it->first << " Page: " << err_it->second << endl;
            err_it++;
        }

        start++;
    }
    cout << "--------------------------------" << endl;
}
 */


int main(int argc, char **argv) {
    if (argc > 1) {
        Config config(argv[1]);
        size_t  max = config.infoGet().size();
        vector<ServerInfo> info = config.infoGet();
        Server server[max];
        max = 0;
        for (vector<ServerInfo>::iterator it = info.begin(); it != info.end(); ++it)
            server[max++] = Server(it->name, it->port, it->root, it->error, it->location);
        for (size_t i = 0; i < max; i++)
            Run(server, max);
    }
    else {
        Server server;
    }
    return (0);
}
