#include "../headers/Server.hpp"

// Parsing

bool Server::parsBuffer(int fd) {
	std::string bufferStr(this->_buffer);
	std::stringstream ss(bufferStr);
	std::string line;

//	 add recvBuffer at the beginning of ss if not empty
	try {
		Client &client = getClient(fd);
		if (!client.isRecvBufferEmpty()) {
			ss.str("");
			ss << client.getRecvBuffer() << bufferStr;
			client.resetRecvBuffer();
		}
		// check if buffer contains a full line
		size_t cRet = ss.str().find('\n');
		if (cRet == std::string::npos) {
			// if not, add buffer to recvBuffer and return
			client.appendRecvBuffer(ss.str());
			return false;
		}
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
//	 tokenize the buffer line by line
	while (std::getline(ss, line)) {
		size_t cRet = line.find('\r');
		if (cRet != std::string::npos)
			line.erase(cRet);

		std::istringstream lineStream(line);
		std::vector<std::string> tokens;
		std::string token;
		while (lineStream >> token) {
			tokens.push_back(token);
		}
		if (!clients[fd]->isRegistered()) {
			if (registrationProcess(fd, tokens))
				return true;
		} else
			processCmd(fd, tokens);
	}
	return false;
}

bool Server::registrationProcess(int fd, std::vector<std::string> &tokens) {
	if (tokens.empty())
		return false; // or 1?
	std::string command = tokens[0];
	std::vector<std::string> params(tokens.begin() + 1, tokens.end());
	if (command == "CAP") {
		if (tokens[1] == "LS") {
			serverSendReply(fd, "", CAPLS, "");
		}
	} else if (handleCommand(fd, command, params)) {
		return true;
	}
	return checkRegistration(fd);
}

bool Server::handleCommand(int fd, const std::string &command, const std::vector<std::string> &params) {
	if (command == "PASS") {
		if (params.empty()) {
			return (serverSendError(fd, "PASS", ERR_NEEDMOREPARAMS), 1);
		}
		if (verifyPassword(fd, params[0]))
			return true;
		else
			clients[fd]->setPassword(params[0]);
	} else if (command == "NICK") {
		if (params.empty()) {
			return (serverSendError(fd, "NICK", ERR_NEEDMOREPARAMS), 0);
		}
		if (verifyNickname(fd, params[0]))
			return false;
		else
			clients[fd]->setNickname(params[0]);
	} else if (command == "USER") {
		if (params.size() < 3) {
			serverSendError(fd, "USER", ERR_NEEDMOREPARAMS);
			return false;
		}
		clients[fd]->setUsername(params[0]);
		std::string realname = params[2].at(0) == ':'
							   ? mergeTokensToString(std::vector<std::string>(params.begin() + 2, params.end()), true)
							   : params[2];
		if (verifyUsername(fd, realname))
			return false;
		else {
			clients[fd]->setRealName(realname);
		}
		if (isBitMask(params[1])) {
			Mode mode = getBitMode(params[1]);
			if (mode == UNKNOWN)
				serverSendError(fd, params[1], ERR_UMODEUNKNOWNFLAG);
			else
				clients[fd]->addMode(mode);
		}
	}
	return false;
}

void Server::processCmd(int fd, std::vector<std::string> &tokens) {
	if (tokens.empty() || tokens[0] == "CAP")
		return;

	std::string command = tokens[0];
	CmdIterator it = cmd.find(command);
	if (it != cmd.end()) {
		(this->*(it->second))(fd, tokens);
	} else {
		serverSendError(fd, command, ERR_UNKNOWNCOMMAND);
	}
}

// Registration utils

bool Server::checkRegistration(int fd) {
	// check if logging is complete
	if (clients[fd]->isLogged() && (!clients[fd]->getNickname().empty() && !clients[fd]->getUsername().empty())) {
		// check if user (nickname) is in database
		if (users.find(clients[fd]->getNickname()) != users.end()) {
            // check if user is already connected
            std::map<int, Client *>::iterator it = clients.begin();
            for (; it != clients.end(); ++it) {
                if (it->second != clients[fd] && it->second->getNickname() == clients[fd]->getNickname()) {
                    serverSendError(fd, clients[fd]->getNickname(), ERR_NICKNAMEINUSE);
                    return true;
                }
            }
		} else {
			// if first time user connect: add to database
			users.insert(std::make_pair(clients[fd]->getNickname(), clients[fd]->getPassword()));
		}
		// registration complete, send welcome
		clients[fd]->setRegistration();
		serverSendReply(fd, "", RPL_WELCOME, clients[fd]->getNickname());
		serverSendReply(fd, "", RPL_YOURHOST, "");
		serverSendReply(fd, "", RPL_CREATED, "");
		serverSendReply(fd, "", RPL_MYINFO, "");
	}
	return false;
}

bool Server::verifyUsername(int fd, const std::string &arg) {
	if (arg.empty()) {
		serverSendError(fd, "", ERR_NEEDMOREPARAMS);
		return true;
	}
	return false;
}

bool Server::verifyNickname(int fd, const std::string &arg) {
	if (arg[0] == ':' || arg[0] == '$')
		return (serverSendError(fd, arg, ERR_ERRONEUSNICKNAME), 1);
	std::string ill = " ,*?!@.";
	for (int i = 0; i < 8; ++i) {
		if (arg.find(ill[i]) != std::string::npos)
			return (serverSendError(fd, arg, ERR_ERRONEUSNICKNAME), 1);
	}
	return false;
}

bool Server::verifyPassword(int fd, const std::string &arg) {
	if (arg.empty()) {
		serverSendError(fd, "", ERR_NEEDMOREPARAMS);
		return false;
	}
    if (arg != _password) {
        serverSendError(fd, "", ERR_PASSWDMISMATCH);
        return true;
    } else {
        clients[fd]->setLog();
    }
	return false;
}

void Server::serverSendError(int fd, const std::string &token, serverRep id) {
	if (id == ERROR) {
		serverSendMessage(fd, "ERROR :Closing connection :" + token + "\r\n");
		return;
	}
	std::stringstream fullReply;
    fullReply << ":" << serverName << " " << paddDigits(id) << " " << getNick(fd);
	if (!token.empty()) {
		fullReply << " " << token;
	}
	fullReply << _serverMessages[id] << "\r\n";
	serverSendMessage(fd, fullReply.str());
}

void Server::serverSendReply(int fd, const std::string &token, serverRep id, const std::string &reply) {
	std::stringstream fullReply;
	if (id == CAPLS) {
        fullReply << "CAP * LS :" << "\r\n";
		serverSendMessage(fd, fullReply.str());
		return;
	}
    fullReply << ":" << serverName << " " << paddDigits(id) << " " << getNick(fd);
	if (!token.empty()) {
		fullReply << " " << token;
	}
	if (_serverMessages.find(id) != _serverMessages.end()) {
		fullReply << _serverMessages[id];
	}
	if (!reply.empty()) {
		fullReply << " :" << reply;
	}
	fullReply << "\r\n";
	serverSendMessage(fd, fullReply.str());
}

void Server::serverSendNotification(int fd, const std::string &prefix, const std::string &command,
									const std::string &parameters) {
	std::stringstream fullNotification;
	fullNotification << ":" << prefix << " " << command << " " << parameters << "\r\n";
	serverSendMessage(fd, fullNotification.str());
}

void Server::serverSendNotification(const std::set<int> &fds, const std::string &prefix, const std::string &command,
									const std::string &parameters) {
	std::stringstream fullNotification;
	fullNotification << ":" << prefix << " " << command << " " << parameters << "\r\n";
	std::string notificationStr = fullNotification.str();

	for (std::set<int>::const_iterator it = fds.begin(); it != fds.end(); ++it) {
		serverSendMessage(*it, notificationStr);
	}
}

void Server::serverSendMessage(int fd, const std::string &message) {
	try {
		getClient(fd).pushSendQueue(message);
	} catch (std::exception &e) {
		std::cout << "[ERR] " << e.what() << std::endl;
	}
}

std::string Server::paddDigits(int i) {
	std::ostringstream stream;
	stream << std::setw(3) << std::setfill('0') << i;
	return stream.str();
}

