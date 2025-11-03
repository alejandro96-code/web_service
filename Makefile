# Variables
NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I includes

# Directorios
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = includes

# Archivos fuente organizados por módulo
SRCS = 	main.cpp \
		src/config/parseoConf.cpp \
		src/core/server.cpp \
		src/http/Request.cpp \
		src/http/Response.cpp \
		src/http/ResponseGet.cpp \
		src/http/ResponsePost.cpp \
		src/http/ResponseDelete.cpp \
		src/http/HttpStatus.cpp \
		src/utils/Autoindex.cpp

# Archivos objeto (mantienen estructura de src/)
OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Colores
GREEN = \033[0;32m
YELLOW = \033[0;33m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(YELLOW)Compilando servidor web...$(RESET)"
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo "$(GREEN)¡Servidor compilado!$(RESET)"

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
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