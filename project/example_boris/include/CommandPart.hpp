#ifndef COMMANDPART_HPP
# define COMMANDPART_HPP
#include "Kernel.hpp"

class ACommand;

class CommandPart : public ACommand {
private:
    CommandPart();
	CommandPart(CommandPart const &source);
	CommandPart &operator=(CommandPart const &source);
    int notifyClients() const;
public:
    virtual ~CommandPart();
    string exec();
	CommandPart(Kernel *kernel);
};
#endif