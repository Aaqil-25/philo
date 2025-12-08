NAME = philo

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
SRCDIR = .
OBJDIR = .

SRCS = main.c parse.c utils.c init.c philosopher.c monitor.c simulation.c
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.c philo.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

