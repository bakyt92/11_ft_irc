#include "CommandPing.hpp"

CommandPing::CommandPing() : ACommand() {}
CommandPing::CommandPing(Kernel *kernel) : ACommand(kernel){}
CommandPing::~CommandPing() {}

std::string CommandPing::exec() {
	showMe();
	if (_args.size() < 2)
		return Response::getInstance().getResponse
			(ERR_NOORIGIN, "", "", CommandProcessor::getCommandNames()[_type]);	
	
	std::string msg = " PONG :" + _kernel->set.serverName + " " + _client->getNickName();
	return Response::getInstance().getResponse
		(302, _kernel->set.serverName, _client->getNickName(), msg);
}
