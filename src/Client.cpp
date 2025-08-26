
#include "../incl/Client.hpp"

Client::Client(int socket, char *address) : socket(socket)
{
	username = "";
	nickname = "";
	password = "";
	ip = address;
	has_to_write = 0;
	is_registered = 0;
	is_authenticated = 0;
}

Client::~Client(void)
{
	close(socket);
}

