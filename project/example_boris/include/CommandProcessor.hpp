#ifndef COMMANDPROCESSOR_HPP
# define COMMANDPROCESSOR_HPP
#include "Kernel.hpp"

#define COMMANDS_AMOUNT 32
class ACommand;
class Kernel;
class ClientCollection;
class ChannelCollection;

class CommandProcessor{
private:
	//поля
	static const char*					_command_names[COMMANDS_AMOUNT];
	Kernel								*_kernel;
	e_command							_type;
	std::vector<std::string>			_args;	// нулевой элемент массива - имя команды
	std::map<std::string, ACommand*>	_commandMap;

	//методы
    ACommand* getCommand(std::string const &message);
	void initCommands();
	CommandProcessor();
	CommandProcessor(CommandProcessor const &source);
	CommandProcessor &operator=(CommandProcessor const &source);
	static int getType(std::string const & message);
public:
    CommandProcessor(Kernel *kernel);
    ~CommandProcessor();
    std::string exec(std::string const &message);

	static const char **getCommandNames();
};

#endif
