#include "../headers/Server.hpp"

Server::Server(int port, const std::string &password) {
	// setting the address family - AF_INET for IPv4
	address.sin_family = AF_INET;
	// setting the port converting port value to network byte order
	address.sin_port = htons(port);
	// setting the IP - INADDR_ANY for any network interface on the machine - converting it to network byte order.
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	// creating the main listening socket and adding it to pollFds container
	socketFd = socket(address.sin_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (socketFd == -1) {
		throw std::runtime_error(
			"Socket error: [" + std::string(strerror(errno)) + "]");
	}
	pollfd serverPollFd;
	serverPollFd.fd = socketFd;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	pollFds.push_back(serverPollFd);
	// binding socket to the port
	if (bind(this->socketFd, (sockaddr *) (&address), sizeof(address)) == -1) {
		throw std::runtime_error(
			"Bind error: [" + std::string(strerror(errno)) + "]");
	}
	this->start = time(0);
	this->_password = password;
	this->serverName = "42.IRC";
	this->serverVersion = "1.0";
	initCmd();
	initChannelMode();
	initServerMessages();
	listenPort();
	memset(_buffer, 0, 1024);
	std::cout << "Server created: address=" << inet_ntoa(address.sin_addr)
			  << ":"
			  << ntohs(address.sin_port)
			  << " socketFD=" << socketFd
			  << " _password=" << this->_password << std::endl;
}

void Server::initCmd() {
	cmd["PRIVMSG"] = &Server::processPrivmsg;
	cmd["NOTICE"] = &Server::processPrivmsg;
	cmd["JOIN"] = &Server::processJoin;
	cmd["INVITE"] = &Server::processInvite;
	cmd["KICK"] = &Server::processKick;
	cmd["TOPIC"] = &Server::processTopic;
	cmd["PART"] = &Server::processPart;
	cmd["MODE"] = &Server::processMode;
	cmd["NAMES"] = &Server::processNames;
	cmd["LIST"] = &Server::processList;
	cmd["PING"] = &Server::processPing;
	cmd["AWAY"] = &Server::processAway;
	cmd["NICK"] = &Server::processNick;
	cmd["QUIT"] = &Server::processQuit;
    cmd["WHO"] = &Server::processWho;
    cmd["WHOIS"] = &Server::processWhois;
}

void Server::initChannelMode() {
	channelMode['i'] = &Server::handleModeI;
	channelMode['t'] = &Server::handleModeT;
	channelMode['k'] = &Server::handleModeK;
	channelMode['l'] = &Server::handleModeL;
	channelMode['o'] = &Server::handleModeO;
    channelMode['b'] = &Server::handleModeB;
}

void Server::initServerMessages() {
	_serverMessages[RPL_WELCOME] = " Welcome to the IRC Network";
	_serverMessages[RPL_YOURHOST] = " :Your host is " + serverName + " version " + serverVersion;
	_serverMessages[RPL_CREATED] = " :This server was created " + static_cast<std::string>(ctime(&start));
	_serverMessages[RPL_MYINFO] = " " + serverName + " " + serverVersion + " available user/channel modes: +is/+itkl";

	_serverMessages[RPL_LISTEND] = " :End of /LIST";
	_serverMessages[RPL_NOTOPIC] = " :No topic is set";
	_serverMessages[RPL_ENDOFNAMES] = " :End of /NAMES list";
	_serverMessages[RPL_UNAWAY] = " :You are no longer marked as being away";
	_serverMessages[RPL_NOWAWAY] = " :You have been marked as being away";
    _serverMessages[RPL_ENDOFWHO] = " :End of WHO list";
    _serverMessages[RPL_ENDOFWHOIS] = " :End of WHOIS list";

	_serverMessages[ERR_NOSUCHNICK] = " :No such nick/channel";
	_serverMessages[ERR_NOSUCHSERVER] = " :No such server";
	_serverMessages[ERR_NOSUCHCHANNEL] = " :No such channel";
	_serverMessages[ERR_CANNOTSENDTOCHAN] = " :Cannot send to channel";
	_serverMessages[ERR_TOOMANYCHANNELS] = " :You have joined too many channels";
	_serverMessages[ERR_TOOMANYTARGETS] = " :Too many targets";
	_serverMessages[ERR_NOORIGIN] = " :No origin specified";
	_serverMessages[ERR_NORECIPIENT] = " :No recipient given";
	_serverMessages[ERR_NOTEXTTOSEND] = " :No text to send";
	_serverMessages[ERR_UNKNOWNCOMMAND] = " :Unknown command";
	_serverMessages[ERR_ERRONEUSNICKNAME] = " :Erroneus nickname";
	_serverMessages[ERR_NICKNAMEINUSE] = " :Nickname is already in use";
	_serverMessages[ERR_USERNOTINCHANNEL] = " :They aren't on that channel";
	_serverMessages[ERR_NOTONCHANNEL] = " :You're not on that channel";
	_serverMessages[ERR_USERONCHANNEL] = " :is already on channel";
	_serverMessages[ERR_NEEDMOREPARAMS] = " :Not enough parameters";
	_serverMessages[ERR_ALREADYREGISTERED] = " :You may not reregister";
	_serverMessages[ERR_PASSWDMISMATCH] = " :Password incorrect";
	_serverMessages[ERR_CHANNELISFULL] = " :Cannot join channel (+l)";
	_serverMessages[ERR_UNKNOWNMODE] = " :is unknown mode char to me";
	_serverMessages[ERR_INVITEONLYCHAN] = " :Cannot join channel (+i)";
	_serverMessages[ERR_BADCHANNELKEY] = " :Cannot join channel (+k)";
	_serverMessages[ERR_CHANOPRIVSNEEDED] = " :You're not channel operator";
	_serverMessages[ERR_UMODEUNKNOWNFLAG] = " :Unknown MODE flag";
	_serverMessages[ERR_USERSDONTMATCH] = " :Cant change mode for other users";
	_serverMessages[ERR_NONICKNAMEGIVEN] = " :No nickname given";
    _serverMessages[RPL_ENDOFBANLIST] = " :End of channel ban list";

}

Server::~Server() {
	// Memory Cleanup
	for (std::map<int, Client *>::iterator it = clients.begin();
		 it != clients.end(); ++it) {
		delete it->second;
	}
	for (std::vector<Channel *>::iterator it = _channels.begin();
		 it != _channels.end(); ++it) {
		delete *it;
	}
	// Closing sockets
	for (std::vector<pollfd>::iterator it = pollFds.begin();
		 it != pollFds.end(); ++it) {
		close(it->fd);
	}
}

void Server::addClient(int clientSocket, std::string clientHostname) {
	// Create a new Client object and insert it into the clients map
	clients.insert(std::make_pair(clientSocket, new Client(clientSocket, clientHostname)));
}

void Server::removeClient(int clientSocket) {
	// removing from _channels
	for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end();) {
		(*it)->removeMember(clientSocket);
		if ((*it)->getMemberFds().empty()) {
			delete *it;
			it = _channels.erase(it);
		} else {
			++it;
		}
	}
	// removing from pollFds
	for (std::vector<pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it) {
		if (it->fd == clientSocket) {
			pollFds.erase(it);
			break;
		}
	}
	// removing from users, deleting and removing from clients
	std::map<int, Client *>::iterator it = clients.find(clientSocket);
	if (it != clients.end()) {
		users.erase(it->second->getNickname());
		delete it->second;
		clients.erase(it);
	}
}

