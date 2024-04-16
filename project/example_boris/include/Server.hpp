#ifndef SERVER_HPP
# define SERVER_HPP
#include "Kernel.hpp"
class Kernel;

class Server {
private:
    int _fd;
    int _status;
    Kernel *_kernel;
    Server();
	Server(Server const &source);
	Server &operator=(Server const &source);
    void checkSockets();
    void checkClients();
    int readCommand(int client_fd);
    void init(std::string const &port);
    void removeClient(int fd);
public:
    Server(Kernel *kernel);
    ~Server();
    void run(std::string const &port);
    void restart();
    void setStatus(int status);
};
#endif