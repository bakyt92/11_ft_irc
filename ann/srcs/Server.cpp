#include "Server.hpp"
////////////////////////////////////////////////////////////////////////////// UTILS
Server::~Server() {
  // close all fd 
  // free clis, chs ?
}

void Server::sigHandler(int sig) {
  std::cout << std::endl << "Signal Received" << std::endl;
  sigReceived = true;
  (void)sig;
}

std::vector<string> split_n(string s) {
  std::vector<string> parts;
  for (size_t pos = s.find("\n"); pos != string::npos; pos = s.find("\n")) {
    if (pos > 0) {
      string part = s.substr(0, pos);
      parts.push_back(part);
    }
    s.erase(0, pos + 1);
  }
  if (s.size() > 0) //////// начало следующей команды в буфере !
    parts.push_back(s);
  return parts;
}

// s = [CAP LS\r\nPASS 2\r\nNICK an\r\nUSER an an 0 :an\r\n]
std::vector<string> split_r_n(string s) {
  // cout << "split s = [";
  // for (size_t i = 0; i < s.size(); i++)
  //   if (s[i] == '\r')
  //     cout << "\\r";
  //   else if (s[i] == '\n')
  //     cout << "\\n";
  //   else
  //     cout << s[i];
  // cout << "]" << endl;
  if (s.size() >= 2 && s[s.size() - 1] == '\n' && s[s.size() - 2] != '\r')
    return split_n(s);
  std::vector<string> parts;
  for (size_t pos = s.find("\r\n"); pos != string::npos; pos = s.find("\r\n")) {
    //cout << "  pos = " << pos << "\n";
    if (pos > 0) {
      string part = s.substr(0, pos);
      //part.erase(std::remove(part.begin(), part.end(), '\r'), part.end()); // ?
      //cout << "  push [" << part << "]\n";
      parts.push_back(part);
    }
    s.erase(0, pos + 2);
  }
  if (s.size() > 0) { //////// начало следующей команды в буфере !
    //s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
    //cout << "  push [" << s << "]\n";
    parts.push_back(s);
  }
  return parts;
}

std::vector<string> split_spaces(string s) {
  std::vector<string> parts;
  size_t pos;
  string afterColon = "";
  if ((pos = s.find(':')) < s.size())
    afterColon = s.substr(pos, s.size() - pos);
  s = s.substr(0, pos); // memory leaks ?
  for (size_t pos = s.find(' '); pos != string::npos; pos = s.find(' ')) {
    if (pos > 0) {
      string part = s.substr(0, pos);
      parts.push_back(part);
    }
    s.erase(0, pos + 1);
  }
  if (s.size() > 0)
    parts.push_back(s);
  if (afterColon.size() > 0)
    parts.push_back(afterColon);
  return parts;
}

std::vector<string> split_comma(string s) {
  std::vector<string> parts;
  for (size_t pos = s.find(','); pos != string::npos; pos = s.find(',')) {
    if (pos > 0) {
      string part = s.substr(0, pos);
      parts.push_back(part);
    }
    s.erase(0, pos + 1);
  }
  if (s.size() > 0)
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
  string mode = "+";
  if (ch->optT == true)
    mode += "t";
  if (ch->optI == true)
    mode += "i";
  if (ch->pass != "")
    mode += "k";
  if (ch->limit < std::numeric_limits<unsigned int>::max())
    mode += "l";
  return mode == "+" ? "by default" : mode;  // +o ? перечислить пользлователей канала и админов?
  //static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (ch->limit) )).str());
}

