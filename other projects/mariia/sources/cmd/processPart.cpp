#include "../../headers/Server.hpp"

void Server::processPart(int fd, const std::vector<std::string> &tokens) {
	if (tokens.size() < 2) {
		serverSendError(fd, "PART", ERR_NEEDMOREPARAMS);
		return;
	}
	std::queue<std::string> channels = split(tokens[1], ',', true);
	if (channels.size() > MAXTARGETS) {
		serverSendError(fd, "PART", ERR_TOOMANYTARGETS);
		return;
	}
	std::string reason;
	if (tokens.size() > 2) {
		reason = tokens[2].at(0) == ':'
				 ? mergeTokensToString(std::vector<std::string>(tokens.begin() + 2, tokens.end()), true)
				 : tokens[2];
	}
	while (!channels.empty()) {
		std::string channelName = channels.front();
		channels.pop();
		Channel *channel = findChannel(channelName);
		if (!channel) {
			serverSendError(fd, channelName, ERR_NOSUCHCHANNEL);
		} else if (!channel->hasMember(fd)) {
			serverSendError(fd, channelName, ERR_NOTONCHANNEL);
		} else {
			serverSendNotification(channel->getMemberFds(), getNickAndHostname(fd), "PART", channelName + " :" + reason);
			removeClientFromChannel(fd, channel);
		}
	}
}