#include "CommandPass.hpp"

CommandPass::CommandPass() : ACommand() {}
CommandPass::CommandPass(Kernel *kernel) : ACommand(kernel){}
CommandPass::~CommandPass() {}

std::string CommandPass::exec() {
	showMe();
	if (_args.size() < 2)
		return Response::getInstance().getResponse
			(ERR_NEEDMOREPARAMS, "", _client->getNickName(), CommandProcessor::getCommandNames()[_type]);
	if (_args[1] != _kernel->set.pass)
		return Response::getInstance().getResponse
			(ERR_PASSWDMISMATCH, "", _client->getNickName(), CommandProcessor::getCommandNames()[_type]);
	else if (_client && !_client->isPassEntered())
	{
		_client->setFlag(PASS_ENTERED);
		return "";
	}
	else
		return Response::getInstance().getResponse
			(ERR_ALREADYREGISTRED, "", _client->getNickName(), CommandProcessor::getCommandNames()[_type]);
}

