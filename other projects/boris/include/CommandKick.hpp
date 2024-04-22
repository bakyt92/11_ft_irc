#ifndef CommandKick_HPP
# define CommandKick_HPP
#include "Kernel.hpp"

class ACommand;

class CommandKick : public ACommand {
private:
    CommandKick();
	CommandKick(CommandKick const &source);
	CommandKick &operator=(CommandKick const &source);
    int notifyClients() const;
public:
    virtual ~CommandKick();
    string exec();
	CommandKick(Kernel *kernel);
};
#endif