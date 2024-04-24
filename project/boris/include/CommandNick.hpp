#ifndef COMMANDNICK_HPP
# define COMMANDNICK_HPP
#include "Kernel.hpp"

class ACommand;
class ClientCollection;
class ChannelCollection;

class CommandNick : public ACommand {
private:
	CommandNick();
	CommandNick(CommandNick const &source);
	CommandNick &operator=(CommandNick const &source);
	bool checkNickName(const std::string& nick) const;

public:
	virtual ~CommandNick();
	std::string exec();

	CommandNick(Kernel *kernel);

	bool isNicknameFree();
};

#endif
