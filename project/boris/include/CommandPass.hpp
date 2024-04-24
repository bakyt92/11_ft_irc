#ifndef COMMANDPASS_HPP
# define COMMANDPASS_HPP
#include "Kernel.hpp"

class ACommand;
class ClientCollection;
class ChannelCollection;

class CommandPass : public ACommand {
private:
    CommandPass();
	CommandPass(CommandPass const &source);
	CommandPass &operator=(CommandPass const &source);

public:
    virtual ~CommandPass();
    std::string exec();

	CommandPass(Kernel *kernel);
};
#endif