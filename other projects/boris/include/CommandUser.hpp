#ifndef COMMANDUSER_HPP
# define COMMANDUSER_HPP
#include "Kernel.hpp"

class ACommand;
class ClientCollection;
class ChannelCollection;

class CommandUser : public ACommand {
private:
    CommandUser();
	CommandUser(CommandUser const &source);
	CommandUser &operator=(CommandUser const &source);

public:
	virtual ~CommandUser();
	std::string exec();

	CommandUser(Kernel *kernel);
};
#endif