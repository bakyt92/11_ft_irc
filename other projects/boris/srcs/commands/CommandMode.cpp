#include "CommandMode.hpp"

CommandMode::CommandMode() : ACommand() {}
CommandMode::CommandMode(Kernel *kernel) : ACommand(kernel){}
CommandMode::~CommandMode() {}

std::string CommandMode::execModeUser(Client *targetClient) const
{
	int result = 0;
	std::string message;
	std::string arg;
	unsigned long i = 1;
	bool isAdding = (_args[2][0] == '+');

	if (std::string("-+").find(_args[2][0]) == std::string::npos)
	{
		return Response::getInstance().
			getResponse(ERR_UMODEUNKNOWNFLAG, "", _client->getNickName(), _args[0]);
	}
	if (!targetClient)
	{
		return Response::getInstance().
			getResponse(ERR_UMODEUNKNOWNFLAG, "", _client->getNickName(), _args[0]);
	}
	while (i < _args[2].size())
	{
		switch (_args[2][i])
		{
			case 'i':
				if (_client == targetClient)
					isAdding ? targetClient->setFlag(INVISIBLE) :
					targetClient->deleteFlag(INVISIBLE);
				else
					result = ERR_USERSDONTMATCH;
				break;
			case 's':
				if (_client == targetClient)
					isAdding ? targetClient->setFlag(SERVER_NOTICE) :
					targetClient->deleteFlag(SERVER_NOTICE);
				else
					result = ERR_USERSDONTMATCH;
				break;
			case 'w':
				if (_client == targetClient)
					isAdding ? targetClient->setFlag(WALLOPS) :
					targetClient->deleteFlag(WALLOPS);
				else
					result = ERR_USERSDONTMATCH;
				break;
			default:
				result = ERR_UNKNOWNMODE;
		}
		if (result)
		{
			arg = ((result != ERR_UNKNOWNMODE) ? _args[1] : _args[0] + " " +
				(isAdding ? "+" : "-") + _args[2][i]);
			message = Response::getInstance().getResponse
					(result, "", _client->getNickName(), arg);
			send(_client->getFd(), message.c_str(), message.size(),
				 IRC_NOSIGNAL);
		}
		i++;
	}
	return "";
}

int CommandMode::handleFlag(char flag, bool isAdding, Channel *targetChannel) const
{
	int result;
	
	switch (flag)
	{
		case 'p':
			result = isAdding ?
				targetChannel->setFlag(_client, PRIVATE) :
				targetChannel->deleteFlag(_client, PRIVATE);
			break;
		case 's':
			result = isAdding ?
				targetChannel->setFlag(_client, SECRET) :
				targetChannel->deleteFlag(_client, SECRET);
			break;
		case 'i':
			result = isAdding ?
				targetChannel->setFlag(_client, INVITEONLY) :
				targetChannel->deleteFlag(_client, INVITEONLY);
			break;
		case 't':
			result = isAdding ?
				targetChannel->setFlag(_client, TOPICBYOPER) :
				targetChannel->deleteFlag(_client, TOPICBYOPER);
			break;
		case 'n':
			result = isAdding ?
				targetChannel->setFlag(_client, NOMSGOUTSIDE) :
				targetChannel->deleteFlag(_client, NOMSGOUTSIDE);
			break;
		case 'm':
			result = isAdding ?
				targetChannel->setFlag(_client, MODERATED) :
				targetChannel->deleteFlag(_client, MODERATED);
			break;
		default:
			result = 0;
	}
	return result;
}

int CommandMode::setLimit(bool isAdding, Channel *targetChannel) const
{
	int		result;
	int		limit;
	const char *c_str = _args[3].c_str();
	char *ptr;
	
	if (isAdding)
	{
		limit = static_cast<int> (strtol(_args[3].c_str(), &ptr, 10));
		if (c_str == ptr)
			return (ERR_NEEDMOREPARAMS);
		result = targetChannel->setLimit(_client, limit);
	}
	else
		result = targetChannel->setLimit(_client,
			std::numeric_limits<unsigned int>::max());
	return result;
	
}

std::string CommandMode::getChannelFlags(Channel *currChannel)
{
	unsigned int flags = currChannel->getFlags();
	std::string result;
	result += (flags & PRIVATE) ? "p" : "";
	result += (flags & SECRET) ? "s" : "";
	result += (flags & INVITEONLY) ? "i" : "";
	result += (flags & TOPICBYOPER) ? "t" : "";
	result += (flags & MODERATED) ? "m" : "";
	result += (flags & NOMSGOUTSIDE) ? "n" : "";
	return result;
}

std::string CommandMode::getUserFlags(Client *currClient)
{
	std::string result;
	result += (currClient->isInvisible()) ? "i" : "";
	result += (currClient->isServerNotice()) ? "s" : "";
	result += (currClient->isWallOps()) ? "w" : "";
	return result;
}

std::string CommandMode::showModeOfChannel(Channel *targetChannel) const
{
	std::string result;
	result = getChannelFlags(targetChannel);
	result = targetChannel->getName() + ((result.empty()) ? "" :
		" +" + result);
	return Response::getInstance().getResponse
		(RPL_CHANNELMODEIS, "", _client->getNickName(), result);;
}

std::string CommandMode::showModeOfUser(Client *targetClient) const
{
	std::string result;
	result = getUserFlags(targetClient);
	result = targetClient->getNickName() + ((result.empty()) ? "" :
		" +" + getUserFlags(targetClient));
	return Response::getInstance().
			getResponse(RPL_UMODEIS, "", _client->getNickName(), result);;
}

