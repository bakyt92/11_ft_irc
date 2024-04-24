
#include "Kernel.hpp"

ACommand::ACommand(Kernel *kernel) :	_kernel(kernel),
										_client(NULL) {}

std::string	ACommand::getPrefix() {
	int length = 0;
	std::string	prefix;
	
	if (_message[length] == ':')
	{
		while (_message[length] != ' ' && (unsigned long)length < _message.size())
			length++;
	}
	else
		return "";
	prefix = _message.substr(0, length);
	_message = _message.substr(length, _message.size() - length);
	return prefix;
}

void ACommand::cropSpaces()
{
	int length = 0;

	while (_message[length] == ' ' && (unsigned long)length < _message.size())
			length++;
	_message = _message.substr(length, _message.size() - length);
}

void ACommand::getArg()
{
	int length = 0;

	if (_message[0] == ':')
	{
		_args.push_back(_message.substr(1, _message.size() - 1));
		_message = "";
	}
	else
	{
		while (_message[length] != ' ' && (unsigned long)length < _message.size())
			length++;
		if (length)
			_args.push_back(_message.substr(0, length));
		_message = _message.substr(length, _message.size() - length);
	}
}

void ACommand::parseMessage()
{
	_prefix = getPrefix();
	while (!_message.empty())
	{
		cropSpaces();
		getArg();		
	}
}

const std::string & ACommand::getCommandName() const
{
	return _args[0];
}

void ACommand::updateCommand(const string &message, e_command type)
{
	_args.clear();
	_client = NULL;
	_message = message;
	_type = type;
	_client = _kernel->_clients->getClientBySocket(_kernel->_clientFD);
	parseMessage();
}

void ACommand::showMe() const
{
	std::cout << "Socket = " << _kernel->_clientFD <<
			  ", nick " << _client->getNickName() << std::endl;
	std::cout << "Command: \"" << _args[0] << "\". Args: ";
	for (unsigned long i = 1; i < _args.size(); ++i)
		std::cout << "\"" << _args[i] << "\"; ";
	std::cout << std::endl;
	
}