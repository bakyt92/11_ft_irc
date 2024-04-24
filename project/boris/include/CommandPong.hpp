#ifndef COMMANDPONG_HPP
# define COMMANDPONG_HPP
#include "Kernel.hpp"

class ACommand;

class CommandPong : public ACommand {
private:
    CommandPong();
	CommandPong(CommandPong const &source);
	CommandPong &operator=(CommandPong const &source);

public:
    virtual ~CommandPong();
    std::string exec();
	CommandPong(Kernel *kernel);
};
#endif