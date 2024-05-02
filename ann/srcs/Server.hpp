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
# define MAX_CHS_PER_USER   5 // to treat ERR_TOOMANYCHANNELS
# define MAX_NB_TARGETS     5 // to treat ERR_TOOMANYTARGETS
# define BUFSIZE          513
# define MAX_CMD_LEN      512
using std::map;
using std::set;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::numeric_limits;


extern bool sigReceived;

struct Cli {
  Cli(int fd_, string host_) : fd(fd_), host(host_), passOk(false), capInProgress(false), nick(""), uName(""), rName(""), invits(set<string>()), bufToSend(""), bufRecv("") {};
  int                      fd;
  string                   host;
  bool                     passOk;
  bool                     capInProgress;
  string                   nick;
  string                   uName;
  string                   rName;
  set<string>              invits;
  string                   bufToSend;
  string                   bufRecv;
};

struct Ch {
  Ch(Cli* adm) : topic(""), pass(""), optI(false), optT(false), limit(std::numeric_limits<unsigned int>::max()), clis(set<Cli*>()), adms(set<Cli*>()) {
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
  int                      fdForNewClis; // один общий для всех клиентов
  vector<struct pollfd>    polls;
  map<int, Cli*>           clis;
  map<string, Ch*>         chs;
  vector<string>           ar;           // the command being treated at the moment, args[0] = command
  Cli                      *cli;         // автор текущей команды
  set<int>                 fdsToEraseNextIteration;
public:
                           Server(string port_, string pass_); 
                           ~Server();
  void                     init();
  void                     run();
  int                      prepareResp(Cli *to, string msg);
  int                      prepareRespAuthorIncluding(Ch *ch, string msg);
  int                      prepareRespExceptAuthor(Ch *ch, string msg);
  void                     sendResp(Cli *to, string msg);
  void                     sendPreparedResps(Cli *to);
  void                     markPollsToSendMsgsTo();
  void                     addNewClient(pollfd poll);
  void                     receiveBufAndExecCmds(int fd);
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
  int                      execNotice();
  int                      execTopic();
  int                      execModeCh();
  int                      execModeCli();
  int                      execModeOneOoption(string opt, string val);
  int                      execPing();
  int                      execWhois();
  int                      execPart();
  int                      execCap();
// utils:
  static  void             sigHandler(int sig);
  Cli*                     getCli(string name);
  Cli*                     getCliOnCh(string &nick, string chName);
  Cli*                     getCliOnCh(Cli* cli, string chName);
  Cli*                     getAdmOnCh(string &nick, string chName);
  Cli*                     getAdmOnCh(Cli* cli, string chName);
  Ch*                      getCh(string chName);
  int                      nbChannels(Cli *c);
  void                     eraseCliFromCh(string nick, string chName);
  void                     eraseUnusedChs();
  void                     eraseUnusedClis();
  void                     clear();
  string                   mode(Ch *ch);
  string                   users(Ch *ch);
  string                   withoutRN(string s);
  string                   toLower(string s);
  string                   infoCmd();
  string                   infoServ();
  vector<string>           splitBufToCmds(string s);
  vector<string>           splitCmdToArgs(string s);
  vector<string>           splitArgToSubargs(string s);
};
#endif