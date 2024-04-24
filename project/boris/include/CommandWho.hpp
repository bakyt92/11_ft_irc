#ifndef IRC_SCHOOL21_CommandWho_H
# define IRC_SCHOOL21_CommandWho_H
#include "Kernel.hpp"

class ACommand;
class ClientCollection;
class ChannelCollection;

class CommandWho : public ACommand {
private:
	CommandWho();
	CommandWho(CommandWho const &source);
	CommandWho &operator=(CommandWho const &source);
	std::string getChannelOfClient(Client *client, bool &isOp) const;

public:
	virtual ~CommandWho();
	std::string exec();
	CommandWho(Kernel *kernel);

};

#endif //IRC_SCHOOL21_CommandWho_H
