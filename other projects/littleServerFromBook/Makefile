CC          = c++
CFLAGS      = -Wall -Wextra -Werror -std=c++98
SRCSS        = server.cpp
OBJSS        = $(SRCSS:.cpp=.o)
SRCSC        = client.cpp
OBJSC        = $(SRCSC:.cpp=.o)

all: 
	clear
	make server
	make client

server: $(OBJSS)
	@$(CC) $(CFLAGS) -o server $(OBJSS)

client: $(OBJSC)
	@$(CC) $(CFLAGS) -o client $(OBJSC)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJSS)
	@rm -f $(OBJSC)

fclean: clean
	@rm -f server
	@rm -f client

re: fclean all

.PHONY: all clean fclean re