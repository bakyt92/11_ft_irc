#ifndef COMMANDNOTICE_HPP
# define COMMANDNOTICE_HPP
#include "Kernel.hpp"

class ACommand;

class CommandNotice : public ACommand {
private:
    CommandNotice();
	CommandNotice(CommandNotice const &source);
	CommandNotice &operator=(CommandNotice const &source);
    int getClientFD(std::string clName);
    int hasDuplicates(std::vector<std::string> receivers);
    int splitReceivers();
    int sendToHost(std::string serverName);
    int sendToChannel(std::string channelName);
    int sendToClientList(std::vector<Client*> clients, std::string sender, std::string msgEnd);
    int sendToClient(Client* receiver, std::string msgEnd);
    std::vector<Client*> _sentList;
public:
    virtual ~CommandNotice();
    std::string exec();
	CommandNotice(Kernel *kernel);
};
#endif