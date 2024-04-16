#ifndef IRC_SCHOOL21_CommandInvite_H
# define IRC_SCHOOL21_CommandInvite_H
#include "Kernel.hpp"

class ACommand;
class ClientCollection;
class ChannelCollection;

class CommandInvite : public ACommand {
private:
	CommandInvite();
	CommandInvite(CommandInvite const &source);
	CommandInvite &operator=(CommandInvite const &source);
	int execInvite() const;

public:
	virtual ~CommandInvite();
	std::string exec();
	CommandInvite(Kernel *kernel);
};

#endif //IRC_SCHOOL21_CommandInvite_H
