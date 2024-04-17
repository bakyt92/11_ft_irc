#include "Server.hpp"
////////////////////////////////////////////////////////////////////////////// UTILS
Server::~Server() {
  // close all fd 
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

int send_(int fd, string msg) {
  if (msg != "")
    send(fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
  return 0;
}

int send_(set<Cli*> clis, string msg) {
  for (set<Cli*>::iterator cli = clis.begin(); cli != clis.end(); cli++) 
    send_((*cli)->fd, msg);
  return 0;
}

int send_(map<int, Cli*> clis, string msg) {
  for (map<int, Cli*>::iterator cli = clis.begin(); cli != clis.end(); cli++) 
    send_(cli->second->fd, msg);
  return 0;
}

Cli* Server::getCli(string &name) {
  for(map<int, Cli* >::iterator cli = clis.begin(); cli != clis.end(); cli++)
    if (cli->second->nick == name)
      return cli->second;
  return NULL;
}

string toString(vector<string> v) { // только для отладки
  string res = "";
  for (vector<string>::iterator it = v.begin(); it != v.end(); it++)
    res += "[" + *it + "] ";
  return res;
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
  int opt = 1;
  for (it = list_ai; it != NULL; it = it->ai_next) {
    if ((fdForNewClis = socket(it->ai_family, it->ai_socktype, it->ai_protocol)) < 0) 
      throw(std::runtime_error("socket"));
    else if (setsockopt(fdForNewClis, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)))
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
  struct pollfd pollForNeewClis = {fdForNewClis, POLLIN, 0};
  polls.push_back(pollForNeewClis);
}

void Server::run() {
  std::cout << "Server is running. Waiting clients to connect >>>\n";
  while (sigReceived == false) {
    if (poll(polls.data(), polls.size(), 1) > 0) {              // poll() в сокетах есть какие-то данные
      for (std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)
        if((poll->revents & POLLIN)) cout << poll->fd << " данные в сокете " << endl;
      for (std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++) // check sockets
        if ((poll->revents & POLLIN) && poll->fd == fdForNewClis) {     // новый клиент подключился к сокету fdServ
          struct sockaddr sa; 
          socklen_t       saLen = sizeof(sa);
          fdForMsgs = accept(poll->fd, &sa, &saLen);
          cout << poll->fd << " new cli, fdForMsgs = " << fdForMsgs << endl;
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
          cout << poll->fd << " recv\n";
          int bytes = recv(cli->fd, buf.data(), buf.size(), 0); 
          cout << poll->fd << " bytes =  " << bytes << endl;
          if (bytes < 0) 
            perror("recv");
          else if (bytes == 0)                                         // клиент отключился
            execQuit();
          else {
            string bufS = string(buf.begin(), buf.end());
            bufS.resize(bytes);
            std::vector<string> cmds = split(bufS, '\n');             // if empty ? 
            for (std::vector<string>::iterator cmd = cmds.begin(); cmd != cmds.end(); cmd++) {
              cout << "I execute " << *cmd << endl;
              args = split(*cmd, ' ');                                // if (args[0][0] == ':') args.erase(args.begin()); // нужен ли префикс?
              exec();
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
  return send_(cli->fd, ": server 421 " + cli->nick + " " + args[0] + " ERR_UNKNOWNCOMMAND\n");
}

int Server::execPass() { 
  if (args.size() < 2)
    return send_(cli->fd, "PASS :Not enough parameters\n");               // ERR_NEEDMOREPARAMS 
  if (cli->passOk)
    return send_(cli->fd, ":You may not reregister\n");                   // ERR_ALREADYREGISTRED 
  cli->passOk = true;
  return 0;
}

int Server::execNick() {
  if (args.size() < 2) 
    return send_(cli->fd, ":No nickname given\n");                        // ERR_NONICKNAMEGIVEN
  for (size_t i = 0; i < args[1].size() && args[1].size() <= 9; ++i)
    if (string("-[]^{}0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM").find(args[1][i]) == string::npos) // 9 chars: a-z A-Z 0-9 - [ ] ^ { }
       return send_(cli->fd, args[1] + " :Erroneus nickname\n");           // ERR_ERRONEUSNICKNAME
  for (std::map<int, Cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++) {
    if (itCli->second->nick.size() == args[1].size()) {
      bool nickInUse = true;
      for (size_t i = -1; i < args[1].size(); i++)
        if (std::tolower(args[1][i]) != std::tolower(itCli->second->nick[i]))
          nickInUse = false;
      if (nickInUse)
        return send_(cli->fd, args[1] + " :Nickname collision\n");         // ERR_NICKNAMEINUSE
    }
  }
  cli->nick = args[1];
  if (cli->uName != "")
    send_(cli->fd, cli->nick + " " + cli->uName + "\n");
  return 0;
}

int Server::execUser() { // args[2] у нас не испольутеся
  if (args.size() < 5)
    return send_(cli->fd, "USER :Not enough parameters\n");               // ERR_NEEDMOREPARAMS 
  if (cli->uName != "")
    return send_(cli->fd, ":You may not reregister\n");                   // ERR_ALREADYREGISTRED 
  cli->uName = args[1];
  cli->rName = args[4];
  if (cli->nick != "")
    send_(cli->fd, cli->nick + " " + cli->uName + "\n");
  return 0;
}

// ERR_NOTOPLEVEL ERR_WILDTOPLEVEL ещё есть эти две, кажется они нам не нужны       
int Server::execPrivmsg() {
  if (cli->passOk)
    return send_(cli->fd, cli->nick + " :User not logged in" );            // ERR_NOLOGIN ? ERR_NOTREGISTERED ?
  if (args.size() == 1) 
    return send_(cli->fd, ":No recipient given (" + args[0] + ")\n");     // ERR_NORECIPIENT
  if (args.size() == 2)
    return send_(cli->fd, ":No text to send\n");                          // ERR_NOTEXTTOSEND
  vector<string> tos = split(args[1], ',');
  for (vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    //if (to->compare(0, 2, "#*") == 0 && !chs.at(*to))                   // a host mask (#<mask>)? или канал?
    //  send_(cli->fd, *to + " :No such nick/channel\n");                 // ERR_NOSUCHNICK
    //else if (to->compare(0, 2, "#*") == 0 && chs.at(*to))                 
    //  send_(chs.at(*to)->clis, ":" + cli->nick + "!" + cli->rName + "@" + cli->host + " PRIVMSG " + ((chs.at(*to)->name == "" ? to->nick : chs.at(*to)->name) + (msg[0] == ':' ? " " : " :") + msg) + "\n");
    if(!getCli(*to))
      send_(cli->fd, *to + " :No such nick/channel\n");                   // ERR_NOSUCHNICK
    else
      send_((getCli(*to))->fd, ":" + cli->nick + "!" + cli->rName + "@" + cli->host + getCli(*to)->nick + " :" + args[2] + "\n");
  return 0;
}


int Server::execJoin() { // вмещает в себе MODE KICK PART QUIT PRIVMSG/NOTICE
  if (cli->passOk)
    return send_(cli->fd, cli->nick + " :User not logged in" );            // ERR_NOLOGIN 
  if (args.size() < 2)
    return send_(cli->fd, ": server 461 JOIN ERR_NEEDMOREPARAMS\n");
  Chan *ch;
  vector<string> chNames = split(args[1], ',');
  vector<string> passes;
  if (args.size() > 2)
    passes = split(args[2], ',');
  unsigned int k = 0;
  for (vector<string>::iterator itCh = chNames.begin(); itCh != chNames.end(); itCh++, k++) {
    ch = chs.at(*itCh);
    string pass = passes.size() > k ? passes[k] : "";
    if (ch != NULL) {
      if (ch->pass != "" && pass != ch->pass)
        return send_(cli->fd,  ": 475 " + *itCh + " ERR_BADCHANNELKEY\n");
      if (ch->clis.size() >= ch->limit)
        return send_(cli->fd,  ": 471 " + *itCh + " ERR_CHANNELISFULL\n");
      if (ch->optI) //  && cli->invitations.insert(ch->name))               
        return send_(cli->fd,  ": 473 " + *itCh + " ERR_INVITEONLYCHAN\n");
      // if (std::find(adms.begin(), adms.end(), cli) == adms.end()) { // func addCLi
      //   clis.insert(cli);
      // else if (!ch->addCli(cli)) {
      //   return send_(ch->clis, ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " JOIN :" + ch->name + "\n");
      //   exec(cliFd, "TOPIC " + ch->name);
      //   exec(cliFd, "MODE "  + ch->name);
      // }
    // } else if (createCh(chs[i], cli)) {
    //           // int Server::createCh(string &ch, s_cli *creator) {
    //           //   if((ch[0] != '&' && ch[0] != '#') || ch.size() > 200 || ch.find(",") != string::npos || ch.find(" ") != string::npos)
    //           //     return -1;
    //           //   chs[ch] = new s_ch(ch, creator);
    //           //   return 0;
    //           // }
    //   return send_(cliFd, ": 403 " + chs[i] + " ERR_NOSUCHCHANNEL\n");
          // создание канала
          // s_ch::s_ch(string name_, s_cli* adm) : name(name_), optI(false), optT(false), pass(""), limit(numeric_limits<unsigned int>::max()) {
          //   clis.insert(adm);
          //   adms.insert(adm);
          // }
    } else {
      ch = chs.at(*itCh);
      if(passes.size() > k && std::find(ch->adms.begin(), ch->adms.end(), cli) == ch->adms.end())
        return send_(cli->fd,  ": 482 " + *itCh + " ERR_CHANOPRIVSNEEDED\n");
      if(passes.size() > k && std::find(ch->adms.begin(), ch->adms.end(), cli) != ch->adms.end())
        ch->pass = passes[k];
      set<Cli*> _clList = ch->clis;
      //for (unsigned long i = 0; i < _clList.size(); i++)
      //  send_(_clList[i]->cli->fd, ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " JOIN :" + ch->name + "\n");
      //exec(cli->fd, "TOPIC " + ch->name);
      //exec(cli->fd, "MODE "  + ch->name);
    }
  }
  return 1;
}

int Server::execInvite() { ///////////////////////
  if (cli->passOk)
    return send_(cli->fd, cli->nick + " :User not logged in" );            // ERR_NOLOGIN 
  if (args.size() < 3 || string("&#").find(args[2][0]) == string::npos)
    return send_(cli->fd, ": 461 " + cli->nick + " INVITE ERR_NEEDMOREPARAMS\n"); 
  Chan *ch = chs.at(args[2]);
  if (!ch)
    return send_(cli->fd, ": 403 " + cli->nick + " INVITE ERR_NOSUCHCHANNEL\n");
  if (!cli)
    return send_(cli->fd, ": 401 " + cli->nick + " INVITE ERR_NOSUCHNICK\n");
  if (std::find(ch->clis.begin(), ch->clis.end(), cli) == ch->clis.end())
    return send_(cli->fd, ": 443 " + cli->nick + " INVITE ERR_USERONCHANNEL\n");
  if (ch->optI && std::find(ch->adms.begin(), ch->adms.end(), cli) == ch->adms.end())
    return send_(cli->fd, ": 482 " + cli->nick + " INVITE ERR_CHANOPRIVSNEEDED\n");
  cli->invitations.insert(args[2]);
  return send_(cli->fd, ": 341 " + cli->nick + " INVITE RPL_INVITING\n");
}

int Server::execTopic() {
  if (cli->passOk)
    return send_(cli->fd, cli->nick + " :User not logged in" );            // ERR_NOLOGIN 
  if (args.size() < 2)
    return send_(cli->fd, ": 461 " + cli->nick + " " + args[0] + " ERR_NEEDMOREPARAMS\n");
  Chan *ch = chs.at(args[1]);
  if (!ch)
    return send_(cli->fd, ": 403 " + cli->nick + " " + args[0] + " ERR_NOSUCHs_ch\n");
  if (std::find(ch->clis.begin(), ch->clis.end(), cli) == ch->clis.end())
    return send_(cli->fd, ": 442 " + cli->nick + " " + args[0] + " ERR_NOTONs_ch\n");
  if (args.size() == 2 && ch->topic.empty()) 
    return send_(cli->fd, ": 331 " + cli->nick + " " + ch->name + " :" + ((ch->topic.empty()) ? "No topic is set" : ch->topic) + " RPL_NOTOPIC\n");
  if (args.size() == 2 && !ch->topic.empty())
    return send_(cli->fd, ": 332 " + cli->nick + " " + ch->name + " :" + ((ch->topic.empty()) ? "No topic is set" : ch->topic) + " RPL_TOPIC\n");
  if (!ch->optT || std::find(ch->adms.begin(), ch->adms.end(), cli) != ch->adms.end())
    return send_(cli->fd, ": 482 " + cli->nick + " TOPIC ERR_CHANOPRIVSNEEDED\n");
  ch->topic = args[2];
  if (!ch)
    send_(cli->fd, ": 401 " + ch->name + " ERR_NOSUCHNICK\n");
  else {
    string msg = args[2][0] == ':' ? args[2].data() + 1 : args[2];
    //send_(ch->clis, ":" + cli->nick + "!" + cli->rName + "@" + cli->host + " PRIVMSG " + ((ch->name == "" ? (*it)->nick : ch->name) + (msg[0] == ':' ? " " : " :") + msg) + "\n");
  }
  string msg = ch->name + " :" + ch->topic;
  send_(cli->fd, ": 332 " + cli->nick + " " + ch->name + " :" + ((ch->topic.empty()) ? "No topic is set" : ch->topic) + " RPL_TOPIC\n");
  return send_(cli->fd, ": " + cli->nick + " " + args[0] + "\n");
}

// bool Cli::getInvitation(const string &ch) {
//   if (invitations.find(ch) != invitations.end()) {
//     invitations.erase(invitations.find(ch));
//     return true;
//   }
//   return false;
// }

int Server::execMode() {
  if (cli->passOk)
    return send_(cli->fd, cli->nick + " :User not logged in" );            // ERR_NOLOGIN 
  if (args.size() < 2)
    return send_(cli->fd, ": 461 " +  cli->nick + " MODE ERR_NEEDMOREPARAMS\n");
  Chan  *ch = chs.at(args[1]);
  if (!ch)
    return send_(cli->fd, ": 403 " + cli->nick + " " + args[1] + " ERR_NOSUCHCHANNEL\n");
  if(std::find(ch->clis.begin(), ch->clis.end(), cli) == ch->clis.end())
    return send_(cli->fd, ": 442 " + cli->nick + " " + args[1] + " ERR_NOTONCHANNEL\n");
  if (args.size() == 2) // show
    return send_(cli->fd, ": 324 " + cli->nick + " " + ch->name + " RPL_CHANNELMODEIS\n"); // тут Борис показывает ещё флаги канала
  if (string("-+").find(args[2][0]) == string::npos)
    return send_(cli->fd, ": 501 " + cli->nick + " " + args[2].substr(0, 1) + " ERR_UMODEUNKNOWNFLAG\n");
  //for (vector<string>::iterator itCh = chNames.begin(); itCh != chNames.end(); itCh++) {
  for (unsigned long i = 1; i < args[2].size(); i++) {
    //int res;
    bool isAdding = (args[2][0] == '+');
    switch (args[2][i]) {
      case 'i':
      case 't':
      case 'l':
        if (isAdding) {
          //const char *c = args[3].c_str();
          char       *ptr;
          int        limit = static_cast<int> (strtol(args[3].c_str(), &ptr, 10));
          if (args[3].c_str() == ptr)
            send_(cli->fd, ": 461 " + cli->nick + " " + args[1] + " ERR_NEEDMOREPARAMS\n");
          else if (std::find(ch->adms.begin(), ch->adms.end(), cli) == ch->adms.end())
            send_(cli->fd, ": 461 " + cli->nick + " " + args[1] + " ERR_NEEDMOREPARAMS\n");
          else 
            ch->limit = limit;
        } 
        else if (std::find(ch->adms.begin(), ch->adms.end(), cli) == ch->adms.end())
          send_(cli->fd, ": 482 " + cli->nick + " " + args[1] + " ERR_CHANOPRIVSNEEDED\n");
        else
          ch->limit = std::numeric_limits<unsigned int>::max();
        break;
      case 'k':
        if (args.size() < 4)
          send_(cli->fd, ": 461 " + cli->nick + " " + args[1] + " ERR_NEEDMOREPARAMS\n");
        else if (ch->pass != "")
          send_(cli->fd, ": 467 " + cli->nick + " " + args[1] + " ERR_KEYSET\n");
        else if(std::find(ch->adms.begin(), ch->adms.end(), cli) == ch->adms.end()) 
          send_(cli->fd,  ": 482 " + cli->nick + " " + args[1] + " ERR_CHANOPRIVSNEEDED\n");
        else if(std::find(ch->adms.begin(), ch->adms.end(), cli) != ch->adms.end())
          ch->pass = args[3];
        break;
      case 'o':
        if (args.size() < 4)
          send_(cli->fd, ": 461 " + cli->nick + " " + args[1] + " ERR_NEEDMOREPARAMS\n");
          // проверить на привелении адс   if (std::find(adms.begin(), adms.end(), from) == adms.end()) return 1;//ERR_CHANOPRIVSNEEDED;
          // else if (getCliByName(args[3]) && isAdding) { 
          //   res = ch->becomesAdmin(cli, getCliByName(args[3]));
                    // adms.insert(cliTo);
                    // string msg = ":" + from->nick + "!" + from->uName + "@127.0.0.1 MODE " + name + " +o " + cliTo->nick + "\n";
                    // for (set<s_cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++)
                    //   send(itCli->fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
                    // std::cout << msg;
          //   send_(fd, ": " + cli->nick + " " + args[1] + " res + \n"); // itoa(res) ///
          // }
          else if (getCli(args[3]) && !isAdding) {
          //res = ch->becomeOrdinaryUser(cli, getCliByName(args[3]));
                // int s_ch::becomeOrdinaryUser(s_cli *from, s_cli *cliTo) {
                //   set<s_cli *>::iterator itTo = std::find(adms.begin(), adms.end(), cliTo);
                //   if (std::find(adms.begin(), adms.end(), from) == adms.end())
                //     return 1; // ERR_CHANOPRIVSNEEDED;
                //   else if (itTo != adms.end()) {
                //     adms.erase(itTo);
                //     string msg = ":" + from->nick + "!" + from->uName + "@127.0.0.1 MODE " + name + " -o " + cliTo->nick + "\n";
                //     for (set<s_cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++)
                //       send((itCli->fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
                //     std::cout << msg;
                //   }
                //   return 0;
                // }
          send_(cli->fd, ": " + cli->nick + " " + args[1] + " res + \n"); // itoa(res) ///
        } 
        else
          send_(cli->fd, ": 401 " + cli->nick+ " " + ch->name + " ERR_NOSUCHNICK\n");
        break;
      default:
        send_(cli->fd, ": 472 " + cli->nick + " " + (args[0] + " " + (isAdding ? "+" : "-") + args[2][i]) + " ERR_UNKNOWNMODE\n");
    }
    if (string("psitnm").find(args[2][i]) != string::npos) {
      // msg = ch->name + (isAdding ? " +" : " -") + args[2][i];
      // ch->sendMessageToCh(RPL_s_chMODEIS, msg);                  // разобраться
    }
  }
  return 0;
}

int Server::execQuit() {
  if (!cli) 
    return 0;
  for (map<string, Chan*>::iterator itCh = chs.begin(); itCh != chs.end(); itCh++) {
    Chan *ch = itCh->second;
    if(!ch || std::find(ch->clis.begin(), ch->clis.end(), cli) == ch->clis.end()) 
      continue;
    send_(ch->clis, ": server ! @" + cli->host + " " + "QUIT " + ch->name + ":" + (args.size() > 1 ? args[1] : "") + "\n");
    // ch->clis.erase(std::remove(ch->clis.begin(), ch->clis.end(), cli), ch->clis.end());
    //ch->adms.erase(std::remove(ch->adms.begin(), ch->adms.end(), cli), ch->adms.end());
    // //if (h->getClis().size() == 0) 
    //  delete ch; continue;
    //if (ch->adms.size() == 0) 
    //  ch->becomesAdmin(cli, ch->clis[i]);
          // adms.insert(cliTo);
          // string msg = ":" + from->nick + "!" + from->uName + "@127.0.0.1 MODE " + name + " +o " + cliTo->nick + "\n";
          // for (set<s_cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++)
          //   send(itCli->fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
          // std::cout << msg;
    // проверить на привелении адс   if (std::find(adms.begin(), adms.end(), from) == adms.end()) return 1;//ERR_CHANOPRIVSNEEDED;
    }
  if(clis.find(cli->fd) == clis.end())
    return 0;
  close(clis.find(cli->fd)->first);
  // if (pollsCLi.find(fd_) == fd_) {
  //   pollsCLi.erase(pollsCLi. begin() + i);
  // delete it->second;
  //clis.erase(clis.find(fd_));
  return 0;
}

int Server::execKick() {
  if (cli->passOk)
    return send_(cli->fd, cli->nick + " :User not logged in" );            // ERR_NOLOGIN 
  if (args.size() < 3) 
    return send_(cli->fd, ": 461 KICK ERR_NEEDMOREPARAMS\n");
  std::vector<string> chNames  = split(args[1], ',');
  std::vector<string> clis = split(args[2], ',');
  for (vector<string>::iterator it = chNames.begin(); it != chNames.end(); it++) {
    Chan *ch = chs.at(*it);
    if (!ch) 
      send_(cli->fd, ": 403 " + *it + " ERR_NOSUCHCHANNEL\n");
    else if(std::find(ch->clis.begin(), ch->clis.end(), cli) == ch->clis.end())
      send_(cli->fd, ": 442 " +  *it + " ERR_NOTONCHANNEL\n");
    else if (std::find(ch->adms.begin(), ch->adms.end(), cli) == ch->adms.end())
      send_(cli->fd, ": 482 " + *it + " ERR_CHANOPRIVSNEEDED\n");
    else
      for (vector<string>::iterator it = clis.begin(); it != clis.end(); it++) {
        Cli *cli = getCli(*it);
        if(std::find(ch->clis.begin(), ch->clis.end(), cli) == ch->clis.end()) {
          for (set<Cli*>::iterator it = ch->clis.begin(); it != ch->clis.end(); it++)
            send_((*it)->fd, ": server ! @" + cli->host + " KICK " + ch->name + " " + (args.size() > 3 ? (*it)->nick + " :" + args[3] :  ":" + (*it)->nick) + "\n");
          // ch->deleteCli(cli);
          // deleteEmptyCh(*ch);
        }
      }
  }
  return 0;
}

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