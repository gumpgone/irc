
#include "../incl/Server.hpp"

volatile bool	g_running;

int	is_number(char *str)
{
	int	i = -1;

	while (str[++i])
	{
		if (str[i] < '0' || str[i] > '9')
			return(0);
	}
	return(1);
}

int	verif_args(int argc, char **argv)
{
	int	port;

	if (argc != 3 || !argv[2][0])
	{
		std::cerr << "Error: Arguments." << std::endl
				  << "Usage: ./ircserv <Port> <Password>" << std::endl;
		return (-1);
	}
	port = atoi(argv[1]);
	if (port < 1024 || port > 65535 || !is_number(argv[1]))
	{
		std::cerr << "Error: Port should exist and not be privileged" << std::endl
				  << "Try a value between 1024 - 65535" << std::endl;
		return (-1);
	}
	return(port);
}

void	sig_handler(int sig)
{
	(void)sig;
	g_running = false;
	//std::cout << "\b\b"; pour retirer le ^C dans le terminal
}

int	main(int argc, char**argv)
{
	int	port = verif_args(argc, argv);
	if (port < 0)
		return (1);
	Server server(port, argv[2]);
	if (server.launch())
		return (1);
	g_running = true;
	signal(SIGINT, sig_handler);
	if(server.run())
		return(1);
	return (0);
}

