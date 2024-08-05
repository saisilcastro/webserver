#include "Server.h"
#include "Stream.h"

void printLocations(const Server& config) {
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



int main(int argc, char **argv) {
    try {
        Server server;
        if (argc > 1) {
            parser(argv[1], server);
        }
        server.run();
    } catch (const runtime_error& e) {
        cerr << e.what() << endl;
        //Using default settings
        Server server;
        server.run();
    }
	return 0;
}

