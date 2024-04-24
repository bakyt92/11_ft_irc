#ifndef COMMANDPING_HPP
# define COMMANDPING_HPP
#include "Kernel.hpp"

class ACommand;

class CommandPing : public ACommand {
private:
    CommandPing();
	CommandPing(CommandPing const &source);
	CommandPing &operator=(CommandPing const &source);

public:
    virtual ~CommandPing();
    std::string exec();
	CommandPing(Kernel *kernel);
};
#endif