#ifndef IRC_SCHOOL21_CommandList_H
# define IRC_SCHOOL21_CommandList_H
#include "Kernel.hpp"

class ACommand;
class ClientCollection;
class ChannelCollection;

class CommandList : public ACommand {
private:
	CommandList();
	CommandList(CommandList const &source);
	CommandList &operator=(CommandList const &source);
	std::string getListOfChannels() const;
	std::string getFlags(Channel *curChannel) const;

public:
	virtual ~CommandList();
	std::string exec();
	CommandList(Kernel *kernel);
};

#endif //IRC_SCHOOL21_CommandList_H
