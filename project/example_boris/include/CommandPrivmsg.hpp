#ifndef COMMANDPRIVMSG_HPP
# define COMMANDPRIVMSG_HPP
#include "Kernel.hpp"

class ACommand;

class CommandPrivmsg : public ACommand {
private:
    CommandPrivmsg();
	CommandPrivmsg(CommandPrivmsg const &source);
	CommandPrivmsg &operator=(CommandPrivmsg const &source);
    int hasDuplicates(std::vector<std::string> receivers);
    int splitReceivers();
    int sendToHost(std::string serverName);
    int sendToChannel(std::string channelName);
    int sendToClientList(std::vector<Client*> clients, std::string sender, std::string msgEnd);
    int sendToClient(Client* receiver, std::string msgEnd);
    std::vector<Client*> _sentList;
public:
    virtual ~CommandPrivmsg();
    std::string exec();
	CommandPrivmsg(Kernel *kernel);
};
#endif