void Server::run() {
	for (std::map<int, Client *>::iterator it = clients.begin();
		 it != clients.end(); ++it) {
		if (!it->second->sendQueueEmpty()) {
			for (std::vector<pollfd>::iterator it2 = pollFds.begin();
				 it2 != pollFds.end(); ++it2) {
				if (it2->fd == it->first) {
					it2->events = POLLOUT;
					break;
				}
			}
		}
	}
	int countEvents = poll(&pollFds[0], pollFds.size(), 0);
	if (countEvents < 0) {
		throw std::runtime_error(
			"Poll error: [" + std::string(strerror(errno)) + "]");
	}
	for (size_t i = 0; i < pollFds.size(); i++) {
		if (pollFds[i].revents & POLLIN) {
			i = receiveData(i);
		}
		if (pollFds[i].revents & POLLOUT) {
			sendData(i);
		}
	}
}

size_t Server::receiveData(size_t index) {
// if index == 0 -> first connection
	if (index == 0) {
		std::pair<int, std::string> connectionInfo = acceptConnection();
		addClient(connectionInfo.first, connectionInfo.second);
	} else {
		memset(_buffer, 0, 1024);
		int bytesRead = recv(pollFds[index].fd, _buffer, sizeof(_buffer) - 1,
							 0);
		if (bytesRead > 0) {
			_buffer[bytesRead] = 0;
			if (parsBuffer(pollFds[index].fd)) {
                clients[pollFds[index].fd]->setQuit(true);
            }
		} else if (bytesRead == 0) {
			processQuit(pollFds[index].fd, std::vector<std::string>());
			removeClient(pollFds[index].fd);
			index--;
		} else {
			throw std::runtime_error("Recv error: [" + std::string(strerror(errno)) + "]");
		}
		resetEvents(index);
	}
	return index;
}

