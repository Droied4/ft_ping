NAME = ft_ping

CC = gcc
IDIR = ./include 
HEADER = $(IDIR)/ping.h
CFLAGS = -Wall -Werror -Wextra -I $(IDIR) -MMD -MP
SRC = ping.c
OBJ = $(SRC:.c=.o)
DEPS = $(OBJ:.o=.d)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ 

-include $(DEPS)

%.o: %.c Makefile 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ)
	@rm -f $(DEPS)

fclean: clean
	@rm -f $(NAME)

re: fclean all

droied:
	@echo
	@echo "By Droied"
	@echo

.PHONY: all clean fclean droied 
