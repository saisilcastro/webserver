#include "Server.h"
#include "Stream.h"

int main(int c, char **v) {
  Stream  data("Makefile");
  cout << reinterpret_cast<char *>(data.getStream());
  (void)c;
  (void)v;
	return 0;
}

void server(int c, char **v) {
  if (c == 1) {
  	Server server;
	  server.run();
  }
  else {
    Server server(v[1]);
  }

}
