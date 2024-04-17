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
using std::map;
using std::set;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::numeric_limits;
extern bool sigReceived;

struct Cli {
  int                      fd;
  string                   host;  // = fdServ ?
  bool                     passOk;
  string                   nick;
  string                   uName;
  string                   rName;
  std::set<string>         invitations;
};

struct Chan {
  string                   name;
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
  string                   pass;
  map<int, Cli* >          clis;
  map<string, Chan*>       chs;
  vector<string>           args;  // the command being treated at the moment, args[0] = command
  Cli                      *cli;  // the client  being treated at the moment
public:
                           Server(string port, string pass);
                           ~Server(); 
   static  void                     sigHandler(int sig);
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
