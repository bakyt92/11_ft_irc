#include "ChannelCollection.hpp"

ChannelCollection::ChannelCollection() {
    _it = _channelCollection.begin();
}

ChannelCollection::ChannelCollection(const ChannelCollection &channelCollection) {
    *this = channelCollection;
}

ChannelCollection &ChannelCollection::operator = (const ChannelCollection &channelCollection) {
    if (this != &channelCollection) {
        *this = channelCollection;
    }
    return *this;
}

ChannelCollection::~ChannelCollection() {
    for (_it = _channelCollection.begin(); _it != _channelCollection.end(); _it++) {
        delete _it->second;
    }
}

Channel* ChannelCollection::getChannelByName(string const &channelName) {
    _it = _channelCollection.find(channelName);
    if (_it == _channelCollection.end())
        return NULL;
    return _it->second;
}

vector<string> ChannelCollection::getListOfChannels() {
	vector<string> _listOfChannels;
	for(_it = _channelCollection.begin(); _it != _channelCollection.end(); _it++) {
        _listOfChannels.push_back(_it->first);
    }
    return _listOfChannels;
}

void ChannelCollection::deleteChannel(string &channelName) {
    _it = _channelCollection.find(channelName);
    if(_it != _channelCollection.end())
        delete _it->second;
    _channelCollection.erase(_it);
}

void ChannelCollection::deleteChannel(Channel &channel) {
    string channelName = channel.getName();
    this->deleteChannel(channelName);
}

int ChannelCollection::createChannel(string &channelName, Client *creator) {
    if((channelName[0] != '&' && channelName[0] != '#') || 
        channelName.size() > 200 ||
        channelName.find(",") != string::npos ||
        channelName.find(" ") != string::npos)
        return -1;
    _channelCollection[channelName] = new Channel(channelName, creator);
    return 0;
}

vector<Channel *> ChannelCollection::getChannelPointers()
{
	vector<Channel *> channels;
	for(_it = _channelCollection.begin(); _it != _channelCollection.end(); _it++) {
		channels.push_back(_it->second);
	}
	return channels;
}

int ChannelCollection::deleteEmptyChannel(Channel &channel) {
	if(channel.getListOfClient().empty()) {
		deleteChannel(channel);
		return 0;
	}
	return 1;
}
