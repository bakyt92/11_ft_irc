#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <map>
#include <vector>
#include <poll.h>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <queue>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <iomanip>

#include "Client.hpp"
#include "Channel.hpp"

class Channel;

enum serverRep {
	CAPLS = 0,
	RPL_WELCOME = 1,
	RPL_YOURHOST = 2,
	RPL_CREATED = 3,
	RPL_MYINFO = 4,
	ERROR = 6,
	RPL_UMODEIS = 221,
	RPL_AWAY = 301,
	RPL_UNAWAY = 305,
	RPL_NOWAWAY = 306,
	RPL_ENDOFWHOIS = 318,
	RPL_ENDOFWHO = 315,
	RPL_LIST = 322,
	RPL_LISTEND = 323,
	RPL_CHANNELMODEIS = 324,
	RPL_NOTOPIC = 331,
	RPL_TOPIC = 332,
	RPL_INVITING = 341,
	RPL_WHOREPLY = 352,
	RPL_NAMREPLY = 353,
	RPL_ENDOFNAMES = 366,
	RPL_ENDOFBANLIST = 368,
	ERR_NOSUCHNICK = 401,
	ERR_NOSUCHSERVER = 402,
	ERR_NOSUCHCHANNEL = 403,
	ERR_CANNOTSENDTOCHAN = 404,
	ERR_TOOMANYCHANNELS = 405,
	ERR_TOOMANYTARGETS = 407,
	ERR_NOORIGIN = 409,
	ERR_NORECIPIENT = 411,
	ERR_NOTEXTTOSEND = 412,
	ERR_UNKNOWNCOMMAND = 421,
	ERR_NONICKNAMEGIVEN = 431,
	ERR_ERRONEUSNICKNAME = 432,
	ERR_NICKNAMEINUSE = 433,
	ERR_USERNOTINCHANNEL = 441,
	ERR_NOTONCHANNEL = 442,
	ERR_USERONCHANNEL = 443,
	ERR_NEEDMOREPARAMS = 461,
	ERR_ALREADYREGISTERED = 462,
	ERR_PASSWDMISMATCH = 464,
	ERR_CHANNELISFULL = 471,
	ERR_UNKNOWNMODE = 472,
	ERR_INVITEONLYCHAN = 473,
	ERR_BADCHANNELKEY = 475,
	ERR_CHANOPRIVSNEEDED = 482,
	ERR_UMODEUNKNOWNFLAG = 501,
	ERR_USERSDONTMATCH = 502
};

static const int MAXCHANNELS = 9; // max number of channels a client can join
static const int MAXTARGETS = 10; // max number of unique targets for commands with targets

class Server {
public:
	typedef std::map<std::string, void (Server::*)(int,
												   const std::vector<std::string> &)> Cmd;
	typedef std::map<std::string, void (Server::*)(int,
												   const std::vector<std::string> &)>::iterator CmdIterator;
	typedef std::map<char, bool (Server::*)(char, const std::string &,
											Channel *, int)> ModeHandler;
	typedef std::map<char, bool (Server::*)(char, const std::string &,
											Channel *,
											int)>::iterator ModeHandlerIterator;
	Server() {};
	Server(int port, const std::string &password);
	~Server();
	static std::string uncapitalizeString(const std::string &input);

	void run();
private:
	std::map<int, std::string> _serverMessages;
	std::string getNick(int fd);
	std::string getNickAndHostname(int fd);
	Client &getClient(int fd);
	int socketFd;
	time_t start;
	sockaddr_in address;
	std::string _password;
	std::string serverName;
	std::string serverVersion;
	std::vector<pollfd> pollFds;
	std::map<int, Client *> clients;
	std::vector<Channel *> _channels;
	char _buffer[1024];
	Cmd cmd;
	ModeHandler channelMode;