int CommandMode::showBanMasks(Channel *channel) const
{
	std::string msg;
	std::vector<std::string> banMasks = channel->getListOfBanMask();
	std::vector<std::string>::iterator it = banMasks.begin();
	while (it != banMasks.end())
	{
		msg = channel->getName() + " " + *it;
		msg = Response::getInstance().
				getResponse(RPL_BANLIST, "", _client->getNickName(), msg);
		std::cout << msg;
		send(_client->getFd(), msg.c_str(), msg.size(), IRC_NOSIGNAL);
		it++;
	}
	msg = channel->getName() + " :End of channel ban list";
	msg = Response::getInstance().
			getResponse(RPL_ENDOFBANLIST, "", _client->getNickName(), msg);
	std::cout << msg;
	send(_client->getFd(), msg.c_str(), msg.size(), IRC_NOSIGNAL);
	return (0);
}

std::string CommandMode::execModeChannel(Channel *targetChannel) const
{
	int result;
	std::string message;
	std::string arg;
	Client	*targetClient;
	unsigned long i = 1;
	bool isAdding = (_args[2][0] == '+');

	if (std::string("-+").find(_args[2][0]) == std::string::npos)
		return Response::getInstance().
			getResponse(ERR_UMODEUNKNOWNFLAG, "", _client->getNickName(), _args[2].substr(0, 1));
	while (i < _args[2].size())
	{
		switch (_args[2][i])
		{
			case 'o':
				if (_args.size() < 4)
					result = ERR_NEEDMOREPARAMS;
				else if ((targetClient = _kernel->_clients->getClientByName(_args[3])))
					result = isAdding ?
						targetChannel->giveChanopPriv(_client, targetClient) :
						targetChannel->takeChanopPriv(_client, targetClient);
				else
					result = ERR_NOSUCHNICK;
				break;
			case 'p':
			case 's':
			case 'i':
			case 't':
			case 'n':
			case 'm':
				result = handleFlag(_args[2][i], isAdding, targetChannel);
				break;
			case 'l':
				result = setLimit(isAdding, targetChannel);
				break;
			case 'b':
				if (isAdding)
					result = (_args.size() > 3) ? targetChannel->setBanMask
						(_client, _args[3]) : showBanMasks(targetChannel);
				else
					result = (_args.size() > 3) ? targetChannel->removeBanMask
							(_client, _args[3]) : showBanMasks(targetChannel);
				break;
			case 'v':
				if (_args.size() < 4)
					result = ERR_NEEDMOREPARAMS;
				else if ((targetClient = _kernel->_clients->getClientByName(_args[3])))
					result = isAdding ?
							 targetChannel->giveSpeakPriv(_client, targetClient) :
							 targetChannel->takeSpeakPriv(_client, targetClient);
				else
					result = ERR_NOSUCHNICK;
				break;
			case 'k':
				if (_args.size() < 4)
					result = ERR_NEEDMOREPARAMS;
				else if (targetChannel->getKeyFlag())
					result = ERR_KEYSET;
				else
					result = targetChannel->setKey(_client, _args[3]);
				break;
			default:
				result = ERR_UNKNOWNMODE;
		}
		switch (result)
		{
			case ERR_NOSUCHNICK:
				message = Response::getInstance().getResponse
						(result, "", _client->getNickName(), targetChannel->getName());
				break;
			case ERR_UNKNOWNMODE:
				arg = _args[0] + " " + (isAdding ? "+" : "-") + _args[2][i];
				message = Response::getInstance().getResponse
						(result, "", _client->getNickName(), arg);
				break;
			case ERR_NEEDMOREPARAMS:
			case ERR_CHANOPRIVSNEEDED:
			case ERR_KEYSET:
				message = Response::getInstance().getResponse
						(result, "", _client->getNickName(), _args[1]);
				break;
			default:
				if (std::string("psitnm").find(_args[2][i]) != std::string::npos)
				{
					message = targetChannel->getName() + (isAdding ? " +" : " -") + _args[2][i];
					targetChannel->sendMessageToChannel(RPL_CHANNELMODEIS, message);
				}
				break;
		}
		if (result)
		{
			std::cout << message;
			send(_client->getFd(), message.c_str(), message.size(),
				 IRC_NOSIGNAL);
		}
		i++;
	}
	return "";
}

std::string CommandMode::exec(){
	showMe();
	if (_args.size() < 2)
		return Response::getInstance().getResponse
				(ERR_NEEDMOREPARAMS, "", _client->getNickName(), CommandProcessor::getCommandNames()[_type]);
	if (std::string("&#").find(_args[1][0]) == std::string::npos)
	{
		Client *targetClient = _kernel->_clients->getClientByName(_args[1]);
		if (!targetClient)
			return Response::getInstance().
					getResponse(ERR_NOSUCHNICK, "", _client->getNickName(), _args[1]);
		return (_args.size() == 2) ? showModeOfUser(targetClient) :
			execModeUser(targetClient);
	}
	else
	{
		Channel	*targetChannel = _kernel->_channels->getChannelByName(_args[1]);
		if (!targetChannel)
			return Response::getInstance().
					getResponse(ERR_NOSUCHCHANNEL, "", _client->getNickName(), _args[1]);
		if (!targetChannel->findClient(_client))
			return Response::getInstance().
					getResponse(ERR_NOTONCHANNEL, "", _client->getNickName(), _args[1]);
		return (_args.size() == 2) ? showModeOfChannel(targetChannel)
								   : execModeChannel(targetChannel);
	}
}