#include "../../headers/Server.hpp"

void    Server::processNick(int fd, const std::vector<std::string> &tokens) {
    if (tokens.size() < 2) {
        serverSendError(fd, clients[fd]->getNickname(), ERR_NONICKNAMEGIVEN);
        return;
    }
    std::map<int, Client *>::iterator it = clients.begin();
    for (; it != clients.end(); ++it) {
        if (it->second != clients[fd] && it->second->getNickname() == Server::uncapitalizeString(tokens[1])) {
            serverSendError(fd, clients[fd]->getNickname(), ERR_NICKNAMEINUSE);
            return;
        }
    }
    if (verifyNickname(fd, tokens[1])) {
        return;
    } else {
        clients[fd]->setNickname(tokens[1]);
        serverSendReply(fd, "", RPL_WELCOME, clients[fd]->getNickname());
    }
}