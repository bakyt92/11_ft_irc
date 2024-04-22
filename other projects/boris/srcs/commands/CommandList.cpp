#include "CommandList.hpp"

CommandList::CommandList() : ACommand() {}
CommandList::CommandList(Kernel *kernel) : ACommand(kernel){}
CommandList::~CommandList() {}


std::string CommandList::getFlags(Channel *currChannel) const
{
	unsigned int flags = currChannel->getFlags();
	std::string result;
	result += (flags & PRIVATE) ? "p": "";
	result += (flags & SECRET) ? "s": "";
	result += (flags & INVITEONLY) ? "i": "";
	result += (flags & TOPICBYOPER) ? "t": "";
	result += (flags & MODERATED) ? "m": "";
	result += (flags & NOMSGOUTSIDE) ? "n": "";
	return result;
}

static std::string	itoa(unsigned int n)
{
	std::string result;

	if (n < 10) return (result += n + 48);
	return (result += itoa(n / 10) + char(n % 10 + 48));
}

std::string CommandList::getListOfChannels() const
{
	std::string list;
	std::string arg;
	Channel *currChannel;
	std::vector<std::string> channels = (_args.size() == 1) ?
			_kernel->_channels->getListOfChannels() :
			_kernel->splitString(_args[2], ',');
	std::vector<string>::iterator it = channels.begin();
	list += Response::getInstance().getResponse
			(RPL_LISTSTART, "", _client->getNickName(), "");
	while (it != channels.end())
	{
		currChannel = _kernel->_channels->getChannelByName(*it);
		if (!currChannel->getSecretFlag() || \
			currChannel->isContainList(currChannel->getListOfClient(), _client))
		{
			if (!currChannel->getPrivateFlag() || \
			currChannel->isContainList(currChannel->getListOfClient(), _client))
			{
				arg += *it + " " + itoa(currChannel->getListOfClient().size());
				arg += " :";
				arg += "[+" + getFlags(currChannel) + "] " + currChannel->getTopic();
			}
			else
				arg += "Priv";
		}
		list += Response::getInstance().getResponse
				(RPL_LIST, "", _client->getNickName(), arg);
		arg.clear();
		it++;
	}
	list += Response::getInstance().getResponse
			(RPL_LISTEND, "", _client->getNickName(), "list end");
	std::cout << "Channel list:\n" << list << std::endl;
	return list;
}

std::string CommandList::exec()
{
	showMe();
	return getListOfChannels();

}