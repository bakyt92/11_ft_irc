#include "CommandNotice.hpp"

CommandNotice::CommandNotice() : ACommand() {}
CommandNotice::CommandNotice(Kernel *kernel) : ACommand(kernel){}
CommandNotice::~CommandNotice() {}
CommandNotice &CommandNotice::operator=(CommandNotice const &source) {
	if (this != &source) {}
	return *this;
}

int CommandNotice::sendToClient(Client* receiver, std::string msgEnd) {
	std::string msg = ":" + _client->getNickName() + \
						"!" + _client->getRealName() + \
						"@" + _client->getHostIp() + \
						" NOTICE " + msgEnd + "\n";
	if (_kernel->contains(_sentList, receiver))
		return 1;
	send(receiver->getFd(), msg.c_str(), msg.length(), 0);
	_sentList.push_back(receiver);
	return 0;
}

int	CommandNotice::sendToClientList(std::vector<Client*> clients, std::string sender, \
		std::string msgEnd) {
	for (unsigned long i = 0; i < clients.size(); i++) {
		if (sender == "")
			sendToClient(clients[i], clients[i]->getNickName() + " :" + msgEnd);
		else
			sendToClient(clients[i], sender + (msgEnd[0] == ':' ? " " : " :") + msgEnd);
	}
	return 0;
}

// ************** при указании получаетлем #channel
// при отсутствии канала - отправляем ошибку и не прерываем
int CommandNotice::sendToChannel(std::string channelName) {
	Channel *_chPoint = _kernel->_channels->getChannelByName(channelName);
	if (!_chPoint) {
		std::string response = Response::getInstance().getResponse
			(ERR_NOSUCHNICK, "", "", channelName);
		send(_kernel->_clientFD, response.c_str(), response.length(), 0);
		return 1;
	}

	Client *_sender = _kernel->_clients->getClientBySocket(_kernel->_clientFD);
	bool chkNotOnChannel = find(_chPoint->getListOfClient().begin(), \
							  _chPoint->getListOfClient().end(), _sender) == \
							  _chPoint->getListOfClient().end();
	bool chkClientFlag = !(_chPoint->isChanopNick(_sender)) && !(_chPoint->isSpeaker(_sender));
	if ((chkNotOnChannel && (_chPoint->getFlags() & NOMSGOUTSIDE)) || \
	   (chkClientFlag && (_chPoint->getFlags() & MODERATED))) {
		std::string response = Response::getInstance().getResponse
			(ERR_CANNOTSENDTOCHAN, "", "", channelName);
		send(_kernel->_clientFD, response.c_str(), response.length(), 0);
		return 1;
	}
	std::string msg = _args[2][0] == ':' ? _args[2].data() + 1 : _args[2];
	sendToClientList(_chPoint->getListOfClient(), channelName, msg);
	return 0;
}

// ************** при указании получаетлем $*<host>/#*<host>
// при отсутствии хоста - отправляем ошибку и не прерываем
int CommandNotice::sendToHost(std::string serverName) {
	std::string _serverName = serverName.substr(2, serverName.length() - 2);
	if (_serverName != _kernel->set.serverName) {
		std::string response = Response::getInstance().getResponse
			(ERR_NOTOPLEVEL, "", "", _serverName);
		send(_client->getFd(), response.c_str(), response.length(), 0);
		return 1;
	}
	std::string msg = _args[2][0] == ':' ? _args[2].data() + 1 : _args[2];
	sendToClientList(_kernel->_clients->getListOfClient(), "", msg);
	return 0;
}

int CommandNotice::splitReceivers() {
    int _startPos = 0;
    int _endPos = 0;
	std::string _receiversStr = _args[1] + ",";
    while ((_endPos = _receiversStr.find(",", _startPos)) > -1) {
        std::string part = _receiversStr.substr(_startPos, _endPos - _startPos);
		if (part == "")
			return 0;
		if (part.size() > 2 && (part[0] == '#' || part[0] == '$')) {
			if (part[1] == '*') //*<host>
				sendToHost(part);
			else 				//#channel
				sendToChannel(part);
		}
		else {					//client
			if (!_kernel->_clients->getClientByName(part)) {
				std::string response = Response::getInstance().getResponse
					(ERR_NOSUCHNICK, "", "", part);
				send(_kernel->_clientFD, response.c_str(), response.length(), 0);
			}
			else {
				sendToClient(_kernel->_clients->getClientByName(part), \
					_kernel->_clients->getClientByName(part)->getNickName() + \
					" :" + _args[2]);
			}
		}
        _startPos = _endPos + 1;
    }
    return 0;
}

int CommandNotice::hasDuplicates(std::vector<std::string> receivers) {
	for (unsigned long i = 0; i < receivers.size(); i++)
		for (unsigned long j = i + 1; j < receivers.size(); j++)
			if (receivers[i] == receivers[j]) {
				std::string response = Response::getInstance().getResponse
					(ERR_TOOMANYTARGETS, "", "", CommandProcessor::getCommandNames()[_type]);
				send(_kernel->_clientFD, response.c_str(), response.length(), 0);
				return 1;
			}
	return 0;
}

std::string CommandNotice::exec() {
	showMe();
	if (_args.size() == 1)
		return Response::getInstance().getResponse
			(ERR_NORECIPIENT, "", "", CommandProcessor::getCommandNames()[_type]);
	if (_args.size() == 2)
		return Response::getInstance().getResponse
			(ERR_NOTEXTTOSEND, "", "", CommandProcessor::getCommandNames()[_type]);
	_sentList.clear();
	splitReceivers();
	return "";
}
