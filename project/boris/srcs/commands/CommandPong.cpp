#include "CommandPong.hpp"

CommandPong::CommandPong() : ACommand() {}
CommandPong::CommandPong(Kernel *kernel) : ACommand(kernel){}
CommandPong::~CommandPong() {}

std::string CommandPong::exec() {
	showMe();
	return "";
}
