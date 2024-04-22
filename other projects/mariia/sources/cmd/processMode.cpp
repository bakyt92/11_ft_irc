#include "../../headers/Server.hpp"

void Server::processMode(int fd, const std::vector<std::string> &tokens) {
	if (tokens.size() < 2) {
		serverSendError(fd, "MODE", ERR_NEEDMOREPARAMS);
		return;
	}

	if (tokens[1].at(0) == '#' || tokens[1].at(0) == '&') {
		processChannelMode(fd, tokens);
	} else {
		processUserMode(fd, tokens);
	}
}

void Server::processChannelMode(int fd, const std::vector<std::string> &tokens) {
	const std::string &channelName = tokens[1];
	Channel *channel = findChannel(channelName);
	std::string nickname = getNick(fd);
	if (!channel) {
		serverSendError(fd, channelName, ERR_NOSUCHCHANNEL);
	} else if (tokens.size() == 2) {
		if (channel->hasMember(fd)) {
			serverSendReply(fd, channelName + " " + channel->getModeStringWithParameters(), RPL_CHANNELMODEIS, "");
		} else {
			serverSendReply(fd, channelName + " " + channel->getModeString(), RPL_CHANNELMODEIS, "");
		}
	} else if (!channel->hasMember(fd)) {
		serverSendError(fd, channelName, ERR_NOTONCHANNEL); // ?
	} else if (!channel->hasOperator(fd)) {
		serverSendError(fd, channelName, ERR_CHANOPRIVSNEEDED);
	} else {
		std::string modes = tokens[2];
		std::string changedModes;
		std::vector<std::string> parametersSet;
		char settingMode = '+';
		char lastSettingMode = '+';
		size_t paramIndex = 3;

		for (size_t i = 0; i < modes.length(); ++i) {
			char mode = modes.at(i);
			if (mode == '+' || mode == '-') {
				settingMode = mode;
				continue; // go to the next iteration to process the channelMode character
			}
			std::string parameter = (modeParameterNeeded(settingMode, mode) && paramIndex < tokens.size())
									? tokens[paramIndex++]
									: ""; // check if the channelMode requires a parameter and take it
			ModeHandlerIterator it = channelMode.find(mode);
			if (it == channelMode.end()) { // check if mode is known
				serverSendError(fd, std::string(1, mode), ERR_UNKNOWNMODE);
				continue;
			}
			if ((this->*(it->second))(settingMode, parameter, channel, fd)) { // if channelMode applied successfully
				parametersSet.push_back(parameter);
				if (lastSettingMode != settingMode) { // add + or - if it changed since last channelMode flag
					changedModes += settingMode;
					lastSettingMode = settingMode;
				}
				changedModes += mode;
			}
		}
		if (!changedModes.empty()) {
			std::string parameters = channelName + " " + changedModes + " " + mergeTokensToString(parametersSet, false);
			serverSendNotification(channel->getMemberFds(), getNickAndHostname(fd), "MODE", parameters);
		}
	}
}

bool Server::modeParameterNeeded(char set, char mode) {
	if (mode == 'o' || (set == '+' && (mode == 'k' || mode == 'l'))) {
		return true;
	}
	return false;
}

bool Server::handleModeI(char set, const std::string &parameter, Channel *channel, int fd) {
	(void) fd;
	(void) parameter;
	if (set == '+') {
		return channel->setMode(INVITEONLY);
	} else {
		return channel->unsetMode(INVITEONLY);
	}
}

bool Server::handleModeT(char set, const std::string &parameter, Channel *channel, int fd) {
	(void) fd;
	(void) parameter;
	if (set == '+') {
		return channel->setMode(TOPICSET);
	} else {
		return channel->unsetMode(TOPICSET);
	}
}

bool Server::handleModeK(char set, const std::string &parameter, Channel *channel, int fd) {
	(void) fd;
	if (set == '+') {
		if (!isValidName(parameter)) {
			return false;
		}
		channel->setPassword(parameter);
		channel->setMode(KEYSET);
		return true;
	} else {
		channel->setPassword("");
		return channel->unsetMode(KEYSET);
	}
}

bool Server::handleModeL(char set, const std::string &parameter, Channel *channel, int fd) {
	(void) fd;
	if (set == '+') {
		std::istringstream iss(parameter);
		int limit;
		if (!(iss >> limit) || limit < static_cast<int>(channel->getMemberFds().size())) {
			return false;
		}
		channel->setLimitMembers(limit);
		channel->setMode(LIMITSET);
		return true;
	} else {
		channel->setLimitMembers(-1);
		return channel->unsetMode(LIMITSET);
	}
}

bool Server::handleModeO(char set, const std::string &parameter, Channel *channel, int fd) {
	if (parameter.empty()) {
		return false;
	}
	Client *client = findClient(parameter);
	if (!client) {
		serverSendError(fd, parameter, ERR_NOSUCHNICK);
		return false;
	}
	if (set == '+') {
		if (!channel->hasMember(client->getSocket())) {
			serverSendError(fd, parameter, ERR_USERNOTINCHANNEL);
			return false;
		}
		return channel->addOperator(client->getSocket());
	} else {
		return channel->removeOperator(client->getSocket());
	}
}

bool Server::handleModeB(char set, const std::string &parameter, Channel *channel, int fd) {
    (void) set;
    (void) parameter;
    (void) channel;
    serverSendReply(fd, "", RPL_ENDOFBANLIST, "");
    return false;
}

void Server::processUserMode(int fd, const std::vector<std::string> &tokens) {
	std::vector<std::string> params(tokens.begin() + 1, tokens.end());
	if (Server::uncapitalizeString(params[0]) != clients[fd]->getNickname()) {
		serverSendError(fd, params[0], ERR_USERSDONTMATCH);
		return;
	}
	std::vector<std::string>::const_iterator it = tokens.begin() + 2;
    if (it == tokens.end()) {
        serverSendReply(fd, "", RPL_UMODEIS, clients[fd]->returnModes());
        return;
    }
	for (; it != tokens.end(); ++it) {
		Mode mode = clients[fd]->getMode(*it);
		if (mode == UNKNOWN || mode == AWAY) {
			serverSendError(fd, *it, ERR_UMODEUNKNOWNFLAG);
			return;
		} else if (it[0][0] == '+') {
			clients[fd]->addMode(mode);
			serverSendReply(fd, "", RPL_UMODEIS, clients[fd]->returnModes());
		} else if (it[0][0] == '-') {
			clients[fd]->removeMode(mode);
			serverSendReply(fd, "", RPL_UMODEIS, clients[fd]->returnModes());
		}
	}
}