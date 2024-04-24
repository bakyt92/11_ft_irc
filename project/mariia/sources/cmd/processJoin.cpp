#include "../../headers/Server.hpp"

void Server::processJoin(int fd, const std::vector<std::string> &tokens) {
	if (tokens.size() < 2) {
		serverSendError(fd, "JOIN", ERR_NEEDMOREPARAMS);
		return;
	}

	std::queue<std::string> channels = split(tokens[1], ',', true);
	if (channels.size() > MAXTARGETS) {
		serverSendError(fd, "JOIN", ERR_TOOMANYTARGETS);
		return;
	}
	std::queue<std::string> passwords = (tokens.size() > 2) ? split(tokens[2], ',', false) : std::queue<std::string>();
	while (!channels.empty()) {
		std::string channelName = channels.front();
		channels.pop();
		std::string password = !passwords.empty() ? passwords.front() : "";
		if (!passwords.empty()) passwords.pop();
		if (findClient(fd)->getChannels().size() == MAXCHANNELS) {
			serverSendError(fd, channelName, ERR_TOOMANYCHANNELS);
			return;
		}
		Channel *channel = findChannel(channelName);
		if (channel) {
			joinExistingChannel(fd, channel, password);
		} else {
			createAndJoinNewChannel(fd, channelName, password);
		}
	}
}

void Server::joinExistingChannel(int fd, Channel *channel, std::string password) {
	if (channel->hasMember(fd)) {
		return;
	}
	if (channel->isModeSet(INVITEONLY) && !channel->hasInvited(fd)) {
		serverSendError(fd, channel->getName(), ERR_INVITEONLYCHAN);
		return;
	}
	if (channel->isModeSet(LIMITSET) && (int) channel->getMemberFds().size() == channel->getLimitMembers()) {
		serverSendError(fd, channel->getName(), ERR_CHANNELISFULL);
		return;
	}
	if (channel->authMember(fd, password)) { // checking password and removing from invited container
		clients[fd]->addChannel(channel->getName());
		sendJoinNotificationsAndReplies(fd, channel);
	} else {
		serverSendError(fd, channel->getName(), ERR_BADCHANNELKEY);
	}
}

void Server::createAndJoinNewChannel(int fd, std::string channelName, std::string password) {
	if (isValidChannelName(channelName)) {
		Channel *newChannel = new Channel(channelName, password);
		newChannel->addMember(fd);
		newChannel->addOperator(fd);
		clients[fd]->addChannel(channelName);
		addChannel(newChannel);
		sendJoinNotificationsAndReplies(fd, newChannel);
	} else {
		serverSendError(fd, channelName, ERR_NOSUCHCHANNEL);
	}
}

void Server::sendJoinNotificationsAndReplies(int fd, const Channel *channel) {
	serverSendNotification(channel->getMemberFds(), getNickAndHostname(fd), "JOIN", channel->getName());
	if (!channel->getTopic().empty()) {
		serverSendReply(fd, channel->getName(), RPL_TOPIC, channel->getTopic());
	} else {
        serverSendReply(fd, channel->getName(), RPL_NOTOPIC, "");
    }
	std::string nicknamesString = mergeTokensToString(getAllChannelMembersNicks(channel), false);
	serverSendReply(fd, channel->getName(), RPL_NAMREPLY, nicknamesString);
	serverSendReply(fd, channel->getName(), RPL_ENDOFNAMES, "");
}

bool Server::isValidChannelName(const std::string &name) {
	if (name[0] != '#' && name[0] != '&') {
		return false;
	}
	return isValidName(name.substr(1));
}