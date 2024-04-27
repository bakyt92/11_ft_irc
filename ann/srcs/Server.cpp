#include "Server.hpp"
////////////////////////////////////////////////////////////////////////////// UTILS
void Server::sigHandler(int sig) {
  cout << endl << "Signal Received\n";
  sigReceived = true;
  (void)sig;
}

string mode(Ch *ch) { // +o ? перечислить пользлователей и админов?
  string mode = "+";
  if(ch->optT == true)
    mode += "t";
  if(ch->optI == true)
    mode += "i";
  if(ch->pass != "")
    mode += "k";
  if(ch->limit < std::numeric_limits<unsigned int>::max())
    mode += "l";
  return mode == "+" ? "default" : mode;
  //static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (ch->limit) )).str());
}

string without_r_n(string s) {     // debugging
  for(size_t pos = s.find('\r'); pos != string::npos; pos = s.find('\r', pos))
    s.replace(pos, 1, "\\r");
  for(size_t pos = s.find('\n'); pos != string::npos; pos = s.find('\n', pos))
    s.replace(pos, 1, "\\n");
  return s;
}

string Server::infoNewCli(int fd) { // debugging
  return "New cli (fd=" + static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (fd) )).str() + ")\n";
}

void Server::printCmd() {          // debugging
  cout << "I execute                 : ";
  for(vector<string>::iterator it = ar.begin(); it != ar.end(); it++)
    cout << "[" << *it << "]" << " ";
  cout << endl;
}

string Server::infoServ() {    // debugging
  string ret = "My clients                : ";
  for(map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); it++)
    ret += "[[" + it->second->nick + "] with buf [" + it->second->buf + "]] ";
  ret += "\n";
  for(map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++) {
    ret += "My channel                : name = " + ch->first + ", topic = " + ch->second->topic + ", pass = " + ch->second->pass + ", users = ";
    for(set<Cli*>::iterator itCli = ch->second->clis.begin(); itCli != ch->second->clis.end(); itCli++)
      ret += (*itCli)->nick + " ";
    ret += ", mode = " + mode(ch->second) + "\n";
  }
  return ret;
}

std::vector<string> split_space(string s) {
  if (s.size() == 0)
    return vector<string>();
  std::vector<string> parts;
  size_t pos;
  string afterColon = "";
  if((pos = s.find(':')) < s.size())
    afterColon = s.substr(pos, s.size() - pos);
  s = s.substr(0, pos);
  for(size_t pos = s.find(' '); pos != string::npos; pos = s.find(' ')) {
    if(pos > 0)
      parts.push_back(s.substr(0, pos));
    s.erase(0, pos + 1);
  }
  if(s.size() > 0)
    parts.push_back(s);
  if(afterColon.size() > 0)
    parts.push_back(afterColon);
  return parts;
}

vector<string> split(string s, char delim) {
  if (s.size() == 0)
    return vector<string>();
  vector<string> parts;
  for(size_t pos = s.find(delim); pos != string::npos; pos = s.find(delim)) {
    if(pos > 0)
      parts.push_back(s.substr(0, pos));
    s.erase(0, pos + 1);
  }
  if(s.size() > 0)
    parts.push_back(s);
  return parts;
}

vector<string> Server::split_r_n(string s) {
  if (s.size() == 0)
    return vector<string>();
  if(s.size() >= 2 && s[s.size() - 1] == '\n' && s[s.size() - 2] != '\r' && s.find("\r\n") == string::npos) { // в конце \n и в буфере одна команда, т.е. почти наерняка это пришло через nc
    s[s.size() - 1] = '\r';
    s += '\n';
  }
  vector<string> parts;
  for(size_t pos = s.find("\r\n"); pos != string::npos; pos = s.find("\r\n")) {
    if(pos > 0)
      parts.push_back(s.substr(0, pos));
    s.erase(0, pos + 2);
  }
  if(s.size() > 0)
    cli->buf = s; // последний кусок сообщения, если он не заканчивается на \r\n (то есть по скорее всего начало следующей команды)
  else
    cli->buf = "";
  return parts;
}

int Server::prepareResp(Cli *to, string msg) {
  to->cmdsToSend.push_back(msg + "\r\n");
  return 0;
}

int Server::prepareResp(Ch *ch, string msg) {
  for(set<Cli*>::iterator to = ch->clis.begin(); to != ch->clis.end(); to++) 
    if((*to)->fd != cli->fd) // но некоторые команды надо и самому себе посылать
      prepareResp(*to, msg);
  return 0;
}

