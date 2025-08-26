
#include "../incl/Server.hpp"
#include <fcntl.h>
#include <unistd.h>

extern bool g_running;

Server::Server(int port, char *password) : port(port), password(password)
{
	fd = 0;
	init_commands();
}

Server::~Server(void)
{
	if (fd >= 3)
		close(fd);
	kill();
}

void Server::init_commands(){
	commands_map["NICK"] = &Server::handle_nick;
       	commands_map["USER"] = &Server::handle_user;
    	commands_map["PASS"] = &Server::handle_pass;
    	commands_map["PING"] = &Server::handle_ping;
    	commands_map["JOIN"] = &Server::handle_ping;
    	commands_map["PRIVMSG"] = &Server::handle_ping;
	//la suite des commandes a installer KICK, ...
}

int	Server::launch(void)
{
	int	opt = 1;
	pollfd	listen_poll;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 3)
		return(std::cerr << "Error while creating the socket" << std::endl, 1);
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		return(std::cerr << "Error while using fcntl()" << std::endl, 1);
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR /*| SO_REUSEPORT(!TEST!)*/, &opt, sizeof(opt)))
		return(std::cerr << "Error while setting socket option" << std::endl, 1);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return(std::cerr << "Error: bind failed" << std::endl, 1);
	if (listen(fd, 3) < 0)
		return(std::cerr << "Error while starting to listen" << std::endl, 1);
	
	listen_poll.fd = fd;
	listen_poll.events = POLLIN;
	listen_poll.revents = 0;
	poll_vector.push_back(listen_poll);
	return (0);
}

int	Server::run(void)
{
	int	activity;
	int	current_client;

	while (g_running == true)
	{
		activity = poll(poll_vector.data(), poll_vector.size(), -1);
		if (activity < 0 && g_running)
		{
			std::cerr << "Error: poll" << std::endl;
			return (1);
		}

		if(poll_vector[0].revents & POLLIN)
		{
			if (new_client())
				continue;
		}
		for (size_t i = 1; i < poll_vector.size(); ++i)
		{
			current_client = poll_vector[i].fd;
			if((poll_vector[i].revents & POLLOUT)
				&& (clients_fd_map[current_client]->has_to_write))
			{
				if (send_to_client(current_client))
					close_connection(current_client, &i);
			}
			if (poll_vector[i].revents & POLLIN)
			{
				if (read_from_client(current_client))
					close_connection(current_client, &i);
			}
		}
	}
	return (0);
}

void	Server::kill(void)
{
	for (size_t i = 1; i < poll_vector.size(); ++i)
		free_client(poll_vector[i].fd);
	std::cout << "Server closed!" << std::endl;
}

