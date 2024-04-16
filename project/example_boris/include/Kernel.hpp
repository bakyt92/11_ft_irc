#ifndef KERNEL_HPP
# define KERNEL_HPP

# include <iostream>
# include <fstream>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/poll.h>
# include <netdb.h>
# include <cstring>
# include <cstdio>
# include <fcntl.h>
# include <unistd.h>
# include <vector>
# include <map>
# include <set>
# include <csignal>
# include <ctime>
# include "Channel.hpp"
# include "Client.hpp"
# include "ClientCollection.hpp"
# include "ChannelCollection.hpp"
# include "Response.hpp"
# include "Server.hpp"
# include "ACommand.hpp"
# include "CommandProcessor.hpp"
# include "CommandPass.hpp"
# include "CommandNick.hpp"
# include "CommandUser.hpp"
# include "CommandOper.hpp"
# include "CommandQuit.hpp"
# include "CommandJoin.hpp"
# include "CommandPart.hpp"
# include "CommandMode.hpp"
# include "CommandTopic.hpp"
# include "CommandNames.hpp"
# include "CommandList.hpp"
# include "CommandInvite.hpp"
# include "CommandKick.hpp"
# include "CommandPrivmsg.hpp"
# include "CommandNotice.hpp"
 #include "CommandWho.hpp"
# include "CommandPing.hpp"
# include "CommandPong.hpp"
# include "CommandUserhost.hpp"
# include "CommandRestart.hpp"

# define TEST_COMMANDS 1;
# define MOTD_FILE_ERR   "Error. Can't open MOTD file!\n" 

#ifdef __APPLE__
# define IRC_NOSIGNAL SO_NOSIGPIPE
#else
# define IRC_NOSIGNAL MSG_NOSIGNAL
#endif


class ClientCollection;
class ChannelCollection;
class CommandProcessor;
class Server;

struct settings {
    int checkdelay;
    std::string pass;
    std::string serverName;
    std::string anotherNet;
    std::vector<std::string>	msgOfTheDay;
    std::map<std::string, std::string> oper;
};

class Kernel {
private:
	Kernel(Kernel const &source);
	Kernel &operator=(Kernel const &source);
    void loadSettings();
public:
    struct settings set;
    int _clientFD;
    ClientCollection *_clients;
    ChannelCollection *_channels;
    CommandProcessor *_cmdProc;
    Server *_server;
    Kernel();
    ~Kernel();
    static std::vector<std::string> splitString(std::string str, char delim);
    std::string getMOTD(const std::string& to);
	static bool isStringFitsMask(const string& mask, const string& str);
    bool contains(std::vector<Client*> list, Client* value);
    void sendToChanel(Channel *channel, Client *from, std::string msgEnd);
    void sendToClient(Client* receiver, std::string msg);
    void updateChannelOper(Channel* channel, Client *oldOper);
    std::string	itoa(int n);
};
#endif // KERNEL_HPP
