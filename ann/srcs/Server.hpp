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
  Cli(int fd_, string host_) : fd(fd_), host(host_), passOk(false), nick(""), uName(""), rName(""), invits(set<string>()) {};
  int                      fd;
  string                   host;
  bool                     passOk;
  string                   nick;
  string                   uName;
  string                   rName;
  set<string>              invits;
};

struct Ch {
  Ch(Cli* adm) : topic(""), optI(false), optT(false), pass(""), limit(numeric_limits<unsigned int>::max()) {
    clis.insert(adm);
    adms.insert(adm);
  };
  string                   topic;
  bool                     optI;  // i option
  bool                     optT;  // t
  string                   pass;  // k
  unsigned int             limit; // l
  set<Cli*>                clis;
  set<Cli*>                adms;  // o
};

class Server {
private:
  string                   port;
  string                   pass;
  int                      fdForNewClis;    // один общий для всех клиентов
  int                      fdForMsgs;       // у каждого киента свой
  vector<struct pollfd>    polls;
  map<int, Cli* >          clis;
  map<string, Ch*>         chs;
  vector<string>           args;  // the command being treated at the moment, args[0] = command
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
};
#endif