int Server::send_(Cli *to, string msg) {
  string end = msg.substr(msg.size() - 3, 2);
  if (end == "\r\n")
    msg = msg.substr(0, msg.size() - 2);
  string toPrint(msg);
  for(size_t pos = toPrint.find("\r\n"); pos != string::npos; pos = toPrint.find("\r\n", pos))
    toPrint.replace(pos, 2, "\n                            ");
  cout << "I send to fd=" << to->fd << "            : [" << toPrint << "]\n";
  msg += "\r\n";
  return send(to->fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
}

int Server::send_(Ch *ch, string msg) {
  for (set<Cli*>::iterator to = ch->clis.begin(); to != ch->clis.end(); to++) 
    if ((*to)->fd != cli->fd)
      send_(*to, msg);
  return 0;
}

void Server::printNewCli(int fd) { // for debugging only
  cout << "New cli (fd=" << fd << ")" << endl;
}

void Server::printCmd() { // for debugging only
  cout << "I execute (cmd from fd=" << static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (cli->fd) )).str() << ") : ";
  for (vector<string>::iterator it = ar.begin(); it != ar.end(); it++)
    cout << "[" << *it << "]" << " ";
  cout << endl;
}

void Server::printServState() { // for debugging only
  cout << "My clients                : ";
  for (map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); it++)
    cout << "[" << it->second->nick << "] ";
  cout << endl;
  for (map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++) {
    cout << "My channel                : name = " << ch->first << ", topic = " << ch->second->topic << ", pass = " << ch->second->pass << ", users = ";
    for (set<Cli*>::iterator itCli = ch->second->clis.begin(); itCli != ch->second->clis.end(); itCli++)
      cout << (*itCli)->nick << " ";
    cout << ", mode = " << mode(ch->second) << endl;
  }
  cout << endl;
}

/////////////////////////////////////////////////////////////////////// PRINCIPAL LOOP
Server::Server(string port_, string pass_) : port(port_), pass(pass_) {}

