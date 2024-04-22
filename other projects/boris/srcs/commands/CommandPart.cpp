#include "CommandPart.hpp"

CommandPart::CommandPart() : ACommand() {}
CommandPart::CommandPart(Kernel *kernel) : ACommand(kernel){}
CommandPart::~CommandPart() {}
CommandPart &CommandPart::operator=(CommandPart const &source) {
	if (this != &source) {}
	return *this;
}

int CommandPart::notifyClients() const{
	std::vector<std::string> _chList = _kernel->splitString(_args[1], ',');
	for (unsigned long i = 0; i < _chList.size(); i++) {
		Channel *_chPoint = _kernel->_channels->getChannelByName(_chList[i]);
		if (!_chPoint) {
			std::string response = Response::getInstance().getResponse
				(ERR_NOSUCHCHANNEL, "", "", _chList[i]);
			send(_kernel->_clientFD, response.c_str(), response.length(), 0);
			continue ;
		}
		if (_chPoint->findClient(_client) == NULL) {
			std::string response = Response::getInstance().getResponse
				(ERR_NOTONCHANNEL, "", "", _chList[i]);
			send(_kernel->_clientFD, response.c_str(), response.length(), 0);
			continue ;
		}
		_kernel->sendToChanel(_chPoint, _client, "PART " + _chPoint->getName());
		_kernel->updateChannelOper(_chPoint, _client);
		_chPoint->deleteClient(_client);
		_kernel->_channels->deleteEmptyChannel(*_chPoint);
	}
    return 0;
}

string CommandPart::exec() {
	showMe();
	if (_args.size() < 2)
		return Response::getInstance().getResponse
			(ERR_NEEDMOREPARAMS, "", "", CommandProcessor::getCommandNames()[_type]);
	notifyClients();
	return "";
}