void Server::sendResp(Cli *to, string msg) {
  cout << "I send to fd=" << to->fd << "            : [" << without_r_n(msg) << "]\n";
  send(to->fd, msg.c_str(), msg.size(), MSG_NOSIGNAL); // flag ?
  for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)
    if (poll->fd == to->fd)
      poll->events = POLLIN;
}

void Server::sendAccumulatedResps(Cli *to) {
  while (to->cmdsToSend.size() > 0) {
    string msg = *(to->cmdsToSend.begin());
    to->cmdsToSend.erase(to->cmdsToSend.begin());
    sendResp(to, msg);
  }
}

/////////////////////////////////////////////////////////////////////// PRINCIPAL LOOP
void Server::init() {
  try {
    signal(SIGINT,  sigHandler);
    signal(SIGPIPE, SIG_IGN);    // to ignore the SIGPIPE signal ?
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
    for (map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); ++it) 
      if (it->second->cmdsToSend.size() > 0)
        for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)
          if (poll->fd == it->first) {
            poll->events = POLLOUT;
            break;
          }
    usleep(1000);
    int countEvents = poll(polls.data(), polls.size(), 1); // delai 1? 0? 1000 и без usleep?
    if (countEvents < 0)
      throw std::runtime_error("Poll error: [" + std::string(strerror(errno)) + "]");
    if(countEvents > 0) { // в сокетах есть данные
      for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++) // check sockets
        if((poll->revents & POLLIN) && poll->fd == fdForNewClis) {    // новый клиент подключился к сокету fdServ
          struct sockaddr sa;
          socklen_t       saLen = sizeof(sa);
          int fdForMsgs = accept(poll->fd, &sa, &saLen); // у каждого киента свой fd
          if(fdForMsgs == -1)
            perror("accept");
          else {
            // <hostname> has a maximum length of 63 characters !
            // Clients connecting from a host which name is longer than 63 characters are registered using the host (numeric) address instead of the host name.
            struct Cli *newCli = new Cli(fdForMsgs, inet_ntoa(((struct sockaddr_in*)&sa)->sin_addr));
            clis[fdForMsgs] = newCli;
            struct pollfd pollForMsgs = {fdForMsgs, POLLIN, 0};
            polls.push_back(pollForMsgs);
            cout << infoNewCli(fdForMsgs) << endl;
          }
          break ;
        }
        else if((poll->revents & POLLIN) && poll->fd != fdForNewClis) { // клиент прислал сообщение в свой fdForMsgs
          if(!(cli = clis.at(poll->fd)))
            continue ;
          vector<unsigned char> buf0(513);
          for(size_t i = 0; i < buf0.size(); i++)
            buf0[i] = '\0';
          int bytes = recv(cli->fd, buf0.data(), buf0.size() - 1, 0); // добавить MSG_NOSIGNAL ?
          if(bytes < 0)
            perror("recv");
          else if(bytes == 0) // клиент пропал
            execQuit();
          else {
            string buf = string(buf0.begin(), buf0.end());
            buf.resize(bytes);
            cout << without_r_n("I have received buf       : [" + buf + "] -> [" + cli->buf + buf + "]") << "\n";
            buf = cli->buf + buf;
            std::vector<string> cmds = split_r_n(buf);
            for(std::vector<string>::iterator cmd = cmds.begin(); cmd != cmds.end(); cmd++) {
              // for(int i = 0; i < ar.size(); i++)
              //   ar[i] = ""; !
              vector<string>().swap(ar);
              ar = split_space(*cmd);
              printCmd();
              execCmd();
            }
            cout << infoServ();
            cout << endl;
          }
        }
        else if (poll->revents & POLLOUT) {
          sendAccumulatedResps(clis.at(poll->fd));
        }
    }
  }
  std::cout << "Terminated\n";
}

