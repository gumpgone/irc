
#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <unistd.h>
# include <iostream>

class Client
{	
public:	
	std::string	password;
	std::string	username;
	std::string	nickname;
	std::string	ip;
	const	int	socket;
	int			has_to_write;
	int			is_registered;
	bool			is_authenticated;
	std::string	recv_buffer;
	std::string	send_buffer;
	
	Client(int socket, char *address);
	~Client(void);
};

#endif

