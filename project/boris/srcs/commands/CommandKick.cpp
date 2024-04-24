#include "CommandKick.hpp"

CommandKick::CommandKick() : ACommand() {}
CommandKick::CommandKick(Kernel *kernel) : ACommand(kernel){}
CommandKick::~CommandKick() {}
CommandKick &CommandKick::operator=(CommandKick const &source) {
	if (this != &source) {}
	return *this;
}

int CommandKick::notifyClients() const {
	Client * _clKicking = _kernel->_clients->getClientBySocket(_kernel->_clientFD);
	std::vector<std::string> _chList = _kernel->splitString(_args[1], ',');
	std::vector<std::string> _clList = _kernel->splitString(_args[2], ',');
	for (unsigned long i = 0; i < _chList.size(); i++) {
		Channel *_chPoint = _kernel->_channels->getChannelByName(_chList[i]);
		if (!_chPoint) {
			std::string response = Response::getInstance().getResponse
				(ERR_NOSUCHCHANNEL, "", "", _chList[i]);
			send(_kernel->_clientFD, response.c_str(), response.length(), 0);
			continue ;
		}
		if (_chPoint->findClient(_clKicking) == NULL) {
			std::string response = Response::getInstance().getResponse
				(ERR_NOTONCHANNEL, "", "", _chList[i]);
			send(_kernel->_clientFD, response.c_str(), response.length(), 0);
			continue ;
		}
		if (!_chPoint->isChanopNick(_clKicking)) {
			std::string response = Response::getInstance().getResponse
				(ERR_CHANOPRIVSNEEDED, "", "", _chList[i]);
			send(_kernel->_clientFD, response.c_str(), response.length(), 0);
			continue ;
		}
		for (unsigned long j = 0; j < _clList.size(); j++) {
			Client *_clPoint = _kernel->_clients->getClientByName(_clList[j]);
			if (_chPoint->findClient(_clPoint) != NULL) {
				std::string msg = "KICK " + _chPoint->getName() + " ";
				if (_args.size() > 3)
					msg += _clList[j] + " :" + _args[3] + "\n";
				else
					msg += ":" + _clList[j] + "\n";
				_kernel->sendToChanel(_chPoint, _clKicking, msg);
				_chPoint->deleteClient(_clPoint);
				_kernel->_channels->deleteEmptyChannel(*_chPoint);
			}
		}
		i++;
	}
    return 0;
}

string CommandKick::exec() {
	showMe();
	if (_args.size() < 3)
		return Response::getInstance().getResponse
			(ERR_NEEDMOREPARAMS, "", "", CommandProcessor::getCommandNames()[_type]);
	notifyClients();
	return "";
}
