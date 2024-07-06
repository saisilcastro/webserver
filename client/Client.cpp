#include "Client.h"

Client::Client(void) socket(-1) : connected(false) {}

Client::Client(int sock, bool _connected) : socket(sock), connect(_connect) {}

Client::~Client(void){}