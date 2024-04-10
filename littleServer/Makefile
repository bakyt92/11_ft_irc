NAME        = ircserv
CC          = c++
CFLAGS      = -Wall -Wextra -Werror -std=c++98
SRCS        = main.cpp srcs/Authenti.cpp srcs/Channel.cpp srcs/Client.cpp srcs/Server.cpp srcs/cmdInvite.cpp srcs/cmdJoin.cpp srcs/cmdKick.cpp srcs/cmdMode.cpp srcs/cmdPart.cpp srcs/cmdPrivmsg.cpp srcs/cmdQuit.cpp srcs/cmdTopic.cpp 
OBJS        = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp srcs/Server.hpp srcs/Client.hpp srcs/Channel.hpp srcs/replies.hpp srcs/Player.hpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re