#ifndef IRC_SCHOOL21_CommandNames_H
# define IRC_SCHOOL21_CommandNames_H
#include "Kernel.hpp"

class ACommand;
class ClientCollection;
class ChannelCollection;

class CommandNames : public ACommand {
private:
	CommandNames();
	CommandNames(CommandNames const &source);
	CommandNames &operator=(CommandNames const &source);
	std::string clientsOfChannelToString(Channel *channel, std::vector<Client *> &clients) const;
	std::string getUsersOutOfChannels(std::set<Client *> &addedClients) const;

public:
	virtual ~CommandNames();
	std::string exec();
	CommandNames(Kernel *kernel);
};

#endif //IRC_SCHOOL21_CommandNames_H
