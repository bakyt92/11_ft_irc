#include "Channel.hpp"

Channel::Channel() {
    this->_flags = 0b00000000;
    _limit = std::numeric_limits<unsigned int>::max();
}

Channel::Channel(string channelName, Client* chanop) {
    this->_name = channelName;
    _listOfClient.push_back(chanop);
    _listOfOper.push_back(chanop);
    this->_flags = 0b00000000;
    _limit = std::numeric_limits<unsigned int>::max();
}

Channel::Channel(string channelName) {
    this->_name = channelName;
    this->_flags = 0b00000000;
}

Channel::Channel(const Channel &channel) :
_name(channel._name) {
    *this = channel;
}

Channel &Channel::operator = (const Channel &channel) {
    if (this != &channel) {
        _name = channel._name;
    }
    return *this;
}

Channel::~Channel() {}

const string Channel::getName() {
    return _name;
}

unsigned int Channel::getLimit() {
    return _limit;
}

string Channel::getKey() {
    if (_flags & ISKEY)
        return _key;
    return "";
}

unsigned int Channel::getPrivateFlag() {
    return _flags & PRIVATE;
}

unsigned int Channel::getSecretFlag() {
    return _flags & SECRET;
}

unsigned int Channel::getInviteOnlyFlag() {
    return _flags & INVITEONLY;
}
unsigned int Channel::getTopicFlag() {
    return _flags & TOPICBYOPER;
}

unsigned int Channel::getModeratedChannel() {
    return _flags & MODERATED;
}

unsigned int Channel::getNoMsgOutsideFlag() {
    return _flags & NOMSGOUTSIDE;
}

unsigned int Channel::getKeyFlag() {
    return _flags & ISKEY;
}

unsigned int Channel::getFlags() {
    return _flags;
}

vector<string> Channel::getListOfBanMask() {
    return _listBanMask;
}

vector<Client *> &Channel::getListOfClient() {
    return _listOfClient;
}

const vector<Client *> &Channel::getListOfOper() const
{
	return _listOfOper;
}

const vector<Client *> &Channel::getListOfSpeaker() const {
	return _listOfSpeaker;
}

bool Channel::isContainList(vector<Client *> &list, Client* client) {
    return (std::find(list.begin(), list.end(), client) != list.end());
}

int Channel::setLimit(Client *client, unsigned int limit) {
    if (!isContainList(_listOfOper, client))
        return ERR_CHANOPRIVSNEEDED;
    _limit = limit;
    return 0;
}

int Channel::setKey(Client *client, string key) {
    if (isContainList(_listOfOper, client)) {
        _key = key;
        _flags |= ISKEY;
        return 0;
    }
    return ERR_CHANOPRIVSNEEDED;
}

int Channel::setFlag(Client *client, unsigned int flag) {
    if (!isContainList(_listOfOper, client))
        return ERR_CHANOPRIVSNEEDED;
    _flags |= flag;
    return 0;
}

int Channel::deleteFlag(Client *client, unsigned int flag) {
    if (!isContainList(_listOfOper, client))
        return ERR_CHANOPRIVSNEEDED;
    _flags &= ~flag;
    return 0;
}

int Channel::setBanMask(Client *client, string banMask) {
	if (!isContainList(_listOfOper, client))
		return ERR_CHANOPRIVSNEEDED;
    _listBanMask.push_back(banMask);
	return 0; //RPL_BANLIST  ; RPL_ENDOFBANLIST
}

int Channel::removeBanMask(Client *client, string banMask) {
	if (!isContainList(_listOfOper, client))
		return ERR_CHANOPRIVSNEEDED;
	std::vector<std::string>::iterator it = std::find(_listBanMask.begin(), _listBanMask.end(), banMask);
	if (it != _listBanMask.end())
		_listBanMask.erase(it);
	return 0; //RPL_BANLIST  ; RPL_ENDOFBANLIST
}

bool Channel::isBanned(string hostname) {
    for(vector<string>::iterator it = _listBanMask.begin(); it != _listBanMask.end(); it++) {
        if (Kernel::isStringFitsMask((*it), hostname))
            return true;
    }
    return false;
}

int Channel::giveChanopPriv(Client *clientFrom, Client *clientTo) {
    if (!isContainList(_listOfOper, clientFrom))
        return ERR_CHANOPRIVSNEEDED;
    _listOfOper.push_back(clientTo);
	std::string message;
	message = ":" + clientFrom->getNickName() + "!" + clientFrom->getUserName() +
			  "@127.0.0.1 MODE " + _name + " +o " + clientTo->getNickName() + "\n";
	std::cout << message;
	vector<Client *>::iterator it = _listOfClient.begin();
	while (it != _listOfClient.end())
	{
		send((*it)->getFd(), message.c_str(), message.size(), IRC_NOSIGNAL);
		it++;
	}
	return 0;
}