	std::map<std::string, std::string> users;
	void initCmd();
	void initChannelMode();
	void initServerMessages();
	Client *findClient(const std::string &nickname);
	Client *findClient(int fd);
	void addClient(int clientSocket, std::string clientHostname);
	void removeClient(int clientSocket);
	void listenPort() const;
	std::pair<int, std::string> acceptConnection();
	bool parsBuffer(int fd);
	bool registrationProcess(int fd, std::vector<std::string> &tokens);
	bool checkRegistration(int fd);
	bool handleCommand(int fd, const std::string &command,
					   const std::vector<std::string> &params);
	void processCmd(int fd, std::vector<std::string> &tokens);
	bool verifyNickname(int fd, const std::string &arg);
	bool verifyPassword(int fd, const std::string &arg);
	bool verifyUsername(int fd, const std::string &arg);
	void serverSendReply(int fd, const std::string &token, serverRep id,
						 const std::string &reply);
	void serverSendError(int fd, const std::string &token, serverRep id);
	void serverSendNotification(int fd, const std::string &prefix,
								const std::string &command,
								const std::string &parameters);
	void
	serverSendNotification(const std::set<int> &fds, const std::string &prefix,
						   const std::string &command,
						   const std::string &parameters);
	void serverSendMessage(int fd, const std::string &message);

	// Commands
	void processPrivmsg(int fd, const std::vector<std::string> &tokens);
	void processJoin(int fd, const std::vector<std::string> &tokens);
	void processInvite(int fd, const std::vector<std::string> &tokens);
	void processKick(int fd, const std::vector<std::string> &tokens);
	void processTopic(int fd, const std::vector<std::string> &tokens);
	void processPart(int fd, const std::vector<std::string> &tokens);
	void processMode(int fd, const std::vector<std::string> &tokens);
	void processChannelMode(int fd, const std::vector<std::string> &tokens);
	void processUserMode(int fd, const std::vector<std::string> &tokens);
	void processNames(int fd, const std::vector<std::string> &tokens);
	void processList(int fd, const std::vector<std::string> &tokens);
	void processPing(int fd, const std::vector<std::string> &tokens);
	void processAway(int fd, const std::vector<std::string> &tokens);
	void processNick(int fd, const std::vector<std::string> &tokens);
	void processQuit(int fd, const std::vector<std::string> &tokens);
	void processWho(int fd, const std::vector<std::string> &tokens);
	void processWhois(int fd, const std::vector<std::string> &tokens);
	bool handleModeT(char set, const std::string &parameter, Channel *channel,
					 int fd);
	bool handleModeI(char set, const std::string &parameter, Channel *channel,
					 int fd);
	bool handleModeK(char set, const std::string &parameter, Channel *channel,
					 int fd);
	bool handleModeL(char set, const std::string &parameter, Channel *channel,
					 int fd);
	bool handleModeO(char set, const std::string &parameter, Channel *channel,
					 int fd);
	bool handleModeB(char set, const std::string &parameter, Channel *channel,
					 int fd);
	void addChannel(Channel *channel);
	void removeChannel(const std::string &channelName);
	void removeClientFromChannel(int fd, Channel *channel);
	Channel *findChannel(const std::string &name);
	std::vector<Channel *> findChannels(std::queue<std::string> names);
	bool isValidChannelName(const std::string &name);
	void joinExistingChannel(int fd, Channel *channel, std::string password);
	void createAndJoinNewChannel(int fd, std::string channelName,
								 std::string password);
	void listChannels(int fd, std::vector<Channel *> &channels);
	std::vector<std::string> getClientsWithoutChannels();
	std::map<std::string, std::vector<std::string> >
	getClientsOfChannels(int fd, std::vector<Channel *> channels);
	std::vector<std::string> getAllChannelMembersNicks(const Channel *channel);
	std::vector<std::string>
	getVisibleChannelMembersNicks(const Channel *channel);
	void sendData(size_t index);
	size_t receiveData(size_t index);
	void resetEvents(size_t index);
	static std::string
	mergeTokensToString(const std::vector<std::string> &tokens,
						bool removeColon);
	void sendJoinNotificationsAndReplies(int fd, const Channel *channel);
	bool checkPmTokens(int fd, const std::vector<std::string> &tokens);
	void
	sendPmToChan(int fd, const std::string &message, const std::string &prefix,
				 const std::string &targetName,
				 const std::string &command);
	void
	sendPmToUser(int fd, const std::string &message, const std::string &prefix,
				 const std::string &targetName,
				 const std::string &command);
	static std::queue<std::string>
	split(const std::string &src, char delimiter, bool unique);
	bool modeParameterNeeded(char set, char mode);
	static bool isValidName(const std::string &name);
	std::string paddDigits(int i);
	static bool isNum(const std::string &str);
	static bool isBitMask(const std::string &str);
	static Mode getBitMode(const std::string str);
	std::pair<std::string, std::string>
		takeFullClientInfo(Client *client, Channel *channel);
};

#endif
