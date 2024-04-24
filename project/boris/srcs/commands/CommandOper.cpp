#include "CommandOper.hpp"

CommandOper::CommandOper() : ACommand() {}
CommandOper::CommandOper(Kernel *kernel) : ACommand(kernel) {}
CommandOper::~CommandOper() {}

std::string CommandOper::exec() {
	showMe();
	std::string result;

	if (_args.size() < 3)
		return Response::getInstance().getResponse
		(ERR_NEEDMOREPARAMS, "", _client->getNickName(), CommandProcessor::getCommandNames()[_type]);
	
	std::map<std::string, std::string>::iterator iter;
	iter = _kernel->set.oper.find(_args[1]);
	if (iter == _kernel->set.oper.end())
		return Response::getInstance().getResponse
			(ERR_NOOPERHOST, "", _client->getNickName(), CommandProcessor::getCommandNames()[_type]);
	if (iter->second != _args[2])
		return Response::getInstance().getResponse
				(ERR_PASSWDMISMATCH, "", _client->getNickName(), CommandProcessor::getCommandNames()[_type]);
	_client->setFlag(IRC_OPERATOR);
	return Response::getInstance().getResponse
		(RPL_YOUREOPER, "", _client->getNickName(), CommandProcessor::getCommandNames()[_type]);
}
