#include "CommandNick.hpp"

CommandNick::CommandNick() : ACommand() {}
CommandNick::CommandNick(Kernel *kernel) : ACommand(kernel) {}
CommandNick::~CommandNick() {}

//Каждый пользователь имеет никнейм длиной не больше девяти (9) символов.
// Доступные символы в нике: a-z, A-Z, 0-9 и спец. символы -, [, ], , ^, {, }
bool CommandNick::checkNickName(const std::string& nick) const
{
	if (nick.size() > 9)
		return false;
	std::string symbols("-[]^{}0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM");
	for (unsigned long i = 0; i < nick.size(); ++i)
	{
		if (symbols.find(nick[i]) == std::string::npos)
			return false;
	}
	return true;
}

std::string CommandNick::exec() {
	showMe();
	std::string result;

	if (_args.size() < 2)
		return Response::getInstance().getResponse
		(ERR_NONICKNAMEGIVEN, "", "", CommandProcessor::getCommandNames()[_type]);
	
	if (_client->getNickName() != _args[1] && \
		!isNicknameFree())
		return Response::getInstance().getResponse
				(ERR_NICKNAMEINUSE, "", _args[1], CommandProcessor::getCommandNames()[_type]);
	if (checkNickName(_args[1]))
	{
		_client->setNickName(_args[1]);
		std::cout << _client->getNickName() + "\n";
	}
	else
		return Response::getInstance().getResponse
				(ERR_ERRONEUSNICKNAME, "", _args[1], "");
	if (_client->connectClient())
		return _kernel->getMOTD(_client->getNickName());
	return "";
}

static void toLower(std::string& str)
{
	int	i;
	i = -1;
	while(str[++i])
		str[i] = static_cast<char> ((str[i] >= 'A' && str[i] <= 'Z') ?
		std::tolower(str[i]) : str[i]);
}

bool CommandNick::isNicknameFree()
{
	std::vector<Client *> clients = _kernel->_clients->getListOfClient();
	std::string newNick = _args[1];
	std::string oldNick;
	toLower(newNick);
	std::vector<Client *>::iterator it = clients.begin();
	while (it != clients.end())
	{
		oldNick = (*it)->getNickName();
		toLower(oldNick);
		if (oldNick == newNick)
			return false;
		it++;
	}
	return true;
}
