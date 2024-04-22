#include "../../headers/Server.hpp"

void Server::processInvite(int fd, const std::vector<std::string> &tokens) {
	if (tokens.size() < 3) {
		serverSendError(fd, "INVITE", ERR_NEEDMOREPARAMS);
		return;
	}

	const std::string &invitedNick = tokens[1];
	const std::string &channelName = tokens[2];
	Client *invitedClient = findClient(invitedNick);
	Channel *channel = findChannel(channelName);
	std::string parameters = invitedNick + " " + channelName;
	if (!invitedClient) {
		serverSendError(fd, invitedNick, ERR_NOSUCHNICK);
	} else if (!channel) {
		serverSendError(fd, channelName, ERR_NOSUCHCHANNEL);
	} else if (!channel->hasMember(fd)) {
		serverSendError(fd, channelName, ERR_NOTONCHANNEL);
	} else if (channel->hasMember(invitedClient->getSocket())) {
		serverSendError(fd, parameters, ERR_USERONCHANNEL);
	} else {
		if (channel->isModeSet(INVITEONLY)) {
			if (!channel->hasOperator(fd)) {
				serverSendError(fd, channelName, ERR_CHANOPRIVSNEEDED);
				return;
			}
			channel->addInvited(invitedClient->getSocket());
		}
		serverSendNotification(invitedClient->getSocket(), getNickAndHostname(fd), "INVITE", parameters);
		serverSendReply(fd, parameters, RPL_INVITING, "");
		if (invitedClient->activeMode(AWAY)) {
			serverSendReply(fd, invitedNick, RPL_AWAY, invitedClient->getAwayMessage());
		}
	}
}