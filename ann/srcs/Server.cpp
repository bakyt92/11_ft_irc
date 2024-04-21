#include "Server.hpp"
////////////////////////////////////////////////////////////////////////////// UTILS
Server::~Server() {
  // close all fd 
  // free clis, chs
}

void Server::sigHandler(int sig) {
  std::cout << std::endl << "Signal Received" << std::endl;
  sigReceived = true;
  (void)sig;
}

std::vector<string> split(string s, char delim) {
  std::vector<string> parts;
  for (size_t pos = s.find(delim); pos != string::npos; pos = s.find(delim)) {
    if (pos > 0) 
      parts.push_back(s.substr(0, pos));
    s.erase(0, pos + 1);
  }
  if (s.size()>0) 
    parts.push_back(s);
  return parts;
}

Cli* Server::getCli(string &name) {
  for(map<int, Cli* >::iterator cli = clis.begin(); cli != clis.end(); cli++)
    if (cli->second->nick == name)
      return cli->second;
  return NULL;
}

int send_(Cli *cli, string msg) {
  if (msg != "")
    send(cli->fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
  return 0;
}

int send_(set<Cli*> clis, string msg) {
  for (set<Cli*>::iterator cli = clis.begin(); cli != clis.end(); cli++) 
    send_(*cli, msg);
  return 0;
}

int send_(map<int, Cli*> clis, string msg) {
  for (map<int, Cli*>::iterator cli = clis.begin(); cli != clis.end(); cli++) 
    send_(cli->second, msg);
  return 0;
}

int send_(Ch *ch, string msg) {
  for (set<Cli*>::iterator cli = ch->clis.begin(); cli != ch->clis.end(); cli++) 
    send_(*cli, msg);
  return 0;
}

void Server::printMe() { // for debugging only
  cout << "I execute  : ";
  for (vector<string>::iterator it = args.begin(); it != args.end(); it++)
    cout << *it << " ";
  cout << endl << "My clients : ";
  for (map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); it++)
    cout << "[" << it->second->nick << "] ";
  cout << endl << "My channels: ";
  for (map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++) {
    cout << ch->first << " (";
    for (set<Cli*>::iterator itCli = ch->second->clis.begin(); itCli != ch->second->clis.end(); itCli++)
      cout << (*itCli)->nick << " ";
    cout << " topic = " << ch->second->topic << ", mode = " << ") ";
  }
  cout << endl << endl;
}

/////////////////////////////////////////////////////////////////////// PRINCIPAL LOOP
Server::Server(string port_, string pass_) : port(port_), pass(pass_) {}

void Server::init() {
  try {
    signal(SIGINT,  sigHandler); // SIGQUIT ?
  }
  catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  sigReceived = false;
  struct addrinfo ai;
  std::memset(&ai, 0, sizeof(ai));
  ai.ai_family   = AF_INET;
  ai.ai_socktype = SOCK_STREAM;
  ai.ai_flags    = AI_PASSIVE;
  struct addrinfo *list_ai;
  if (getaddrinfo(NULL, port.c_str(), &ai, &list_ai))
    throw(std::runtime_error("getaddrinfo"));
  struct addrinfo *it = NULL;
  int notUsed = 1;
  for (it = list_ai; it != NULL; it = it->ai_next) {
    if ((fdForNewClis = socket(it->ai_family, it->ai_socktype, it->ai_protocol)) < 0) 
      throw(std::runtime_error("socket"));
    else if (setsockopt(fdForNewClis, SOL_SOCKET, SO_REUSEADDR , &notUsed, sizeof(notUsed)))
      throw(std::runtime_error("setsockopt"));
    else if (bind(fdForNewClis, it->ai_addr, it->ai_addrlen)) { 
      close(fdForNewClis);
      it->ai_next == NULL ? throw(std::runtime_error("bind")) : perror("bind"); 
    }
    else
      break ;
  }
  freeaddrinfo(list_ai);
  if (listen(fdForNewClis, 10))
    throw(std::runtime_error("listen"));
  struct pollfd pollForNewClis = {fdForNewClis, POLLIN, 0};
  polls.push_back(pollForNewClis);
}

void Server::run() {
  std::cout << "Server is running. Waiting clients to connect >>>\n";
  while (sigReceived == false) {
    if (poll(polls.data(), polls.size(), 1) > 0) {              // poll() в сокетах есть какие-то данные
      for (std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++) // check sockets
        if ((poll->revents & POLLIN) && poll->fd == fdForNewClis) {     // новый клиент подключился к сокету fdServ
          struct sockaddr sa; 
          socklen_t       saLen = sizeof(sa);
          fdForMsgs = accept(poll->fd, &sa, &saLen);
          if (fdForMsgs == -1)
            perror("accept");
          else {
            struct Cli *newCli = new Cli(fdForMsgs, inet_ntoa(((struct sockaddr_in*)&sa)->sin_addr)); 
            clis[fdForMsgs] = newCli;
            struct pollfd pollForMsgs = {fdForMsgs, POLLIN, 0};
            polls.push_back(pollForMsgs);
          }
          break ;
        }
        else if ((poll->revents & POLLIN) && poll->fd != fdForNewClis) { // клиент прислал сообщение в свой fdForMsgs
          if (!(cli = clis.at(poll->fd)))
            continue ;
          vector<unsigned char> buf(512);
          int bytes = recv(cli->fd, buf.data(), buf.size(), 0); 
          if (bytes < 0) 
            perror("recv");
          else if (bytes == 0)                                         // клиент отключился
            execQuit();
          else {
            string bufS = string(buf.begin(), buf.end());
            bufS.resize(bytes);
            std::vector<string> cmds = split(bufS, '\n');             // if empty ? 
            for (std::vector<string>::iterator cmd = cmds.begin(); cmd != cmds.end(); cmd++) {
              args = split(*cmd, ' ');                                // if (args[0][0] == ':') args.erase(args.begin()); // нужен ли префикс?
              if (cli) {
                exec();
                printMe();
              }
            }
          }
        }
    }
    usleep(1000);
  }
  std::cout << "Terminated\n";
}

//////////////////////////////////////////////////////////////////////////////////////// IRC COMMANDS
int Server::exec() {
  if (args[0] == "PASS")        
    return execPass();
  if (args[0] == "NICK")  
    return execNick();
  if (args[0] == "USER")
    return execUser();
  if (args[0] == "QUIT") 
    return execQuit();
  if (args[0] == "TOPIC")
    return execTopic();
  if (args[0] == "JOIN")
    return execJoin();
  if (args[0] == "INVITE")
    return execInvite();
  if (args[0] == "KICK")   
    return execKick();
  if (args[0] == "PRIVMSG" || args[0] == "NOTICE")
    return execPrivmsg();
  if (args[0] == "MODE")  
    return execMode();
  return send_(cli, args[0] + " " + "<command> :Unknown command\n");        // ERR_UNKNOWNCOMMAND
}

int Server::execPass() { 
  if(args.size() < 2)
    return send_(cli, "PASS :Not enough parameters\n");                     // ERR_NEEDMOREPARAMS 
  if(cli->passOk)
    return send_(cli, ":You may not reregister\n");                         // ERR_ALREADYREGISTRED 
  if(args[1] != pass)
    return 0;                                                                // ?
  cli->passOk = true;
  return 0;
}

int Server::execNick() {
  if(args.size() < 2 || args[1].size() == 0) 
    return send_(cli, ":No nickname given\n");                              // ERR_NONICKNAMEGIVEN
  for (size_t i = 0; i < args[1].size() && args[1].size() <= 9; ++i) 
    if(string("-[]^{}0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM").find(args[1][i]) == string::npos)
      return send_(cli, args[1] + " :Erroneus nickname\n");                 // ERR_ERRONEUSNICKNAME
  for (std::map<int, Cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++) {
    if(itCli->second->nick.size() == args[1].size()) {
      bool nickInUse = true;
      for (size_t i = 0; i < args[1].size(); i++)
        if(std::tolower(args[1][i]) != std::tolower(itCli->second->nick[i]))
          nickInUse = false;
      if(nickInUse)
        return send_(cli, args[1] + " :Nickname collision\n");              // ERR_NICKNAMEINUSE
    }
  }
  cli->nick = args[1];
  if(cli->uName != "")
    send_(cli, cli->nick + " " + cli->uName + "\n");
  return 0;
}

