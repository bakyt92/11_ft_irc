NAME = ircserv

COMPILE = c++ -std=c++98 -Wall -Wextra -Werror

SRCDIR = sources
CMDDIR = $(SRCDIR)/cmd
HEADERDIR = headers

SRCS = main.cpp Server.cpp Client.cpp Channel.cpp parsingServer.cpp utils.cpp
CMDSRCS = processInvite.cpp processJoin.cpp processKick.cpp processList.cpp processMode.cpp \
processNames.cpp processPart.cpp processPing.cpp processPrivmsg.cpp processTopic.cpp \
processAway.cpp processNick.cpp processQuit.cpp processWho.cpp

HEADERS = Server.hpp Client.hpp Channel.hpp

OBJPATH = .obj

OBJS = $(addprefix $(OBJPATH)/, $(SRCS:%.cpp=%.o)) $(addprefix $(OBJPATH)/, $(CMDSRCS:%.cpp=%.o))

$(OBJPATH)/%.o: $(SRCDIR)/%.cpp $(addprefix $(HEADERDIR)/, $(HEADERS))
	@mkdir -p $(OBJPATH)
	$(COMPILE) $(FLAGS) -o $@ -c $<

$(OBJPATH)/%.o: $(CMDDIR)/%.cpp $(addprefix $(HEADERDIR)/, $(HEADERS))
	@mkdir -p $(OBJPATH)
	$(COMPILE) $(FLAGS) -o $@ -c $<

all: $(NAME)

$(NAME): $(OBJS)
	$(COMPILE) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJS) $(OBJPATH)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re