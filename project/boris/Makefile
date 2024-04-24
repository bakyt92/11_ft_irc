NAME		=	ircserv
CPP			=	clang++ $(CFLAGS)
RM			=	rm -f
CFLAGS		=	-Wall -Wextra -Werror -g3 -std=c++98 -Iinclude/
# CFLAGS		=	-g3 -std=c++98 -Iinclude/
COMM_PATH	=	commands/
COMM_SRCS	=	ACommand.cpp CommandProcessor.cpp Response.cpp \
				CommandPass.cpp CommandUser.cpp CommandNick.cpp \
				CommandQuit.cpp CommandPrivmsg.cpp CommandNotice.cpp \
				CommandPart.cpp CommandKick.cpp CommandMode.cpp \
				CommandJoin.cpp CommandInvite.cpp CommandList.cpp \
				CommandUserhost.cpp CommandRestart.cpp CommandTopic.cpp \
				CommandNames.cpp CommandPing.cpp CommandPong.cpp \
				CommandWho.cpp CommandOper.cpp

CHNL_PATH	=	
CHNL_SRCS	=	Channel.cpp ChannelCollection.cpp Client.cpp ClientCollection.cpp
SRVR_PATH	=	
SRVR_SRCS	=	Server.cpp
SRCS		=	$(addprefix srcs/, main.cpp Kernel.cpp\
					$(addprefix $(COMM_PATH), $(COMM_SRCS)) \
					$(addprefix $(CHNL_PATH), $(CHNL_SRCS)) \
					$(addprefix $(SRVR_PATH), $(SRVR_SRCS)) \
				)
			
OBJS		=	$(SRCS:.cpp=.o)
HDRS		=	
CYAN		=	\x1B[36m
MAGENTA		=	\x1B[35m
GREEN		=	\033[92m
ENDCOLOR	=	\x1B[0m

all:		$(NAME)

$(NAME):	$(OBJS)
			$(CPP) $(OBJS) -o $(NAME)
			echo "${GREEN}"$(NAME)" is compiled${ENDCOLOR}"

clean:
			$(RM) $(OBJS)
			echo objs deleted

fclean:		clean
			$(RM) $(NAME)
			echo $(NAME) deleted

re:			fclean all

%.o:		%.cpp $(HDRS)
			$(CPP) -c $< -o $@
			echo $@ compilled

run:		all
			./ircserv 1234 1234

.PHONY:		all clean fclean re so bonus norm

.SILENT: