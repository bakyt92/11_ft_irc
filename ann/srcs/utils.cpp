#include "Server.hpp"
class Server;

string Server::users(Ch *ch) { // возможно это надо будет переделать в команду NAMES, RPL_NAMREPLY
  if (ch == NULL)
    return "ch is NULL\n";
  if (ch->size() == 0)
    return "no users";
  string ret = "users: ";
  for(set<Cli*>::iterator it = ch->clis.begin(); it != ch->clis.end(); it++)
    if (ch->adms.find(*it) == ch->adms.end())
      ret += (*it)->nick + " ";
    else
      ret += "@" + (*it)->nick + " ";
  return ret;
}

string Server::mode(Ch *ch) {
  if (ch == NULL)
    return "ch is NULL\n";
  string mode = "+";
  if(ch->optT == true)
    mode += "t";
  if(ch->optI == true)
    mode += "i";
  if(ch->pass != "")
    mode += "k";
  if(ch->limit < std::numeric_limits<unsigned int>::max())
    mode += "l";
  if (mode == "+")
    mode = "";
  mode += ", " + users(ch);
  return mode;
}

string Server::withoutRN(string s) {                // debugging
  for(size_t pos = s.find('\r'); pos != string::npos; pos = s.find('\r', pos))
    s.replace(pos, 1, "\\r");
  for(size_t pos = s.find('\n'); pos != string::npos; pos = s.find('\n', pos))
    s.replace(pos, 1, "\\n");
  return s;
}

string Server::toLower(string s) {
  for(size_t i = 0; i < s.size(); i++)
    if(s[i] >= 'A' && s[i] <= 'Z')
      s[i] = std::tolower(s[i]);
  return s;
}

string Server::infoCmd() {          // debugging
  string ret = "I execute                 : ";
  for(vector<string>::iterator it = ar.begin(); it != ar.end(); it++)
    ret += "[" + *it + "] ";
  return ret + "\n";
}

string Server::infoServ() {        // debugging
  string ret;
  string myChar;
  for(map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); it++) {
    myChar = it->second->passOk ? 'T' : 'F';
    ret += "My client                 : nick = " + it->second->nick + ", bufR = [" + it->second->bufRecv + "], rName = ["+ it->second->rName + "], uName = " + it->second->uName + ", passOk = " + myChar + "\n";
  }
  for(map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++)
    ret += "My channel                : name = " + ch->first + ", topic = " + ch->second->topic + ", pass = " + ch->second->pass + ", limit = " + static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (ch->second->limit) )).str() + ", mode = " + mode(ch->second) + "\n";
  ret += "My polls                  : ";
  for(vector<pollfd>::iterator it = polls.begin(); it != polls.end(); it++)
    ret += static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (it->fd) )).str() + " ";
  return ret + "\n";
}

vector<string> Server::splitBufToCmds(string s) {
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
    cli->bufRecv = s; // последний кусок сообщения, если он не заканчивается на \r\n (то есть это скорее всего начало следующей команды)
  else
    cli->bufRecv = "";
  return parts;
}

vector<string> Server::splitCmdToArgs(string s) {
  if (s.size() == 0)
    return vector<string>();
  std::vector<string> parts;
  size_t pos;
  string afterColon = "";
  if((pos = s.find(':')) < s.size())
    afterColon = s.substr(pos + 1, s.size() - pos);
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
  if(parts.size() > 17)
    parts.resize(17);
  return parts;
}

vector<string> Server::splitArgToSubargs(string s) {
  if (s.size() == 0)
    return vector<string>();
  vector<string> parts;
  for(size_t pos = s.find(','); pos != string::npos; pos = s.find(',')) {
    if(pos > 0)
      parts.push_back(s.substr(0, pos));
    s.erase(0, pos + 1);
  }
  if(s.size() > 0)
    parts.push_back(s);
  return parts;
}

int Server::prepareResp(Cli *to, string msg) {
  if(msg.size() > MAX_CMD_LEN - 2)
    msg.resize(MAX_CMD_LEN - 2);
  // if(to->passOk && to->nick != "" && to->uName != "") << не позволяет отправлять сообщения
  to->bufToSend += (msg + "\r\n");
  return 0;
}

int Server::prepareRespAuthorIncluding(Ch *ch, string msg) {
  for(set<Cli*>::iterator to = ch->clis.begin(); to != ch->clis.end(); to++) 
    if((*to)->fd != cli->fd)
      prepareResp(*to, msg);
  return 0;
}

int Server::prepareRespExceptAuthor(Ch *ch, string msg) {
  for(set<Cli*>::iterator to = ch->clis.begin(); to != ch->clis.end(); to++) 
    prepareResp(*to, msg);
  return 0;
}

