#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <limits>
#include <iostream>
#include <algorithm>
#include <vector>
#include "Kernel.hpp"

using std::cout;
using std::endl;
using std::exception;
using std::ostream;
using std::string;
using std::vector;

#define PRIVATE 0b00000001		/*p - private channel flag*/
#define SECRET 0b00000010		/*s - secret channel flag*/
#define INVITEONLY 0b00000100	/*i - invite-only channel flag*/
#define TOPICBYOPER 0b00001000	/*t - topic settable by channel operator only flag*/
#define MODERATED 0b00010000	/*m - moderated channel*/
#define NOMSGOUTSIDE 0b00100000 /*n - no messages to channel from clients on the outside*/
#define ISKEY 0b01000000

class Client;

class Channel {
 public:
	Channel(string channelName, Client *chanop);
	Channel(string channelName);
	~Channel();

	const string getName();
	long getChannelId();
	unsigned int getLimit();
	string getKey();
	unsigned int getPrivateFlag();
	unsigned int getSecretFlag();
	unsigned int getInviteOnlyFlag();
	unsigned int getTopicFlag();
	unsigned int getModeratedChannel();
	unsigned int getNoMsgOutsideFlag();
	unsigned int getFlags();
	/* MODE #ChannelName +b */
	vector<string> getListOfBanMask();

	/* methods handling of Client list */
	vector<Client *> &getListOfClient();
	const vector<Client *> &getListOfOper() const;
	const vector<Client *> &getListOfSpeaker() const;

	int setFlag(Client *client, unsigned int flag);
	int deleteFlag(Client *client, unsigned int flag);

	/* l - set the user limit to channel */
	int setLimit(Client *client, unsigned int limit);

	/*k - set a channel key (password)*/
	int setKey(Client *client, string key);
	unsigned int getKeyFlag();

	/*b - set a ban mask to keep users out*/
	int setBanMask(Client *client, string banMask);
	int removeBanMask(Client *client, string banMask);
	bool isBanned(string hostname);

	/*o - give/take channel operator privileges*/
	int giveChanopPriv(Client *clientFrom, Client *clientTo);
	int takeChanopPriv(Client *clientFrom, Client *clientTo);

	/*v - give/take the ability to speak on a moderated channel*/
	int giveSpeakPriv(Client *clientFrom, Client *clientTo);
	int takeSpeakPriv(Client *clientFrom, Client *clientTo);

	/* topic */
	int setTopic(const string &topic, Client *client);
	const string &getTopic() const;

	int addNewClient(Client *client);
	int deleteClient(Client *client);
	int isChanopNick(Client *client);
	int isSpeaker(Client *client);
	bool isContainList(vector<Client *> &list, Client *client);
	Client *findClient(Client *client);
	int sendMessageToChannel(int code, const string &answer);


private:
	string				_name;
	unsigned int		_limit;
	string				_key;
	string				_topic;
	unsigned int		_flags;
	vector<string>		_listBanMask;
	vector<Client *>	_listOfClient;
	vector<Client *>	_listOfOper;
	vector<Client *>	_listOfSpeaker;

	Channel();
	Channel(const Channel &channel);
	Channel &operator=(const Channel &channel);
};

#endif		//CHANNEL_HPP
