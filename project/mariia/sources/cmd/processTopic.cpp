#include "../../headers/Server.hpp"

void Server::processTopic(int fd, const std::vector<std::string> &tokens) {
	if (tokens.size() < 2) {
		serverSendError(fd, "TOPIC", ERR_NEEDMOREPARAMS);
		return;
	}

	const std::string &channelName = tokens[1];
	Channel *channel = findChannel(channelName);
	if (!channel) {
		serverSendError(fd, channelName, ERR_NOSUCHCHANNEL);
	} else if (tokens.size() == 2) {
		std::string topic = channel->getTopic();
		if (topic.empty()) {
			serverSendReply(fd, channelName, RPL_NOTOPIC, "");
		} else {
			serverSendReply(fd, channelName, RPL_TOPIC, topic);
		}
	} else {
		if (!channel->hasMember(fd)) {
			serverSendError(fd, channelName, ERR_NOTONCHANNEL);
		} else if (channel->isModeSet(TOPICSET) && !channel->hasOperator(fd)) {
			serverSendError(fd, channelName, ERR_CHANOPRIVSNEEDED);
		} else {
			std::string topic = tokens[2].at(0) == ':'
								? mergeTokensToString(std::vector<std::string>(tokens.begin() + 2, tokens.end()), true)
								: tokens[2];
			channel->setTopic(topic);
			serverSendNotification(channel->getMemberFds(), getNickAndHostname(fd), "TOPIC", channelName + " :" + topic);
		}
	}
}