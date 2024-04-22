#ifndef ACOMMAND_HPP
# define ACOMMAND_HPP
# include "Kernel.hpp"

class ACommand;
class Kernel;
class ClientCollection;
class ChannelCollection;

enum	e_command
{
	CMD_PASS,
	CMD_NICK,
	CMD_USER,
	CMD_OPER,
	CMD_QUIT,
	CMD_JOIN,
	CMD_PART,
	CMD_MODE,
	CMD_TOPIC,
	CMD_NAMES,
	CMD_LIST,
	CMD_INVITE,
	CMD_KICK,
	CMD_PRIVMSG,
	CMD_NOTICE,
	CMD_WHO,
	CMD_KILL,
	CMD_PING,
	CMD_USERHOST,
	CMD_PONG,
	CMD_RESTART,
	CMD_NONE
};

// Родитель для всех команд клиента
class ACommand {
 protected:
	Kernel						*_kernel;
	Client						*_client;
	std::string					_message;
	std::string					_prefix;
	e_command					_type;
	std::vector<std::string>	_args; // нулевой элемент - имя команды
	
	std::string	getPrefix();
	void cropSpaces();
	void getArg();
	void parseMessage();
	ACommand(){};
	void showMe() const;

 public:
	void updateCommand(const std::string &, e_command);
	ACommand(Kernel *kernel);
	virtual ~ACommand() {};
	const std::string &getCommandName() const;
	virtual std::string exec() = 0;
};
#endif		//ACOMMAND_HPP
