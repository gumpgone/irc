
#include "../incl/Server.hpp"

void	Server::check_carriage(Client* current_client)
{
	size_t pos;
	std::string request;

	while (true)
	{
		pos = current_client->recv_buffer.find("\r\n", 0);
		if (pos == std::string::npos)
			break ;
		request = current_client->recv_buffer.substr(0, pos + 2);
		current_client->recv_buffer.erase(0, pos + 2);

		std::cout << "Ligne |" << request << "|" << std::endl;
		parse_request(request, current_client);
	}
}

void	Server::parse_request(std::string request, Client* current_client)
{
	std::istringstream iss(request);
    	std::string command;
    	std::vector<std::string> args;
    	std::string arg;

    // Extrait la commande (le premier mot)
    	iss >> command;

    // Extrait les arguments restants
    	while (iss >> arg) {
        	args.push_back(arg);
    	}

    // Convertit la commande en majuscules pour une comparaison non sensible à la casse
    for (size_t i = 0; i < command.length(); ++i) {
        command[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(command[i])));
    }
	std::map<std::string, CommandHandler>::iterator it = commands_map.find(command);
    if (it != commands_map.end()) {
        // Si la commande est trouvée, appelle la fonction correspondante
        // C'est ici que la syntaxe change pour appeler un pointeur sur une fonction membre
        (this->*(it->second))(current_client, args);
    } else {
        std::string err_msg = ":irc.local 421 " + command + " :Unknown command\r\n";
        current_client->send_buffer += err_msg;
        current_client->has_to_write = true;
    }
}


void Server::handle_nick(Client* client, const std::vector<std::string>& args)
{

	if (!client->is_authenticated) {
        client->send_buffer += ":irc.local 451 :You have not registered\r\n"; // ERR_NOTREGISTERED
        client->has_to_write = true;
        return;
    }
    // Logique pour gérer la commande NICK
    if (args.empty()) {
        // Gérer l'erreur 431 : ERR_NONICKNAMEGIVEN
        client->send_buffer += ":irc.local 431 :No nickname given\r\n";
        client->has_to_write = true;
        return;
    }

    std::string new_nick = args[0];
    // Tu devras ici vérifier si le nickname est déjà pris, s'il est valide, etc.
    client->nickname = new_nick;

    check_registration(client);
}

void Server::handle_user(Client* client, const std::vector<std::string>& args)
{
	 if (!client->is_authenticated) {
        client->send_buffer += ":irc.local 451 :You have not registered\r\n"; // ERR_NOTREGISTERED
        client->has_to_write = true;
        return;
    }
    // Logique pour gérer la commande USER
    if (args.size() < 4) {
        // Gérer l'erreur 461 : ERR_NEEDMOREPARAMS
        client->send_buffer += ":irc.local 461 USER :Not enough parameters\r\n";
        client->has_to_write = true;
        return;
    }

    // Le premier argument est le username
    client->username = args[0];
    client->is_registered = true; // On assume que si USER est appelé, on peut considérer le client comme enregistré

    // Envoie le message de bienvenue (RPL_WELCOME)
    client->send_buffer += ":irc.local 001 " + client->nickname + " :Welcome to the ft_irc network, " + client->nickname + "!\r\n";
    client->has_to_write = true;

    check_registration(client);
}

void Server::handle_ping(Client* client, const std::vector<std::string>& args)
{
    // Logique pour gérer la commande PING
    if (args.empty()) {
        // Gérer l'erreur 409 : ERR_NOORIGIN
        client->send_buffer += ":irc.local 409 :No origin specified\r\n";
        client->has_to_write = true;
        return;
    }

    std::string pong_response = "PONG :" + args[0] + "\r\n";
    client->send_buffer += pong_response;
    client->has_to_write = true;
}

void Server::handle_pass(Client* client, const std::vector<std::string>& args)
{
    // Vérifie s'il y a un argument (le mot de passe)
    if (args.empty()) {
        client->send_buffer += ":irc.local 461 PASS :Not enough parameters\r\n"; // ERR_NEEDMOREPARAMS
        client->has_to_write = true;
        return;
    }

    // Le mot de passe a déjà été donné
    if (client->is_authenticated) {
        client->send_buffer += ":irc.local 462 :You may not reregister\r\n"; // ERR_ALREADYREGISTRED
        client->has_to_write = true;
        return;
    }

    // Vérifie si le mot de passe est correct
    if (args[0] == this->password) {
        client->is_authenticated = true;
        std::cout << "Client (" << client->nickname << ") authenticated." << std::endl;
    } else {
        client->send_buffer += ":irc.local 464 :Password incorrect\r\n"; // ERR_PASSWDMISMATCH
        client->has_to_write = true;
    }
}


