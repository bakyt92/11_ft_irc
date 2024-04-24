#include "../../headers/Server.hpp"

void Server::processWho(int fd, const std::vector<std::string> &tokens) {
	if (tokens.size() < 2) {
		serverSendError(fd, "WHO", ERR_NEEDMOREPARAMS);
		return;
	}
	std::string targetName = tokens[1];
	std::vector<std::pair<std::string, std::string> > info;
	if (targetName.at(0) == '#' || targetName.at(0) == '&') {
		Channel *channel = findChannel(targetName);
		if (channel) {
			std::set<int> members = channel->getMemberFds();
			for (std::set<int>::iterator it = members.begin(); it != members.end(); ++it) {
				Client *client = clients[*it];
				if (!client->activeMode(INVISIBLE) || channel->hasMember(fd)) {
					info.push_back(takeFullClientInfo(client, channel));
				}
			}
		}
	} else {
		Client *client = findClient(targetName);
		if (client) {
			info.push_back(takeFullClientInfo(client, NULL));
		}
	}
	for (std::vector<std::pair<std::string, std::string> >::iterator it = info.begin(); it != info.end(); ++it) {
		serverSendReply(fd, it->first, RPL_WHOREPLY, it->second);
	}
	serverSendReply(fd, targetName, RPL_ENDOFWHO, "");
}

std::pair<std::string, std::string> Server::takeFullClientInfo(Client *client, Channel *channel) {
	std::string clientInfo;
	clientInfo
		.append(channel ? channel->getName() : "*")
		.append(" ~").append(client->getUsername())
		.append(" ").append(client->getHostname())
		.append(" ").append(serverName)
		.append(" ").append(client->getNickname())
		.append(" ").append(client->activeMode(AWAY) ? "G" : "H")
		.append(channel && channel->hasOperator(client->getSocket()) ? "@" : "");
	std::string hopcountAndRealName = "0 " + client->getRealName();
	return std::make_pair(clientInfo, hopcountAndRealName);
}

void    Server::processWhois(int fd, const std::vector<std::string> &tokens) {
    (void)tokens;
    serverSendReply(fd, "", RPL_ENDOFWHOIS, "");
}