NAME		=	ircserv
CPP			=	clang++ $(CFLAGS)
CFLAGS		=	-Wall -Wextra -Werror -g3 -std=c++98 # -Iinclude -fsanitize=address
SRCS		=	srcs/main.cpp srcs/Server.cpp
OBJS		=	$(SRCS:.cpp=.o)

all:		$(NAME)

$(NAME):	$(OBJS)
			$(CPP) $(OBJS) -o $(NAME)
			echo $(NAME)" is compiled "
clean:
			rm -f $(OBJS)

fclean:		clean
			rm -f $(NAME)

re:			fclean all

%.o:		%.cpp $(HDRS)
			$(CPP) -c $< -o $@
			echo $@ compilled

run:		all
			./ircserv 4444 2

.PHONY:		all clean fclean re so bonus norm

.SILENT: