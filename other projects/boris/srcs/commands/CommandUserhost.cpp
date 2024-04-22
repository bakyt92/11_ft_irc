#include "CommandUserhost.hpp"

CommandUserhost::CommandUserhost() : ACommand() {}
CommandUserhost::CommandUserhost(Kernel *kernel) : ACommand(kernel){}
CommandUserhost::~CommandUserhost() {}
CommandUserhost &CommandUserhost::operator=(CommandUserhost const &source) {
	if (this != &source) {}
	return *this;
}

string CommandUserhost::exec() {
	showMe();
	if (_args.size() < 2)
		return Response::getInstance().getResponse
			(ERR_NEEDMOREPARAMS, "", "", CommandProcessor::getCommandNames()[_type]);
	std::string msg; 
	for (unsigned long i = 1; i < 6 && i < _args.size(); i++) {
		Client * _cl = _kernel->_clients->getClientByName(_args[i]);
		if (_cl)
			msg += _args[i] + "=+@" + _cl->getHostIp() + " ";
		}
	if (msg != "") {
		std::string response = Response::getInstance().getResponse
			(302, _kernel->set.serverName, _client->getNickName(), ":" + msg);
		send(_kernel->_clientFD, response.c_str(), response.length(), 0);
	}
	return "";
}
