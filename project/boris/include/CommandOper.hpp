#ifndef CommandOper_HPP
# define CommandOper_HPP
#include "Kernel.hpp"

class ACommand;
class ClientCollection;
class ChannelCollection;

class CommandOper : public ACommand {
private:
	CommandOper();
	CommandOper(CommandOper const &source);
	CommandOper &operator=(CommandOper const &source);
public:
	virtual ~CommandOper();
	std::string exec();

	CommandOper(Kernel *kernel);
};
#endif