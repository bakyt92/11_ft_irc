#ifndef COMMANDQUIT_HPP
# define COMMANDQUIT_HPP
#include "Kernel.hpp"

class ACommand;

class CommandQuit : public ACommand {
private:
    CommandQuit();
	CommandQuit(CommandQuit const &source);
	CommandQuit &operator=(CommandQuit const &source);
public:
    virtual ~CommandQuit();
    string exec();
	CommandQuit(Kernel *kernel);
};
#endif