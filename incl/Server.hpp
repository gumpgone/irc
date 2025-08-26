
#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sstream>
# include <vector>
# include <map>
# include <sys/socket.h>
# include <netinet/in.h>
# include <poll.h>
# include <unistd.h>
# include <cstdlib>
# include <cstring>
# include <csignal>
# include "./Channel.hpp"
# include "./Client.hpp"

class Server
{
private:
	const short						port;
	const char						*password;
	int								fd;
	bool							is_running;
	struct sockaddr_in				addr;
	std::vector<pollfd>				poll_vector;
	std::map<std::string, Client*> 	clients_str_map;
	std::map<int, Client*> 			clients_fd_map;
	std::map<std::string, Channel*>	channels_map;
	
	//pointeur sur fonctions 	
	typedef void (Server::*CommandHandler)(Client*, const std::vector<std::string>&);
    	std::map<std::string, CommandHandler> commands_map;

	int				new_client(void);
	void			free_client(int client_fd);
	int				read_from_client(int client_fd);
	int				send_to_client(int client_fd);
	void			close_connection(int client_fd, size_t *i);
	void			check_carriage(Client* client);
	void			parse_request(std::string request, Client* current_client);
	
	//commandes
	void handle_nick(Client* client, const std::vector<std::string>& args);
    	void handle_user(Client* client, const std::vector<std::string>& args);
    	void handle_ping(Client* client, const std::vector<std::string>& args);
	void handle_pass(Client* client, const std::vector<std::string>& args);
	void handle_join(Client* client, const std::vector<std::string>& args);
	void handle_privmsg(Client* client, const std::vector<std::string>& args);

	void init_commands();
	//check si le user a bien fini de s'enregister 
	void check_registration(Client* client);

public:
	Server(int port, char *password);
	~Server(void);

	int		launch(void);
	int		run(void);
	void	kill(void);

};

# endif

