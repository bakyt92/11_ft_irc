#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Server.hpp"

Server::Server() { }

Server::Server(Kernel *kernel) {_kernel = kernel;}

Server::Server(Server const &source) {*this = source; }

Server &Server::operator=(Server const &source) {
	if (this != &source) {}
	return *this;
}

Server::~Server() {}

void Server::setStatus(int status) {_status = status;}

void raiseError(const char *pErrMessage, bool sysError, bool fatal) {
    if (sysError)
        perror(pErrMessage);
    else
        std::cerr << pErrMessage;
    if (fatal)
        exit(EXIT_FAILURE);
}

std::vector<std::string> splitCommand(unsigned char *command){
    std::string printable;
	int i = -1;
    while (command[++i])
        if (command[i] > 31 || command[i] == 10)
            printable += command[i];
    std::vector<std::string> commandList;
    int _startPos = 0;
    int _endPos = 0;
    while ((_endPos = printable.find("\n", _startPos)) > -1) {
        int offset = (_endPos > 0 && printable[_endPos - 1] == '\r');
        std::string part = printable.substr(_startPos, _endPos - _startPos - offset);
        commandList.push_back(part);
        _startPos = _endPos + 1 + offset;
    }
    return commandList;
}

void Server::init(std::string const &port) {
    int _opt = 1;
    struct addrinfo _set;
    struct addrinfo *_addrinfo;
    struct addrinfo *_ai;
    std::memset(&_set, 0, sizeof(_set));
    _set.ai_family = AF_INET;
    _set.ai_socktype = SOCK_STREAM;
    _set.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port.c_str(), &_set, &_addrinfo))
        raiseError("getaddrinfo", true, true);
    for (_ai = _addrinfo; _ai != NULL; _ai=_ai->ai_next) {
        if ((_fd = socket(_ai->ai_family, _ai->ai_socktype, _ai->ai_protocol)) < 0) {
            raiseError("socket", true, false);
            continue;
        }
        if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR , &_opt, sizeof(_opt)))
            raiseError("setsockopt", true, true);
        if (bind(_fd, _ai->ai_addr, _ai->ai_addrlen)) {
            close(_fd);
            continue;
        }
        break ;
    }
    if (_ai == NULL)
        raiseError("bind", true, true);
    freeaddrinfo(_addrinfo);
}

void Server::run(std::string const &port) {
    _status = 1;
    if (atoi(port.c_str()) < 1024)
        raiseError("Wrong port number!\n", false, true);
    init(port);
    if (fcntl(_fd, F_SETFL, O_NONBLOCK))
        raiseError("set nonblock", true, true);
    if (listen(_fd, 10))
        raiseError("listen", true, true);
    _kernel->_clients->createNewClient(_fd, "127.0.0.1");
    Client *server = _kernel->_clients->getClientBySocket(_fd);
    server->setNickName(_kernel->set.serverName);
    server->setUserName(_kernel->set.serverName);
    server->setRealName(_kernel->set.serverName);
    std::cout << "Server is running. Waiting clients to connect >>>\n";
    while (_status > 0) {
        checkSockets();
        usleep(1000);
        checkClients();
    }
    if (_status < 0) {
        restart();
        run(port);
    }
    std::cout << "Terminating...\n";
}

void Server::removeClient(int fd) {
    _kernel->_clientFD = fd;
    _kernel->_cmdProc->exec("QUIT");
}

void Server::checkClients() {
    std::vector<Client *> _clList = _kernel->_clients->getListOfClient();
    time_t now = time(0);
    int _clInactive;
    int _lastPing;
    for (unsigned long i = 1; i < _clList.size(); i++) {
        _lastPing = _clList[i]->getLastPing();
        _clInactive = now - (_lastPing * (_lastPing < 0 ? -1 : 1));
        if (_lastPing < 0 && _clInactive < _kernel->set.checkdelay + 1)
            continue;
        if (_clInactive > _kernel->set.checkdelay)
            removeClient(_clList[i]->getFd());
        else if (_clInactive > 30) {
            std::string msg = ":" + _kernel->set.serverName + " PING :" + _kernel->set.serverName + "\n";
            send(_clList[i]->getFd(), msg.c_str(), msg.size(), IRC_NOSIGNAL);
            _clList[i]->setLastPing(_lastPing * -1);
        }
    }
}

int Server::readCommand(int client_fd) {
    unsigned char buffer[1024] = {0};
    int bytesread = recv(client_fd, buffer, 1024, 0);
    if (bytesread <= 0) {
        if (bytesread < 0)
            raiseError("read", true, false);
        return bytesread;
    }
    std::vector<std::string> commandList = splitCommand(buffer);
    std::vector<std::string>::iterator iter = commandList.begin();
    while (iter != commandList.end()) {
        _kernel->_clientFD = client_fd;
        std::string cmd = iter->c_str();
        std::string response;
        if (cmd.size() > 512)
            response = _kernel->_cmdProc->exec(cmd.substr(512));
        else
		{
			timeval start;
			timeval end;
			gettimeofday(&start, NULL);
			response = _kernel->_cmdProc->exec(cmd);
			gettimeofday(&end, NULL);
			std::cout << "delta usec = " << end.tv_sec * 1000000 + end.tv_usec
			- start.tv_sec * 1000000 - start.tv_usec << std::endl;
		}
		if (response != "")
            send(client_fd, response.c_str(), response.size(), 0);
        std::cout << response << std::endl;
        iter++;
    }
    return bytesread;
}

void Server::checkSockets() {
    std::vector<struct pollfd> _sockets = _kernel->_clients->getSockets();
    int ask = poll(_sockets.data(), _sockets.size(), 1);
    if (ask < 1)
        return ;
    for (unsigned long i = 0; i < _sockets.size(); i++) {
        if (_sockets[i].revents & POLLIN) {
            if (_sockets[i].fd == _fd) {
                struct sockaddr _sockaddr;
                socklen_t _sockaddr_len = sizeof(_sockaddr);
                int _client_fd = accept(_fd, &_sockaddr, &_sockaddr_len);
                std::string _ip_addr = inet_ntoa(((struct sockaddr_in*)&_sockaddr)->sin_addr);
                if (_client_fd < 0)
                    raiseError("accept", true, false);
                else
                    _kernel->_clients->createNewClient(_client_fd, _ip_addr);
            }
            else if (!(readCommand(_sockets[i].fd)))
                removeClient(_sockets[i].fd);
        }
    }
    return ;
}

void Server::restart() {
    std::cout << "Restarting...\n";
    delete _kernel->_clients;
    delete _kernel->_channels;
    delete _kernel->_cmdProc;
    _kernel->_clientFD = 0;
    _kernel->_clients = new ClientCollection();
    _kernel->_channels = new ChannelCollection();
    _kernel->_cmdProc = new CommandProcessor(_kernel);
    std::cout << "Server restarted!\n";
}
