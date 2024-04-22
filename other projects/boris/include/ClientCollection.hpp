#ifndef CLIENTCOLLECTION_HPP
# define CLIENTCOLLECTION_HPP
#include "Kernel.hpp"

using std::map;
using std::vector;

class Client;

class ClientCollection {
 public:
    ClientCollection();
    ~ClientCollection();

    vector<Client *> getListOfClient();
    vector<struct pollfd> getSockets();
    int createNewClient(int &fd, std::string hostip);
    int deleteClient(int &fd);

    Client* getClientBySocket(int &fd);
    Client* getClientByName(const string &name);

 private:
    map<int, Client* > _clientMap;
    map<int, Client* >::iterator _it;
    vector<struct pollfd> _sockets;
    ClientCollection(const ClientCollection &clientCollection);
    ClientCollection& operator = (const ClientCollection &clientCollection);
};

#endif  //CLIENTCOLLECTION_HPP