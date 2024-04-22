#include "../headers/Client.hpp"
#include "../headers/Server.hpp"

Client::Client(int socket, std::string hostname)
	: _socketFd(socket),
	  _logged(false),
	  _registered(false),
	  _modes(0),
	  _hostname(hostname),
	  _recvBuffer(""),
	  _awayMessage(""),
	  _quit(false) {
}

Client::~Client() {
}

void Client::setNickname(const std::string &nickname) {
	_nickname = Server::uncapitalizeString(nickname);
}

void Client::setUsername(const std::string &username) {
	Client::_username = username;
}

void Client::setLog() {
	Client::_logged = true;
}

void Client::setRegistration() {
	Client::_registered = true;
}

void Client::setPassword(const std::string &password) {
	Client::_password = password;
}

void Client::setRealName(const std::string &real_name) {
	_realName = real_name;
}

void Client::setAwayMessage(const std::string &away) {
	Client::_awayMessage = away;
}

void Client::setQuit(bool quit) {
	Client::_quit = quit;
}

const std::string &Client::getNickname() const {
	return _nickname;
}

const std::string &Client::getUsername() const {
	return _username;
}

bool Client::isRegistered() const {
	return _registered;
}

bool Client::isLogged() const {
	return _logged;
}

int Client::getSocket() const {
	return _socketFd;
}

const std::string &Client::getPassword() const {
	return _password;
}

std::string Client::getRealName() const {
	return _realName;
}

const std::string &Client::getHostname() const {
	return _hostname;
}

bool Client::isQuit() const {
	return _quit;
}

void Client::pushSendQueue(std::string send) {
	_sendQueue.push(send);
}

std::string Client::popSendQueue() {
	std::string ret(_sendQueue.front());
	_sendQueue.pop();
	return ret;
}

bool Client::sendQueueEmpty() {
	return _sendQueue.empty();
}

void Client::appendRecvBuffer(std::string recv) {
	_recvBuffer.append(recv);
}

std::string Client::getRecvBuffer() {
	return _recvBuffer;
}

void Client::resetRecvBuffer() {
	_recvBuffer.clear();
}

bool Client::isRecvBufferEmpty() {
	return _recvBuffer.empty();
}

const std::string &Client::getAwayMessage() const {
	return _awayMessage;
}

const std::vector<std::string> &Client::getChannels() const {
	return _channels;
}

void Client::addChannel(const std::string &channel) {
	_channels.push_back(channel);
}

void Client::removeChannel(const std::string &channel) {
	for (std::vector<std::string>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		if (*it == channel) {
			_channels.erase(it);
			break;
		}
	}
}

std::string Client::returnModes() {
	std::string fullModes;

	if (activeMode(AWAY))
		fullModes.append("a");
	if (activeMode(INVISIBLE))
		fullModes.append("i");
	if (!fullModes.empty()) {
		fullModes.insert(0, "+");
		return fullModes;
	} else
		return "";
}

Mode	Client::getMode(const std::string &mode) {
	if (mode.size() != 2)
		return UNKNOWN;
	if (mode[0] != '+' && mode[0] != '-')
		return UNKNOWN;
	switch (mode[1]) {
		case 'a' :
			return AWAY;
		case 'i':
			return INVISIBLE;
		default:
			return UNKNOWN;
	}
}

void	Client::addMode(Mode mode) {
	if (activeMode(mode))
		return;
	_modes |= mode;
}

void	Client::removeMode(Mode mode) {
	if (!activeMode(mode))
		return;
	_modes &= ~mode;
}

bool	Client::activeMode(Mode mode) const {
	return (_modes & mode) == mode;
}