#include "Server.hpp"
class Server;

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
  if((cli->passOk && cli->nick != "" && cli->uName != "" && !cli->capInProgress)) {
    if(ar[0] == "PRIVMSG" || ar[0] == "NOTICE")                                         // проверить NOTICE
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
    return prepareResp(cli, "421 " + ar[0] + " " + " :is unknown mode char to me");     // ERR_UNKNOWNCOMMAND
  }
  else
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOTREGISTERED
}

// если неправильный пароль никакого сообщения?
int Server::execPass() {
  if(cli->passOk)
    return prepareResp(cli, "462 :You may not reregister");                             // ERR_ALREADYREGISTRED
  if(ar.size() < 2 || ar[1] == "")
    return prepareResp(cli, "461 PASS :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  if(ar[1] == pass && cli->nick != "" && cli->uName != "" && !cli->capInProgress) {
    cli->passOk = true;
    prepareResp(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  }
  return 0;
}

// not implemented here ERR_UNAVAILRESOURCE ERR_RESTRICTED ERR_NICKCOLLISION
int Server::execNick() {
  if(ar.size() < 2 || ar[1].size() == 0)
    return prepareResp(cli, "431 :No nickname given");                                  // ERR_NONICKNAMEGIVEN
  if(ar[1].size() > 9 || ar[1].find_first_not_of("-[]^{}0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM") != string::npos)
    return prepareResp(cli, "432 " + ar[1] + " :Erroneus nickname");                    // ERR_ERRONEUSNICKNAME
  for(std::map<int, Cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++) {
    if(toLower(ar[1]) == toLower(itCli->second->nick)) {
      fdsToEraseNextIteration.insert(cli->fd);
      return prepareResp(cli, "433 " + ar[1] + " :Nickname is already in use");         // ERR_NICKNAMEINUSE
    }
  }
  cli->nick = ar[1];
  if(cli->uName != "" && cli->passOk && !cli->capInProgress)                                   // cli->capInProgress значит, что мы прошли регистрацию сразу пачкой команд через irssi, нам не надо отправлять тут сообщение
    prepareResp(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

int Server::execUser() {
  if(ar.size() < 5)
    return prepareResp(cli, "461 USER :Not enough parameters");                        // ERR_NEEDMOREPARAMS 
  if(cli->uName != "")                                                                 // cli->rName != "" ?
    return prepareResp(cli, "462 :You may not reregister");                            // ERR_ALREADYREGISTRED тут надо протестировать!
  cli->uName = ar[1];
  cli->rName = ar[4];
  if(cli->nick != "" && cli->passOk && !cli->capInProgress)
    prepareResp(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

int Server::execCap() {
  if(ar.size() < 2)
    return 0;
  if(ar[1] == "LS") {
    cli->capInProgress = true;
    return prepareResp(cli, "CAP * LS :");
  }
  if(ar[1] == "END") {
    cli->capInProgress = false;
    prepareResp(cli, "001"); // :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME целиком не отпраляется.!
  }
  return 0;
}

// not implemented here: ERR_CANNOTSENDTOCHAN ERR_NOTOPLEVEL ERR_WILDTOPLEVEL RPL_AWAY 
int Server::execPrivmsg() {
  if(ar.size() == 1) 
    return prepareResp(cli, "411 :No recipient given (" + ar[0] + ")");                      // ERR_NORECIPIENT протестировать
  if(ar.size() == 2)
    return prepareResp(cli, "412 :No text to send");                                   // ERR_NOTEXTTOSEND протестировать
  vector<string> tos = split(ar[1], ',');
  for(vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    *to = toLower(*to);
  set<std::string> tosSet(tos.begin(), tos.end());                                     // то же самое но без дубликатов
  if(tosSet.size() < tos.size() || tos.size() > 10)
    return prepareResp(cli, "407 " + ar[1] + " not valid recipients");                 // ERR_TOOMANYTARGETS сколько именно можно? проверить
  for(vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    if((*to)[0] == '#' && chs.find(*to) == chs.end())
      prepareResp(cli, "401 " + *to + " :No such nick/channel");                    // ERR_NOSUCHNICK
    else if((*to)[0] == '#')
      prepareResp(chs[*to], "PRIVMSG " + *to + " :" + ar[2]);
    else if((*to)[0] != '#' && !getCli(*to))
      prepareResp(cli, "401 " + *to + " :No such nick/channel");                    // ERR_NOSUCHNICK
    else if((*to)[0] != '#')
      prepareResp(getCli(*to), "PRIVMSG " + *to + " :" + ar[2]);
  return 0;
}

// not implemented here: ERR_BANNEDFROMCHAN ERR_BADCHANMASK ERR_NOSUCHCHANNEL ERR_TOOMANYCHANNELS ERR_UNAVAILRESOURCE 
int Server::execJoin() {
  if(ar.size() < 2)
    return prepareResp(cli, "461 JOIN :Not enough parameters");                     // ERR_NEEDMOREPARAMS 
  vector<string> tos = split(ar[1], ',');
  for(vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    *to = toLower(*to);
  set<std::string> tosSet(tos.begin(), tos.end());
  if (tosSet.size() < tos.size() || tos.size() > 10)
    return prepareResp(cli, "407 " + ar[1] + " not valid hannel names");            // ERR_TOOMANYTARGETS сколько именно можно?
  vector<string> passes  = ar.size() >= 3 ? split(ar[2], ',') : vector<string>();
  for(vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    if(to->size() > 200 || (*to)[0] != '#' || to->find_first_of("\0") != string::npos) // ^G ?
      prepareResp(cli, "403 " + *to + " :No such channel");                         // ERR_NOSUCHCHANNEL
    else {
      chs[*to] = (chs.find(*to) == chs.end()) ? new Ch(cli) : chs[*to];
      string pass = "";
      if (passes.size() > 1) {
        string pass = *(passes.begin());
        passes.erase(passes.begin());
      }
      if(chs[*to]->pass != "" && pass != chs[*to]->pass)
        prepareResp(cli, "475 :" + *to + " Cannot join channel (+k)");              // ERR_BADCHANNELKEY
      if(chs[*to]->size() >= chs[*to]->limit)
        prepareResp(cli, "471 " + *to + " :Cannot join channel (+l)");              // ERR_CHANNELISFULL
      else if(chs[*to]->optI && cli->invits.find(*to) == cli->invits.end())
        prepareResp(cli, "473 " + *to + " :Cannot join channel (+i)");              // ERR_INVITEONLYCHAN
      else {
        chs[*to]->clis.insert(cli);
        prepareResp(chs[*to], cli->nick + " JOIN " + *to);
        prepareResp(cli, "332 " + *to + " " + chs[*to]->topic);                     // RPL_TOPIC ?
        prepareResp(cli, "353 " + *to + " " /* перечилсить все ники*/);             // RPL_NAMREPLY ?
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
      prepareResp(cli, "403 :" + *chName + " :No such channel");                        // ERR_NOSUCHCHANNEL
    else if(chs[*chName]->clis.find(cli) == chs[*chName]->clis.end())
      prepareResp(cli, "442 " + *chName + " :You're not on that channel");              // ERR_NOTONCHANNEL
    else {
      prepareResp(chs[*chName], cli->nick + " PART :" + *chName);                       // нужно ли сообщение для автора команды?
      chs.erase(*chName);
      if(chs[*chName]->size() == 0)
        chs.erase(*chName);
      else if(chs[*chName]->adms.size() == 0)
        chs[*chName]->adms.insert(*(chs[*chName]->clis.begin()));                       // сделать самого старого пользователя админом
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
  if(chs[ar[2]]->clis.find(getCli(ar[1])) != chs[ar[2]]->clis.end()) 
    return prepareResp(cli, "443 " + ar[1] + " " + ar[2] + " :is already on channel");  // ERR_USERONCHANNEL
  //getCli(ar[1])->invits.insert(ar[2]);  <== удалить???
  chs[ar[2]]->clis.insert(getCli(ar[1]));
  return prepareResp(chs[ar[2]], "341 " + ar[2] + " " + ar[1]);                          // RPL_INVITING
}

// not implemented here ERR_NOCHANMODES
int Server::execTopic() {
  if(ar.size() < 2)
    return prepareResp(cli, "461 TOPIC :Not enough parameters");                        // ERR_NEEDMOREPARAMS
  if(chs.find(ar[1]) == chs.end())
    return prepareResp(cli, "403 " + ar[1] + " :No such channel");                      // ERR_NOSUCHCHANNEL
  if (ar.size() == 3 && (ar[2] == "" || ar[2] == ":")) {
    chs[ar[1]]->topic = "";
    return prepareResp(chs[ar[1]], "331 " + ar[1] + " :No topic is set");               // RPL_NOTOPIC
  }
  if(ar.size() == 2 && chs[ar[1]]->topic == "")
    return prepareResp(chs[ar[1]], "331 " + ar[1] + " :No topic is set");               // RPL_NOTOPIC
  if(ar.size() == 2 && chs[ar[1]]->topic != "")
    return prepareResp(chs[ar[1]], "332 " + ar[1] + " :" + chs[ar[1]]->topic);          // RPL_TOPIC
  if(chs[ar[1]]->clis.empty() || chs[ar[1]]->clis.find(cli) == chs[ar[1]]->clis.end()) 
    return prepareResp(cli, "442 " + ar[1] + " :You're not on that channel");           // ERR_NOTONCHANNEL
  if(chs[ar[1]]->adms.find(cli) == chs[ar[1]]->adms.end()) 
    return prepareResp(cli, "482 " + ar[1] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
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
      prepareResp(cli, "442 " + *chName + " :You're not on that channel");              // ERR_NOTONCHANNEL
    else if(chs[*chName]->adms.find(cli) == chs[*chName]->adms.end()) 
      prepareResp(cli, "482 " + *chName + " :You're not channel operator");             // ERR_CHANOPRIVSNEEDED
    else {
      for(vector<string>::iterator targetCli = targetClis.begin(); targetCli != targetClis.end(); targetCli++)
        if(chs[*chName]->clis.empty() || chs[*chName]->clis.find(getCli(*targetCli)) == chs[*chName]->clis.end())
          prepareResp(cli, "441 " + *targetCli + " " + *chName + " :They aren't on that channel"); // ERR_USERNOTINCHANNEL <== вот эта функция не работает. 
        else if(chs[*chName]->clis.size() > 0 && chs[*chName]->clis.find(getCli(*targetCli)) != chs[*chName]->clis.end()) {
          eraseCliFromCh(*targetCli, *chName);                                          // send_(chs[*chName], " KICK"); ?
          if(chs[*chName]->size() == 0)
            chs.erase(*chName);
        }
    }
  return 0;
}

int Server::execQuit() {
  fdsToEraseNextIteration.insert(cli->fd);
  // prepareResp(chs[ar[1]], "... " + cli->nick + " left the channel");                 // нужно ли?
  return 0;
}

// not implemented here: ERR_NOCHANMODES RPL_BANLIST RPL_ENDOFBANLIST RPL_EXCEPTLIST RPL_ENDOFEXCEPTLIST RPL_INVITELIST RPL_ENDOFINVITELIST RPL_UNIQOPIS (creator of the channel)
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
      prepareResp(cli, "401 :" + ar[1] + " No such nick");                              // ERR_NOSUCHNICK
    else
      prepareResp(cli, getCli(ar[1])->nick + " " + getCli(ar[1])->uName + " " + getCli(ar[1])->host + " * :" + getCli(ar[1])->rName); // RPL_WHOISUSER
  return prepareResp(cli, "318" + nicks[0] + " :End of WHOIS list");                    // RPL_ENDOFWHOIS ?
}