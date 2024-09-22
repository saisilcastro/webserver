#ifndef SERVER_H
#define SERVER_H

#include <Config.h>
#include <Protocol.h>
#include <vector>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>

class Stream;

using namespace std;
typedef struct sockaddr_in SockAddrIn;

class Server{
	string              host;
	string              port;
	string              root;
	map<string, string>	error;
	vector<Location>    location;
  size_t              maxBodySize;
  string              mime;
	Protocol            master;
	bool                transfer;
public:
	Server(void);
	Server(string host, string port, string root, map<string, string> error, vector<Location> location);
	int					        serverSocket(int type);
  string              locationValueByName(string);
  string              createPacket(int);
	void				        requestTreat(int, string);
	string			        mimeMaker(string);
	void				        contentMaker(int, string, string, void *, size_t);
	void				        response(int, string, string);
	void				        postPrepare(string);
	void				        setHost(string value);
	void				        setPort(string value);
	void				        setRoot(string value);
	void				        setError(map<string, string> value);
	void				        setLocation(vector<Location> value);
	string				      getHost(void);
	string				      getPort(void);
  string              getRoot(void);
	map<string, string>	getError(void);
	vector<Location>	  getLocation(void);

	void				        print(void);
	void				        run(void);
	void				        execute(int);
	~Server(void);
};

void    Run(Server *, int);

#endif
