#include "Kernel.hpp"

Kernel *_k;

void signal_handler(int signal) {
	if (signal == SIGQUIT || signal == SIGTERM || signal == SIGINT)
		_k->_server->setStatus(0);
}

int main(int argc, char *argv[])
{
	std::string port;
	std::string password;
	
	if (argc < 3 || argc > 4) {
		std::cout << "Invalid arguments! Run ./ircserv <port> <password>\n";
		exit(1);
	}
	std::signal(SIGINT, signal_handler);
	port = (argc == 3) ? argv[1] : argv[2];
	password = (argc == 3) ? argv[2] : argv[3];
	_k = new Kernel();
	_k->set.pass = password;
	if (argc == 4)
		_k->set.anotherNet = argv[1];
	_k->_server->run(port);
	delete _k;
	return 0; 
}