#include "Server.hpp"
class Server;

int Server::execCmd() {
  if(ar.size() == 0)
    return 0;
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
  if(ar[0] == "PRIVMSG")
    return execPrivmsg();
  if(ar[0] == "NOTICE")
    return execNotice();
  if(ar[0] == "JOIN")
    return execJoin();
  if(ar[0] == "PART")
    return execPart();
  if(ar[0] == "MODE" && ar.size() > 1 && ar[1][0] == '#')
    return execModeCh();
  if(ar[0] == "MODE" && ar.size() > 1 && ar[1][0] != '#')
    return execModeCli();
  if(ar[0] == "MODE" && ar.size() == 1)
    return prepareResp(cli, "461 MODE :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  if(ar[0] == "TOPIC")
    return execTopic();
  if(ar[0] == "INVITE")
    return execInvite();
  if(ar[0] == "KICK")
    return execKick();
  return prepareResp(cli, "421 " + ar[0] + " " + " :is unknown mode char to me");      // ERR_UNKNOWNCOMMAND
}

// комманды, необходимые для регистрации: PASS NICK USER CAP PING WHOIS
int Server::execPass() {
  if(cli->passOk)
    return prepareResp(cli, "462 :Unauthorized command (already registered)");          // ERR_ALREADYREGISTRED
  if(ar.size() < 2 || ar[1] == "")
    return prepareResp(cli, "461 PASS :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  if(ar[1] != pass) {
    fdsToEraseNextIteration.insert(cli->fd);
    prepareResp(cli, "464 :" + cli->nick + " :Password incorrect");                     // ERR_PASSWDMISMATCH
  }
  cli->passOk = true;
  if(cli->passOk && cli->nick != "" && cli->uName != "" && !cli->capInProgress)
    prepareResp(cli, "001 :" + cli->nick + ": Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

// not implemented here ERR_UNAVAILRESOURCE ERR_RESTRICTED ERR_NICKNAMEOLLISION
int Server::execNick() {
  if(ar.size() < 2 || ar[1].size() == 0)
    return prepareResp(cli, "431 :No nickname given");                                  // ERR_NONICKNAMEGIVEN
  if(ar[1].size() > 9 || ar[1].find_first_not_of("-[]^{}0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM") != string::npos)
    return prepareResp(cli, "432 " + ar[1] + " :Erroneus nickname");                    // ERR_ERRONEUSNICKNAME
  for(std::map<int, Cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++)
    if(toLower(ar[1]) == toLower(itCli->second->nick) && cli->nick != "")
      return prepareResp(cli, "433 " + cli->nick + " " + ar[1] + " :Nickname is already in use");         // ERR_NICKNAMEINUSE
  cli->nick = ar[1];
  if (cli->nick != "" && cli->uName != "" && cli->passOk && !cli->capInProgress) // cli->capInProgress значит, что мы прошли регистрацию сразу пачкой команд через irssi, нам не надо отправлять тут сообщение
    prepareResp(cli, "001 :" + cli->nick + ": Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

int Server::execUser() {
  if(ar.size() < 5)
    return prepareResp(cli, "461 USER :Not enough parameters");                         // ERR_NEEDMOREPARAMS 
  if(cli->uName != "")
    return prepareResp(cli, ":username can not be empty");                              // 
  cli->uName = ar[1];
  cli->rName = ar[4];
  if(cli->nick != "" && cli->passOk && !cli->capInProgress)
    prepareResp(cli, "001 :" + cli->nick + ": Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

int Server::execCap() {
  if(ar.size() < 2)
    return 0;
  if(ar[1] == "LS") {
    cli->capInProgress = true;
    return prepareResp(cli, "CAP * LS :");
  }
  if(ar[1] == "END" && cli->passOk && cli->nick != "" && cli->uName != "") {
    cli->capInProgress = false;
    return prepareResp(cli, "001 :" + cli->nick); // :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME целиком не отпраляется, но для irssi это кажется не проблема
  }
  return 0;
}

int Server::execPing() {
  return prepareResp(cli, "PONG");
}

// not implemented here: RPL_WHOISCHANNELS RPL_WHOISOPERATOR RPL_AWAY RPL_WHOISIDLE
int Server::execWhois() {
  if(ar.size() < 2)
    return prepareResp(cli, "431 :No nickname given");                                  // ERR_NONICKNAMEGIVEN
  std::vector<string> nicks = splitArgToSubargs(ar[1]);
  for(vector<string>::iterator nick = nicks.begin(); nick != nicks.end(); nick++)
    if(getCli(*nick) == NULL)
      prepareResp(cli, "401 :" + *nick + " No such nick");                              // ERR_NOSUCHNICK
    else {
      prepareResp(cli, "311 " + *nick + " " + getCli(*nick)->uName + " " + getCli(*nick)->host + " * :" + getCli(*nick)->rName); // RPL_WHOISUSER
      prepareResp(cli, "318 " + *nick + " :End of WHOIS list");                         // RPL_ENDOFWHOIS
    }
  return 0;
}

// not implemented here: ERR_CANNOTSENDTOCHAN ERR_NOTOPLEVEL ERR_WILDTOPLEVEL RPL_AWAY ERR_TOOMANYTARGETS
int Server::execPrivmsg() {
  if(!cli->passOk || cli->nick== "" || cli->uName == "")
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOTREGISTERED
  if(ar.size() == 1) 
    return prepareResp(cli, "411 :No recipient given (" + ar[0] + ")");                 // ERR_NORECIPIENT протестировать
  if(ar.size() == 2)
    return prepareResp(cli, "412 :No text to send");                                    // ERR_NOTEXTTOSEND протестировать
  vector<string> tos = splitArgToSubargs(ar[1]);
  if((set<std::string>(tos.begin(), tos.end())).size() < tos.size() || tos.size() > MAX_NB_TARGETS)
    return prepareResp(cli, "407 " + ar[1] + " not valid recipients");                  // ERR_TOOMANYTARGETS сколько именно можно?
  for(vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    if((*to)[0] == '#' && chs.find(toLower(*to)) == chs.end())
      prepareResp(cli, "401 " + *to + " :No such nick/channel " + (*to));                        // ERR_NOSUCHNICK
    else if((*to)[0] == '#' && getCliOnCh(cli->nick, *to) == NULL)
      prepareResp(cli, "404 :" + cli->nick + "!" + cli->uName + "@127.0.0.1 " + *to + " :Cannot send to channel"); // 404 - not on channel ?
    else if((*to)[0] == '#' && getCliOnCh(cli->nick, *to) != NULL)
      prepareRespExceptAuthor(getCh(*to), ":" + cli->nick + "!" + cli->uName + "@127.0.0.1 PRIVMSG " + ar[1] + " :" + ar[2]);
    else if((*to)[0] != '#' && !getCli(*to))
      prepareResp(cli, "401 " + *to + " :No such nick/channel " + (*to));                      // ERR_NOSUCHNICK
    else if((*to)[0] != '#')
      prepareResp(getCli(*to), ":" + cli->nick + "!" + cli->uName + "@127.0.0.1 PRIVMSG " + ar[1] + " :" + ar[2]);
  return 0;
}

int Server::execNotice() {
  if(!cli->passOk || cli->nick== "" || cli->uName == "" || ar.size() < 3)
    return 0;
  vector<string> tos = splitArgToSubargs(ar[1]);
  for(vector<string>::iterator to = tos.begin(); to != tos.end(); to++)
    if((*to)[0] == '#' && getCh(*to) != NULL)
      prepareRespExceptAuthor(getCh(*to), ":" + cli->nick + "!" + cli->uName + "@127.0.0.1 NOTICE " + ar[1] + " :\033[1;31m" + ar[2] + "\033[0m");
    else if((*to)[0] != '#' && getCli(*to) != NULL)
      prepareResp(getCli(*to), ":" + cli->nick + "!" + cli->uName + "@127.0.0.1 NOTICE " + ar[1] + " :\033[1;31m" + ar[2] + "\033[0m");
  return 0;
}

// not implemented here: ERR_BANNEDFROMCHAN ERR_BADCHANMASK ERR_NOSUCHCHANNEL ERR_UNAVAILRESOURCE
int Server::execJoin() {
  if(!cli->passOk || cli->nick== "" || cli->uName == "")
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOTREGISTERED
  if(ar.size() < 2)
    return prepareResp(cli, "461 JOIN :Not enough parameters");                         // ERR_NEEDMOREPARAMS 
  if(ar.size() == 2 && ar[1] == "0") {
    ar[1] = "";
    for(map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++)
      if(getCliOnCh(cli, ch->first) != NULL)
        ar[1] += ch->first + ",";
    if(ar[1].size() > 0)
      ar[1].resize(ar[1].size() - 1);
    return execPart();
  }
  vector<string> chNames = splitArgToSubargs(ar[1]);
  if ((set<std::string>(chNames.begin(), chNames.end())).size() < chNames.size() || chNames.size() > MAX_NB_TARGETS)
    return prepareResp(cli, ar[1] + " :407 recipients. Too many targets.");             // ERR_TOOMANYTARGETS
  vector<string> passes = ar.size() >= 3 ? splitArgToSubargs(ar[2]) : vector<string>();
  for(vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++) {
    if (nbChannels(cli) > MAX_CHS_PER_USER - 1)
      return prepareResp(cli, "405 " + ar[1] + " :You have joined too many channels");  // ERR_TOOMANYCHANNELS
    else if(chName->size() <= 1 || chName->size() > 200 || (*chName)[0] != '#' || chName->find_first_of("0\\") != string::npos)
      prepareResp(cli, "403 " + *chName + " :No such channel");                         // ERR_NOSUCHCHANNEL ?
    else if(getCliOnCh(cli->nick, *chName) != NULL) 
      prepareResp(cli, ":you are already on channel " + *chName);                       // already on channel
    else {
      if(getCh(*chName) == NULL)
        chs[*chName] = new Ch(cli);
      string pass = "";
      if (passes.size() > 0) {
        pass = *(passes.begin());
        passes.erase(passes.begin());
      }
      if(getCh(*chName)->pass != "" && pass != getCh(*chName)->pass)
        prepareResp(cli, "475 " + *chName + " :Cannot join channel (+k)");              // ERR_BADCHANNELKEY
      else if(getCh(*chName)->size() >= getCh(*chName)->limit)
        prepareResp(cli, "471 " + *chName + " :Cannot join channel (+l)");              // ERR_CHANNELISFULL
      else if(getCh(*chName)->optI && cli->invits.find(*chName) == cli->invits.end())
        prepareResp(cli, "473 " + *chName + " :Cannot join channel (+i)");              // ERR_INVITEONLYCHAN
      else {
        getCh(*chName)->clis.insert(cli);
        if(cli->invits.find(*chName) != cli->invits.end())
          cli->invits.erase(*chName);
        prepareRespAuthorIncluding(getCh(*chName), ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " JOIN " + *chName); // ok channel
        if(getCh(*chName)->topic != "")
          prepareResp(cli, "332 " + cli->nick + " " + *chName + " :" + getCh(*chName)->topic); // RPL_TOPIC                      // ok channel
        prepareRespAuthorIncluding(getCh(*chName), ":localhost 353 " + cli->nick + "!" + cli->uName + "@127.0.0.1" + " = " + *chName + " :" + users(getCh(*chName)));
        // prepareResp(cli, ":localhost 353 " + cli->nick + "!" + cli->uName + "@127.0.0.1" + " = " + *chName + " :" + users(getCh(*chName)));               // RPL_NAMREPLY
      } 
    }
  }
  return 0;
}

int Server::execPart() {
  if(!cli->passOk || cli->nick== "" || cli->uName == "")
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOTREGISTERED
  if(ar.size() < 2)
    return prepareResp(cli, "461 PART :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  vector<string> chNames = splitArgToSubargs(ar[1]);
  for(vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++)
    if(getCh(*chName) == NULL)
      prepareResp(cli, "403 " + *chName + " :No such channel");                         // ERR_NOSUCHCHANNEL
    else if(getCliOnCh(cli, *chName) == NULL)
      prepareResp(cli, "442 " + *chName + " :You're not on that channel");              // ERR_NOTONCHANNEL
    else {
      if (ar.size() > 2) {
        prepareRespAuthorIncluding(getCh(ar[1]), ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " PART " + ar[1] + " :" + ar[2]);
//      prepareRespAuthorIncluding(getCh(ar[1]), ":localhost 353 " + cli->nick + "!" + cli->uName + "@127.0.0.1" + " = " + ar[1] + " :" + users(getCh(ar[1])));
      }
      else {
        prepareRespAuthorIncluding(getCh(ar[1]), ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " PART " + ar[1]);
//      prepareRespAuthorIncluding(getCh(ar[1]), ":localhost 353 " + cli->nick + "!" + cli->uName + "@127.0.0.1" + " = " + ar[1] + " :" + users(getCh(ar[1])));
      }
      eraseCliFromCh(cli->nick, *chName);
    }
  return 0;
}

// not implemented here ERR_BADCHANMASK
int Server::execKick() {
  if(!cli->passOk || cli->nick== "" || cli->uName == "")
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOTREGISTERED
  if(ar.size() < 3)
    return prepareResp(cli, "461 KICK :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  std::vector<string> chNames = splitArgToSubargs(ar[1]);
  std::vector<string> targets = splitArgToSubargs(ar[2]);
  for(vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++) {
    if(getCh(*chName) == NULL)
      prepareResp(cli, "403 " + *chName + " :No such channel");                         // ERR_NOSUCHCHANNEL
    else if(getCliOnCh(cli, *chName) == NULL)
      prepareResp(cli, "442 " + *chName + " :You're not on that channel");              // ERR_NOTONCHANNEL
    else if(getAdmOnCh(cli, *chName) == NULL)
      prepareResp(cli, "482 " + *chName + " :You're not channel operator");             // ERR_CHANOPRIVSNEEDED
    else {
      for(vector<string>::iterator target = targets.begin(); target != targets.end(); target++) {
        if(getCliOnCh(*target, *chName) == NULL)
          prepareResp(cli, "441 " + *target + " " + *chName + " :They aren't on that channel"); // ERR_USERNOTINCHANNEL <== вот эта функция не работает
        else {
          if (ar.size() > 3) {
            prepareRespAuthorIncluding(getCh(ar[1]), ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " KICK " + ar[1] + " " + ar[2] + " :" + ar[3]);
          }
          else {
            prepareRespAuthorIncluding(getCh(ar[1]), ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " KICK " + ar[1] + " " + ar[2]);
          }
          eraseCliFromCh(*target, *chName);
        }
      }
    }
  }
  return 0;
}

// not implemented here RPL_AWAY
int Server::execInvite() {
  if(!cli->passOk || cli->nick== "" || cli->uName == "")
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOTREGISTERED
  if(ar.size() < 3)
    return prepareResp(cli, "461 INVITE :Not enough parameters");                       // ERR_NEEDMOREPARAMS
  if(getCli(ar[1]) == NULL)
    return prepareResp(cli, "401 " + ar[1] + " :No such nick");                         // ERR_NOSUCHNICK
  if(getCh(ar[2]) == NULL)
    return prepareResp(cli, "403 " + ar[1] + " :No such channel");                      // ERR_NOSUCHCHANNEL
  if(getCh(ar[2]) != NULL && getCliOnCh(cli, ar[2]) == NULL)
    return prepareResp(cli, "442 " + ar[2] + " :You're not on that channel");           // ERR_NOTONCHANNEL
  if(getCh(ar[2]) != NULL && getAdmOnCh(cli, ar[2]) == NULL && getCh(ar[2])->optI == true)
    return prepareResp(cli, "482 " + ar[2] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
  if(getCh(ar[2]) != NULL && getCliOnCh(ar[1], ar[2]) != NULL)
    return prepareResp(cli, "443 " + ar[1] + " " + ar[2] + " :is already on channel");  // ERR_USERONCHANNEL
  if(getCh(ar[2]) == NULL)
    chs[ar[2]] = new Ch(cli);
  getCli(ar[1])->invits.insert(ar[2]);
  prepareResp(cli, ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " 341 " + ar[1] + " " + ar[2]);                                       // RPL_INVITING
  prepareResp(getCli(ar[1]), ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " INVITE " + ar[1] + " " + ar[2]);  
  return prepareRespAuthorIncluding(getCh(ar[2]), ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " INVITE " + ar[1] + " " + ar[2]);
}

// not implemented here ERR_NOCHANMODES
int Server::execTopic() {
  if(!cli->passOk || cli->nick== "" || cli->uName == "")
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOTREGISTERED
  if(ar.size() < 2)
    return prepareResp(cli, "461 TOPIC :Not enough parameters");                        // ERR_NEEDMOREPARAMS
  if(getCh(ar[1]) == NULL)
    return prepareResp(cli, "403 " + ar[1] + " :No such channel");                      // ERR_NOSUCHCHANNEL
  if(getCliOnCh(cli, ar[1]) == NULL)
    return prepareResp(cli, "442 " + ar[1] + " :You're not on that channel");           // ERR_NOTONCHANNEL
  if(getAdmOnCh(cli, ar[1]) == NULL && getCh(ar[1])->optT == true)
    return prepareResp(cli, "482 " + ar[1] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
  if(ar.size() == 2 && getCh(ar[1])->topic == "")  
    return prepareResp(cli, "331 :" + cli->nick + " " + ar[1] + " :No topic is set"); // RPL_NOTOPIC
  if(ar.size() == 2 && getCh(ar[1])->topic != "")  
    return prepareResp(cli, "332 :" + cli->nick + " " + ar[1] + " :" + getCh(ar[1])->topic);              // RPL_TOPIC
  if(ar.size() >= 3 && (ar[2] == ":" || ar[2] == "")) { 
    getCh(ar[1])->topic = "";
    prepareRespAuthorIncluding(getCh(ar[1]), ":" + cli->nick + "@localhost" + " TOPIC " + ar[1] + " :No topic is set");
    return prepareResp(cli, "331 :" + cli->nick + " " + ar[1] + " :No topic is set"); // RPL_NOTOPIC
  }
  getCh(ar[1])->topic = ar[2];
  prepareResp(cli, "332 :" + cli->nick + " " + ar[1] + " :1" + getCh(ar[1])->topic);                        // RPL_TOPIC
  return prepareRespAuthorIncluding(getCh(ar[1]), ":" + cli->nick + "@localhost" + " TOPIC " + ar[1] + " :2" + getCh(ar[1])->topic);
}

int Server::execQuit() {
  for(map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); it++) {
    if (ar.size() == 2)
      prepareResp(it->second, it->second->nick + "!" + it->second->uName + "@" + it->second->host + " QUIT :Quit:" + ar[1]);
    else 
      prepareResp(it->second, it->second->nick + "!" + it->second->uName + "@" + it->second->host + " QUIT :Quit:");
  }
  fdsToEraseNextIteration.insert(cli->fd);
  return 0;
}

// not implemented here: ERR_NOCHANMODES RPL_BANLIST RPL_ENDOFBANLIST RPL_EXCEPTLIST RPL_ENDOFEXCEPTLIST RPL_INVITELIST RPL_ENDOFINVITELIST RPL_UNIQOPIS (creator of the channel)
int Server::execModeCh() {  //  +i   -i   +t   -t   -k   -l   +k mdp   +l 5   +o alice,bob   -o alice,bob
  if(!cli->passOk || cli->nick== "" || cli->uName == "")
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOTREGISTERED
  if(getCh(ar[1]) == NULL)
    return prepareResp(cli, "403 " + ar[1] + " :No such channel");                      // ERR_NOSUCHCHANNEL
  if(getCliOnCh(cli, ar[1]) == NULL)
    return prepareResp(cli, "442 " + ar[1] + " :You're not on that channel");           // ERR_NOTONCHANNEL
  if(getAdmOnCh(cli, ar[1]) == NULL)
    return prepareResp(cli, "482 " + ar[1] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
  if(ar.size() == 2)
    return prepareResp(cli, "MODE " + ar[1] + " " + mode(getCh(ar[1])));                // RPL_CHANNELMODEIS
  for(size_t i = 2; i < ar.size(); ) {
    string opt = ar[i];
    string val = (ar.size() >= i + 2 && (opt == "+k" || opt == "+l" || opt == "+o" || opt == "-o")) ? ar[3] : "";
    if((opt == "+o" || opt == "-o") && val != "") {
      vector<string> vals = splitArgToSubargs(val);
      for(vector<string>::iterator it = vals.begin(); it != vals.end(); it++)
        execModeOneOoption(opt, *it);
    }
    else 
      execModeOneOoption(opt, val);
    i++;
    if (opt == "+k" || opt == "+l" || opt == "+o" || opt == "-o")
      i++;
  }
  return 0;
}

int Server::execModeOneOoption(string opt, string val) {
  char *notUsed;
  if(opt != "+i" && opt != "-i" && opt != "+t" && opt != "-t" && opt != "+l" && opt != "-l" && opt != "+k" && opt != "-k" && opt != "+o" && opt != "-o")
    return prepareResp(cli, "472 " + opt + " :is unknown mode char to me for " + val); // ERR_UNKNOWNMODE
  if(val == "" && opt != "+i" && opt != "-i" && opt != "+t" && opt != "-t" && opt != "-l" && opt != "-k" && opt != "+o" && opt != "-o")
    return prepareResp(cli, "461 MODE :Not enough parameters");                          // ERR_NEEDMOREPARAMS
  if(opt == "+k" && getCh(ar[1])->pass != "")
    return prepareResp(cli, "467 " + ar[1] + " :Channel key already set");               // ERR_KEYSET
  if(opt == "+l" && (atoi(ar[3].c_str()) < static_cast<int>(0) || static_cast<unsigned int>(atoi(ar[3].c_str())) > std::numeric_limits<unsigned int>::max()))
    return prepareResp(cli, "472 " + cli->nick + " " + opt + " :is unknown mode char to me"); // ERR_UNKNOWNMODE
  if((opt == "+o" || opt == "-o") && getCliOnCh(val, ar[1]) == NULL)
    return prepareResp(cli, "441 " + val + " " + ar[1] + " :They aren't on that channel!!!"); // ERR_USERNOTINCHANNEL
  if(opt == "+l" && (atoi(val.c_str()) < static_cast<int>(0) || static_cast<unsigned int>(atoi(ar[3].c_str())) > std::numeric_limits<unsigned int>::max()))
    return prepareResp(cli, "? " + opt + " " + val + " MODE :bad option value");          // ?
  if(opt == "+i")
    getCh(ar[1])->optI = true;
  else if(opt == "-i")
    getCh(ar[1])->optI = false;
  else if(opt == "+t")
    getCh(ar[1])->optT = true;
  else if(opt == "-t")
    getCh(ar[1])->optT = false;
  else if(opt == "-l")
    getCh(ar[1])->limit = std::numeric_limits<unsigned int>::max();
  else if(opt == "+k")
    getCh(ar[1])->pass = val; // no limitations for a pass ?
  else if(opt == "-k")
    getCh(ar[1])->pass = "";
  else if(opt == "+l" && atoi(val.c_str()) >= static_cast<int>(0) && static_cast<unsigned int>(atoi(val.c_str())) <= std::numeric_limits<unsigned int>::max())
    getCh(ar[1])->limit = static_cast<int>(strtol(val.c_str(), &notUsed, 10));
  else if(opt == "+o")
    getCh(ar[1])->adms.insert(getCli(val));
  else if(opt == "-o" && getAdmOnCh(val, ar[1]) != NULL)
    getCh(ar[1])->adms.erase(getCli(val));
  return prepareRespAuthorIncluding(getCh(ar[1]), ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " MODE " + ar[1] + " " + opt);
}

// not implemented ERR_UMODEUNKNOWNFLAG
// partially implemented RPL_UMODEIS
int Server::execModeCli() {
  if(getCli(ar[1]) == NULL)
    return prepareResp(cli, "401 :" + ar[1] + " No such nick");                           // ERR_NOSUCHNICK
  if(getCli(ar[1])->nick != cli->nick)
    return prepareResp(cli, "502 :" + ar[1] + " :Can't change mode for other users");     // ERR_USERSDONTMATCH
  if(ar.size() == 2)
    return prepareResp(cli, "221 :" + cli->nick + " ");                                   // RPL_UMODEIS
  if(ar.size() > 3 && ar[2] == "+i")
    return prepareResp(cli, ":" + cli->nick + "!" + cli->uName + "@" + cli->host + " MODE " + ar[1] + " +i ");
  return 0;
}
