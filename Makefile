NAME = ft_ping

CC = gcc
IDIR = ./include 
CFLAGS = -I $(IDIR)
SRC = ping.c
OBJ = $(SRC:.c=.o)

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) 

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all

droied:
	@echo
	@echo "By Droied"
	@echo

.PHONY: all clean fclean droied 
