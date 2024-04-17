#include "Server.hpp"

bool sigReceived = false;

int main(int argc, char *argv[]) {
  if (argc != 3) {   //if (atoi(argv[1].c_str()) < 1024 || atoi(argv[1].c_str()) > 49151) 
    std::cout << "Invalid arguments. Run ./ircserv <port> <password> (port number (should be between 1024 and 49151)\n";
    return 0;
  }
  Server s = Server(argv[1], argv[2]);
  s.init();
  s.run();
  return 0; 
}