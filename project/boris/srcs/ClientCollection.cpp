#include "ClientCollection.hpp"

ClientCollection::ClientCollection() {}

ClientCollection::~ClientCollection() {
    for(_it = _clientMap.begin(); _it != _clientMap.end(); _it++) {
        close(_it->first);
        delete _it->second;
    }
    _clientMap.clear();
}

ClientCollection::ClientCollection(const ClientCollection &clientCollection) {
    *this = clientCollection;
}

ClientCollection &ClientCollection::operator= (const ClientCollection &clientCollection) {
    if (this != &clientCollection) {
        *this = clientCollection;
    }
    return *this;
}


vector<Client *> ClientCollection::getListOfClient() {
    vector<Client *> clients;
    for(_it = _clientMap.begin(); _it != _clientMap.end(); _it++) {
        clients.push_back(_it->second);
    }
    return clients;
}

vector<struct pollfd> ClientCollection::getSockets() {
    return _sockets;
}

int ClientCollection::createNewClient(int &fd, std::string hostip) {
    _clientMap[fd] = new Client();
    _clientMap[fd]->setHostIp(hostip);
    _clientMap[fd]->setFd(fd);
    struct pollfd _clientPoll;
    _clientPoll.fd = fd;
    _clientPoll.events = POLLIN | POLLPRI;
    _clientPoll.revents = 0;
    _sockets.push_back(_clientPoll);
    return 0;
}

int ClientCollection::deleteClient(int &fd) {
    _it = _clientMap.find(fd);
    if(_it == _clientMap.end())
        return 1;
    delete _it->second;
    close(_it->first);
    for (unsigned long i = 0; i < _sockets.size(); i++) 
        if (_sockets[i].fd == fd) {
            _sockets.erase(_sockets. begin() + i);
            break;
        }
    _clientMap.erase(_it);
    return 0;  
}

Client* ClientCollection::getClientBySocket(int &fd) {
    _it = _clientMap.find(fd);
    if(_it != _clientMap.end())
        return _it->second;
    return NULL;
}


Client* ClientCollection::getClientByName(const string &name) {
    for(_it = _clientMap.begin(); _it != _clientMap.end(); _it++) {
        if (_it->second->getNickName() == name)
            return _it->second;
    }
    return NULL;
}