void Server::init() {
  try {
    signal(SIGINT,  sigHandler);
    signal(SIGPIPE, SIG_IGN);    // to ignore the SIGPIPE signal
    // SIitCliGQUIT ?
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
  if (getaddrinfo(NULL, port.c_str(), &hints, &listRes))
    throw(std::runtime_error("getaddrinfo"));
  int notUsed = 1;
  for (struct addrinfo* hint = listRes; hint != NULL; hint = hint->ai_next) {
    if ((fdForNewClis = socket(hint->ai_family, hint->ai_socktype, hint->ai_protocol)) < 0) 
      throw(std::runtime_error("socket"));
    else if (setsockopt(fdForNewClis, SOL_SOCKET, SO_REUSEADDR , &notUsed, sizeof(notUsed)))
      throw(std::runtime_error("setsockopt"));
    else if (bind(fdForNewClis, hint->ai_addr, hint->ai_addrlen)) { 
      close(fdForNewClis);
      hint->ai_next == NULL ? throw(std::runtime_error("bind")) : perror("bind"); 
    }
    else
      break ;
  }
  freeaddrinfo(listRes);
  if (listen(fdForNewClis, SOMAXCONN))
    throw(std::runtime_error("listen"));
  struct pollfd pollForNewClis = {fdForNewClis, POLLIN, 0};
  polls.push_back(pollForNewClis);
}

void Server::run() {
  std::cout << "Server is running. Waiting clients to connect >>>\n";
  while (sigReceived == false) {
    if (poll(polls.data(), polls.size(), 1) > 0) { // в сокетах есть данные, попробовать 1000 вместо 1 и убрать unsleep
      for (std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++) // check sockets
        if ((poll->revents & POLLIN) && poll->fd == fdForNewClis) {    // новый клиент подключился к сокету fdServ
          struct sockaddr sa;
          socklen_t       saLen = sizeof(sa);
          int fdForMsgs = accept(poll->fd, &sa, &saLen); // у каждого киента свой fd
          if (fdForMsgs == -1)
            perror("accept");
          else {
            struct Cli *newCli = new Cli(fdForMsgs, inet_ntoa(((struct sockaddr_in*)&sa)->sin_addr)); 
            clis[fdForMsgs] = newCli;
            struct pollfd pollForMsgs = {fdForMsgs, POLLIN, 0};
            polls.push_back(pollForMsgs);
            printNewCli(fdForMsgs);
          }
          break ;
        }
        else if ((poll->revents & POLLIN) && poll->fd != fdForNewClis) { // клиент прислал сообщение в свой fdForMsgs
          if (!(cli = clis.at(poll->fd)))
            continue ;
          vector<unsigned char> buf(513); // 512 ?
          for (int i = 0; i < 513; i++)
            buf[i] = '\0';
          int bytes = recv(cli->fd, buf.data(), buf.size() - 1, 0); // size - 1 ? // + добавить к send() и recv() MSG_NOSIGNAL ?
          if (bytes < 0) 
            perror("recv");
          else if (bytes == 0)                                 // клиент пропал
            execQuit(); 
          else {
            string bufS = string(buf.begin(), buf.end());
            bufS.resize(bytes);
            std::vector<string> cmds = split_r_n(bufS);
            for (std::vector<string>::iterator cmd = cmds.begin(); cmd != cmds.end(); cmd++) {
              // for (int i = 0; i < ar.size(); i++)
              //   ar[i] = ""; ?
              vector<string>().swap(ar);
              ar = split_spaces(*cmd);
              printCmd();
              exec();
              printServState();
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
  if (ar[0] == "PING")
    return execPing();
  if (ar[0] == "CAP")
    return execCap();
  if (ar[0] != "PRIVMSG" && ar[0] != "NOTICE" && ar[0] != "WHOIS" && ar[0] != "JOIN" && ar[0] != "PART" && ar[0] != "MODE" && ar[0] != "TOPIC" && ar[0] != "INVITE" && ar[0] != "KICK")
    return send_(cli, "421 " + ar[0] + " " + " :is unknown mode char to me");                // ERR_UNKNOWNCOMMAND
  if(!cli->passOk || cli->nick == "" || cli->uName == "" || !cli->capOk) // выше этой строки те команды, которые можно выполнять без аккаунт
    return send_(cli, "451 " + cli->nick + " :User not logged in" ); // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  if (ar[0] == "PRIVMSG" || ar[0] == "NOTICE") //
    return execPrivmsg();
  if (ar[0] == "WHOIS")
    return execWhois();
  if (ar[0] == "JOIN")
    return execJoin();
  if (ar[0] == "PART")
    return execPart();
  if (ar[0] == "MODE")
    return execMode();
  if (ar[0] == "TOPIC")
    return execTopic();
  if (ar[0] == "INVITE")
    return execInvite();
  if (ar[0] == "KICK")
    return execKick();
  return send_(cli, "421 " + ar[0] + " " + " :is unknown mode char to me");                // ERR_UNKNOWNCOMMAND
}

int Server::execPass() {
  if(ar.size() < 2)
    return send_(cli, "461 PASS :Not enough parameters");                             // ERR_NEEDMOREPARAMS
  if(cli->passOk)
    return send_(cli, "462 :You may not reregister");                                 // ERR_ALREADYREGISTRED
  if(ar[1] == pass)
    cli->passOk = true;
  if (cli->nick != "" && cli->uName != "" && cli->capOk)
    send_(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

// not implemented here ERR_UNAVAILRESOURCE ERR_RESTRICTED
int Server::execNick() {
  if(ar.size() < 2 || ar[1].size() == 0) 
    return send_(cli, "431 :No nickname given");                                      // ERR_NONICKNAMEGIVEN
  for (size_t i = 0; i < ar[1].size() && ar[1].size() <= 9; ++i)
    if(string("-[]^{}0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM").find(ar[1][i]) == string::npos)
      return send_(cli, "432 " + ar[1] + " :Erroneus nickname");                      // ERR_ERRONEUSNICKNAME
  for (std::map<int, Cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++) {
    if(itCli->second->nick.size() == ar[1].size()) {
      bool nickInUse = true;
      for (size_t i = 0; i < ar[1].size(); i++)
        if(std::tolower(ar[1][i]) != std::tolower(itCli->second->nick[i]))
          nickInUse = false;
      if(nickInUse)
        return send_(cli, "436 " + ar[1] + " :Nickname collision"); // ERR_NICKCOLLISION ERR_NICKNAMEINUSE ?
    }
  }
  cli->nick = ar[1];
  if(cli->uName != "" && cli->passOk && cli->capOk)
    send_(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

int Server::execUser() {
  if(ar.size() < 5)
    return send_(cli, "461 USER :Not enough parameters");                        // ERR_NEEDMOREPARAMS
  if(cli->uName != "")
    return send_(cli, "462 :You may not reregister");                            // ERR_ALREADYREGISTRED
  cli->uName = ar[1];
  cli->rName = ar[4];
  if (cli->nick != "" && cli->passOk && cli->capOk)
    send_(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

// I execute (cmd from fd=5) : [PRIVMSG] [#ch] [aaaa] NON nc -> irssi 
// I send to fd=4            : [a #ch :aaaa]

// I execute (cmd from fd=4) : [PRIVMSG] [#ch] [:ananan] OK irssi ->nc
// I send to fd=5            : [an #ch ::ananan]

//  execute (cmd from fd=4) : [PRIVMSG] [#ch] [aaa] NON nc -> irssi 
// I send to fd=5            : [a #ch :aaa]

// I execute (cmd from fd=5) : [PRIVMSG] [#ch] [:ananan] OK irssi ->nc
// I send to fd=4            : [an #ch ::ananan]

// not implemented here: ERR_CANNOTSENDTOCHAN ERR_NOTOPLEVEL ERR_WILDTOPLEVEL ERR_TOOMANYTARGETS RPL_AWAY
int Server::execPrivmsg() {
  if(ar.size() == 1) 
    return send_(cli, "411 :No recipient given (" + ar[1] + ")");                 // ERR_NORECIPIENT
  if(ar.size() == 2)
    return send_(cli, "412 :No text to send");                                    // ERR_NOTEXTTOSEND
  vector<string> tos = split_comma(ar[1]);
  //string toSendToCli = ""; ?
  for (vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    if((*to)[0] == '#' && chs.find(*to) == chs.end())
      send_(cli, "401 " + *to + " :No such channel");                 // ERR_NOSUCHNICK
    else if((*to)[0] == '#')
      send_(chs[*to], "PRIVMSG " + *to + " :" + ar[2]);
    else if((*to)[0] != '#' && !getCli(*to))
      send_(cli, "401 " + *to + " :No such nick");                 // ERR_NOSUCHNICK
    else if((*to)[0] != '#')
      send_(getCli(*to), "PRIVMSG " + *to + " ::" + ar[2]);
  return 0;
}

// not implemented here: ERR_BANNEDFROMCHAN ERR_BADCHANMASK ERR_NOSUCHCHANNEL ERR_TOOMANYCHANNELS ERR_TOOMANYTARGETS ERR_UNAVAILRESOURCE 
// `JOIN #foo,&bar fubar` вход на канал #foo, используя ключ "fubar" и на канал &bar без использования ключа - я пока ничего не сделала насчёт `&`, надо ли?
// Once a user has joined a channel, he receives information about all commands his server receives affecting the channel: JOIN, MODE, KICK, QUIT, PRIVMSG/NOTICE
int Server::execJoin() {
  if(ar.size() < 2)
    return send_(cli, "461 JOIN :Not enough parameters");                             // ERR_NEEDMOREPARAMS 
  vector<string> chNames = split_comma(ar[1]);
  //vector<string> passes  = ar.size() > 2 ? split_comma(ar[2]) : vector<string>();
  for (vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++) {
    if(chName->size() > 200 || (*chName)[0] != '#') // проверить
      send_(cli, *chName + " :Cannot join channel (bad channel name)");           // ?
    else {
      chs[*chName] = (chs.find(*chName) == chs.end()) ? new Ch(cli) : chs[*chName]; // зачем указан RR_NOSUCHCHANNEL ?
      // string pass = passes.size() > i ? passes[i] : "";
      // if(chs[*chName]->pass != "" && pass != chs[*chName]->pass)
      //   send_(cli, "'475 :" + *chName + " Cannot join channel (+k)\n");                    // ERR_BADCHANNELKEY
      if(chs[*chName]->size() >= chs[*chName]->limit) 
        send_(cli, "471 " + *chName + " :Cannot join channel (+l)");                       // ERR_CHANNELISFULL
      else if(chs[*chName]->optI && cli->invits.find(*chName) == cli->invits.end())
        send_(cli, "473 " + *chName + " :Cannot join channel (+i)");                       // ERR_INVITEONLYCHAN
      else {
        chs[*chName]->clis.insert(cli);
        send_(chs[*chName], cli->nick + " JOIN " + *chName);  // два send ?
        send_(cli, *chName + " " + chs[*chName]->topic + " " + mode(chs[*chName])); // RPL_TOPIC + RPL_NAMREPLY !
      }                                        // нужно ли исключение "пользователь уже на канале" ?
    }
  }
  return 0;
}

int Server::execPart() {
  if(ar.size() < 2)
    return send_(cli, "461 JOIN :Not enough parameters");                          // ERR_NEEDMOREPARAMS
  vector<string> chNames = split_comma(ar[1]);
  for (vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++)
    if (chs.find(*chName) == chs.end())
      send_(cli, "403 :" + *chName + " :No such channel"); // ERR_NOSUCHCHANNEL
    else if(chs[*chName]->clis.find(cli) == chs[*chName]->clis.end())
      send_(cli, "442 " + *chName + " :You're not on that channel");                         // ERR_NOTONCHANNEL
    else {
      send_(chs[*chName], cli->nick + " PART :" + *chName);                         // нужно ли сообщение
      chs.erase(*chName);
    }
  return 0;
}

// not implemented here RPL_AWAY
int Server::execInvite() {
  if(ar.size() < 3)
    return send_(cli, "461 INVITE :Not enough parameters");                       // ERR_NEEDMOREPARAMS 
  if(chs.find(ar[2]) == chs.end())
    return send_(cli, "403 " + ar[2] + " :No such channel");                      // ERR_NOSUCHCHANNEL ???
  if(chs[ar[2]]->adms.find(cli) == chs[ar[2]]->adms.end()) 
    return send_(cli, "482 " + ar[2] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
  if(chs[ar[2]]->clis.find(cli) == chs[ar[2]]->clis.end()) 
    return send_(cli, "442 " + ar[2] + " :You're not on that channel");           // ERR_NOTONCHANNEL
  if(getCli(ar[1]) == NULL)
    return send_(cli, "401 :" + ar[1] + " No such nick");                         // ERR_NOSUCHNICK
  if(chs[ar[2]]->clis.find(cli) != chs[ar[2]]->clis.end()) 
    return send_(cli, "443 " + ar[1] + " " + ar[2] + " :is already on channel");  // ERR_USERONCHANNEL
  getCli(ar[1])->invits.insert(ar[2]);
  send_(chs[ar[2]], "341" + ar[2] + " " + ar[1]);                                 // RPL_INVITING
  return 0;
}

// not implemented here ERR_NOCHANMODES
int Server::execTopic() {
  if(ar.size() < 1)
    return send_(cli, "461 TOPIC :Not enough parameters");                       // ERR_NEEDMOREPARAMS
  if(chs.find(ar[1]) == chs.end())
    return send_(cli, "403 " + ar[1] + " :No such channel");                      // ERR_NOSUCHCHANNEL
  if(chs[ar[1]]->clis.empty() || chs[ar[1]]->clis.find(cli) == chs[ar[1]]->clis.end()) 
    return send_(cli, "442 " + ar[1] + " :You're not on that channe");            // ERR_NOTONCHANNEL
  if(chs[ar[1]]->adms.find(cli) == chs[ar[1]]->adms.end()) 
    return send_(cli, "482 " + ar[1] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
  if (ar.size() == 2) {
    chs[ar[1]]->topic = "";
    return send_(chs[ar[2]], "331 " + ar[1] + " :No topic is set");               // RPL_NOTOPIC
  }
  chs[ar[1]]->topic = ar[2];
  return send_(chs[ar[1]], "332 " + ar[1] + " :" + ar[2]);                        // RPL_TOPIC
}

// not implemented here ERR_BADCHANMASK
int Server::execKick() {
  if(ar.size() < 3)
    return send_(cli, "461 KICK :Not enough parameters");                             // ERR_NEEDMOREPARAMS
  std::vector<string> chNames    = split_comma(ar[1]);
  std::vector<string> targetClis = split_comma(ar[2]);
  for (vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++)
    if(chs.find(*chName) == chs.end())
      send_(cli, "403 " + *chName + " :No such channel");                                  // ERR_NOSUCHCHANNEL
    else if(chs[*chName]->clis.empty() || chs[*chName]->clis.find(cli) == chs[*chName]->clis.end()) 
      send_(cli, "442 " + *chName + " :You're not on that channe");               // ERR_NOTONCHANNEL
    else if(chs[*chName]->adms.find(cli) == chs[*chName]->adms.end()) 
      send_(cli, "482 " + *chName + " :You're not channel operator");             // ERR_CHANOPRIVSNEEDED
    else {
      for (vector<string>::iterator targetCli = targetClis.begin(); targetCli != targetClis.end(); targetCli++)
        if(chs[*chName]->clis.empty() || chs[*chName]->clis.find(cli) == chs[*chName]->clis.end())
          send_(cli, "442 " + *targetCli + " " + *chName + " :They aren't on that channel"); // ERR_USERNOTINCHANNEL
        else if (chs[*chName]->clis.size() > 0 && chs[*chName]->clis.find(getCli(*targetCli)) != chs[*chName]->clis.end())
          chs[*chName]->erase(*targetCli);                                          // send_(chs[*chName], " KICK"); ?
      // if (chs[*chName]->size() == 0)
      //   chs.erase(*chName);
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
  for (vector<string>::iterator ch = chsToRm.begin(); ch != chsToRm.end(); ch++)
    chs.erase(*ch);
  close(clis.find(cli->fd)->first);
  //polls.erase(std::remove(polls.begin(), polls.end(), cli->fd), polls.end());
  clis.erase(clis.find(cli->fd));
  return 0;
}

// not implemented here: ERR_NOCHANMODES RPL_BANLIST RPL_ENDOFBANLIST RPL_EXCEPTLIST RPL_ENDOFEXCEPTLIST RPL_INVITELIST RPL_ENDOFINVITELIST RPL_UNIQOPIS (creator of the channel)
// `MODE` устанвливает один параметр за раз, например `MODE -t` должна работать, а `MODE -tpk` нет, нормально ли это?
int Server::execMode() {
  char *notUsed; // ?
  if(ar.size() < 2)
    return send_(cli, "461 MODE :Not enough parameters");                             // ERR_NEEDMOREPARAMS
  if(chs.find(ar[1]) == chs.end())
    return send_(cli, ar[1] + " :No such channel");                                   // ERR_NOSUCHCHANNEL ???
  if(chs[ar[1]]->clis.empty() || chs[ar[1]]->clis.find(cli) == chs[ar[1]]->clis.end()) 
    return send_(cli, "442 " + ar[1] + " :You're not on that channel");               // ERR_NOTONCHANNEL ???
  if(chs[ar[1]]->adms.find(cli) == chs[ar[1]]->adms.end())
    return send_(cli, "482 " + ar[1] + " :You're not channel operator");              // ERR_CHANOPRIVSNEEDED
  if(ar.size() == 2)
    return send_(cli, ar[1] + " mode = " + mode(chs[ar[1]]));                         // RPL_CHANNELMODEIS
  if(ar.size() == 3 && ar[2] == "+i")
    return (chs[ar[1]]->optI = true);
  if(ar.size() == 3 && ar[2] == "-i")
    return (chs[ar[1]]->optI = false);
  if(ar.size() == 3 && ar[2] == "+t")
    return (chs[ar[1]]->optT = true);
  if(ar.size() == 3 && ar[2] == "-t")
    return (chs[ar[1]]->optT = false);
  if(ar.size() == 3 && ar[2] == "-l")
    return chs[ar[1]]->limit = std::numeric_limits<unsigned int>::max();
  if(ar.size() == 3 && ar[2] == "-k")
    return (chs[ar[1]]->pass = "", 0);
  if(ar.size() == 3 && (ar[2] == "+k" || ar[1] == "+l" || ar[1] == "+o" || ar[1] == "-o"))
    return send_(cli, "461 MODE :Not enough parameters");                             // ERR_NEEDMOREPARAMS
  if(ar.size() == 3)
    return send_(cli, "461 MODE :Not enough parameters");                             // ERR_NEEDMOREPARAMS
  if(ar[2] == "+k" && chs[ar[1]]->pass != "")
    return send_(cli, ar[1] + " :Channel key already set");                           // ERR_KEYSET
  if(ar[2] == "+k")
    return (chs[ar[1]]->pass = ar[3], 0);
  if(ar[2] == "+l" && atoi(ar[3].c_str()) >= static_cast<int>(0) && static_cast<unsigned int>(atoi(ar[3].c_str())) <= std::numeric_limits<unsigned int>::max())
    return (chs[ar[1]]->limit = static_cast<int>(strtol(ar[3].c_str(), &notUsed, 10)), 0);
  if(ar[2] == "+o")
    return send_(cli, ar[3] + " " + ar[1] + " :They aren't on that channel"); // ERR_USERNOTINCHANNEL
  if(ar[2] == "-o")
    return chs[ar[1]]->adms.erase(getCli(ar[3]));
  return send_(cli, ar[0] + " " + ar[1] + " " + ar[2] + " " + ar[3] + " :is unknown mode char to me"); // ERR_UNKNOWNMODE
}

int Server::execPing() {
  return send_(cli, "PONG");
}

int Server::execPong() {
  return send_(cli, "PING");
}

// not implemented here: RPL_WHOISCHANNELS RPL_WHOISOPERATOR RPL_AWAY RPL_WHOISIDLE                 
int Server::execWhois() {
  if(ar.size() < 2)
    return send_(cli, "431 :No nickname given");                                      // ERR_NONICKNAMEGIVEN
  std::vector<string> nicks = split_comma(ar[1]);
  string toSend = "";
  for (vector<string>::iterator nick = nicks.begin(); nick != nicks.end(); nick++)
    if(getCli(ar[1]) == NULL)
      toSend += "401 :" + ar[1] + " No such nick\r\n";                                       // ERR_NOSUCHNICK
    else
      toSend += getCli(ar[1])->nick + " " + getCli(ar[1])->uName + " " + getCli(ar[1])->host + " * :" + getCli(ar[1])->rName + "\r\n"; // RPL_WHOISUSER
  return send_(cli, toSend + " " + nicks[0] + " :End of WHOIS list\r\n");              // RPL_ENDOFWHOIS ?
}

int Server::execCap() {
  if(ar.size() >= 2 && ar[1] == "LS") {
    cli->capOk = false;
    return send_(cli, "CAP * LS :");
  }
  else if(ar.size() >= 2 && ar[1] == "END") {
    cli->capOk = true;
    send_(cli, "001"); // !!! :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
    //send_(cli, "002"); // :Your host is " + cli->host  + ", running version 1.0");            // RPL_YOURHOST
    //send_(cli, "003"); // :This server was created 24.04.2024");                  // RPL_CREATED // ?
    //send_(cli, "004"); // :ircserv 1.0  ptkio");                                // RPL_MYINFO
    // 1 send() per select() !!!
  }
  return 0;
}

int main(int argc, char *argv[]) {
  // проверить с помощью strtol, не выходим ли за рамки int
  if(argc != 3 || atoi(argv[1]) < 1024 || atoi(argv[1]) > 65535) {
    std::cout << "Invalid arguments.\nRun ./ircserv <port> <password>, port should be between 1024 and 65535\n";
    return 0;
  }
  Server s = Server(argv[1], argv[2]);
  s.init();
  s.run();
  return 0;
}