#include "CommandJoin.hpp"

CommandJoin::CommandJoin() : ACommand() {}
CommandJoin::CommandJoin(Kernel *kernel) : ACommand(kernel){}
CommandJoin::~CommandJoin() {}
CommandJoin &CommandJoin::operator=(CommandJoin const &source) {
	if (this != &source) {}
	return *this;
}

int CommandJoin::sendCorrectRs(Channel *curChannel) const {
	string chName = curChannel->getName();
	vector<Client*> _clList = curChannel->getListOfClient();
	string result = ":" + _client->getNickName() + \
				"!" + _client->getUserName() + \
				"@" + _client->getHostIp() + " JOIN :" + chName + "\n";
	for (unsigned long i = 0; i < _clList.size(); i++)
		send(_clList[i]->getFd(), result.c_str(), result.length(), 0);
	result = _kernel->_cmdProc->exec("TOPIC " + chName);
	send(_kernel->_clientFD, result.c_str(), result.size(), IRC_NOSIGNAL);
	result = _kernel->_cmdProc->exec("NAMES " + chName);
	send(_kernel->_clientFD, result.c_str(), result.size(), IRC_NOSIGNAL);
	result = _kernel->_cmdProc->exec("MODE " + chName);
	send(_kernel->_clientFD, result.c_str(), result.size(), IRC_NOSIGNAL);
	return 0;
}

int CommandJoin::checkExistChannel(Channel *curCh, Client *curCl, string key) const {
    cout << "flag = " << curCh->getFlags() << endl;
    if ((curCh->getFlags() & ISKEY) && key != curCh->getKey()) {
        return ERR_BADCHANNELKEY;
    } else if (curCh->getListOfClient().size() >= curCh->getLimit()) {
        return ERR_CHANNELISFULL;
    } else if (curCh->getInviteOnlyFlag() && !curCl->getInvitation(curCh->getName())) {
        return ERR_INVITEONLYCHAN;
    } else if (curCh->isBanned(curCl->getHostIp())) {
        return ERR_BANNEDFROMCHAN;
    } else if (!curCh->addNewClient(curCl)) {
		sendCorrectRs(curCh);
		return 0;
    }
    return 300;		//ошибка при создании клиента, как ее идентифицировать???
}

int CommandJoin::join() const {
	Channel *curChannel;
    Client *curClient = _kernel->_clients->getClientBySocket(_kernel->_clientFD);
    vector<string> channels = _kernel->splitString(_args[1], ',');
    vector<string> keys;
    string result;
    if (_args.size() > 2)
	    keys = _kernel->splitString(_args[2], ',');
	for (unsigned long i = 0; i < channels.size(); i++) {
        curChannel = _kernel->_channels->getChannelByName(channels[i]);
        if (curChannel != NULL) {
            string key = (keys.size() > i) ? keys[i] : "";
			int checkRs = checkExistChannel(curChannel, curClient, key);
            result = Response::getInstance().getResponse
            (checkRs, "", "", channels[i]);
			if(checkRs)
				send(_kernel->_clientFD, result.c_str(), result.length(), 0);
        } else if (_kernel->_channels->createChannel(channels[i], _client)) {
                result = Response::getInstance().getResponse
                (ERR_NOSUCHCHANNEL, "", "", channels[i]);
			    send(_kernel->_clientFD, result.c_str(), result.length(), 0);
        } else {
			curChannel = _kernel->_channels->getChannelByName(channels[i]);
			if(keys.size() > i) {
				curChannel->setKey(_client, keys[i]);
			}
			sendCorrectRs(curChannel);
		}
	}
    return 0;
}

string CommandJoin::exec() {
	showMe();
	if (_args.size() < 2)
		return Response::getInstance().getResponse
			(ERR_NEEDMOREPARAMS, "", "", CommandProcessor::getCommandNames()[_type]);
    join();
	return "";
}
