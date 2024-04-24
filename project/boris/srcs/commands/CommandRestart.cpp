#include "CommandRestart.hpp"

CommandRestart::CommandRestart() : ACommand() {}
CommandRestart::CommandRestart(Kernel *kernel) : ACommand(kernel){}
CommandRestart::~CommandRestart() {}
CommandRestart &CommandRestart::operator=(CommandRestart const &source) {
	if (this != &source) {}
	return *this;
}

int CommandRestart::notifyClients() const {
	std::string txt = " :Server restarting, reconnect after 1 minute please\n";
	std::vector<Client*> _clList = _kernel->_clients->getListOfClient();
	for (unsigned long i = 1; i < _clList.size(); i++) {
		std::string msg = ":" + _kernel->set.serverName + " PRIVMSG " + _clList[i]->getNickName() + txt;
		send(_clList[i]->getFd(), msg.c_str(), msg.length(), 0);
	}
    return 0;
}

string CommandRestart::exec() {
	showMe();
	if (!this->_client->isIrcOperator())
		return Response::getInstance().getResponse
			(ERR_NOPRIVILEGES, "", "", CommandProcessor::getCommandNames()[_type]);
	notifyClients();
	_kernel->_server->setStatus(-1);
	return "";
}
