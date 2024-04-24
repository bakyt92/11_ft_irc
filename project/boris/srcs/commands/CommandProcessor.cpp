#include "CommandProcessor.hpp"

const char*	CommandProcessor::_command_names[] = {
		"PASS", "NICK", "USER", "OPER", "QUIT", "JOIN", "PART", "MODE",
		"TOPIC", "NAMES", "LIST", "INVITE", "KICK", "PRIVMSG", "NOTICE",
		"WHO", "KILL", "PING", "USERHOST", "PONG", "RESTART"};


void CommandProcessor::initCommands() {
	this->_commandMap.insert(std::make_pair("PASS",	new CommandPass(_kernel)));
	this->_commandMap.insert(std::make_pair("NICK",	new CommandNick(_kernel)));
	this->_commandMap.insert(std::make_pair("USER",	new CommandUser(_kernel)));
	this->_commandMap.insert(std::make_pair("OPER",	new CommandOper(_kernel)));
	this->_commandMap.insert(std::make_pair("QUIT",	new CommandQuit(_kernel)));
	this->_commandMap.insert(std::make_pair("JOIN",	new CommandJoin(_kernel)));
	this->_commandMap.insert(std::make_pair("PART",	new CommandPart(_kernel)));
	this->_commandMap.insert(std::make_pair("MODE",	new CommandMode(_kernel)));
	this->_commandMap.insert(std::make_pair("TOPIC",	new CommandTopic(_kernel)));
	this->_commandMap.insert(std::make_pair("NAMES",	new CommandNames(_kernel)));
	this->_commandMap.insert(std::make_pair("LIST",	new CommandList(_kernel)));
	this->_commandMap.insert(std::make_pair("INVITE",	new CommandInvite(_kernel)));
	this->_commandMap.insert(std::make_pair("KICK",	new CommandKick(_kernel)));
	this->_commandMap.insert(std::make_pair("PRIVMSG",	new CommandPrivmsg(_kernel)));
	this->_commandMap.insert(std::make_pair("NOTICE",	new CommandNotice(_kernel)));
	this->_commandMap.insert(std::make_pair("WHO",		new CommandWho(_kernel)));
	this->_commandMap.insert(std::make_pair("PING",	new CommandPing(_kernel)));
	this->_commandMap.insert(std::make_pair("PONG",	new CommandPong(_kernel)));
	this->_commandMap.insert(std::make_pair("USERHOST",	new CommandUserhost(_kernel)));
	this->_commandMap.insert(std::make_pair("RESTART",	new CommandRestart(_kernel)));
}

CommandProcessor::CommandProcessor() {}

CommandProcessor::CommandProcessor(Kernel *kernel) : _kernel(kernel){
	initCommands();
}

CommandProcessor::~CommandProcessor()
{
	std::map<std::string, ACommand*>::iterator it = _commandMap.begin();
	while (it != _commandMap.end())
	{
		delete it->second;
		it++;
	}
}

CommandProcessor::CommandProcessor(CommandProcessor const & source) {
    _kernel = source._kernel;
}

CommandProcessor & CommandProcessor::operator=(CommandProcessor const & source){
	if (this == &source)
		return (*this);
	_kernel = source._kernel;
	return (*this);
}

int CommandProcessor::getType(std::string const & message)
{
	unsigned long start = 0, length = 0;
	int i = 0;
	std::string cmd_name;
	
	if (message[start] == ':')
	{
		while (message[start] != ' ' && start < message.size())
			start++;
		while (message[start] == ' ' && start < message.size())
			start++;
	}
	while (message[start + length] != ' ' && (start + length) < message.size())
		length++;
	cmd_name = message.substr(start, length);
	while (i != CMD_NONE && _command_names[i] != cmd_name)
		i++;
	return i;
}

ACommand* CommandProcessor::getCommand(const std::string & message) {
	ACommand	*command;
	std::string	prefix;

	_type = static_cast<e_command>(getType(message));
	if (_type == CMD_NONE ||
		_commandMap.find(_command_names[_type]) == _commandMap.end())
	{
		int length = 0;
		while (message[length] != ' ' && (unsigned long)length < message.size())
				length++;
			if (length)
				_args.push_back(message.substr(0, length));
		return NULL;
	}
	command = _commandMap.find(_command_names[_type])->second;
	command->updateCommand(message, _type);
	return command;
}

std::string CommandProcessor::exec(std::string const &message)
{
	ACommand	*command = getCommand(message);
	Client		*client = _kernel->_clients->getClientBySocket(_kernel->_clientFD);

	std::cout << "message: \"" << message << "\"\n";
	if (!client)
		return "Client doesn't exist\n";
	if (!client->isPassEntered() && (message.find("PASS") != 0) \
		&& (message.find("QUIT") != 0))
		return Response::getInstance().getResponse
				(ERR_NOTREGISTERED, "", "", "");
	if (client->isPassEntered() && !client->isActive() && (message.find("QUIT") != 0)
		&& (message.find("NICK") != 0) && (message.find("USER") != 0))
		return Response::getInstance().getResponse
				(ERR_NOLOGIN, "", "", "");
	client->updateLastPing();
	if (command)
		return command->exec();
	std::cout << "CommandProcessor: Unknown command =(\n";
	return Response::getInstance().getResponse
		(ERR_UNKNOWNCOMMAND, "", client->getNickName(), _args[0]);
}

const char **CommandProcessor::getCommandNames()
{
	return _command_names;
}

