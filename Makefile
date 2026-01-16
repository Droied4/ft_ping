NAME = ft_ping
NAME_BONUS = ft_ping_bonus

CC = gcc

OBJS_PATH=./build

FLAGS = -Wall -Werror -Wextra 
DEP_FLAGS = -MMD -MP
ADR_FLAGS = -fsanitize=address -g
CFLAGS = $(FLAGS) -I $(IDIR) $(DEP_FLAGS) $(ADR_FLAGS)

SRC = ping.c
IDIR = ./include 
HEADER = $(IDIR)/ping.h
OBJ = $(addprefix $(OBJS_PATH)/, ${SRC:.c=.o})
DEPS = ${OBJ:.o=.d}

BONUS = bonus
BONUS_IDIR = $(BONUS)/include
BONUS_HEADER = $(BONUS_IDIR)/ping_bonus.h
BONUS_CFLAGS = $(FLAGS) -I $(BONUS_IDIR) $(DEP_FLAGS) 
BONUS_SCR = $(BONUS)/ping_bonus.c
BONUS_OBJ = $(addprefix $(OBJS_PATH)/, ${BONUS_SCR:.c=.o})
BONUS_DEPS = ${BONUS_OBJ:.o=.d}

all: $(NAME)

-include $(DEPS)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ 

$(OBJS_PATH)/%.o: %.c Makefile  
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

bonus: $(NAME_BONUS)

-include $(BONUS_DEPS)

$(NAME_BONUS): $(BONUS_OBJ)
	$(CC) $(BONUS_CFLAGS) $^ -o $@

$(OBJS_PATH)/bonus/%.o: bonus/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(BONUS_CFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJS_PATH)

fclean: clean
	@rm -f $(NAME)
	@rm -f $(NAME_BONUS)

re: fclean all

droied:
	@echo
	@echo "By Droied"
	@echo

.PHONY: all clean fclean re droied 
