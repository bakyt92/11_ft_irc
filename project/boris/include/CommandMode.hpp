#ifndef IRC_SCHOOL21_COMMANDMODE_H
# define IRC_SCHOOL21_COMMANDMODE_H
#include "Kernel.hpp"

class ACommand;
class ClientCollection;
class ChannelCollection;

class CommandMode : public ACommand {
private:
	CommandMode();
	CommandMode(CommandMode const &source);
	CommandMode &operator=(CommandMode const &source);
	std::string execModeChannel(Channel *targetChannel) const;
	int handleFlag(char flag, bool isAdding, Channel *targetChannel) const;
	int setLimit(bool isAdding, Channel *targetChannel) const;
	std::string execModeUser(Client *targetClient) const;
	std::string showModeOfChannel(Channel *targetChannel) const;
	std::string showModeOfUser(Client *targetClient) const;

	static std::string getChannelFlags(Channel *currChannel);
	static std::string getUserFlags(Client *currClient);
	int	showBanMasks(Channel *channel) const;

public:
	virtual ~CommandMode();
	std::string exec();
	CommandMode(Kernel *kernel);
};



#endif //IRC_SCHOOL21_COMMANDMODE_H
