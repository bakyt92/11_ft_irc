#ifndef CLIENT_HPP
# define CLIENT_HPP
#include "Kernel.hpp"

using std::cout;
using std::endl;
using std::string;
using std::ostream;
using std::exception;

#define ACTIVE			0b1
#define INVISIBLE		0b10
#define RECEIPTNOTE		0b100
#define SERVER_NOTICE	0b1000
#define WALLOPS			0b10000
#define IRC_OPERATOR	0b100000
#define PASS_ENTERED	0b10000000

class Client {
 private:
    string 	            _nickName;
    string 	            _userName;
	int	                _ip;
    int                 _fd;
    string              _hostip;
    string				_realName;
    unsigned int		_flags;
	std::set<string>	_invitations;
    int                 _lastping;
    Client(const Client &client);
    Client& operator = (const Client &client);

 public:
    Client();
	Client(const string& name);
	~Client() {};

	const	string &getUserName() const;
	void	setUserName(const string &userName);
	string	getNickName();
    string  getRealName();
    int     getIp();
    int     getFd();
    int 	setNickName(const string& name);
    int		setRealName(const string& name);
    void    setHostIp(std::string hostip);
    string  getHostIp();
    void    setIp();
    void    setFd(int fd);
    void    setFlag(unsigned int flag);
    void    deleteFlag(unsigned int flag);
	void	addInvitation(const string& channelName);
    bool    isActive();
    bool    isIrcOperator();
	bool	getInvitation(const string& channelName);
    bool    isInvisible();
	bool	isPassEntered();
	bool	isServerNotice();
	bool	isWallOps();
	void    updateLastPing();
    time_t  getLastPing();
    void    setLastPing(time_t lastPing);
	bool	connectClient();
};
#endif  	//CLIENT_HPP
