#ifndef IRC_SCHOOL21_CommandTopic_H
# define IRC_SCHOOL21_CommandTopic_H
#include "Kernel.hpp"

class ACommand;
class ClientCollection;
class ChannelCollection;

class CommandTopic : public ACommand {
private:
	CommandTopic();
	CommandTopic(CommandTopic const &source);
	CommandTopic &operator=(CommandTopic const &source);


public:
	virtual ~CommandTopic();
	std::string exec();
	CommandTopic(Kernel *kernel);
};

#endif //IRC_SCHOOL21_CommandTopic_H
