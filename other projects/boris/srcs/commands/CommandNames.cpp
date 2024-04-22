#include "CommandNames.hpp"

CommandNames::CommandNames() : ACommand() {}
CommandNames::CommandNames(Kernel *kernel) : ACommand(kernel){}
CommandNames::~CommandNames() {}

std::string CommandNames::getUsersOutOfChannels(std::set<Client *> &addedClients) const
{
	std::string result = "* * :";
	std::vector<Client *> allClients = _kernel->_clients->getListOfClient();
	std::vector<Client *>::iterator it = allClients.begin();

	while (it != allClients.end())
	{
		if (addedClients.find(*it) == addedClients.end())
		{
			if (!(*it)->isInvisible() || *it == _client)
				result += (*it)->getNickName() + " ";
		}
		it++;
	}
	return (result.size() != 5) ? result.substr(0, result.size() - 1) : "";
}
std::string CommandNames::clientsOfChannelToString(Channel *channel, std::vector<Client *> &clients) const
{
	std::string result = "= " + channel->getName() + " :";
	std::vector<Client *>::iterator it = clients.begin();
	while (it != clients.end())
	{
		if (!(*it)->isInvisible() || *it == _client)
		{
			result += (channel->isChanopNick(*it)) ? "@" : "";
			result += (*it)->getNickName() + " ";
		}
		it++;
	}
	return result.substr(0, result.size() - 1);
}

std::string CommandNames::exec()
{
	showMe();
	std::string				names;
	std::string				namesOfChannel;
	Channel					*currChannel;
	std::vector<Client *>	clientsOfCurrChannel;
	std::set<Client *>		addedClients;

	std::vector<std::string> channels = (_args.size() == 1) ?
		_kernel->_channels->getListOfChannels() :
		Kernel::splitString(_args[1], ',');
	std::vector<string>::iterator it = channels.begin();
	while (it != channels.end())
	{
		currChannel = _kernel->_channels->getChannelByName(*it);
		if (currChannel)
		{
			clientsOfCurrChannel = currChannel->getListOfClient();
			addedClients.insert(clientsOfCurrChannel.begin(), clientsOfCurrChannel.end());
			if (!currChannel->getSecretFlag() || \
            	currChannel->isContainList(clientsOfCurrChannel, _client))
			{
				if (!currChannel->getPrivateFlag() || \
            		currChannel->isContainList(clientsOfCurrChannel, _client))
				{
					namesOfChannel += clientsOfChannelToString(currChannel,
						clientsOfCurrChannel);
				}
			}
			names += (!namesOfChannel.empty()) ? Response::getInstance().getResponse
					(RPL_NAMREPLY, "", _client->getNickName(), namesOfChannel) : "";
		}
		namesOfChannel.clear();
		it++;
	}
	names += (_args.size() != 2) ? Response::getInstance().getResponse
			(RPL_NAMREPLY, "", _client->getNickName(), getUsersOutOfChannels(addedClients)) : "";
	std::string endList = ((channels.size() == 1) ? *channels.begin() : "*") + " :End of /NAMES list\"";
	names += Response::getInstance().getResponse
			(RPL_ENDOFNAMES, "", _client->getNickName(), endList);
	return names;

}