// MSG_NOSIGNAL = не посылать SIGPIPE, если другая сторона обрывает соединение (signal(SIGPIPE, SIG_IGN) тогда не нужен)
// send returns as soon as the user buffer has been copied into kernel buffer
// send returns -1: 
//   this is not an error
//   send couldn't possibly return an error, since it has already returned by the time this is detected
//   the msg is silently resent until acknowledged (or until TCP gives up)
//   in case the other end closes the connection or someone pulls out the ethernet cable, you will (should, and not necessarily immediately) see POLLHUP, POLLRDHUP, or POLLERR
//   When that happens, you know that nobody is listening at the other end any more
// Events like routers going down and cables being pulled :
//   do not necessarily break a TCP connection, at least not immediately
//   This can only be detected when a send is attempted, and the destination isn't reachable
//   That could happen only after minutes or hours (or someone could in the mean time plug the cable back in, and you never know!)
void Server::sendPreparedResps(Cli *to) {
  cout << "I send buf to fd=" << to->fd << "        : [" << withoutRN(to->bufToSend) << "]\n";
  ssize_t nbBytesReallySent = send(to->fd, (to->bufToSend).c_str(), (to->bufToSend).size(), MSG_NOSIGNAL | MSG_DONTWAIT);
  if (nbBytesReallySent == (ssize_t)to->bufToSend.size()) {
    to->bufToSend = "";
    for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)
      if (poll->fd == to->fd)
        poll->events = POLLIN;
  }
  else
    to->bufToSend.erase(0, nbBytesReallySent);
}

void Server::markPollsToSendMsgsTo() {
  for (map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); ++it)
    if (it->second->bufToSend.size() > 0)
      for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)
        if (poll->fd == it->first) {
          poll->events = POLLOUT;
          break ;
        }
}

Ch* Server::getCh(string chName) {
  for(map<string, Ch* >::iterator it = chs.begin(); it != chs.end(); it++)
    if(toLower(it->first) == toLower(chName))
      return it->second;
  return NULL;
}

Cli* Server::getCli(string nick) {
  for(map<int, Cli* >::iterator it = clis.begin(); it != clis.end(); it++)
    if(toLower(it->second->nick) == toLower(nick))
      return it->second;
  return NULL;
}

Cli* Server::getCliOnCh(string &nick, string chName) {
  if(getCh(chName) == NULL)
    return NULL;
  for(set<Cli*>::iterator it = chs[chName]->clis.begin(); it != chs[chName]->clis.end(); it++)
    if(toLower((*it)->nick) == toLower(nick))
      return *it;
  return NULL;
}

Cli* Server::getCliOnCh(Cli* cli, string chName) {
  return getCliOnCh(cli->nick, chName);
}

Cli* Server::getAdmOnCh(string &nick, string chName) {
  if(getCh(chName) == NULL)
    return NULL;
  for(set<Cli*>::iterator it = chs[chName]->adms.begin(); it != chs[chName]->adms.end(); it++)
    if(toLower((*it)->nick) == toLower(nick))
      return *it;
  return NULL;
}

Cli* Server::getAdmOnCh(Cli* cli, string chName) {
  return getAdmOnCh(cli->nick, chName);
}

int Server::nbChannels(Cli *c) {
  int nbChannels = 0;
  for(map<string, Ch* >::iterator ch = chs.begin(); ch != chs.end(); ch++)
    if(ch->second->clis.find(c) != ch->second->clis.end())
      nbChannels++;
  return nbChannels;
}

void Server::eraseCliFromCh(string nick, string chName) {
  if(getCli(nick)->invits.find(chName) != getCli(nick)->invits.end()) //
    getCli(nick)->invits.erase(chName);
  if(getCh(chName)->clis.count(getCli(nick)) > 0)
    getCh(chName)->clis.erase(getCli(nick));
  if(getCh(chName)->adms.count(getCli(nick)) > 0)
    getCh(chName)->adms.erase(getCli(nick));
  if(getCh(chName)->adms.size() == 0 && getCh(chName)->clis.size() > 0)
    getCh(chName)->adms.insert(*(getCh(chName)->clis.begin())); // сделать самого старого пользователя админом
}

void Server::eraseUnusedChs() {
  set<string> toErases;
  for(map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++)
    if(ch->second->size() == 0)
      toErases.insert(ch->first);
  for(set<string>::iterator toErase = toErases.begin(); toErase != toErases.end(); toErase++) {
    delete getCh(*toErase);
    chs.erase(*toErase);
  }
}

void Server::eraseUnusedClis() {                                              // вызывать только перед вызовом poll
  set<int> reallyRemouved;
  for(set<int>::iterator fdToErase = fdsToEraseNextIteration.begin(); fdToErase != fdsToEraseNextIteration.end(); fdToErase++) {
    if(clis.find(*fdToErase) != clis.end() && clis[*fdToErase]->bufToSend == "") {
      for(map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++) // стереть его изо всех каналов
        eraseCliFromCh(clis[*fdToErase]->nick, ch->first);
      for(map<int, Cli*> ::iterator it = clis.begin(); it != clis.end(); it++)
        if(it->first == *fdToErase) {
          close(it->first);
          delete it->second;
          clis.erase(it->first);
          break ;
        }
      for(vector<pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)
        if(poll->fd == *fdToErase) {
          polls.erase(poll);
          break ;
      }
      reallyRemouved.insert(*fdToErase);
    }
    else if(clis.find(*fdToErase) == clis.end()) {
      for(vector<pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)
        if(poll->fd == *fdToErase) {
          polls.erase(poll);
          break ;
      }
      reallyRemouved.insert(*fdToErase);
    }
  }
  for(set<int>::iterator it = reallyRemouved.begin(); it != reallyRemouved.end(); it++)
    fdsToEraseNextIteration.erase(*it);
}

void Server::clear() {
  eraseUnusedClis();
  eraseUnusedChs();
  ar.clear();
  cli = NULL;
}