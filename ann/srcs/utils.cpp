#include "Server.hpp"
class Server;

string Server::mode(Ch *ch) { // +o ? перечислить пользлователей и админов?
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
}

string Server::without_r_n(string s) {                // debugging
  for(size_t pos = s.find('\r'); pos != string::npos; pos = s.find('\r', pos))
    s.replace(pos, 1, "\\r");
  for(size_t pos = s.find('\n'); pos != string::npos; pos = s.find('\n', pos))
    s.replace(pos, 1, "\\n");
  return s;
}

string Server::infoNewCli(int fd) { // debugging
  return "New cli (fd=" + static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (fd) )).str() + ")\n";
}

string Server::infoCmd() {          // debugging
  string ret = "I execute                 : ";
  for(vector<string>::iterator it = ar.begin(); it != ar.end(); it++)
    ret += "[" + *it + "] ";
  return ret + "\n";
}

string Server::infoServ() {        // debugging
  string ret = "My clients                : ";
  for(map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); it++)
    ret += "[[" + it->second->nick + "] with bufR [" + it->second->bufRecv + "]] ";
  ret += "\nMy polls                  : ";
  for(vector<pollfd>::iterator it = polls.begin(); it != polls.end(); it++)
    ret += "[" + static_cast< std::ostringstream &>((std::ostringstream() << std::dec << (it->fd) )).str() + "] ";
  ret += "\n";
  for(map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++) {
    ret += "My channel                : name = " + ch->first + ", topic = " + ch->second->topic + ", pass = " + ch->second->pass + ", users = ";
    for(set<Cli*>::iterator itCli = ch->second->clis.begin(); itCli != ch->second->clis.end(); itCli++)
      ret += (*itCli)->nick + " ";
    ret += ", mode = " + mode(ch->second) + "\n";
  }
  return ret;
}

vector<string> Server::split_space(string s) {
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

vector<string> Server::split(string s, char delim) {
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
    cli->bufRecv = s; // последний кусок сообщения, если он не заканчивается на \r\n (то есть это скорее всего начало следующей команды)
  else
    cli->bufRecv = "";
  return parts;
}

int Server::prepareResp(Cli *to, string msg) {
  if(msg.size() > 510)
    msg.resize(510);
  to->bufToSend += (msg + "\r\n");
  return 0;
}

int Server::prepareResp(Ch *ch, string msg) {
  for(set<Cli*>::iterator to = ch->clis.begin(); to != ch->clis.end(); to++) 
    if((*to)->fd != cli->fd) // некоторые команды надо и самому себе посылать
      prepareResp(*to, msg);
  return 0;
}

void Server::sendPreparedResps(Cli *to) {
  cout << "I send to fd=" << to->fd << "            : [" << without_r_n(to->bufToSend) << "]\n";
  int bytes = send(to->fd, (to->bufToSend).c_str(), (to->bufToSend).size(), MSG_NOSIGNAL); // не посылать SIGPIPE, если другая сторона обрывает соединение, signal(SIGPIPE, SIG_IGN) не нужно
  if (bytes == -1)
    std::cerr << "send() faild" << std::endl;
  else if (bytes == 0)
    std::cerr << "send() faild" << std::endl; // распрощаться с этим клиентом ?
  else { 
    to->bufToSend = "";
    for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)
      if (poll->fd == to->fd)
        poll->events = POLLIN;
  }
}

void Server::eraseUnusedPolls() {
  for(set<int>::iterator fdToErase = fdsToErase.begin(); fdToErase != fdsToErase.end(); fdToErase++)
    for(vector<pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)
      if(poll->fd == *fdToErase) {
        polls.erase(poll);
        break ;
      }
  fdsToErase.clear();
}

void Server::markClientsToSendDtataTo() {
  for (map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); ++it) 
    if (it->second->bufToSend.size() > 0)
      for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++)
        if (poll->fd == it->first) {
          poll->events = POLLOUT;
          break ;
        }
}

Cli* Server::getCli(string &nick) {
  for(map<int, Cli* >::iterator it = clis.begin(); it != clis.end(); it++)
    if(it->second->nick == nick)
      return it->second;
  return NULL;
};

void Server::eraseCli(string nick) {
  fdsToErase.insert(getCli(nick)->fd);
  for(map<string, Ch*>::iterator ch = chs.begin(); ch != chs.end(); ch++) { // стереть его изо всех каналов 
  std::cout << "I erase the cli (fd = " << getCli(nick)->fd << ") from my " << clis.size() << " clis ";
  for(map<int, Cli*> ::iterator it = clis.begin(); it != clis.end(); it++) {
    if(it->first == getCli(nick)->fd) {
      close(it->first);
      delete it->second;
      clis.erase(it->first);
      break ;
    }
  }
}
}

void Server::eraseCliFromCh(string nick, string chName) {
  chs[chName]->clis.erase(getCli(nick));
  chs[chName]->adms.erase(getCli(nick));
  if (chs[chName]->size() == 0)
    chs.erase(chName);
}