void Server::check_registration(Client* client) {
    if (client->is_authenticated && !client->nickname.empty() && !client->username.empty() && !client->is_registered) {
        client->is_registered = true;
        client->send_buffer += ":irc.local 001 " + client->nickname + " :Welcome to the ft_irc network, " + client->nickname + "!\r\n";
        client->has_to_write = true;
    }
}

void Server::handle_join(Client* client, const std::vector<std::string>& args)
{
    if (!client->is_registered) {
        client->send_buffer += ":irc.local 451 :You have not registered\r\n";
        client->has_to_write = true;
        return;
    }

    if (args.empty()) {
        client->send_buffer += ":irc.local 461 JOIN :Not enough parameters\r\n";
        client->has_to_write = true;
        return;
    }

    std::string channel_name = args[0];

    if (channel_name[0] != '#') {
        client->send_buffer += ":irc.local 403 " + channel_name + " :No such channel\r\n";
        client->has_to_write = true;
        return;
    }

    std::map<std::string, Channel*>::iterator it = channels_map.find(channel_name);
    Channel* channel;

    if (it == channels_map.end()) {
        channel = new Channel(channel_name);
        channels_map[channel_name] = channel;
        
        channel->addOperator(client);
        std::cout << "Channel " << channel_name << " created." << std::endl;
    } else {
        channel = it->second;
    }

    if (!channel->isMember(client)) {
        channel->addMember(client);
    }
    
    std::string join_msg = ":" + client->nickname + " JOIN " + channel_name + "\r\n";
    client->send_buffer += join_msg;

    std::string names_msg = ":irc.local 353 " + client->nickname + " = " + channel_name + " :";
	const std::vector<Client*>& members = channel->getMembers();
    for (std::vector<Client*>::const_iterator it_members = members.begin(); it_members != members.end(); ++it_members) {
        Client* member = *it_members;

        // Utilise la variable 'member' ici pour ajouter son nickname au message
        if (channel->isOperator(member)) {
            names_msg += "@"; // Ajouter le préfixe d'opérateur
        }
        names_msg += member->nickname;
        names_msg += " ";
    }
    // Supprimer l'espace final
    if (!members.empty()) {
        names_msg.resize(names_msg.size() - 1);
    }
    client->send_buffer += names_msg + "\r\n";

    client->send_buffer += ":irc.local 366 " + client->nickname + " " + channel_name + " :End of /NAMES list\r\n";
    client->has_to_write = true;    
}

void Server::handle_privmsg(Client* client, const std::vector<std::string>& args)
{
    if (!client->is_registered) {
        client->send_buffer += ":irc.local 451 :You have not registered\r\n";
        client->has_to_write = true;
        return;
    }

    if (args.size() < 2) {
        client->send_buffer += ":irc.local 461 PRIVMSG :Not enough parameters\r\n";
        client->has_to_write = true;
        return;
    }

    std::string recipient = args[0];
    std::string message = args[1];

    if (recipient[0] == '#') {
        std::map<std::string, Channel*>::iterator it = channels_map.find(recipient);
        if (it == channels_map.end()) {
            client->send_buffer += ":irc.local 403 " + recipient + " :No such channel\r\n";
            client->has_to_write = true;
            return;
        }
        
        Channel* channel = it->second;
        const std::vector<Client*>& members = channel->getMembers();
        for (std::vector<Client*>::const_iterator it_members = members.begin(); it_members != members.end(); ++it_members) {
            Client* member = *it_members;
            if (member->socket != client->socket) {
                member->send_buffer += ":" + client->nickname + " PRIVMSG " + recipient + " :" + message + "\r\n";
                member->has_to_write = true;
            }
        }
    } else {
        std::map<std::string, Client*>::iterator it = clients_str_map.find(recipient);
        if (it == clients_str_map.end()) {
            client->send_buffer += ":irc.local 401 " + recipient + " :No such nick/channel\r\n";
            client->has_to_write = true;
            return;
        }

        Client* recipient_client = it->second;
        recipient_client->send_buffer += ":" + client->nickname + " PRIVMSG " + recipient + " :" + message + "\r\n";
        recipient_client->has_to_write = true;
    }
}

