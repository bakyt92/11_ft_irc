#include "Server.hpp"

bool sigReceived;

Server::Server(string port_, string pass_) : port(port_), pass(pass_), fdsToErase(set<int>()) {}

Server::~Server() {
  for(map<string, Ch*>::iterator it = chs.begin(); it != chs.end(); it++)
    delete it->second;
  for(map<int, Cli*> ::iterator it = clis.begin(); it != clis.end(); it++) {
    close(it->first);
    delete it->second;
  }
  clis.clear();
  polls.clear();
  close(fdForNewClis);
}

void Server::sigHandler(int sig) {
  cout << endl << "Signal Received\n";
  sigReceived = true;
  (void)sig;
}

void Server::init() {
  try {
    signal(SIGINT,  sigHandler); // catch ctrl + c
    signal(SIGQUIT, sigHandler); // catch ctrl + '\'
    signal(SIGTERM, sigHandler); // catch kill command
   }
  catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  sigReceived = false;
  struct addrinfo hints, *listRes;
  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;
  if(getaddrinfo(NULL, port.c_str(), &hints, &listRes))
    throw std::runtime_error("getaddrinfo error: [" + std::string(strerror(errno)) + "]");
  int notUsed = 1;
  for(struct addrinfo* hint = listRes; hint != NULL; hint = hint->ai_next) {
    if((fdForNewClis = socket(hint->ai_family, hint->ai_socktype, hint->ai_protocol)) < 0)
      throw std::runtime_error("function socket error: [" + std::string(strerror(errno)) + "]");
    else if(setsockopt(fdForNewClis, SOL_SOCKET, SO_REUSEADDR , &notUsed, sizeof(notUsed)))
      throw std::runtime_error("setsockopt error: [" + std::string(strerror(errno)) + "]");
    else if(bind(fdForNewClis, hint->ai_addr, hint->ai_addrlen)) { 
      close(fdForNewClis);
      hint->ai_next == NULL ? throw("bind error: [" + std::string(strerror(errno)) + "]") : perror("bind error"); // tested
    }
    else
      break ;
  }
  freeaddrinfo(listRes);
  if(listen(fdForNewClis, SOMAXCONN))
    throw std::runtime_error("listen error: [" + std::string(strerror(errno)) + "]");
  struct pollfd pollForNewClis = {fdForNewClis, POLLIN, 0};
  polls.push_back(pollForNewClis);
}

void Server::run() {
  std::cout << "Server is running. Waiting clients to connect >>>\n";
  while (sigReceived == false) {
    eraseUnusedPolls();
    markClienToSendMsgsTo();
    int countEvents = poll(polls.data(), polls.size(), 1000);                        // наблюдаем за всеми сокетами сразу, есть ли там что-то для нас
    if (countEvents < 0)
      throw std::runtime_error("Poll error: [" + std::string(strerror(errno)) + "]");
    if(countEvents > 0) {                                                            // в каких=то сокетах есть данные
      for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)  // новый клиент подключился к сокету fdServ
        if((poll->revents & POLLIN) && poll->fd == fdForNewClis) {
          addNewClient(*poll);
          break;
        }
        else if((poll->revents & POLLIN) && poll->fd != fdForNewClis)                // клиент прислал нам сообщение через свой fdForMsgs
          receiveMsgAndExecCmds(poll->fd);
        else if (poll->revents & POLLOUT)                                            // есть сообщение для отпраки какому-то клиенту
          sendPreparedResps(clis.at(poll->fd));
    }
  }
  std::cout << "Terminated\n";
}

void Server::addNewClient(pollfd poll) {
  struct sockaddr sa;
  socklen_t       saLen = sizeof(sa);
  int fdForMsgs = accept(poll.fd, &sa, &saLen);                                       // у каждого клиента свой fd для сообщений
  if(fdForMsgs == -1)
    return perror("accept");
  struct Cli *newCli = new Cli(fdForMsgs, inet_ntoa(((struct sockaddr_in*)&sa)->sin_addr));
  clis[fdForMsgs] = newCli;
  struct pollfd pollForMsgs = {fdForMsgs, POLLIN, 0};
  polls.push_back(pollForMsgs);
  cout << "New cli (fd=" + static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (fdForMsgs) )).str() + ")\n";
}

void Server::receiveMsgAndExecCmds(int fd) {
  if(!(cli = clis.at(fd)))
    return ;
  vector<unsigned char> buf0(513);
  for(size_t i = 0; i < buf0.size(); i++)
    buf0[i] = '\0';
  int bytes = recv(cli->fd, buf0.data(), buf0.size() - 1, MSG_NOSIGNAL);
  if(bytes < 0)
    perror("recv");                                                                       // ошибка, но не делаем execQuit(), возможно клиент ещё тут
  else if(bytes == 0)                                                                     // клиент пропал
    eraseCli(cli->nick);
  else {
    string buf = string(buf0.begin(), buf0.end());
    buf.resize(bytes);
    cout << without_r_n("I have received buf       : [" + buf + "] -> [" + cli->bufRecv + buf + "]") << "\n";
    buf = cli->bufRecv + buf;
    std::vector<string> cmds = split_r_n(buf);
    for(std::vector<string>::iterator cmd = cmds.begin(); cmd != cmds.end(); cmd++) {
      vector<string>().swap(ar); // попробовать убрать
      ar = split_space(*cmd);
      cout << infoCmd();
      execCmd();
    }
    cout << infoServ() << endl;
  }
}