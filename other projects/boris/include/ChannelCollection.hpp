#ifndef CHANNELCOLLECTION_HPP
# define CHANNELCOLLECTION_HPP
#include "Kernel.hpp"

using std::map;
using std::vector;

class Channel;

class ChannelCollection {
 public:
    ChannelCollection();
    ~ChannelCollection();
    
    vector<string> getListOfChannels();
	vector<Channel *> getChannelPointers();
    void deleteChannel(Channel &channel);
    void deleteChannel(string &channelName);
    int createChannel(string &channelName, Client *creator);
	int deleteEmptyChannel(Channel &channel);
    Channel* getChannelByName(string const &channelName);

 private:
    map<const string, Channel*> _channelCollection;
    map<const string, Channel*>::iterator _it;
    ChannelCollection(const ChannelCollection &channelCollection);
    ChannelCollection& operator = (const ChannelCollection &channelCollection);
};
#endif		//CHANNELCOLLECTION_HPP
