#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <set>
#include <queue>


enum Mode {
    AWAY = 0b000001, // a: user is flagged as away
    INVISIBLE = 0b001000, // i: marks a users as invisible
    UNKNOWN
};

class Client {
    private:
        int         _socketFd;
        bool        _logged;
        bool        _registered;
        unsigned int _modes;
        std::string _nickname;
        std::string _username;
		std::string _realName;
        std::string _password;
		std::string _hostname;
		std::string	_recvBuffer;
		std::queue<std::string> _sendQueue;
		std::string _awayMessage;
		std::vector<std::string> _channels;
		bool 		_quit;

    public:
        Client(int socket, std::string hostname);
        ~Client();

        const std::string &getNickname() const;
        const std::string &getUsername() const;
        const std::string &getPassword() const;
		const std::string &getHostname() const;
        int getSocket() const;
		const std::string &getAwayMessage() const;
		const std::vector<std::string> &getChannels() const;
		bool isQuit() const;
		void setQuit(bool quit);
	    bool isRegistered() const;
        bool isLogged() const;
        void setNickname(const std::string &nickname);
        void setUsername(const std::string &username);
        void setPassword(const std::string &password);
        void setRegistration();
		void setRealName(const std::string &real_name);
		void setLog();
        void setAwayMessage(const std::string &away);
        void addMode(Mode mode);
        void removeMode(Mode mode);
        bool activeMode(Mode mode) const;
        Mode getMode(const std::string &mode);
        std::string returnModes();
		std::string getRecvBuffer();
		void appendRecvBuffer(std::string recv);
		void resetRecvBuffer();
		bool isRecvBufferEmpty();
		std::string getRealName() const;
		void pushSendQueue(std::string send);
		std::string popSendQueue();
		bool sendQueueEmpty();
		void addChannel(const std::string &channel);
		void removeChannel(const std::string &channel);
};

#endif
