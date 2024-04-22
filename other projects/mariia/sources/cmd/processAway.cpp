#include "../../headers/Server.hpp"

// User should not be able to use MODE +a command but use away command

void    Server::processAway(int fd, const std::vector<std::string> &tokens) {
    if (!clients[fd]->activeMode(AWAY) && tokens.size() < 2) {
        serverSendError(fd, "AWAY", ERR_NEEDMOREPARAMS);
        return;
    }
    if (!clients[fd]->activeMode(AWAY)) {
        std::string away;
        std::vector<std::string>::const_iterator it = tokens.begin() + 1;
        for (; it != tokens.end(); ++it) {
            std::string token = *it;
            if (token[0] == ':') {
                token = token.substr(1);
            }
            away.append(token);
            if (it + 1 != tokens.end()) {
                away.append(" ");
            }
        }
        clients[fd]->setAwayMessage(away);
        clients[fd]->addMode(AWAY);
        serverSendReply(fd, clients[fd]->getNickname(), RPL_NOWAWAY, "");
    }
    else {
        clients[fd]->removeMode(AWAY);
        serverSendReply(fd, clients[fd]->getNickname(), RPL_UNAWAY, "");
    }
}