//////////////////////////////////////////////////////////////////////////////////////// IRC COMMANDS
int Server::execCmd() {
  if(ar.size() == 0)
    return 0; //
  if(ar[0] == "PASS")
    return execPass();
  if(ar[0] == "NICK")
    return execNick();
  if(ar[0] == "USER")
    return execUser();
  if(ar[0] == "QUIT")
    return execQuit();
  if(ar[0] == "PING")
    return execPing();
  if(ar[0] == "CAP")
    return execCap();
  if(ar[0] == "WHOIS")
    return execWhois();
  std::cout << "here\n";
  if((!cli->passOk || cli->nick == "" || cli->uName == "" || !cli->capOk) && (ar[0] == "PRIVMSG" || ar[0] == "NOTICE" || ar[0] == "JOIN" || ar[0] == "PART" || ar[0] == "MODE" || ar[0] == "TOPIC" || ar[0] == "INVITE" || ar[0] == "KICK")) // нельзя выполнять без входа
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  std::cout << "here 2\n";
  if(ar[0] == "PRIVMSG" || ar[0] == "NOTICE")
    return execPrivmsg();
  if(ar[0] == "JOIN")
    return execJoin();
  if(ar[0] == "PART")
    return execPart();
  if(ar[0] == "MODE")
    return execMode();
  if(ar[0] == "TOPIC")
    return execTopic();
  if(ar[0] == "INVITE")
    return execInvite();
  if(ar[0] == "KICK")
    return execKick();
  return prepareResp(cli, "421 " + ar[0] + " " + " :is unknown mode char to me");       // ERR_UNKNOWNCOMMAND
}

