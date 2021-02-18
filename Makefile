NAME = ush

CC = clang
CFLAGS = -std=c11 $(addprefix -W, all extra error pedantic) -g

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = inc

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
INC_FILES = $(wildcard $(INC_DIR)/*.h)
OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC_FILES:%.c=%.o)))

LIB_DIR = libmx
LIB_FILES:= $(LIB_DIR)/libmx.a
LIB_INC_FILES:= $(LIB_DIR)/inc

MKDIR = mkdir -p
RM = rm -rf

all: install clean

install : $(LIB_FILES) $(NAME)

$(NAME): $(OBJ_FILES)
	@$(CC) $(CFLAGS) $^ -L$(LIB_DIR) -lmx -o $@
	@printf "\r\33[2K$@ \033[32;1mcreated\033[0m\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_FILES)
	@$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_DIR) -I $(LIB_INC_FILES)
	@printf "\r\33[2K$(NAME) \033[33;1mcompile \033[0m$(<:$(SRC_DIR)/%.c=%)\r"

$(OBJ_FILES): | $(OBJ_DIR)

$(OBJ_DIR):
	@$(MKDIR) $@

$(LIB_FILES):
	@make -sC $(LIB_DIR)

clean:
	@$(RM) $(OBJ_DIR)
	@printf "$(OBJ_DIR) in $(NAME) \033[31;1mdeleted\033[0m\n"

uninstall:
	@make $@ -sC $(LIB_DIR)
	@rm -rf $(NAME)
	@printf "$(NAME) \033[31;1muninstalled\033[0m\n"

reinstall: uninstall all

.PHONY: all install uninstall clean reinstall
