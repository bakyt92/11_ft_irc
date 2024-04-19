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
    return ;                                                                // ?
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
    return send_(cli, args[2] + " :No such channel\n");                     // ERR_NOSUCHANNEL ? 
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
    return send_(cli, args[1] + " :No such channel\n");                     // ERR_NOSUCHANNEL
  if(chs[args[1]]->clis.empty() || chs[args[1]]->clis.find(cli) == chs[args[1]]->clis.end()) 
    return send_(cli, args[1] + " :You're not on that channe\n");           // ERR_NOTONCHANNEL
  if(chs[args[1]]->optT && chs[args[1]]->adms.find(cli) == chs[args[1]]->adms.end()) 
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
      send_(cli, *chName + " :No such channel\n");                          // ERR_NOSUCHANNEL
    else if(chs[*chName]->clis.empty() || chs[*chName]->clis.find(cli) == chs[*chName]->clis.end()) 
      send_(cli, *chName + " :You're not on that channe\n");                // ERR_NOTONCHANNEL
    else if(chs[*chName]->optT && chs[*chName]->adms.find(cli) == chs[*chName]->adms.end()) 
      send_(cli, *chName + " :You're not channel operator\n");              // ERR_CHANOPRIVSNEEDED
    else
      for (vector<string>::iterator targetCli = targetClis.begin(); targetCli != targetClis.end(); targetCli++)
        if(chs[*chName]->clis.size() > 0 && chs[*chName]->clis.find(getCli(*targetCli)) != chs[*chName]->clis.end())
          chs[*chName]->erase(*targetCli);                                  // send_(chs[*chName], " KICK \n"); ?
      // if (chs[*chName]->size() == 0)
      //   chs.erase(*chName);
  }
  return 0;
}

int Server::execQuit() {
  for (map<string, Ch*>::iterator itCh = chs.begin(); itCh != chs.end(); itCh++) {
    Ch *ch = itCh->second;
    if(!ch || std::find(ch->clis.begin(), ch->clis.end(), cli) == ch->clis.end()) 
      continue;
    //send_(ch->clis, ": server ! @" + cli->host + " " + "QUIT " + ch->name + ":" + (args.size() > 1 ? args[1] : "") + "\n");
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

int Server::execMode() {
  if (cli->passOk)
    return send_(cli, cli->nick + " :User not logged in" );            // ERR_NOLOGIN 
  if (args.size() < 2)
    return send_(cli, ": 461 " +  cli->nick + " MODE ERR_NEEDMOREPARAMS\n");
  Ch  *ch = chs.at(args[1]);
  if (!ch)
    return send_(cli, ": 403 " + cli->nick + " " + args[1] + " ERR_NOSUCHCHANNEL\n");
  if(std::find(ch->clis.begin(), ch->clis.end(), cli) == ch->clis.end())
    return send_(cli, ": 442 " + cli->nick + " " + args[1] + " ERR_NOTONCHANNEL\n");
  // if (args.size() == 2) // show
  //   return send_(cli->fd, ": 324 " + cli->nick + " " + ch->name + " RPL_CHANNELMODEIS\n"); // тут Борис показывает ещё флаги канала
  if (string("-+").find(args[2][0]) == string::npos)
    return send_(cli, ": 501 " + cli->nick + " " + args[2].substr(0, 1) + " ERR_UMODEUNKNOWNFLAG\n");
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
            send_(cli, ": 461 " + cli->nick + " " + args[1] + " ERR_NEEDMOREPARAMS\n");
          else if (std::find(ch->adms.begin(), ch->adms.end(), cli) == ch->adms.end())
            send_(cli, ": 461 " + cli->nick + " " + args[1] + " ERR_NEEDMOREPARAMS\n");
          else 
            ch->limit = limit;
        } 
        else if (std::find(ch->adms.begin(), ch->adms.end(), cli) == ch->adms.end())
          send_(cli, ": 482 " + cli->nick + " " + args[1] + " ERR_CHANOPRIVSNEEDED\n");
        else
          ch->limit = std::numeric_limits<unsigned int>::max();
        break;
      case 'k':
        if (args.size() < 4)
          send_(cli, ": 461 " + cli->nick + " " + args[1] + " ERR_NEEDMOREPARAMS\n");
        else if (ch->pass != "")
          send_(cli, ": 467 " + cli->nick + " " + args[1] + " ERR_KEYSET\n");
        else if(std::find(ch->adms.begin(), ch->adms.end(), cli) == ch->adms.end()) 
          send_(cli,  ": 482 " + cli->nick + " " + args[1] + " ERR_CHANOPRIVSNEEDED\n");
        else if(std::find(ch->adms.begin(), ch->adms.end(), cli) != ch->adms.end())
          ch->pass = args[3];
        break;
      case 'o':
        if (args.size() < 4)
          send_(cli, ": 461 " + cli->nick + " " + args[1] + " ERR_NEEDMOREPARAMS\n");
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
          send_(cli, ": " + cli->nick + " " + args[1] + " res + \n"); // itoa(res) ///
        } 
        // else
        //   send_(cli->fd, ": 401 " + cli->nick+ " " + ch->name + " ERR_NOSUCHNICK\n");
        break;
      default:
        send_(cli, ": 472 " + cli->nick + " " + (args[0] + " " + (isAdding ? "+" : "-") + args[2][i]) + " ERR_UNKNOWNMODE\n");
    }
    if (string("psitnm").find(args[2][i]) != string::npos) {
      // msg = ch->name + (isAdding ? " +" : " -") + args[2][i];
      // ch->sendMessageToCh(RPL_s_chMODEIS, msg);                  // разобраться
    }
  }
  return 0;
    //   else if(passes.size() > i && std::find(ch->adms.begin(), ch->adms.end(), cli) == ch->adms.end()) { // (cmd is not from admin)
    //   send_(cli->fd,  ": 482 " + *chName + " ERR_CHANOPRIVSNEEDED\n");
    //   ch->pass = passes[i];
    //   send_(ch->clis, ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " JOIN :" + chName + "\n");
    // }
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