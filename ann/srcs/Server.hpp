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
//# include <string>
# include <sstream>
# include <sys/poll.h>
# include <vector>
# include <unistd.h>
using std::map;
using std::set;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::numeric_limits;

bool sigReceived;

struct Cli {
  Cli(int fd_, string host_) : fd(fd_), host(host_), passOk(false), capOk(true), nick(""), uName(""), rName(""), invits(set<string>()) {};
  int                      fd;
  string                   host;
  bool                     passOk;
  bool                     capOk;
  string                   nick;
  string                   uName;
  string                   rName;
  set<string>              invits;
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
  void                     erase(string nick_) { 
                             for(set<Cli*>::iterator it = clis.begin(); it != clis.end(); it++) 
                               if ((*it)->nick == nick_) { 
                                 clis.erase(*it); 
                                 break;
                               }
                             for(set<Cli*>::iterator it = adms.begin(); it != adms.end(); it++)
                               if ((*it)->nick == nick_) {
                                 adms.erase(*it);
                                 break;
                               }
                           }  
  void                     erase(Cli *cli) { erase(cli->nick); } 
};

class Server {
private:
  string                   port;
  string                   pass;
  int                      fdForNewClis;    // один общий для всех клиентов
  vector<struct pollfd>    polls;
  map<int, Cli*>           clis;
  map<string, Ch*>         chs;
  vector<string>           ar;  // the command being treated at the moment, args[0] = command
  Cli                      *cli;  // the client  being treated at the moment
public:
                           Server(string port, string pass);
                           ~Server(); 
  static  void             sigHandler(int sig);
  void                     init();
  void                     run();
  Cli*                     getCli(string &name);
  int                      exec(); 
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
  int                      execPong();
  int                      execWhois();
  int                      execCap();
  void                     printNewCli(int fd);
  void                     printCmd();
  void                     printServState();
};
#endif
