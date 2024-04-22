#ifndef COMMANDUSERHOST_HPP
# define COMMANDUSERHOST_HPP
#include "Kernel.hpp"

class ACommand;

class CommandUserhost : public ACommand {
private:
    CommandUserhost();
	CommandUserhost(CommandUserhost const &source);
	CommandUserhost &operator=(CommandUserhost const &source);
public:
    virtual ~CommandUserhost();
    string exec();
	CommandUserhost(Kernel *kernel);
};
#endif