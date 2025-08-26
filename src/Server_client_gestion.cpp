
#include "../incl/Server.hpp"
#include <fcntl.h>
#include <arpa/inet.h>

int	Server::new_client(void)
{
	int					new_socket;
	pollfd				client_poll;
	struct sockaddr_in	client_addr;
	int					addr_len = sizeof(client_addr);
	
	new_socket = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);
	if (new_socket < 0)
		return (1);
	if (fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0)
	{
		close(new_socket);
		return(1);
	}
	std::cout << "Nouveau client connecté (" << new_socket << ")." << std::endl;

	Client*	new_client = new Client(new_socket, inet_ntoa(client_addr.sin_addr));
	clients_fd_map[new_socket] = new_client;

	client_poll.events = POLLIN | POLLOUT;
	client_poll.revents = 0;
	client_poll.fd = new_socket;
	poll_vector.push_back(client_poll);
	return(0);
}

void	Server::free_client(int fd_client)
{
	std::string			nick;

	nick = clients_fd_map[fd_client]->nickname;
	delete clients_fd_map[fd_client];
	clients_fd_map.erase(fd_client);
	if(nick.empty() == 0)
		clients_str_map.erase(nick);
}

int	Server::read_from_client(int client_fd)
{
	char 	buffer[1024];
	ssize_t	byte_received;

	byte_received = recv(client_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
	// std::cout << "buffer : " << buffer << std::endl;
	if (byte_received > 0)
	{
		clients_fd_map[client_fd]->recv_buffer.append(buffer, byte_received);
		check_carriage(clients_fd_map[client_fd]);
	}
	else if (byte_received <= 0)
	{
		std::cerr << "Error: recv() failed with socket (" << client_fd << "). Closing connection..." << std::endl;
		return (1);
	}
	return(0);
}

int	Server::send_to_client(int client_fd)
{
	ssize_t			byte_sended;
	std::string&	to_send =  clients_fd_map[client_fd]->send_buffer;
	ssize_t			len_string = to_send.length();

	if (len_string == 0)
        return 0;
	std::cout << ">> Envoi: " << to_send << " (len=" << len_string << ")" << std::endl; 

	byte_sended = send(client_fd, to_send.c_str(), len_string, MSG_DONTWAIT | MSG_NOSIGNAL);
	if (byte_sended == len_string)
		clients_fd_map[client_fd]->has_to_write = 0;
	if (byte_sended >= 0)
	{
		size_t	len_new_str = len_string - byte_sended;
		to_send = to_send.substr(byte_sended, len_new_str);
	}
	else
	{
		std::cerr << "Error: send() failed with socket (" << client_fd
				  << "). Closing the connection..." << std::endl;
		return (1);
	}
	return(0);
}

void	Server::close_connection(int client_fd, size_t *i)
{
	std::string	nick_name;

	nick_name = clients_fd_map[client_fd]->nickname;
	std::cout << "Client (";
	if (nick_name.empty() == 0)
		std::cout << nick_name;
	else
		std::cout << client_fd;
	std::cout << ") has disconnected" <<std::endl;
	free_client(client_fd);
	poll_vector.erase(poll_vector.begin() + (*i));
	--(*i);
}
