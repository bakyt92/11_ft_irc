# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ufitzhug <ufitzhug@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/03 21:45:27 by ufitzhug          #+#    #+#              #
#    Updated: 2024/05/03 21:45:30 by ufitzhug         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	ircserv
CPP			=	c++ $(CFLAGS)
CFLAGS		=	-Wall -Wextra -Werror -g3 -std=c++98 # -Iinclude -fsanitize=address
SRCS		=	srcs/Server.cpp srcs/Commands.cpp srcs/main.cpp srcs/utils.cpp
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
			echo $@ compiled

run:		all
			./ircserv 6667 2

.PHONY:		all clean fclean re so bonus norm

.SILENT: