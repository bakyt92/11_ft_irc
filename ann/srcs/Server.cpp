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

string mode(Ch *ch) {
  return "mode : top=" + ch->topic + " optT=" + (ch->optT ? "1" : "0") + " optI=" + (ch->optI ? "1" : "0") + " pass=" + ch->pass + " lim=" + (static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (ch->limit) )).str());
}

int send_(Cli *cli, string msg) {
  if (msg != "") {
    msg += "\r\n";
    send(cli->fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
  }
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
  for (vector<string>::iterator it = ar.begin(); it != ar.end(); it++)
    cout << *it << " ";
  cout << endl << "My clients : ";
  for (map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); it++)
    cout << "[" << it->second->nick << "] ";
  cout << endl << "My channels: ";
  for (map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++) {
    cout << ch->first << ", users : ";
    for (set<Cli*>::iterator itCli = ch->second->clis.begin(); itCli != ch->second->clis.end(); itCli++)
      cout << (*itCli)->nick << " ";
    cout << ", " << mode(ch->second) << " ";
  }
  cout << endl << endl;
}

/////////////////////////////////////////////////////////////////////// PRINCIPAL LOOP
Server::Server(string port_, string pass_) : port(port_), pass(pass_) {}

void Server::init() {
  try {
    signal(SIGINT,  sigHandler);
    signal(SIGPIPE, SIG_IGN);    // to ignore the SIGPIPE signal
    // SIGQUIT ?
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
    if (poll(polls.data(), polls.size(), 1) > 0) {                     // в сокетах есть какие-то данные
      for (std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++) // check sockets
        if ((poll->revents & POLLIN) && poll->fd == fdForNewClis) {    // новый клиент подключился к сокету fdServ
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
          else if (bytes == 0)                                         // клиент пропал
            execQuit();
          else {
            string bufS = string(buf.begin(), buf.end());
            bufS.resize(bytes);
            std::vector<string> cmds = split(bufS, '\n');             // if empty ? 
            for (std::vector<string>::iterator cmd = cmds.begin(); cmd != cmds.end(); cmd++) {
              ar = split(*cmd, ' ');                                  // if (ar[0][0] == ':') ar.erase(ar.begin()); // нужен ли префикс?
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
  if (ar.size() == 0)        
    return 0; // ?
  if (ar[0] == "PASS")        
    return execPass();
  if (ar[0] == "NICK")  
    return execNick();
  if (ar[0] == "USER")
    return execUser();
  if (ar[0] == "QUIT") 
    return execQuit();
  if (ar[0] == "TOPIC")
    return execTopic();
  if (ar[0] == "JOIN")
    return execJoin();
  if (ar[0] == "INVITE")
    return execInvite();
  if (ar[0] == "KICK")   
    return execKick();
  if (ar[0] == "PRIVMSG" || ar[0] == "NOTICE")
    return execPrivmsg();
  if (ar[0] == "MODE")  
    return execMode();
  if (ar[0] == "PING") 
    return execPing();
  if (ar[0] == "WHOIS") 
    return execWhois();
  return send_(cli, ar[0] + " " + "<command> :Unknown command");                  // ERR_UNKNOWNCOMMAND
}

int Server::execPass() { 
  if(ar.size() < 2)
    return send_(cli, "PASS :Not enough parameters");                             // ERR_NEEDMOREPARAMS 
  if(cli->passOk)
    return send_(cli, ":You may not reregister");                                 // ERR_ALREADYREGISTRED 
  if(ar[1] != pass)
    return 0;                                                                       // ?
  cli->passOk = true;
  return 0;
}

int Server::execNick() {
  if(ar.size() < 2 || ar[1].size() == 0) 
    return send_(cli, ":No nickname given");                                      // ERR_NONICKNAMEGIVEN
  for (size_t i = 0; i < ar[1].size() && ar[1].size() <= 9; ++i) 
    if(string("-[]^{}0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM").find(ar[1][i]) == string::npos)
      return send_(cli, ar[1] + " :Erroneus nickname");                           // ERR_ERRONEUSNICKNAME
  for (std::map<int, Cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++) {
    if(itCli->second->nick.size() == ar[1].size()) {
      bool nickInUse = true;
      for (size_t i = 0; i < ar[1].size(); i++)
        if(std::tolower(ar[1][i]) != std::tolower(itCli->second->nick[i]))
          nickInUse = false;
      if(nickInUse)
        return send_(cli, ar[1] + " :Nickname collision");                        // ERR_NICKNAMEINUSE
    }
  }
  cli->nick = ar[1];
  if(cli->uName != "")
    send_(cli, cli->nick + " " + cli->uName);
  return 0;
}

int Server::execUser() {
  if(ar.size() < 5)
    return send_(cli, "USER :Not enough parameters");                             // ERR_NEEDMOREPARAMS 
  if(cli->uName != "")
    return send_(cli, ":You may not reregister");                                 // ERR_ALREADYREGISTRED 
  cli->uName = ar[1];
  cli->rName = ar[4];
  if (cli->nick != "")
    send_(cli, cli->nick + " " + cli->uName);
  return 0;
}

int Server::execPrivmsg() {
  if(!cli->passOk || cli->nick == "" || cli->uName == "") // ?
    return send_(cli, cli->nick + " :User not logged in" );                       // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  if(ar.size() == 1) 
    return send_(cli, ":No recipient given (" + ar[0] + ")");                     // ERR_NORECIPIENT
  if(ar.size() == 2)
    return send_(cli, ":No text to send");                                        // ERR_NOTEXTTOSEND
  vector<string> tos = split(ar[1], ',');
  for (vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    if(((*to)[0] == '#' && chs.find(*to) == chs.end()) || ((*to)[0] != '#' && !getCli(*to)))
      send_(cli, *to + " :No such nick/channel");                                 // ERR_NOSUCHNICK
    else if((*to)[0] == '#' && chs[*to]->clis.find(cli) != chs[*to]->clis.end())    // ERR_NOTONCHANNEL нужно ?
      send_(chs[*to], cli->rName + " PRIVMSG " + *to + " " + ar[2]);
    else
      send_(getCli(*to), cli->rName + " PRIVMSG " + *to + " " + ar[2]);
  return 0;
}

int Server::execJoin() {
  if(!cli->passOk || cli->nick == "" || cli->uName == "")                   
    return send_(cli, cli->nick + " :User not logged in" );                       // ERR_NOLOGIN ? ERR_NOTREGISTERED ? 
  if(ar.size() < 2)
    return send_(cli, "JOIN :Not enough parameters");                             // ERR_NEEDMOREPARAMS 
  vector<string> chNames = split(ar[1], ',');
  //vector<string> passes  = ar.size() > 2 ? split(ar[2], ',') : vector<string>();  // доделать
  //passes.insert(passes.end(), chNames.size() - passes.size(), 0);
  size_t i = 0;
  for (vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++, i++) {
    if(chName->size() > 200 || (*chName)[0] != '#') // проверить
      send_(cli, *chName + " :Cannot join channel (bad channel name)");           // ?
    else {
      chs[*chName] = (chs.find(*chName) == chs.end()) ? new Ch(cli) : chs[*chName]; // ERR_NOSUCHCHANNEL ? 
      //string pass = passes.size() > i ? passes[i] : "";
      // if(chs[*chName]->pass != "" && pass != chs[*chName]->pass)
      //   send_(cli, *chName + " :Cannot join channel (+k)\n");                    // ERR_BADCHANNELKEY
      if(chs[*chName]->size() >= chs[*chName]->limit) 
        send_(cli, *chName + " :Cannot join channel (+l)");                       // ERR_CHANNELISFULL
      else if(chs[*chName]->optI && cli->invits.find(*chName) == cli->invits.end())
        send_(cli, *chName + " :Cannot join channel (+i)");                       // ERR_INVITEONLYCHAN
      else {
        chs[*chName]->clis.insert(cli);
        send_(chs[*chName], cli->nick + " JOIN :" + *chName);
        send_(cli, *chName + " " + chs[*chName]->topic + mode(chs[*chName])); 
      }                                                                             // нужно ли исключение "пользователь уже на канале" ?   
    }
  }
  return 1;
}

int Server::execInvite() {
  if(!cli->passOk || cli->nick == "" || cli->uName == "")                   
    return send_(cli, cli->nick + " :User not logged in" );                       // ERR_NOLOGIN ? ERR_NOTREGISTERED ? 
  if(ar.size() < 3)
    return send_(cli, "INVITE :Not enough parameters");                           // ERR_NEEDMOREPARAMS 
  if(chs.find(ar[2]) == chs.end())
    return send_(cli, ar[2] + " :No such channel");                               // ERR_NOSUCHCHANNEL ? 
  if(chs[ar[2]]->adms.find(cli) == chs[ar[2]]->adms.end()) 
    return send_(cli, ar[2] + " :You're not channel operator");                   // ERR_CHANOPRIVSNEEDED
  if(chs[ar[2]]->clis.find(cli) == chs[ar[2]]->clis.end()) 
    return send_(cli, ar[2] + " :You're not on that channel");                    // ERR_NOTONCHANNEL
  if(getCli(ar[1]) == NULL)
    return send_(cli, ar[1] + " :No such nick");                                  // ERR_NOSUCHNICK
  getCli(ar[1])->invits.insert(ar[2]);
  send_(chs[ar[2]], ar[2] + " " + ar[1]);                                    // RPL_INVITING
  return 0;
}

int Server::execTopic() {
  if(!cli->passOk || cli->nick == "" || cli->uName == "")                   
    return send_(cli, cli->nick + " :User not logged in" );                       // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  if(ar.size() < 1)
    return send_(cli, "TOPIC :Not enough parameters");                            // ERR_NEEDMOREPARAMS
  if(chs.find(ar[1]) == chs.end())
    return send_(cli, ar[1] + " :No such channel");                               // ERR_NOSUCHCHANNEL
  if(chs[ar[1]]->clis.empty() || chs[ar[1]]->clis.find(cli) == chs[ar[1]]->clis.end()) 
    return send_(cli, ar[1] + " :You're not on that channe");                     // ERR_NOTONCHANNEL
  if(chs[ar[1]]->adms.find(cli) == chs[ar[1]]->adms.end()) 
    return send_(cli, ar[1] + " :You're not channel operator");                   // ERR_CHANOPRIVSNEEDED
  if (ar.size() == 2) {
    chs[ar[1]]->topic = "";
    return send_(chs[ar[2]], ar[1] + " No topic is set");                         // RPL_NOTOPIC
  }
  chs[ar[1]]->topic = ar[2];                                             
  return send_(chs[ar[1]], ar[1] + " " + ar[2]);                             // RPL_TOPIC
}

int Server::execKick() {
  printMe();
  if(!cli->passOk || cli->nick == "" || cli->uName == "")                   
    return send_(cli, cli->nick + " :User not logged in" );                       // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  if(ar.size() < 3)
    return send_(cli, "KICK :Not enough parameters");                             // ERR_NEEDMOREPARAMS
  std::vector<string> chNames    = split(ar[1], ',');
  std::vector<string> targetClis = split(ar[2], ',');
  for (vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++) {
    if(chs.find(*chName) == chs.end())
      send_(cli, *chName + " :No such channel");                                  // ERR_NOSUCHCHANNEL
    else if(chs[*chName]->clis.empty() || chs[*chName]->clis.find(cli) == chs[*chName]->clis.end()) 
      send_(cli, *chName + " :You're not on that channe");                        // ERR_NOTONCHANNEL
    else if(chs[*chName]->adms.find(cli) == chs[*chName]->adms.end()) 
      send_(cli, *chName + " :You're not channel operator");                      // ERR_CHANOPRIVSNEEDED
    else {
      for (vector<string>::iterator targetCli = targetClis.begin(); targetCli != targetClis.end(); targetCli++)
        if(chs[*chName]->clis.size() > 0 && chs[*chName]->clis.find(getCli(*targetCli)) != chs[*chName]->clis.end())
          chs[*chName]->erase(*targetCli);                                          // send_(chs[*chName], " KICK"); ?
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
    return send_(cli, cli->nick + " :User not logged in" );                       // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  if(chs.find(ar[1]) == chs.end())
    return send_(cli, ar[1] + " :No such channel");                               // ERR_NOSUCHCHANNEL
  if(chs[ar[1]]->clis.empty() || chs[ar[1]]->clis.find(cli) == chs[ar[1]]->clis.end()) 
    return send_(cli, ar[1] + " :You're not on that channel");                    // ERR_NOTONCHANNEL
  if(chs[ar[1]]->adms.find(cli) == chs[ar[1]]->adms.end()) 
    return send_(cli, ar[1] + " :You're not channel operator");                   // ERR_CHANOPRIVSNEEDED
  if(ar.size() < 2)
    return send_(cli, "MODE :Not enough parameters");                             // ERR_NEEDMOREPARAMS
  if(ar.size() == 2)
    return send_(cli, ar[1] + " " + mode(chs[ar[1]]));                            // RPL_CHANNELMODEIS 
  if(ar.size() == 3 && ar[2].compare("+i") == 0)
    return (chs[ar[1]]->optI = true);
  if(ar.size() == 3 && ar[2].compare("-i") == 0)
    return (chs[ar[1]]->optI = false);
  if(ar.size() == 3 && ar[2].compare("+t") == 0)
    return (chs[ar[1]]->optT = true);
  if(ar.size() == 3 && ar[2].compare("-t") == 0)
    return (chs[ar[1]]->optT = false);
  if(ar.size() == 3 && ar[2].compare("-l") == 0)
    return chs[ar[1]]->limit = std::numeric_limits<unsigned int>::max();
  if(ar.size() == 3 && ar[2].compare("-k") == 0)
    return (chs[ar[1]]->pass = "", 0);
  if(ar.size() == 3 && (ar[2].compare("+k") == 0 || ar[1].compare("+l") == 0 || ar[1].compare("+o") == 0 || ar[1].compare("-o") == 0))
    return send_(cli, "MODE :Not enough parameters");                             // ERR_NEEDMOREPARAMS
  if(ar.size() == 3)
    return send_(cli, "MODE :Not enough parameters");                             //  ERR_NEEDMOREPARAMS
  if(ar[2].compare("+k") == 0 && chs[ar[1]]->pass != "")
    return send_(cli, ar[1] + " :Channel key already set");                       // ERR_KEYSET
  if(ar[2].compare("+k") == 0)
    return (chs[ar[1]]->pass = ar[3], 0);
  if(ar[2].compare("+l") == 0 && atoi(ar[3].c_str()) >= static_cast<int>(0) && static_cast<unsigned int>(atoi(ar[3].c_str())) <= std::numeric_limits<unsigned int>::max())
    return (chs[ar[1]]->limit = static_cast<int>(strtol(ar[3].c_str(), &notUsed, 10)), 0);
  if(ar[2].compare("+o") == 0)
    return (chs[ar[1]]->adms.insert(getCli(ar[3])), 0); // если такого ника нет?
  if(ar[2].compare("-o") == 0)
    return chs[ar[1]]->adms.erase(getCli(ar[3]));
  return send_(cli, ar[0] + " " + ar[1] + " " + ar[2] + " " + ar[3] + " :is unknown mode char to me"); // ERR_UNKNOWNMODE
}

int Server::execPing() {
  return send_(cli, "PONG");
}

// not implemented here: RPL_WHOISCHANNELS RPL_WHOISOPERATOR RPL_AWAY RPL_WHOISIDLE                 
int Server::execWhois() {
  if(ar.size() < 2)
    return send_(cli, ":No nickname given");                                      // ERR_NONICKNAMEGIVEN
  std::vector<string> nicks = split(ar[1], ',');
  for (vector<string>::iterator nick = nicks.begin(); nick != nicks.end(); nick++)
    if(getCli(ar[1]) == NULL)
      send_(cli, ar[1] + " :No such nick");                                       // ERR_NOSUCHNICK
    else
      send_(cli, getCli(ar[1])->nick + " " + getCli(ar[1])->uName + " " + getCli(ar[1])->host + " * :" + getCli(ar[1])->rName + "\n"); // RPL_WHOISUSER
  return send_(cli, nicks[0] + " :End of WHOIS list");                            // RPL_ENDOFWHOIS ?
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