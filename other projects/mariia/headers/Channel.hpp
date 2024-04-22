#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <set>

#define TOPICSET     0b000001 // if set topic is settable by channel operator only
#define INVITEONLY      0b000010 // if set clients can join only if invited
#define KEYSET            0b000100 // if set clients can join only with password
#define LIMITSET        0b001000 // if set no more clients than limit value can join

class Channel {
    private:
        std::string _name;
        std::string _topic;
        unsigned int _mode;
        std::string _password;
        std::set<int> _memberFds;
        std::set<int> _operatorFds;
        std::set<int> _invitedFds;
        int _limitMembers;

    public:
        Channel(const std::string &name, std::string &password);
        ~Channel();
        const std::string &getName() const;
        const std::string &getTopic() const;
        std::string getModeString() const;
        std::string getModeStringWithParameters() const;
        const std::set<int> &getMemberFds() const;
        int getLimitMembers() const;
        void setTopic(const std::string &topic);
        void setPassword(const std::string &password);
        void setLimitMembers(int limitMembers);
        bool setMode(unsigned int mode);
        bool unsetMode(unsigned int mode);
        bool isModeSet(unsigned int mode) const;
        void addMember(int clientFd);
        void removeMember(int clientFd);
        bool hasMember(int clientFd);
        bool addOperator(int clientFd);
        bool removeOperator(int clientFd);
        bool hasOperator(int clientFd) const;
        void addInvited(int clientFd);
        void removeInvited(int clientFd);
        bool hasInvited(int clientFd);
        bool authMember(int clientFd, std::string &password);
};


#endif
