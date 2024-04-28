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

bool sigReceived;

struct Cli {
  Cli(int fd_, string host_) : fd(fd_), host(host_), passOk(false), capOk(true), nick(""), uName(""), rName(""), /*invits(set<string>()), */bufToSend(""), bufRecv("") {};
  int                      fd;
  string                   host;
  bool                     passOk;
  bool                     capOk;
  string                   nick;
  string                   uName;
  string                   rName;
  //set<string>              invits;
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
  vector<string>           ar;    // the command being treated at the moment, args[0] = command
  Cli                      *cli;  // автор текущей команды
public:
                           Server(string port_, string pass_) : port(port_), pass(pass_) {};
                           ~Server();
  static  void             sigHandler(int sig);
  void                     init();
  void                     run();
  Cli*                     getCli(string &name) {
                             for(map<int, Cli* >::iterator it = clis.begin(); it != clis.end(); it++)
                               if(it->second->nick == name)
                                 return it->second;
                             return NULL;
                           };
  int                      prepareResp(Cli *to, string msg);
  int                      prepareResp(Ch *ch, string msg);
  void                     sendResp(Cli *to, string msg);
  void                     sendPreparedResps(Cli *to);
  vector<string>           split_r_n(string s);
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
  string                   infoNewCli(int fd);
  string                   infoCmd();
  string                   infoServ();
  void                     erase(Ch *toErase) {
                             for(map<string, Ch*>::iterator it = chs.begin(); it != chs.end(); it++)
                               if((it->second) == toErase)
                                 chs.erase(it);
                           };
  void                     erase(Cli *toErase) {
                             std::cout << "erase  cli " << toErase->fd;
                             printf(" %p\n", toErase);
                             std::cout << "iter on " << polls.size() << " polls:\n";
                             for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++) {
                               std::cout << "  erase poll fd = " << poll->fd << " ? (seek fd " << toErase->fd << ")\n";
                               if(poll->fd == toErase->fd) {
                                 polls.erase(poll);
                                 std::cout << "    oui erase\n";
                                 break ;
                               }
                             }
                             std::cout << "  new polls.size =" << polls.size() << "\n";
                             std::cout << "iter on " << polls.size() << " polls:\n";
                             for(std::vector<struct pollfd>::iterator poll = polls.begin(); poll != polls.end(); poll++) {
                               std::cout << "  erase poll fd = " << poll->fd << " ? (seek fd " << fdForNewClis << ")\n";
                               if(poll->fd == fdForNewClis) {
                                 std::cout << "    oui erase\n";
                                 polls.erase(poll);
                                 break ;
                               }
                             }
                             std::cout << "  new polls.size =" << polls.size() << "\n";
                             polls.clear();
                            //  std::cout << "iter on " << clis.size() << " clis:\n";
                            //  for(map<int, Cli*>::iterator it = clis.begin(); it != clis.end(); it++)
                            //    if((it->second) == toErase) {
                            //      delete it->second;
                            //      clis.erase(it);
                            //    }
                           };
};
#endif
