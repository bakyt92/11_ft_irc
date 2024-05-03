#include "Server.hpp"

bool sigReceived;

Server::Server(string port_, string pass_) : port(port_), pass(pass_), fdsToEraseNextIteration(set<int>()) {}

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

// SOL_SOCKET = установка параметров на уровне сокета
// 1 настройка = (optname, optval, optlen)
// SO_KEEPALIVE отслеживаниe на серверной стороне клиентских соединений и их принудительного отключения
// create the socket in non-blocking mode https://stackoverflow.com/questions/1543466/how-do-i-change-a-tcp-socket-to-be-non-blocking
// ещё есть https://www.ibase.ru/keepalive/
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
  hints.ai_family   = AF_INET;      // the address family AF_INET =r IPv4
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;
  if(getaddrinfo(NULL, port.c_str(), &hints, &listRes))
    throw std::runtime_error("getaddrinfo error: [" + std::string(strerror(errno)) + "]");
  int optVal = 1;
  for(struct addrinfo* hint = listRes; hint != NULL; hint = hint->ai_next)
    if((fdForNewClis = socket(hint->ai_family, hint->ai_socktype | SOCK_NONBLOCK | SOCK_CLOEXEC| SOCK_NONBLOCK | SOCK_CLOEXEC, hint->ai_protocol)) < 0)
      throw std::runtime_error("function socket error: [" + std::string(strerror(errno)) + "]");
    else if(setsockopt(fdForNewClis, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)))
      throw std::runtime_error("setsockopt error: [" + std::string(strerror(errno)) + "]");
    else if(setsockopt(fdForNewClis, SOL_SOCKET, SO_REUSEPORT, &optVal, sizeof(optVal)))
      throw std::runtime_error("setsockopt error: [" + std::string(strerror(errno)) + "]");
    else if(bind(fdForNewClis, hint->ai_addr, hint->ai_addrlen)) {
      close(fdForNewClis);
      hint->ai_next == NULL ? throw std::runtime_error("bind error: [" + std::string(strerror(errno)) + "]") : perror("bind error");
    }
    else
      break ;
  freeaddrinfo(listRes);
  if(listen(fdForNewClis, SOMAXCONN))
    throw std::runtime_error("listen error: [" + std::string(strerror(errno)) + "]");
  struct pollfd pollForNewClis = {fdForNewClis, POLLIN, 0};
  polls.push_back(pollForNewClis);
}

// poll allows waiting for status updates on more than one socket in a single synchronous call
void Server::run() {
  std::cout << "Server is running. Waiting clients to connect >\n";
  while (sigReceived == false) {
    eraseUnusedClis();
    eraseUnusedChs();
    markPollsToSendMsgsTo();
    int countEvents = poll(polls.data(), polls.size(), 100);                       // наблюдаем за всеми сокетами сразу, есть ли там что-то для нас
    if (countEvents < 0)
      throw std::runtime_error("Poll error: [" + std::string(strerror(errno)) + "]");
    if(countEvents > 0)                                                            // в каких=то сокетах есть данные
      for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++) {
        if((poll->revents & POLLIN) && poll->fd == fdForNewClis) {                 // новый клиент подключился к сокету fdServ
          addNewClient(*poll);
          break;
        }
        else if((poll->revents & POLLIN) && poll->fd != fdForNewClis)              // клиент прислал нам сообщение через свой fdForMsgs
          receiveBufAndExecCmds(poll->fd);
        else if(poll->revents & POLLOUT)                                          // есть сообщения для отпраки клиентам
          sendPreparedResps(clis.at(poll->fd));
      }
  }
  std::cout << "Terminated\n";
}

// inet_ntoa() converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
void Server::addNewClient(pollfd poll) {
  struct sockaddr sa;
  socklen_t       saLen = sizeof(sa);
  int fdForMsgs = accept(poll.fd, &sa, &saLen);                                       // у каждого клиента свой fd для сообщений
  if(fdForMsgs == -1)
    return perror("accept");
  clis[fdForMsgs] = new Cli(fdForMsgs, inet_ntoa(((struct sockaddr_in*)&sa)->sin_addr));;
  struct pollfd pollForMsgs = {fdForMsgs, POLLIN, 0};
  polls.push_back(pollForMsgs);
  cout << "*** New cli (fd=" + static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (fdForMsgs) )).str() + ")\n\n";
}

void Server::receiveBufAndExecCmds(int fd) {
  if(!(cli = clis.at(fd)))
    return ;
  vector<unsigned char> buf0(BUFSIZE); // std::vector is the recommended way of implementing a variable-length buffer in C++
  for(size_t i = 0; i < buf0.size(); i++)
    buf0[i] = '\0';
  int nbBytesReallyReceived = recv(cli->fd, buf0.data(), buf0.size() - 1, MSG_NOSIGNAL | MSG_DONTWAIT);
  if(nbBytesReallyReceived < 0)
    perror("recv");                                                                  // ошибка, но возможно клиент ещё тут
  else if(nbBytesReallyReceived == 0)                                                                // клиент пропал
    fdsToEraseNextIteration.insert(cli->fd);
  else {
    string buf = string(buf0.begin(), buf0.end());
    buf.resize(nbBytesReallyReceived);
    if(buf.substr(0, 4) != "PING")
      cout << "\n" << withoutRN("I have received from " + static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (cli->fd))).str() + " buf: [" + buf + "] -> [" + cli->bufRecv + buf + "]") << "\n";
    buf = cli->bufRecv + buf;
    std::vector<string> cmds = splitBufToCmds(buf);
    for(std::vector<string>::iterator cmd = cmds.begin(); cmd != cmds.end(); cmd++) {
      //vector<string>().swap(ar); // попробовать убрать
      ar.clear();
      ar = splitCmdToArgs(*cmd);
      cout << infoCmd();
      execCmd();
    }
    cout << infoServ();
  }
}