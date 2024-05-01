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
  if(ar[0] == "PRIVMSG")
    return execPrivmsg();
  if(ar[0] == "NOTICE")
    return execNotice();
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
  return prepareResp(cli, "421 " + ar[0] + " " + " :is unknown mode char to me");      // ERR_UNKNOWNCOMMAND
}

// комманды, необходимые для регистрации: PASS NICK USER CAP PING WHOIS
int Server::execPass() {
  if(cli->passOk)
    return prepareResp(cli, "462 :Unauthorized command (already registered)");          // ERR_ALREADYREGISTRED
  if(ar.size() < 2 || ar[1] == "")
    return prepareResp(cli, "461 PASS :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  if(ar[1] == pass)
    cli->passOk = true;
  if(cli->passOk && cli->nick != "" && cli->uName != "" && !cli->capInProgress)
    prepareResp(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  return 0;
}

// not implemented here ERR_UNAVAILRESOURCE ERR_RESTRICTED
int Server::execNick() {
  if(ar.size() < 2 || ar[1].size() == 0)
    return prepareResp(cli, "431 :No nickname given");                                  // ERR_NONICKNAMEGIVEN
  if(ar[1].size() > 9 || ar[1].find_first_not_of("-[]^{}0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM") != string::npos)
    return prepareResp(cli, "432 " + ar[1] + " :Erroneus nickname");                    // ERR_ERRONEUSNICKNAME
  for(std::map<int, Cli *>::iterator itCli = clis.begin(); itCli != clis.end(); itCli++) {
    if(toLower(ar[1]) == toLower(itCli->second->nick) && cli->nick != "")
      return prepareResp(cli, "433 " + ar[1] + " :Nickname is already in use");         // ERR_NICKNAMEINUSE
    else if (toLower(ar[1]) == toLower(itCli->second->nick) && cli->nick == "") {
      fdsToEraseNextIteration.insert(cli->fd);
      return prepareResp(cli, "436 " + ar[1] + " :Nickname collision KILL");            // ERR_NICKNAMEOLLISION
    }
  }
  if (cli->nick == "" && cli->uName != "" && cli->passOk && !cli->capInProgress) // cli->capInProgress значит, что мы прошли регистрацию сразу пачкой команд через irssi, нам не надо отправлять тут сообщение
    prepareResp(cli, "001 :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME
  cli->nick = ar[1];
  return 0;
}

int Server::execUser() {
  if(ar.size() < 5)
    return prepareResp(cli, "461 USER :Not enough parameters");                         // ERR_NEEDMOREPARAMS 
  if(cli->uName != "")
    return prepareResp(cli, "462 :Unauthorized command (already registered)");          // ERR_ALREADYREGISTRED тут надо протестировать!
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
    prepareResp(cli, "CAP * LS :"); // КОСТЫЛЬ для тестов, после тестов вернуть на строку с RETURN 
    return prepareResp(cli, "001"); // ПОМЕНЯТЬ RETURN prepareResp(cli, "CAP * LS :") на строку выше
  }
  if(ar[1] == "END" && cli->passOk && cli->nick != "" && cli->uName != "") {
    cli->capInProgress = false;
    return prepareResp(cli, "001"); // :Welcome to the Internet Relay Network " + cli->nick + "!" + cli->uName + "@" + cli->host); // RPL_WELCOME целиком не отпраляется, но для irssi это кажется не проблема
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
    return prepareResp(cli, "411 :No recipient given (PRIVMSG)");                       // ERR_NORECIPIENT
  if(ar.size() == 2)
    return prepareResp(cli, "412 :No text to send");                                    // ERR_NOTEXTTOSEND
  if(ar[1][0] == '#' && getCh(ar[1]) == NULL)
    return prepareResp(cli, "401 " + ar[1] + " :No such nick/channel");                 // ERR_NOSUCHNICK
  if(ar[1][0] != '#' && getCli(ar[1]) == NULL)
    return prepareResp(cli, "401 " + ar[1] + " :No such nick/channel");                 // ERR_NOSUCHNICK
  if(ar[1][0] == '#')
    return prepareRespAuthorIncluding(chs[ar[1]], ":" + cli->nick + "!" + cli->uName + "@127.0.0.1 PRIVMSG " + ar[1] + " :" + ar[2]);
  if(ar[1][0] != '#')
    return prepareResp(getCli(ar[1]), ":" + cli->nick + "!" + cli->uName + "@127.0.0.1 PRIVMSG " + ar[1] + " :" + ar[2]);
  return 0;
}

int Server::execNotice() {
  if(!cli->passOk || cli->nick== "" || cli->uName == "" || ar.size() < 3)
    return 0;
  if(ar[1][0] == '#' && getCh(ar[1]) != NULL)
    return prepareRespExceptAuthor(chs[ar[1]], ": " + cli->nick + ": " + ar[2]);
  if(ar[1][0] != '#' && getCli(ar[1]) != NULL)
    return prepareResp(getCli(ar[1]), ": " + cli->nick + ": " + ar[2]);
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
      ar[1] += ch->first + ",";
    if(ar[1].size() > 0)
      ar[1].resize(ar[1].size() - 1);
    return execPart();
  }
  vector<string> chNames = splitArgToSubargs(ar[1]);
  if ((set<std::string>(chNames.begin(), chNames.end())).size() < chNames.size() || chNames.size() > MAX_NB_TARGETS)
    return prepareResp(cli, "407 " + ar[1] + ": 407 recipients. Abort message.");       // ERR_TOOMANYTARGETS
  vector<string> passes = ar.size() >= 3 ? splitArgToSubargs(ar[2]) : vector<string>();
  for(vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++)
    if (nbChannels(cli) > MAX_CHS_PER_USER - 1)
      return prepareResp(cli, "405 " + ar[1] + " :You have joined too many channels"); // ERR_TOOMANYCHANNELS
    else if(chName->size() > 200 || (*chName)[0] != '#' || chName->find_first_of("\0") != string::npos)
      prepareResp(cli, "403 " + *chName + " :No such channel");                         // ERR_NOSUCHCHANNEL сообщение
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
        prepareRespAuthorIncluding(getCh(*chName), cli->nick + "!" + cli->uName + "@" + cli->host + " JOIN :" + cli->nick + " is joining " + *chName);
        prepareResp(cli, "332 " + cli->nick + " " + *chName + " :" + getCh(*chName)->topic); // RPL_TOPIC
        prepareResp(cli, "353 " + *chName + " " + users(getCh(*chName)));               // RPL_NAMREPLY но не в точности
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
    else if(getCliOnCh(cli->nick, *chName) == NULL)
      prepareResp(cli, "442 " + *chName + " :You're not on that channel");              // ERR_NOTONCHANNEL
    else {
      prepareRespAuthorIncluding(getCh(*chName), ": " + cli->nick + " quits " + *chName + (ar.size() >= 3 ? " : " + ar[2] : ""));
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
  std::vector<string> chNames    = splitArgToSubargs(ar[1]);
  std::vector<string> targets = splitArgToSubargs(ar[2]);
  for(vector<string>::iterator chName = chNames.begin(); chName != chNames.end(); chName++)
    if(getCh(*chName) == NULL)
      prepareResp(cli, "403 " + *chName + " :No such channel");                         // ERR_NOSUCHCHANNEL
    else if(getCliOnCh(cli->nick, *chName) == NULL)
      prepareResp(cli, "442 " + *chName + " :You're not on that channel");              // ERR_NOTONCHANNEL
    else if(getAdmOnCh(cli->nick, *chName) == NULL)
      prepareResp(cli, "482 " + *chName + " :You're not channel operator");             // ERR_CHANOPRIVSNEEDED
    else {
      for(vector<string>::iterator target = targets.begin(); target != targets.end(); target++)
        if(getCliOnCh(*target, *chName) == NULL)
          prepareResp(cli, "441 " + *target + " " + *chName + " :They aren't on that channel"); // ERR_USERNOTINCHANNEL <== вот эта функция не работает
        else {
          prepareRespAuthorIncluding(chs[*chName], ": " + (ar.size() >= 4 ? ar[3] : "") + " " + *target + " is kicked from " + *chName + " by " + cli->nick);
          eraseCliFromCh(*target, *chName);
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
    return prepareResp(cli, "403 " + ar[2] + " :No such channel");                      // ERR_NOSUCHCHANNEL
  if(getCliOnCh(cli, ar[2]) == NULL)
    return prepareResp(cli, "442 " + ar[2] + " :You're not on that channel");           // ERR_NOTONCHANNEL
  if(getCliOnCh(ar[1], ar[2]) != NULL)
    return prepareResp(cli, "443 " + ar[1] + " " + ar[2] + " :is already on channel");  // ERR_USERONCHANNEL
  if(getAdmOnCh(cli, ar[2]) == NULL)
    return prepareResp(cli, "482 " + ar[2] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
  getCh(ar[2])->clis.insert(getCli(ar[1]));
  return prepareRespAuthorIncluding(getCh(ar[2]), "341 " + ar[2] + " " + ar[1]);        // RPL_INVITING
}

int Server::execTopic() {
  if(!cli->passOk || cli->nick== "" || cli->uName == "")
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOTREGISTERED
  if(ar.size() < 2)
    return prepareResp(cli, "461 TOPIC :Not enough parameters");                        // ERR_NEEDMOREPARAMS
  if(getCh(ar[1]) == NULL)
    return prepareResp(cli, "403 " + ar[1] + " :No such channel");                      // ERR_NOSUCHCHANNEL
  if(ar.size() == 2 && getCh(ar[1])->topic == "")
    return prepareResp(cli, "331 " + ar[1] + " :No topic is set");                      // RPL_NOTOPIC
  if(ar.size() == 2 && getCh(ar[1])->topic != "")
    return prepareResp(cli, "332 " + ar[1] + " :" + getCh(ar[1])->topic);               // RPL_TOPIC
  if (ar.size() == 3 && (ar[2] == "" || ar[2] == ":")) {
    getCh(ar[1])->topic = "";
    return prepareResp(cli, "331 " + ar[1] + " :No topic is set");                      // RPL_NOTOPIC
  }
  if(getCliOnCh(cli->nick, ar[1]))
    return prepareResp(cli, "442 " + ar[1] + " :You're not on that channel");           // ERR_NOTONCHANNEL
  if(getAdmOnCh(cli, ar[2]) == NULL)
    return prepareResp(cli, "482 " + ar[1] + " :You're not channel operator");          // ERR_CHANOPRIVSNEEDED
  if (getCh(ar[1])->optT == true)
    return prepareResp(cli, "477 " + ar[1] + " :Channel doesn't support modes");        // ERR_NOCHANMODES
  getCh(ar[1])->topic = ar[2];
  return prepareRespAuthorIncluding(getCh(ar[1]), "332 " + ar[1] + " :" + ar[2]);
  return  prepareResp(cli, cli->nick + "!" + cli->uName + "@" + cli->host + " TOPIC :" + ar[1] + " " + ar[2]);
}

int Server::execQuit() {
  fdsToEraseNextIteration.insert(cli->fd);
  for(map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); it++)
    prepareResp(it->second, it->second->nick + "!" + it->second->uName + "@" + it->second->host + " QUIT :Quit: By for now");
  return 0;
}

// not implemented here: ERR_NOCHANMODES RPL_BANLIST RPL_ENDOFBANLIST RPL_EXCEPTLIST RPL_ENDOFEXCEPTLIST RPL_INVITELIST RPL_ENDOFINVITELIST RPL_UNIQOPIS (creator of the channel)
int Server::execMode() {
  if(!cli->passOk || cli->nick== "" || cli->uName == "")
    return prepareResp(cli, "451 " + cli->nick + " :User not logged in" );              // ERR_NOTREGISTERED
  char *notUsed;
  if(ar.size() < 2)
    return prepareResp(cli, "461 MODE :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  if(getCh(ar[1]) == NULL)
    return prepareResp(cli, "403 " + ar[1] + " :No such channel");                     // ERR_NOSUCHCHANNEL
  if(getCh(ar[1])->clis.empty() || getCliOnCh(cli->nick, ar[1]) == NULL)
    return prepareResp(cli, "442 " + ar[1] + " :You're not on that channel");          // ERR_NOTONCHANNEL
  if(getAdmOnCh(cli->nick, ar[1]) == NULL)
    return prepareResp(cli, "482 " + ar[1] + " :You're not channel operator");         // ERR_CHANOPRIVSNEEDED
  if(ar.size() == 2)
    return prepareResp(cli, "MODE " + ar[1] + " " + mode(getCh(ar[1])));                // RPL_CHANNELMODEIS
  if(ar.size() == 3 && (ar[2] == "+k" || ar[2] == "+l" || ar[2] == "+o" || ar[2] == "-o"))
    return prepareResp(cli, "461 MODE :Not enough parameters");                         // ERR_NEED  && ar[2] &= "-t")MOREPARAMS
  if(ar.size() == 3 && ar[2] != "+i" && ar[2] != "-i" && ar[2] != "+t" && ar[2] != "-t" && ar[2] != "-l" && ar[2] != "-k")
    return prepareResp(cli, "461 MODE :Not enough parameters");                         // ERR_NEEDMOREPARAMS
  if(ar.size() >= 4 && ar[2] == "+k" && getCh(ar[1])->pass != "")
    return prepareResp(cli, "467 " + ar[1] + " :Channel key already set");             // ERR_KEYSET
  if(ar.size() >= 4 && ar[2] == "+o" && getCliOnCh(ar[3], ar[1]))
    return prepareResp(cli, "441 " + ar[3] + " " + ar[1] + " :They aren't on that channel"); // ERR_USERNOTINCHANNEL
  if(ar.size() == 3 && ar[2] == "+i")
    getCh(ar[1])->optI = true;
  else if(ar.size() == 3 && ar[2] == "-i")
    getCh(ar[1])->optI = false;
  else if(ar.size() == 3 && ar[2] == "+t")
    getCh(ar[1])->optT = true;
  else if(ar.size() == 3 && ar[2] == "-t")
    getCh(ar[1])->optT = false;
  else if(ar.size() == 3 && ar[2] == "-l")
    getCh(ar[1])->limit = std::numeric_limits<unsigned int>::max();
  else if(ar.size() == 3 && ar[2] == "-k")
    getCh(ar[1])->pass = "";
  else if(ar.size() >= 4 && ar[2] == "+k")
    getCh(ar[1])->pass = ar[3];
  else if(ar.size() >= 4 && ar[2] == "+l" && atoi(ar[3].c_str()) >= static_cast<int>(0) && static_cast<unsigned int>(atoi(ar[3].c_str())) <= std::numeric_limits<unsigned int>::max())
    getCh(ar[1])->limit = static_cast<int>(strtol(ar[3].c_str(), &notUsed, 10));
  else if(ar.size() >= 4 && ar[2] == "-o")
    getCh(ar[1])->adms.erase(getCli(ar[3]));
  else
    return prepareResp(cli, "472 " + ar[0] + " " + ar[1] + " " + ar[2] + " " + ar[3] + " :is unknown mode char to me"); // ERR_UNKNOWNMODE
  return prepareResp(cli, cli->nick + "!" + cli->uName + "@" + cli->host + " MODE " + ar[1] + " " + mode(getCh(ar[1])));
}