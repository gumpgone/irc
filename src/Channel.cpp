
#include "../incl/Channel.hpp"
#include "../incl/Client.hpp"

// Constructeur du canal
Channel::Channel(const std::string& name) : name(name), topic(""), user_limit(0), is_invite_only(false), is_topic_protected(false) {
}

// Destructeur voir si on alloue des ressources pour free ???
Channel::~Channel(void) {
}

// Getters
const std::string& Channel::getName() const {
    return name;
}

const std::string& Channel::getTopic() const {
    return topic;
}

const std::vector<Client*>& Channel::getMembers() const {
    return members;
}

const std::vector<Client*>& Channel::getOperators() const {
    return operators;
}

//checker
bool Channel::isInviteOnly() const {
    return is_invite_only;
}

bool Channel::isTopicProtected() const {
    return is_topic_protected;
}

bool Channel::isMember(Client* client) const {
    for (std::vector<Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
        if (*it == client) {
            return true;
        }
    }
    return false;
}

bool Channel::isOperator(Client* client) const {
    for (std::vector<Client*>::const_iterator it = operators.begin(); it != operators.end(); ++it) {
        if (*it == client) {
            return true;
        }
    }
    return false;
}


// Setters et méthodes de gestion
void Channel::setTopic(const std::string& new_topic) {
    topic = new_topic;
}

void Channel::addMember(Client* client) {
    if (!isMember(client)) {
        members.push_back(client);
    }
}

void Channel::removeMember(Client* client) {
    for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
        if (*it == client) {
            members.erase(it);
            break;
        }
    }
    // Gérer la suppression du client de la liste des opérateurs
    removeOperator(client);
}

void Channel::addOperator(Client* client) {
    if (isMember(client) && !isOperator(client)) {
        operators.push_back(client);
    }
}

void Channel::removeOperator(Client* client) {
    for (std::vector<Client*>::iterator it = operators.begin(); it != operators.end(); ++it) {
        if (*it == client) {
            operators.erase(it);
            break;
        }
    }
}
