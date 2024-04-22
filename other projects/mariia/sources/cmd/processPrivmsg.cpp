#include "../../headers/Server.hpp"

bool Server::checkPmTokens(int fd, const std::vector<std::string> &tokens) {
	if (tokens.size() == 1 || tokens.size() == 2) {
		serverSendError(fd, "", (tokens.size() == 1) ? ERR_NORECIPIENT : ERR_NOTEXTTOSEND);
		return false;
	}
	return true;
}

void Server::sendPmToUser(int fd, const std::string &message, const std::string &prefix, const std::string &targetName,
						  const std::string &command) {

	Client *receiver = findClient(targetName);
	if (receiver) {
		std::string parameters = targetName + " :" + message;
		serverSendNotification(receiver->getSocket(), prefix, command, parameters);
		if (command == "PRIVMSG" && receiver->activeMode(AWAY)) {
			serverSendReply(fd, targetName, RPL_AWAY, receiver->getAwayMessage());
		}
	} else if (command == "PRIVMSG") {
		serverSendError(fd, targetName, ERR_NOSUCHNICK);
	}
}

void Server::sendPmToChan(int fd, const std::string &message, const std::string &prefix, const std::string &targetName,
						  const std::string &command) {

	Channel *channel = findChannel(targetName);
	if (channel) {
		if (channel->hasMember(fd)) {
			std::string parameters = channel->getName() + " :" + message;
			std::set<int> receiversFds(channel->getMemberFds());
			receiversFds.erase(fd);
			serverSendNotification(receiversFds, prefix, command, parameters);
		} else if (command == "PRIVMSG") {
			serverSendError(fd, targetName, ERR_CANNOTSENDTOCHAN);
		}
	} else if (command == "PRIVMSG") {
		serverSendError(fd, targetName, ERR_NOSUCHNICK);
	}
}

void Server::processPrivmsg(int fd, const std::vector<std::string> &tokens) {
	if (!checkPmTokens(fd, tokens))
		return;

	std::queue<std::string> targets = split(tokens[1], ',', true);
	if (targets.size() > MAXTARGETS) {
		serverSendError(fd, tokens[0], ERR_TOOMANYTARGETS);
		return;
	}

	std::string message = tokens[2].at(0) == ':'
						  ? mergeTokensToString(std::vector<std::string>(tokens.begin() + 2, tokens.end()), true)
						  : tokens[2];
	std::string prefix = getNickAndHostname(fd);
	while (!targets.empty()) {
		const std::string targetName = targets.front();
		targets.pop();
		if (targetName.at(0) == '#' || targetName.at(0) == '&') {
			sendPmToChan(fd, message, prefix, targetName, tokens[0]);
		} else {
			sendPmToUser(fd, message, prefix, targetName, tokens[0]);
		}
	}
}