#include "CommandInvite.hpp"

CommandInvite::CommandInvite() : ACommand() {}
CommandInvite::CommandInvite(Kernel *kernel) : ACommand(kernel){}
CommandInvite::~CommandInvite() {}

int CommandInvite::execInvite() const
{
	Channel *targetChannel = _kernel->_channels->getChannelByName(_args[2]);
	Client *targetClient = _kernel->_clients->getClientByName(_args[1]);
	if (!targetChannel)
		return ERR_NOSUCHCHANNEL;
	if (!targetClient)
		return ERR_NOSUCHNICK;
	if (targetChannel->isContainList(targetChannel->getListOfClient(), targetClient))
		return ERR_USERONCHANNEL;
	if (!targetChannel->getInviteOnlyFlag() || \
			targetChannel->isChanopNick(_client))
		targetClient->addInvitation(_args[2]);
	else
		return ERR_CHANOPRIVSNEEDED;
	return RPL_INVITING;
}

std::string CommandInvite::exec(){
	showMe();
	int result;

	if (_args.size() < 3 || \
		std::string("&#").find(_args[2][0]) == std::string::npos)
		result = ERR_NEEDMOREPARAMS;
	else
		result = execInvite();
	return
		Response::getInstance().
			getResponse(result, "", _client->getNickName(), _args[0]);
}