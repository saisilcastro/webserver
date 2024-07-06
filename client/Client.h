#ifndef CLIENT_H
#define CLIENT_H

class Client {
	public:
		Client(void);
		Client(int, bool);
		~Client(void);
	protected:
		int socket;
		bool connected;
};

#endif