int Channel::takeChanopPriv(Client *clientFrom, Client *clientTo) {
    vector<Client *>::iterator itTo = std::find(_listOfOper.begin(), _listOfOper.end(), clientTo);
    if (!isContainList(_listOfOper, clientFrom))
        return ERR_CHANOPRIVSNEEDED;
    else if (itTo != _listOfOper.end())
	{
		_listOfOper.erase(itTo);
		std::string message;
		message = ":" + clientFrom->getNickName() + "!" + clientFrom->getUserName() +
				  "@127.0.0.1 MODE " + _name + " -o " + clientTo->getNickName() + "\n";
		std::cout << message;
		vector<Client *>::iterator it = _listOfClient.begin();
		while (it != _listOfClient.end())
		{
			send((*it)->getFd(), message.c_str(), message.size(), IRC_NOSIGNAL);
			it++;
		}
	}
	return 0;
}

int Channel::giveSpeakPriv(Client *clientFrom, Client *clientTo) {
	if (!isContainList(_listOfOper, clientFrom))
        return ERR_CHANOPRIVSNEEDED;
    _listOfSpeaker.push_back(clientTo);
	std::string message;
	message = ":" + clientFrom->getNickName() + "!" + clientFrom->getUserName() +
			  "@127.0.0.1 MODE " + _name + " +v " + clientTo->getNickName() + "\n";
	std::cout << message;
	vector<Client *>::iterator it = _listOfClient.begin();
	while (it != _listOfClient.end())
	{
		send((*it)->getFd(), message.c_str(), message.size(), IRC_NOSIGNAL);
		it++;
	}
    return 0;
}

int Channel::takeSpeakPriv(Client *clientFrom, Client *clientTo) {
	vector<Client *>::iterator itTo = std::find(_listOfSpeaker.begin(), _listOfSpeaker.end(), clientTo);
    if (!isContainList(_listOfOper, clientFrom))
        return ERR_CHANOPRIVSNEEDED;
    else if (itTo != _listOfSpeaker.end())
	{
		_listOfSpeaker.erase(itTo);
		std::string message;
		message = ":" + clientFrom->getNickName() + "!" + clientFrom->getUserName() +
				  "@127.0.0.1 MODE " + _name + " -v " + clientTo->getNickName() + "\n";
		std::cout << message;
		vector<Client *>::iterator it = _listOfClient.begin();
		while (it != _listOfClient.end())
		{
			send((*it)->getFd(), message.c_str(), message.size(), IRC_NOSIGNAL);
			it++;
		}
	}
	return 0;
}

int Channel::addNewClient(Client *client) {
    if (!isContainList(_listOfClient, client)) {
        _listOfClient.push_back(client);
        return 0;
    }
    return 1;
}

int Channel::deleteClient(Client *client) {
    vector<Client *>::iterator it = std::find(_listOfClient.begin(), _listOfClient.end(), client);
    if (it != _listOfClient.end()) {
        if(isChanopNick((*it))) {
            vector<Client *>::iterator itOper = std::find(_listOfOper.begin(), _listOfOper.end(), client);
            _listOfOper.erase(itOper);
        }
        _listOfClient.erase(it);
        return 0;
    }
    return ERR_NOSUCHNICK;
}

int Channel::isChanopNick(Client *client) {
    if (isContainList(_listOfOper, client))
        return 1;
    return 0;
}

int Channel::isSpeaker(Client *client) {
	if (isContainList(_listOfSpeaker, client))
        return 1;
    return 0;
}

const string &Channel::getTopic() const
{
	return _topic;
}

int Channel::setTopic(const string &topic, Client *client)
{
	if (!(_flags & TOPICBYOPER) || isContainList(_listOfOper, client))
	{
		_topic = topic;
		return 1;
	}
	return 0;
}

Client* Channel::findClient(Client *client) {
    vector<Client *>::iterator it = std::find(_listOfClient.begin(), _listOfClient.end(), client);
    if(it == _listOfClient.end())
        return NULL;
    return *it;
}

int Channel::sendMessageToChannel(int code, const string &answer)
{
	std::string message;
	vector<Client *>::iterator it = _listOfClient.begin();
	while (it != _listOfClient.end())
	{
		message = Response::getInstance().getResponse(code, "",
			(*it)->getNickName(), answer);
		std::cout << message;
		send((*it)->getFd(), message.c_str(), message.size(), IRC_NOSIGNAL);
		it++;
	}
	return 0;
}
