# Variables
NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I includes

# Directorios
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = includes

# Archivos fuente
SRCS = 	$(SRC_DIR)/main.cpp \
		$(SRC_DIR)/parseoConf.cpp \
		$(SRC_DIR)/server.cpp \
		$(SRC_DIR)/Request.cpp \
		$(SRC_DIR)/Response.cpp

# Archivos objeto (se guardarán en obj/)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Colores
GREEN = \033[0;32m
YELLOW = \033[0;33m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(YELLOW)Compilando servidor web...$(RESET)"
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo "$(GREEN)¡Servidor compilado!$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(YELLOW)Limpiando archivos objeto...$(RESET)"
	@rm -rf $(OBJ_DIR)
	@echo "$(GREEN)¡Archivos objeto Limpiados!$(RESET)"

fclean: clean
	@echo "$(YELLOW)Limpiando servidor...$(RESET)"
	@rm -f $(NAME)
	@echo "$(GREEN)¡Servidor limpiado!$(RESET)"

re: fclean all

.PHONY: all clean fclean re