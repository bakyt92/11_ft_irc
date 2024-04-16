#ifndef COMMANDJOIN_HPP
# define COMMANDJOIN_HPP

#include "Kernel.hpp"

class ACommand;

class CommandJoin : public ACommand {
 public:
    virtual ~CommandJoin();
    string exec();
    int join() const;
    int checkExistChannel(Channel *curCh, Client *curCl, string key) const;
	int sendCorrectRs(Channel *curChannel) const;
	CommandJoin(Kernel *kernel);

 private:
    CommandJoin();
	CommandJoin(CommandJoin const &source);
	CommandJoin &operator=(CommandJoin const &source);
};

#endif  //COMMANDJOIN_HPP
