#ifndef SERVER_HPP
# define SERVER_HPP
# include <algorithm>
# include <arpa/inet.h>
# include <csignal>
# include <cstring>
# include <iostream>
# include <fstream>
# include <limits>
# include <map>
# include <netdb.h>
# include <set>
# include <stdlib.h>
# include <sstream>
# include <sys/poll.h>
# include <sys/socket.h>
# include <vector>
# include <unistd.h>
using std::map;
using std::set;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::numeric_limits;

extern bool sigReceived;

struct Cli {
  Cli(int fd_, string host_) : fd(fd_), host(host_), passOk(false), capOk(true), nick(""), uName(""), rName(""), invits(set<string>()), bufToSend(""), bufRecv("") {};
  int                      fd;
  string                   host;
  bool                     passOk;
  bool                     capOk;
  string                   nick;
  string                   uName;
  string                   rName;
  set<string>              invits;
  string                   bufToSend;
  string                   bufRecv;
};

struct Ch {
  Ch(Cli* adm) : topic(""), pass(""), optI(false), limit(std::numeric_limits<unsigned int>::max()) {
    clis.insert(adm);
    adms.insert(adm);
  };
  string                   topic;
  string                   pass;  // k option
  bool                     optI;  // i 
  bool                     optT;  // t 
  unsigned int             limit; // l
  set<Cli*>                clis;
  set<Cli*>                adms;  // o
  unsigned int             size() { return clis.size(); }
};

class Server {
private:
  string                   port;
  string                   pass;
  int                      fdForNewClis;    // один общий для всех клиентов
  vector<struct pollfd>    polls;
  map<int, Cli*>           clis;
  map<string, Ch*>         chs;
  vector<string>           ar;    // the command being treated at the moment, args[0] = command
  Cli                      *cli;  // автор текущей команды
  set<int>                 fdsToErase;
public:
                           Server(string port_, string pass_); // : port(port_), pass(pass_), pollsToErase(set<pollfd>()) {};
                           ~Server();
  void                     init();
  void                     run();
  int                      prepareResp(Cli *to, string msg);
  int                      prepareResp(Ch *ch, string msg);
  void                     sendResp(Cli *to, string msg);
  void                     sendPreparedResps(Cli *to);
  void                     markClienToSendMsgsTo();
  void                     addNewClient(pollfd poll);
  void                     receiveMsgAndExecCmds(int fd);
// commands:
  int                      execCmd();
  int                      execPass();
  int                      execNick();
  int                      execUser();
  int                      execQuit();
  int                      execJoin();
  int                      execKick();
  int                      execInvite();
  int                      execPrivmsg();
  int                      execTopic();
  int                      execMode();
  int                      execPing();
  int                      execWhois();
  int                      execPart();
  int                      execCap();
// utils:
  static  void             sigHandler(int sig);
  Cli*                     getCli(string &name);
  void                     eraseUnusedPolls();
  void                     eraseCli(string nick);
  void                     eraseCliFromCh(string nick, string chName);
  string                   mode(Ch *ch);
  string                   without_r_n(string s);
  string                   infoNewCli(int fd);
  string                   infoCmd();
  string                   infoServ();
  vector<string>           split_r_n(string s);
  vector<string>           split_space(string s);
  vector<string>           split(string s, char delim);
};
#endif