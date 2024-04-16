#include "CommandUser.hpp"

CommandUser::CommandUser() : ACommand() {}
CommandUser::CommandUser(Kernel *kernel) : ACommand(kernel) {}
CommandUser::~CommandUser() {}

std::string CommandUser::exec() {
	showMe();
	std::string currName;
	if (_args.size() < 5)
		return Response::getInstance().getResponse
		(ERR_NEEDMOREPARAMS, "", _client->getNickName(), CommandProcessor::getCommandNames
		()[_type]);

	if (_client->getUserName().empty())
	{
		_client->setUserName(_args[1]);
		_client->setRealName(_args[4]);
		std::string ee = _client->getNickName() + " " + _client->getUserName();
		if (_client->connectClient())
			return _kernel->getMOTD(_client->getNickName());
		return "";
	}
	else
		return Response::getInstance().getResponse
			(ERR_ALREADYREGISTRED, "", _client->getNickName(), CommandProcessor::getCommandNames()
			[_type]);
}
