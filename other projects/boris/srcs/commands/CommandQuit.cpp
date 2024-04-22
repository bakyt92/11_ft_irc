#include "CommandQuit.hpp"

CommandQuit::CommandQuit() : ACommand() {}
CommandQuit::CommandQuit(Kernel *kernel) : ACommand(kernel){}
CommandQuit::~CommandQuit() {}
CommandQuit &CommandQuit::operator=(CommandQuit const &source) {
	if (this != &source) {}
	return *this;
}

string CommandQuit::exec() {
	showMe();
	Client *_quitClient = _kernel->_clients->getClientBySocket(_kernel->_clientFD);
	if (_quitClient) {
		vector<string> _chList = _kernel->_channels->getListOfChannels();
		for (unsigned long i = 0; i < _chList.size(); i++) {
			Channel *_chPoint = _kernel->_channels->getChannelByName(_chList[i]);
			if (_chPoint->findClient(_quitClient) != NULL) {
				std::string msg = "QUIT ";
				if (_args.size() > 1)
					msg += _chList[i] + ":" + _args[1] + "\n";
				else 
					msg += ":" + _chList[i] + "\n";
				_kernel->sendToChanel(_chPoint, _quitClient, msg);
				_kernel->updateChannelOper(_chPoint, _client);
				_chPoint->deleteClient(_client);
				_kernel->_channels->deleteEmptyChannel(*_chPoint);
			}
		}
	}
	_kernel->_clients->deleteClient(_kernel->_clientFD);
	return "";
}
