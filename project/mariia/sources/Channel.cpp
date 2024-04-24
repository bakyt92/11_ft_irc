#include "../headers/Channel.hpp"
#include "../headers/Server.hpp"

Channel::Channel(const std::string &name, std::string &password) {
	_name = Server::uncapitalizeString(name);
	_password = password;
	_topic = "";
	_mode = 0;
	setMode(TOPICSET);
	if (!_password.empty()) {
		setMode(KEYSET);
	}
}

Channel::~Channel() {
}

const std::string &Channel::getName() const {
	return _name;
}

const std::string &Channel::getTopic() const {
	return _topic;
}

std::string Channel::getModeString() const {
	std::string modeString = "+";

	if (isModeSet(INVITEONLY)) {
		modeString += 'i';
	}
	if (isModeSet(TOPICSET)) {
		modeString += 't';
	}
	if (isModeSet(KEYSET)) {
		modeString += 'k';
	}
	if (isModeSet(LIMITSET)) {
		modeString += 'l';
	}
	return modeString;
}

std::string Channel::getModeStringWithParameters() const {
	std::string modeString = getModeString();
	if (isModeSet(KEYSET) && !_password.empty()) {
		modeString += " " + _password;
	}
	if (isModeSet(LIMITSET) && _limitMembers > 0) {
		std::ostringstream oss;
		oss << _limitMembers;
		modeString += " " + oss.str();
	}
	return modeString;
}

const std::set<int> &Channel::getMemberFds() const {
	return _memberFds;
}

int Channel::getLimitMembers() const {
	return _limitMembers;
}

void Channel::setTopic(const std::string &topic) {
	_topic = topic;
}

void Channel::setPassword(const std::string &password) {
	_password = password;
}

void Channel::setLimitMembers(int limitMembers) {
	Channel::_limitMembers = limitMembers;
}

bool Channel::setMode(unsigned int mode) {
	if (isModeSet(mode)) {
		return false;
	}
	_mode |= mode;
	return true;
}

bool Channel::unsetMode(unsigned int mode) {
	if (!isModeSet(mode)) {
		return false;
	}
	_mode &= ~mode;
	return true;
}

bool Channel::isModeSet(unsigned int mode) const {
	return (_mode & mode) == mode;
}


void Channel::addMember(int clientFd) {
	_memberFds.insert(clientFd);
}

void Channel::removeMember(int clientFd) {
	removeInvited(clientFd);
	removeOperator(clientFd);
	_memberFds.erase(clientFd);
}

bool Channel::hasMember(int clientFd) {
	return _memberFds.find(clientFd) != _memberFds.end();
}

bool Channel::authMember(int clientFd, std::string &password) {
	if (password != _password) {
		return false;
	}
	removeInvited(clientFd);
	addMember(clientFd);
	return true;
}

bool Channel::addOperator(int clientFd) {
	return _operatorFds.insert(clientFd).second;
}

bool Channel::removeOperator(int clientFd) {
	return _operatorFds.erase(clientFd);
}

bool Channel::hasOperator(int clientFd) const {
	return _operatorFds.find(clientFd) != _operatorFds.end();
}

void Channel::addInvited(int clientFd) {
	_invitedFds.insert(clientFd);
}

void Channel::removeInvited(int clientFd) {
	_invitedFds.erase(clientFd);
}

bool Channel::hasInvited(int clientFd) {
	return _invitedFds.find(clientFd) != _invitedFds.end();
}