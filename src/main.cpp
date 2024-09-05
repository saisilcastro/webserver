#include "Server.h"
#include "Stream.h"

int main(int argc, char **argv) {
    if (argc > 1) {
        Config config(argv[1]);
        size_t  max = config.infoGet().size();
        vector<ServerInfo> info = config.infoGet();
        Server server[max];
        max = 0;
		vector<ServerInfo>::iterator it = info.begin();
		vector<ServerInfo>::iterator ite = info.end();

		if(it == ite)
			cout << "Igual" << endl;
		else
			cout << "Diferente" << endl;

        for (vector<ServerInfo>::iterator it = info.begin(); it != info.end(); ++it)
			server[max++] = Server(it->name, it->port, it->root, it->error, it->location, it->maxBodySize);

		config.print();
		for (size_t i = 0; i < max; i++)
            Run(server, max);
    }
    else {
        Server server;
    }
    return (0);
}
