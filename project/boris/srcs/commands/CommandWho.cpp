#include "CommandWho.hpp"

CommandWho::CommandWho() : ACommand() {}
CommandWho::CommandWho(Kernel *kernel) : ACommand(kernel){}
CommandWho::~CommandWho() {}


std::string CommandWho::getChannelOfClient(Client *client, bool &isOp) const
{
	//сначала ищем канала, где пользователь оператор
	std::vector<Channel *> channels = _kernel->_channels->getChannelPointers();
	std::string result;
	std::vector<Client *>	clientsOfCurrChannel;
	std::vector<Client *>	operatorsOfCurrChannel;

	std::vector<Channel *>::iterator it = channels.begin();
	isOp = false;
	while (it != channels.end())
	{
		clientsOfCurrChannel = (*it)->getListOfClient();
		operatorsOfCurrChannel = (*it)->getListOfOper();
		if ((*it)->isContainList(clientsOfCurrChannel, client))
		{
			if ((*it)->isContainList(clientsOfCurrChannel, _client) ||
				(!(*it)->getSecretFlag() && !(*it)->getPrivateFlag()))
			{
				if ((*it)->isContainList(operatorsOfCurrChannel, client))
				{
					isOp = true;
					return (*it)->getName();
				}
				else
					result = result.empty() ? (*it)->getName() : result;
			}
		}
		it++;
	}
	return result.empty() ? "*" : result;
}

std::string CommandWho::exec()
{
	showMe();
	std::string				answer;
	std::string				clientInfo;
	bool 					isOperator;
	std::vector<Client *>	clients = _kernel->_clients->getListOfClient();

	std::vector<Client *>::iterator it = clients.begin();
	while (it != clients.end())
	{
		if ((*it)->isActive() && (_client == *it || !(*it)->isInvisible()) &&
			Kernel::isStringFitsMask(_args[1], (*it)->getNickName()))
		{
			clientInfo += getChannelOfClient(*it, isOperator) + " ";
			clientInfo += (*it)->getUserName() + " " + (*it)->getHostIp() + 
				" " + _kernel->set.serverName + " " + (*it)->getNickName() + " H" 
				+ (isOperator ? "@" : "") + " :0 " + (*it)->getRealName();
			answer += Response::getInstance().getResponse
					(RPL_WHOREPLY, "", _client->getNickName(), clientInfo);
		}
		clientInfo.clear();
		it++;
	}
	std::string endList = _client->getNickName() + " :End of /WHO list\"";
	answer += Response::getInstance().getResponse
			(RPL_ENDOFWHO, "", _client->getNickName(), endList);
	return answer;

}