// если неправильный пароль никакого сообщения?
int Server::execPass() {
  if(ar.size() < 2 || ar[1] == "")
    return prepareResp(cli, "461 PASS :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  if(cli->passOk)
    return prepareResp(cli, "462 :You may not reregister");                             // ERR_ALREADYREGISTRED
  if(ar[1] == pass)
    cli->passOk = true;
  if(cli->nick != "" && cli->uName != "" && cli->capOk)
    prepareResp(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

// not implemented here ERR_UNAVAILRESOURCE ERR_RESTRICTED ERR_NICKCOLLISION
int Server::execNick() {
  if(ar.size() < 2 || ar[1].size() == 0) 
    return prepareResp(cli, "431 :No nickname given");                                  // ERR_NONICKNAMEGIVEN
  if(ar[1].size() > 9 || ar[1].find_first_not_of("-[]^{}0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM") != string::npos)
    return prepareResp(cli, "432 " + ar[1] + " :Erroneus nickname"); // levensta :IRCat 432 rrrrrrrrrrrrrrrrrrrr :Erroneus nickname                   // ERR_ERRONEUSNICKNAME
  for(std::map<int, Cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++)
    if(ar[1].size() == itCli->second->nick.size()) {
      bool nicknameInUse = true;
      for(size_t i = 0; i < ar[1].size(); i++)
        if(std::tolower(ar[1][i]) != std::tolower(itCli->second->nick[i]))
          nicknameInUse = false;
      if (nicknameInUse)
        return prepareResp(cli, "433 " + ar[1] + " :Nickname is already in use"); // levensta: :IRCat 433  a :Nickname is already in use    // ERR_NICKNAMEINUSE
    }
  cli->nick = ar[1];
  if(cli->uName != "" && cli->passOk && cli->capOk)
    prepareResp(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

int Server::execUser() {
  if(ar.size() < 5)
    return prepareResp(cli, "461 USER :Not enough parameters"); // levensta :IRCat 461  USER :Not enough parameters                        // ERR_NEEDMOREPARAMS 
  if(cli->uName != "")
    return prepareResp(cli, "462 :You may not reregister"); // levensta регистрирует пользлвтаеля даже если rName = ""       // ERR_ALREADYREGISTRED
  cli->uName = ar[1];
  cli->rName = ar[4];
  if(cli->nick != "" && cli->passOk && cli->capOk)
    prepareResp(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

// not implemented here: ERR_CANNOTSENDTOCHAN ERR_NOTOPLEVEL ERR_WILDTOPLEVEL ERR_TOOMANYTARGETS RPL_AWAY
int Server::execPrivmsg() {
  if(ar.size() == 1) 
    return prepareResp(cli, "411 :No recipient given (" + ar[1] + ")"); // levensta :IRCat 411 a :No recipient given (PRIVMSG)                // ERR_NORECIPIENT
  if(ar.size() == 2)
    return prepareResp(cli, "412 :No text to send"); // levensta :IRCat 412 a :No text to send          // ERR_NOTEXTTOSEND
  vector<string> tos = split(ar[1], ',');
  for(vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    if((*to)[0] == '#' && chs.find(*to) == chs.end())
      prepareResp(cli, "401 " + *to + " :No such nick/channel"); // levensta :IRCat 401 a #ch :No such nick/channel     // ERR_NOSUCHNICK
    else if((*to)[0] == '#')
      prepareResp(chs[*to], "PRIVMSG " + *to + " :" + ar[2]);
    else if((*to)[0] != '#' && !getCli(*to))
      prepareResp(cli, "401 " + *to + " :No such nick/channel");                         // ERR_NOSUCHNICK
    else if((*to)[0] != '#')
      prepareResp(getCli(*to), "PRIVMSG " + *to + " :" + ar[2]);                         // ERR_NOSUCHNICK
  return 0;
}

// not implemented here: ERR_BANNEDFROMCHAN ERR_BADCHANMASK ERR_NOSUCHCHANNEL ERR_TOOMANYCHANNELS ERR_TOOMANYTARGETS ERR_UNAVAILRESOURCE 
// Once a user has joined a channel, he receives information about JOIN, MODE, KICK, QUIT, PRIVMSG/NOTICE
// levensta: если второй раз JOIN #ch, то ничего не происходит
int Server::execJoin() {
  if(ar.size() < 2)
    return prepareResp(cli, "461 JOIN :Not enough parameters"); // levensta :IRCat 461 a JOIN :Not enough parameters // ERR_NEEDMOREPARAMS 
  vector<string> chNames = split(ar[1], ',');
  vector<string> passes  = ar.size() >= 3 ? split(ar[2], ',') : vector<string>();
  for(vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++)
    if(chName->size() > 200 || (*chName)[0] != '#' || chName->find_first_of("\0") != string::npos) // ^G ?
      prepareResp(cli, "403 " + *chName + " :No such channel"); // levensta :IRCat 403 a ff :No such channel   // ERR_NOSUCHCHANNEL
    else {
      chs[*chName] = (chs.find(*chName) == chs.end()) ? new Ch(cli) : chs[*chName];
      string pass = "";
      if (passes.size() > 1) {
        string pass = *(passes.begin());
        passes.erase(passes.begin());
      }
      if(chs[*chName]->pass != "" && pass != chs[*chName]->pass)
        prepareResp(cli, "475 :" + *chName + " Cannot join channel (+k)");       // ERR_BADCHANNELKEY
      if(chs[*chName]->size() >= chs[*chName]->limit)
        prepareResp(cli, "471 " + *chName + " :Cannot join channel (+l)");       // ERR_CHANNELISFULL
      else if(chs[*chName]->optI && cli->invits.find(*chName) == cli->invits.end())
        prepareResp(cli, "473 " + *chName + " :Cannot join channel (+i)");       // ERR_INVITEONLYCHAN
      else {
        chs[*chName]->clis.insert(cli);
        prepareResp(chs[*chName], cli->nick + " JOIN " + *chName);
        prepareResp(cli, "332 " + *chName + " " + chs[*chName]->topic);          // RPL_TOPIC ?
        prepareResp(cli, "353 " + *chName + " " /* перечилсить все ники*/);      // RPL_NAMREPLY ?
      }
    }
  return 0;
}

int Server::execPart() {
  if(ar.size() < 2)
    return prepareResp(cli, "461 PART :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  vector<string> chNames = split(ar[1], ',');
  for(vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++)
    if(chs.find(*chName) == chs.end())
      prepareResp(cli, "403 :" + *chName + " :No such channel");                 // ERR_NOSUCHCHANNEL
    else if(chs[*chName]->clis.find(cli) == chs[*chName]->clis.end())
      prepareResp(cli, "442 " + *chName + " :You're not on that channel");       // ERR_NOTONCHANNEL
    else {
      prepareResp(chs[*chName], cli->nick + " PART :" + *chName);           // нужно ли сообщение для автора команды?
      chs.erase(*chName);
      if(chs[*chName]->size() == 0)
        erase(chs[*chName]);
      else if(chs[*chName]->adms.size() == 0)
        chs[*chName]->adms.insert(*(chs[*chName]->clis.begin())); // сделать самого старого пользователя админом
    }
  return 0;
}

// not implemented here RPL_AWAY
int Server::execInvite() {
  if(ar.size() < 3)
    return prepareResp(cli, "461 INVITE :Not enough parameters");                       // ERR_NEEDMOREPARAMS 
  if(chs.find(ar[2]) == chs.end())
    return prepareResp(cli, "403 " + ar[2] + " :No such channel");                      // ERR_NOSUCHCHANNEL ?
  if(chs[ar[2]]->adms.find(cli) == chs[ar[2]]->adms.end()) 
    return prepareResp(cli, "482 " + ar[2] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
  if(chs[ar[2]]->clis.find(cli) == chs[ar[2]]->clis.end()) 
    return prepareResp(cli, "442 " + ar[2] + " :You're not on that channel");           // ERR_NOTONCHANNEL
  if(getCli(ar[1]) == NULL)
    return prepareResp(cli, "401 :" + ar[1] + " No such nick");                         // ERR_NOSUCHNICK
  if(chs[ar[2]]->clis.find(cli) != chs[ar[2]]->clis.end()) 
    return prepareResp(cli, "443 " + ar[1] + " " + ar[2] + " :is already on channel");  // ERR_USERONCHANNEL
  getCli(ar[1])->invits.insert(ar[2]);
  return prepareResp(chs[ar[2]], "341" + ar[2] + " " + ar[1]);                          // RPL_INVITING
}

// not implemented here ERR_NOCHANMODES
int Server::execTopic() {
  if(ar.size() < 1)
    return prepareResp(cli, "461 TOPIC :Not enough parameters");                        // ERR_NEEDMOREPARAMS
  if(chs.find(ar[1]) == chs.end())
    return prepareResp(cli, "403 " + ar[1] + " :No such channel");                      // ERR_NOSUCHCHANNEL
  if(chs[ar[1]]->clis.empty() || chs[ar[1]]->clis.find(cli) == chs[ar[1]]->clis.end()) 
    return prepareResp(cli, "442 " + ar[1] + " :You're not on that channe");            // ERR_NOTONCHANNEL
  if(chs[ar[1]]->adms.find(cli) == chs[ar[1]]->adms.end()) 
    return prepareResp(cli, "482 " + ar[1] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
  if(ar.size() == 2) {
    chs[ar[1]]->topic = "";
    return prepareResp(chs[ar[2]], "331 " + ar[1] + " :No topic is set");               // RPL_NOTOPIC
  }
  chs[ar[1]]->topic = ar[2];
  return prepareResp(chs[ar[1]], "332 " + ar[1] + " :" + ar[2]);                        // RPL_TOPIC
}

// not implemented here ERR_BADCHANMASK
int Server::execKick() {
  if(ar.size() < 3)
    return prepareResp(cli, "461 KICK :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  std::vector<string> chNames    = split(ar[1], ',');
  std::vector<string> targetClis = split(ar[2], ',');
  for(vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++)
    if(chs.find(*chName) == chs.end())
      prepareResp(cli, "403 " + *chName + " :No such channel");                         // ERR_NOSUCHCHANNEL
    else if(chs[*chName]->clis.empty() || chs[*chName]->clis.find(cli) == chs[*chName]->clis.end()) 
      prepareResp(cli, "442 " + *chName + " :You're not on that channe");               // ERR_NOTONCHANNEL
    else if(chs[*chName]->adms.find(cli) == chs[*chName]->adms.end()) 
      prepareResp(cli, "482 " + *chName + " :You're not channel operator");             // ERR_CHANOPRIVSNEEDED
    else {
      for(vector<string>::iterator targetCli = targetClis.begin(); targetCli != targetClis.end(); targetCli++)
        if(chs[*chName]->clis.empty() || chs[*chName]->clis.find(cli) == chs[*chName]->clis.end())
          prepareResp(cli, "441 " + *targetCli + " " + *chName + " :They aren't on that channel"); // ERR_USERNOTINCHANNEL
        else if(chs[*chName]->clis.size() > 0 && chs[*chName]->clis.find(getCli(*targetCli)) != chs[*chName]->clis.end()) {
          chs[*chName]->erase(*targetCli);                                        // send_(chs[*chName], " KICK"); ?
          if(chs[*chName]->size() == 0)
            erase(chs[*chName]);
        }
    }
  return 0;
}

int Server::execQuit() {
  vector<string> chsToRm;
  for(map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++) {
    ch->second->erase(cli);
    if(ch->second->size() == 0)
      chsToRm.push_back(ch->first);
  }
  for(vector<string>::iterator ch = chsToRm.begin(); ch != chsToRm.end(); ch++)
    chs.erase(*ch);
  close(clis.find(cli->fd)->first);
  //polls.erase(std::remove(polls.begin(), polls.end(), cli->fd), polls.end());
  clis.erase(clis.find(cli->fd));
  return 0;
}

// not implemented here: ERR_NOCHANMODES RPL_BANLIST RPL_ENDOFBANLIST RPL_EXCEPTLIST RPL_ENDOFEXCEPTLIST RPL_INVITELIST RPL_ENDOFINVITELIST RPL_UNIQOPIS (creator of the channel)
// `MODE` устанвливает один параметр за раз, например `MODE -t` должна работать, а `MODE -tpk` нет, нормально ли это?
// админа обознгачать "nick@" всякий раз, когда он ассоциируется с каналом
int Server::execMode() {
  char *notUsed; // ?
  if(ar.size() < 2)
    return prepareResp(cli, "461 MODE :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  if(chs.find(ar[1]) == chs.end())
    return prepareResp(cli, "403 " + ar[1] + " :No such channel"); // levensta :IRCat 403 a #ch :No such channel // ERR_NOSUCHCHANNEL ???
  if(chs[ar[1]]->clis.empty() || chs[ar[1]]->clis.find(cli) == chs[ar[1]]->clis.end()) 
    return prepareResp(cli, "442 " + ar[1] + " :You're not on that channel");           // ERR_NOTONCHANNEL ???
  if(chs[ar[1]]->adms.find(cli) == chs[ar[1]]->adms.end())
    return prepareResp(cli, "482 " + ar[1] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
  if(ar.size() == 2)
    return prepareResp(cli, ar[1] + " mode = " + mode(chs[ar[1]]));                     // RPL_CHANNELMODEIS
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
    return prepareResp(cli, "461 MODE :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  if(ar.size() == 3)
    return prepareResp(cli, "461 MODE :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  if(ar[2] == "+k" && chs[ar[1]]->pass != "")
    return prepareResp(cli, ar[1] + " :Channel key already set");                       // ERR_KEYSET
  if(ar[2] == "+k")
    return (chs[ar[1]]->pass = ar[3], 0); // levensta :a!a@127.0.0.1 MODE #ch +k
  if(ar[2] == "+l" && atoi(ar[3].c_str()) >= static_cast<int>(0) && static_cast<unsigned int>(atoi(ar[3].c_str())) <= std::numeric_limits<unsigned int>::max())
    return (chs[ar[1]]->limit = static_cast<int>(strtol(ar[3].c_str(), &notUsed, 10)), 0);
  if(ar[2] == "+o")
    return prepareResp(cli, ar[3] + " " + ar[1] + " :They aren't on that channel");     // ERR_USERNOTINCHANNEL
  if(ar[2] == "-o")
    return chs[ar[1]]->adms.erase(getCli(ar[3]));
  return prepareResp(cli, ar[0] + " " + ar[1] + " " + ar[2] + " " + ar[3] + " :is unknown mode char to me"); // ERR_UNKNOWNMODE
}

int Server::execPing() {
  return prepareResp(cli, "PONG");
}

// not implemented here: RPL_WHOISCHANNELS RPL_WHOISOPERATOR RPL_AWAY RPL_WHOISIDLE
int Server::execWhois() {
  if(ar.size() < 2)
    return prepareResp(cli, "431 :No nickname given");                                  // ERR_NONICKNAMEGIVEN
  std::vector<string> nicks = split(ar[1], ',');
  for(vector<string>::iterator nick = nicks.begin(); nick != nicks.end(); nick++)
    if(getCli(ar[1]) == NULL)
      prepareResp(cli, "401 :" + ar[1] + " No such nick");                            // ERR_NOSUCHNICK
    else
      prepareResp(cli, getCli(ar[1])->nick + " " + getCli(ar[1])->uName + " " + getCli(ar[1])->host + " * :" + getCli(ar[1])->rName); // RPL_WHOISUSER
  return prepareResp(cli, "318" + nicks[0] + " :End of WHOIS list");              // RPL_ENDOFWHOIS ?
}

int Server::execCap() {
  if(ar.size() >= 2 && ar[1] == "LS") {
    cli->capOk = false;
    return prepareResp(cli, "CAP * LS :");
  }
  else if(ar.size() >= 2 && ar[1] == "END") {
    cli->capOk = true;
    prepareResp(cli, "001"); // :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME целиком не отпраляется.!
  }
  return 0;
}

int main(int argc, char *argv[]) {
  // проверить с помощью strtol, не выходим ли за рамки int !
  if(argc != 3 || atoi(argv[1]) < 1024 || atoi(argv[1]) > 65535) {
    std::cout << "Invalid arguments.\nRun ./ircserv <port> <password>, port should be between 1024 and 65535\n";
    return 0;
  }
  Server s = Server(argv[1], argv[2]);
  try {
    s.init();
    s.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 0;
  }
  return 0;
}