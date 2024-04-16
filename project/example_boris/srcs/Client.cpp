#include "Client.hpp"

Client::Client() {
	_flags = 0;
	updateLastPing();
}

Client::Client(const string& name) : _nickName(name) {
	_flags = 0;
	updateLastPing();
}

Client &Client::operator = (const Client &client) {
    if (this != &client) {
		_nickName = client._nickName;
    }
    return *this;
}

string Client::getNickName() {
	return _nickName;
}

void Client::setHostIp(std::string hostip) {
	_hostip = hostip;
}

std::string Client::getHostIp() {
	return _hostip;
}

bool Client::isActive() {
	return _flags & ACTIVE;
}

bool Client::isIrcOperator() {
	return _flags & IRC_OPERATOR;
}

bool Client::isInvisible() {
	return _flags & INVISIBLE;
}

bool Client::isPassEntered() {
	return _flags & PASS_ENTERED;
}

bool Client::isServerNotice() {
	return _flags & SERVER_NOTICE;
}

bool Client::isWallOps() {
	return _flags & WALLOPS;
}

void Client::setFlag(unsigned int flag) {
	_flags |= flag;
}

void Client::deleteFlag(unsigned int flag) {
    _flags &= ~flag;
}

int Client::setNickName(const string &name)
{
	_nickName = name;
	return 0;
}

int Client::setRealName(const string &name)
{
	_realName = name;
	return 0;
}

string Client::getRealName()
{
	return _realName;
}

time_t Client::getLastPing()
{
	return _lastping;
}

void Client::setLastPing(time_t lastPing)
{
	_lastping = lastPing;
}

void Client::updateLastPing() 
{
    _lastping = time(0); 
}

int Client::getFd() {
	return _fd;
} 

void Client::setFd(int fd) {
	_fd = fd;
} 

bool Client::connectClient()
{
	if (_nickName.empty() || _userName.empty() || _flags & ACTIVE)
		return false;
	setFlag(ACTIVE);
	return true;
}

void Client::addInvitation(const string &channelName)
{
	_invitations.insert(channelName);
}

bool Client::getInvitation(const string &channelName)
{
	std::set<string>::iterator it = _invitations.find(channelName);
	if (it != _invitations.end())
	{
		_invitations.erase(it);
		return true;
	}
	return false;
}

const string &Client::getUserName() const
{
	return _userName;
}

void Client::setUserName(const string &userName)
{
	_userName = userName;
}
