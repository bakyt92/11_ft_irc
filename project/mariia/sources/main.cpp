#include "../headers/Server.hpp"

static bool running = true;

void signalHandler(int signum) {
    (void) signum;
	std::cout << "[QUITTING]" << std::endl;
	running = false;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		std::cerr << "ERROR! Usage: " << argv[0] << " <port> <_password>"
				  << std::endl;
		return 1;
	}
	try {
		Server server (atoi(argv[1]), std::string(argv[2]));
		signal(SIGINT, signalHandler);
		while (running) {
			try {
				server.run();
			} catch (std::exception &exception) {
				std::cerr << "[ERROR]\t" << exception.what() << std::endl;
			}
		}
	} catch (std::exception &exception) {
		std::cerr << "[ERROR]\t" << exception.what() << std::endl;
		return 1;
	}

	return 0;
}