int Server::execUser() {
  if(args.size() < 5)
    return send_(cli, "USER :Not enough parameters\n");                     // ERR_NEEDMOREPARAMS 
  if(cli->uName != "")
    return send_(cli, ":You may not reregister\n");                         // ERR_ALREADYREGISTRED 
  cli->uName = args[1];
  cli->rName = args[4];
  if (cli->nick != "")
    send_(cli, cli->nick + " " + cli->uName + "\n");
  return 0;
}

int Server::execPrivmsg() {
  if(!cli->passOk || cli->nick == "" || cli->uName == "") // ?
    return send_(cli, cli->nick + " :User not logged in\n" );               // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  if(args.size() == 1) 
    return send_(cli, ":No recipient given (" + args[0] + ")\n");           // ERR_NORECIPIENT
  if(args.size() == 2)
    return send_(cli, ":No text to send\n");                                // ERR_NOTEXTTOSEND
  vector<string> tos = split(args[1], ',');
  for (vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    if(((*to)[0] == '#' && chs.find(*to) == chs.end()) || ((*to)[0] != '#' && !getCli(*to)))
      send_(cli, *to + " :No such nick/channel\n");                         // ERR_NOSUCHNICK
    else if((*to)[0] == '#' && chs[*to]->clis.find(cli) != chs[*to]->clis.end()) // ERR_NOTONCHANNEL нужно ?
      send_(chs[*to], cli->rName + " PRIVMSG " + *to + " " + args[2] + "\n");
    else
      send_(getCli(*to), cli->rName + " PRIVMSG " + *to + " " + args[2] + "\n");
  return 0;
}

int Server::execJoin() {
  if(!cli->passOk || cli->nick == "" || cli->uName == "")                   
    return send_(cli, cli->nick + " :User not logged in\n" );               // ERR_NOLOGIN ? ERR_NOTREGISTERED ? 
  if(args.size() < 2)
    return send_(cli, "JOIN :Not enough parameters\n");                     // ERR_NEEDMOREPARAMS 
  vector<string> chNames = split(args[1], ',');
  //vector<string> passes  = args.size() > 2 ? split(args[2], ',') : vector<string>();
  //passes.insert(passes.end(), chNames.size() - passes.size(), 0);
  size_t i = 0;
  for (vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++, i++) {
    if(chName->size() > 200 || (*chName)[0] != '#') // проверить
      send_(cli, *chName + " :Cannot join channel (bad channel name)\n");   // ?
    else {
      chs[*chName] = (chs.find(*chName) == chs.end()) ? new Ch(cli) : chs[*chName]; // ERR_NOSUCHCHANNEL ? 
      //string pass = passes.size() > i ? passes[i] : "";
      // if(chs[*chName]->pass != "" && pass != chs[*chName]->pass)
      //   send_(cli, *chName + " :Cannot join channel (+k)\n");            // ERR_BADCHANNELKEY
      if(chs[*chName]->size() >= chs[*chName]->limit) 
        send_(cli, *chName + " :Cannot join channel (+l)\n");               // ERR_CHANNELISFULL
      else if(chs[*chName]->optI && cli->invits.find(*chName) == cli->invits.end())
        send_(cli, *chName + " :Cannot join channel (+i)\n");               // ERR_INVITEONLYCHAN
      else {
        chs[*chName]->clis.insert(cli);
        send_(chs[*chName], cli->nick + " JOIN :" + *chName + "\n");
        send_(cli, *chName + " " + chs[*chName]->topic + " mode " + "\n");  // как выглядит mode?
      }                                                                     // нужно ли исключение "пользователь уже на канале" ?   
    }
  }
  return 1;
}

int Server::execInvite() {
  if(!cli->passOk || cli->nick == "" || cli->uName == "")                   
    return send_(cli, cli->nick + " :User not logged in\n" );               // ERR_NOLOGIN ? ERR_NOTREGISTERED ? 
  if(args.size() < 3)
    return send_(cli, "INVITE :Not enough parameters\n");                   // ERR_NEEDMOREPARAMS 
  if(chs.find(args[2]) == chs.end())
    return send_(cli, args[2] + " :No such channel\n");                     // ERR_NOSUCHCHANNEL ? 
  if(chs[args[2]]->adms.find(cli) == chs[args[2]]->adms.end()) 
    return send_(cli, args[2] + " :You're not channel operator\n");         // ERR_CHANOPRIVSNEEDED
  if(chs[args[2]]->clis.find(cli) == chs[args[2]]->clis.end()) 
    return send_(cli, args[2] + " :You're not on that channel\n");          // ERR_NOTONCHANNEL
  if(getCli(args[1]) == NULL)
    return send_(cli, args[1] + " :No such nick\n");                        // ERR_NOSUCHNICK
  getCli(args[1])->invits.insert(args[2]);
  send_(chs[args[2]], args[2] + " " + args[1] + "\n");                      // RPL_INVITING
  return 0;
}

int Server::execTopic() {
  if(!cli->passOk || cli->nick == "" || cli->uName == "")                   
    return send_(cli, cli->nick + " :User not logged in\n" );               // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  if(args.size() < 1)
    return send_(cli, "TOPIC :Not enough parameters\n");                    // ERR_NEEDMOREPARAMS
  if(chs.find(args[1]) == chs.end())
    return send_(cli, args[1] + " :No such channel\n");                     // ERR_NOSUCHCHANNEL
  if(chs[args[1]]->clis.empty() || chs[args[1]]->clis.find(cli) == chs[args[1]]->clis.end()) 
    return send_(cli, args[1] + " :You're not on that channe\n");           // ERR_NOTONCHANNEL
  if(chs[args[1]]->adms.find(cli) == chs[args[1]]->adms.end()) 
    return send_(cli, args[1] + " :You're not channel operator\n");         // ERR_CHANOPRIVSNEEDED
  if (args.size() == 2) {
    chs[args[1]]->topic = "";
    return send_(chs[args[2]], args[1] + " No topic is set\n");             // RPL_NOTOPIC
  }
  chs[args[1]]->topic = args[2];
  return send_(chs[args[1]], args[1] + " " + args[2] + "\n");               // RPL_TOPIC
}

int Server::execKick() {
  printMe();
  if(!cli->passOk || cli->nick == "" || cli->uName == "")                   
    return send_(cli, cli->nick + " :User not logged in\n" );               // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  if(args.size() < 3)
    return send_(cli, "TOPIC :Not enough parameters\n");                    // ERR_NEEDMOREPARAMS
  std::vector<string> chNames    = split(args[1], ',');
  std::vector<string> targetClis = split(args[2], ',');
  for (vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++) {
    if(chs.find(*chName) == chs.end())
      send_(cli, *chName + " :No such channel\n");                          // ERR_NOSUCHCHANNEL
    else if(chs[*chName]->clis.empty() || chs[*chName]->clis.find(cli) == chs[*chName]->clis.end()) 
      send_(cli, *chName + " :You're not on that channe\n");                // ERR_NOTONCHANNEL
    else if(chs[*chName]->adms.find(cli) == chs[*chName]->adms.end()) 
      send_(cli, *chName + " :You're not channel operator\n");              // ERR_CHANOPRIVSNEEDED
    else {
      for (vector<string>::iterator targetCli = targetClis.begin(); targetCli != targetClis.end(); targetCli++)
        if(chs[*chName]->clis.size() > 0 && chs[*chName]->clis.find(getCli(*targetCli)) != chs[*chName]->clis.end())
          chs[*chName]->erase(*targetCli);                                  // send_(chs[*chName], " KICK \n"); ?
      // if (chs[*chName]->size() == 0)
      //   chs.erase(*chName);
    }
  }
  return 0;
}

int Server::execQuit() {
  vector<string> chsToRm;
  for (map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++) {
    ch->second->erase(cli);
    if (ch->second->size() == 0) 
      chsToRm.push_back(ch->first);
  }
  for (vector<string>::iterator chName = chsToRm.begin(); chName != chsToRm.end(); chName++)
    chs.erase(*chName);
  if(clis.find(cli->fd) != clis.end())
    close(clis.find(cli->fd)->first);
  //polls.erase(std::remove(polls.begin(), polls.end(), cli->fd), polls.end());
  clis.erase(clis.find(cli->fd));
  return 0;
}

int Server::execMode() {
  char *notUsed; // ?
  if(!cli->passOk || cli->nick == "" || cli->uName == "")
    return send_(cli, cli->nick + " :User not logged in\n" );                         // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  if(chs.find(args[1]) == chs.end())
    return send_(cli, args[1] + " :No such channel\n");                               // ERR_NOSUCHCHANNEL
  if(chs[args[1]]->clis.empty() || chs[args[1]]->clis.find(cli) == chs[args[1]]->clis.end()) 
    return send_(cli, args[1] + " :You're not on that channe\n");                     // ERR_NOTONCHANNEL
  if(chs[args[1]]->adms.find(cli) == chs[args[1]]->adms.end()) 
    return send_(cli, args[1] + " :You're not channel operator\n");                   // ERR_CHANOPRIVSNEEDED
  if(args.size() < 2)
    return send_(cli, "MODE :Not enough parameters\n");                               // ERR_NEEDMOREPARAMS
  if(args.size() == 2)
    return send_(cli, args[1] + " topic=" + chs[args[1]]->topic + " pass=" + chs[args[1]]->pass + " " /*+ chs[args[1]]->limit + " " */+ (chs[args[1]]->optI ? "invite-only " : "") /*+ admins*/); // RPL_CHANNELMODEIS 
  if(args.size() == 3 && args[1].compare("+i"))
    return (chs[args[1]]->optI = true);
  if(args.size() == 3 && args[1].compare("-i"))
    return (chs[args[1]]->optI = false);
  if(args.size() == 3 && (args[1].compare("+t") || args[1].compare("+p") || args[1].compare("+l") || args[1].compare("+o")))
    return send_(cli, "MODE :Not enough parameters\n");                               // ERR_NEEDMOREPARAMS
  if(args.size() == 4 && args[1].compare("+t"))
    return (chs[args[1]]->topic = args[3], 0);
  if(args.size() == 3 && args[1].compare("-t"))
    return (chs[args[1]]->topic = "", 0);
  if(args.size() == 4 && args[1].compare("+k") && chs[args[1]]->pass == "") 
    return send_(cli, args[1] + " :Channel key already set\n");                       // ERR_KEYSET
  if(args.size() == 4 && args[1].compare("+k"))
    return (chs[args[1]]->pass = args[3], 0);
  if(args.size() == 3 && args[1].compare("-k"))
    return (chs[args[1]]->pass = "", 0);
  if(args.size() == 3 && args[1].compare("+l"))
    return (chs[args[1]]->limit = static_cast<int>(strtol(args[3].c_str(), &notUsed, 10)), 0); // проверить число [0; INT_MAX]
  if(args.size() == 3 && args[1].compare("-l"))
    return chs[args[1]]->limit = std::numeric_limits<unsigned int>::max();
  if(args.size() == 3 && args[1].compare("+o"))
    return (chs[args[1]]->adms.insert(getCli(args[3])), 0);
  if(args.size() == 3 && args[1].compare("-o"))
    return chs[args[1]]->adms.erase(getCli(args[3]));
  return chs[args[1]]->adms.erase(getCli(args[3]));                                   // ERR_UNKNOWNMODE
}

int main(int argc, char *argv[]) {
  if (argc != 3 || atoi(argv[1]) < 1024 || atoi(argv[1]) > 49151) {  
    std::cout << "Invalid arguments.\nRun ./ircserv <port> <password>, port should be between 1024 and 49151\n";
    return 0;
  }
  Server s = Server(argv[1], argv[2]);
  s.init();
  s.run();
  return 0; 
}