void Server::sendData(size_t index) {
	try {
		Client &c = getClient(pollFds[index].fd);
		if (!c.sendQueueEmpty()) {
			std::string msg = c.popSendQueue();
			const char *dataPtr = msg.c_str();
			ssize_t dataRemaining = msg.length();
			ssize_t n = send(pollFds[index].fd, dataPtr, dataRemaining, 0);
			if (dataRemaining > n) {
				c.pushSendQueue(msg.substr(n));
				pollFds[index].events = POLLOUT;
			} else if (n < 0) {
				throw std::runtime_error("Send error");
			} else if (n == 0) {
				processQuit(pollFds[index].fd, std::vector<std::string>());
				removeClient(pollFds[index].fd);
				throw std::runtime_error("Connection closed");
			}
		}
		if (c.isQuit()) {
			close(pollFds[index].fd);
			removeClient(pollFds[index].fd);
		}
		pollFds[index].events = POLLIN;
	}
	catch (std::exception &e) {
		std::cout << "[ERR] " << e.what() << std::endl;
	}
	resetEvents(index);
}

void Server::resetEvents(size_t index) {
	pollFds[index].revents = 0;
}

void Server::listenPort() const {
	if (listen(socketFd, SOMAXCONN) == -1) {
		throw std::runtime_error("ERROR! Cannot listen on the socket");
	}
	std::cout << "Server is listening for incoming connections" << std::endl;
}

std::pair<int, std::string> Server::acceptConnection() {
	sockaddr_in clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);
	pollfd clientPollFd;

	// accept connection and dd the new client's socket to the pollFds container
	int clientSocket = accept(socketFd, (sockaddr *) (&clientAddress),
							  &clientAddressLength);
	if (clientSocket == -1) {
		throw std::runtime_error(
			"Accept error: [" + std::string(strerror(errno)) + "]");
	}
	if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1) {
		// Handle error
		throw std::runtime_error(
			"Fcntl error: [" + std::string(strerror(errno)) + "]");
	}
	clientPollFd.fd = clientSocket;
	clientPollFd.events = POLLIN;
	clientPollFd.revents = 0;
	pollFds.push_back(clientPollFd);
	// print information about the accepted connection
	std::cout << "Accepted connection from: "
			  << inet_ntoa(clientAddress.sin_addr) << ":"
			  << ntohs(clientAddress.sin_port)
			  << " at fd=" << clientSocket << std::endl;
	return std::make_pair(clientSocket, inet_ntoa(clientAddress.sin_addr));
}

// Channel getters

std::string Server::getNickAndHostname(int fd) {
	std::string  res = clients[fd]->getNickname() + "@" + clients[fd]->getHostname();
	return res;
}

std::string Server::getNick(int fd) {
	if (clients.find(fd) != clients.end()) {
		std::string nick = clients[fd]->getNickname();
		return nick;
	}
	return "";
}

void Server::addChannel(Channel *channel) {
	_channels.push_back(channel);
}

void Server::removeChannel(const std::string &channelName) {
	for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		if ((*it)->getName() == Server::uncapitalizeString(channelName)) {
			delete *it;
			_channels.erase(it);
			break;
		}
	}
}

Channel *Server::findChannel(const std::string &name) {
	std::string lowerName = uncapitalizeString(name);
	for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		if ((*it)->getName() == lowerName) {
			return *it;
		}
	}
	return NULL;
}

void Server::removeClientFromChannel(int fd, Channel *channel) {
	channel->removeMember(fd);
	clients[fd]->removeChannel(channel->getName());
	if (channel->getMemberFds().empty()) {
		removeChannel(channel->getName());
	}
}

std::vector<Channel *> Server::findChannels(std::queue<std::string> names) {
	std::vector<Channel *> channels;
	while (!names.empty()) {
		Channel *channel = findChannel(names.front());
		if (channel) {
			channels.push_back(channel);
		}
		names.pop();
	}
	return channels;
}

Client *Server::findClient(const std::string &nickname) {
	std::string lowerNickname = uncapitalizeString(nickname);
	for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if ( it->second->getNickname() == lowerNickname) {
			return it->second;
		}
	}
	return NULL;
}

Client *Server::findClient(int fd) {
	std::map<int, Client *>::iterator it = clients.find(fd);
	if (it == clients.end()) {
		return NULL;
	}
	return it->second;
}

Client &Server::getClient(int fd) {
	if (clients.find(fd) == clients.end()) {
		throw std::runtime_error("Cannot find client with fd");
	}
	return (*this->clients[fd]);
}
