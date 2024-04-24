#include "../../headers/Server.hpp"

void Server::processKick(int fd, const std::vector<std::string> &tokens) {
	if (tokens.size() < 3) {
		serverSendError(fd, "KICK", ERR_NEEDMOREPARAMS);
		return;
	}

	const std::string &channelName = tokens[1];
	const std::string &targetNick = tokens[2];
	std::string reason = targetNick;
	if (tokens.size() > 3) {
		reason = tokens[3].at(0) == ':'
				 ? mergeTokensToString(std::vector<std::string>(tokens.begin() + 3, tokens.end()), true)
				 : tokens[3];
	}
	Channel *channel = findChannel(channelName);
	if (!channel) {
		serverSendError(fd, channelName, ERR_NOSUCHCHANNEL);
	} else if (!channel->hasMember(fd)) {
		serverSendError(fd, channelName, ERR_NOTONCHANNEL);
	} else if (!channel->hasOperator(fd)) {
		serverSendError(fd, channelName, ERR_CHANOPRIVSNEEDED);
	} else {
		Client *targetClient = findClient(targetNick);
		if (!targetClient || !channel->hasMember(targetClient->getSocket())) {
			serverSendError(fd, targetNick + " " + channelName, ERR_USERNOTINCHANNEL);
		} else {
			std::string parameters = channelName + " " + targetNick + " :" + reason;
			serverSendNotification(channel->getMemberFds(), getNickAndHostname(fd), "KICK", parameters);
			removeClientFromChannel(targetClient->getSocket(), channel);
		}
	}
}