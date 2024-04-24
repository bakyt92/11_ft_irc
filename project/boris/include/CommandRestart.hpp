#ifndef COMMANDRESTART_HPP
# define COMMANDRESTART_HPP
#include "Kernel.hpp"

class ACommand;

class CommandRestart : public ACommand {
private:
    CommandRestart();
	CommandRestart(CommandRestart const &source);
	CommandRestart &operator=(CommandRestart const &source);
    int notifyClients() const;
public:
    virtual ~CommandRestart();
    string exec();
	CommandRestart(Kernel *kernel);
};
#endif