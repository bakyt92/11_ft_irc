#include "Kernel.hpp"

Kernel::Kernel() {
    _clientFD = 0;
    
    _clients = new ClientCollection();
    _channels = new ChannelCollection();
    _cmdProc = new CommandProcessor(this);
    _server = new Server(this);
    loadSettings();
	std::string		line;
	std::ifstream	filestream("msgOfTheDay");
	if (filestream.is_open())
	{
		while (getline(filestream, line))
			set.msgOfTheDay.push_back(line);
		filestream.close();
	}
	else
		set.msgOfTheDay.push_back(MOTD_FILE_ERR);
}

Kernel::~Kernel() {
    delete _clients;
    delete _channels;
    delete _cmdProc;
    delete _server;
}

Kernel::Kernel(Kernel const & source) {
    this->_clientFD = source._clientFD;
    this->_clients = source._clients;
    this->_channels = source._channels;
    this->_server = source._server;
    this->_cmdProc = source._cmdProc;
}

Kernel & Kernel::operator=(Kernel const & source){
	if (this != &source) {}
	return (*this);
}

void Kernel::loadSettings() {
	std::string							line;
	std::ifstream						filestream("settings");
	std::map<std::string, std::string> settings;
	settings["serverName"] = "ircServer";
	settings["checkDelay"] = "60";
	if (filestream.is_open()) {
		while (getline(filestream, line)) {
			std::vector<std::string> tmp = splitString(line, '=');
			if (tmp.size() > 1)
				settings[tmp[0]] = tmp[1];
		}
		filestream.close();
		set.checkdelay = atoi((settings["checkDelay"]).data());
		set.serverName = settings["serverName"];
		Response::getInstance().setServerName(set.serverName);
		int i = 1;
		std::map<std::string, std::string>::iterator iter;
		while ((iter = settings.find("Operator" + itoa(i++))) != settings.end()) {
			std::vector<std::string> params = splitString(iter->second, ':');
			if (params.size() > 1)
				set.oper[params[0]] = params[1];
		}
	}
}

void Kernel::sendToClient(Client* receiver, std::string msg) {
	send(receiver->getFd(), msg.c_str(), msg.length(), IRC_NOSIGNAL);
	std::cout << msg;
}

void Kernel::sendToChanel(Channel *channel, Client *from, std::string msgEnd) {
	std::string msgStart = ":" + from->getNickName() + \
		"!" + from->getUserName() + \
		"@" + from->getHostIp() + " ";
	int posToReplace = msgEnd.find("${receiver}");
	std::vector<Client*> _clList = channel->getListOfClient();
	for (unsigned long i = 0; i < _clList.size(); i++) {
		std::string msg = msgStart;
		if  (posToReplace > -1)
			msg += msgEnd.replace(posToReplace, 11, _clList[i]->getNickName());
		else
			msg += msgEnd;
		msg += "\n";
		sendToClient(_clList[i], msg);
	}
}

void Kernel::updateChannelOper(Channel* channel, Client *oldOper) {
	if (channel && channel->getListOfOper().size() <= 1 && channel->getListOfClient().size() > 1) {
		int i = 0;
		while (channel->getListOfClient()[i] == oldOper)
			i++;
		if (channel->getListOfClient()[i] && channel->getListOfClient()[i] != oldOper) {
			Client* newOper = channel->getListOfClient()[i];
			channel->giveChanopPriv(oldOper, newOper);
		}
	}
}

std::string	Kernel::itoa(int n)
{
	std::string result;

	if (n < 10) return (result += n + 48);
	return (result += itoa(n / 10) + char(n % 10 + 48));
}

bool Kernel::contains(std::vector<Client*> list, Client* value) {
	return (std::find(list.begin(), list.end(), value) != list.end());
}

std::vector<std::string> Kernel::splitString(std::string str, char delim) {
	str += delim;
    int _startPos = 0;
    int _endPos = 0;
	std::vector<std::string> result;
    while ((_endPos = str.find(delim, _startPos)) > -1) {
        std::string part = str.substr(_startPos, _endPos - _startPos);
		if (part == "")
			return result;
		result.push_back(part);
        _startPos = _endPos + 1;
	}
	return result;
}


bool Kernel::isStringFitsMask(const string& mask, const string& str) {
	char const *p = mask.c_str();
	char const *s = str.c_str();
	char const *rp = 0;         // последняя проверенная звездочка
	char const *rs = 0;         // часть строки, которая относится к найденой звездочке
	while (1) {
		if (*p == '*') {
			rs = s;
			rp = ++p;
		}
		else if (!*s) {
			return !*p;
		}
		else if (*s == *p || *p == '?') {
			++s;
			++p;
		}
		else if (rs) {
			s = ++rs;
			p = rp;
		}
		else {
			return false;
		}
	}
}

std::string Kernel::getMOTD(const std::string& to)
{
	std::string result;
	std::vector<std::string>::iterator it = set.msgOfTheDay.begin();
	if (*it == MOTD_FILE_ERR)
		return Response::getInstance().getResponse(ERR_NOMOTD, "", to, "");
	result += Response::getInstance().getResponse(RPL_MOTDSTART, "", to, set.serverName + " Message of the day - ");
	while (it != set.msgOfTheDay.end())
	{
		result += Response::getInstance().getResponse(RPL_MOTD, "", to, ":- " + *it);
		it++;
	}
	result += Response::getInstance().getResponse(RPL_ENDOFMOTD, "", to, ":End of /MOTD command");
	return result;
}