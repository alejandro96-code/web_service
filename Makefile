# Variables
NAME = server
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
SRCS = main.cpp parseoConf.cpp

OBJS = $(SRCS:.cpp=.o)

# Colores
GREEN = \033[0;32m
YELLOW = \033[0;33m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(YELLOW)Compilando servidor web...$(RESET)"
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo "$(GREEN)¡Servidor compilado!$(RESET)"

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(YELLOW)Limpiando archivos objeto...$(RESET)"
	@rm -f $(OBJS)
	@echo "$(GREEN)¡Archivos objeto Limpiados!$(RESET)"

fclean: clean
	@echo "$(YELLOW)Limpiando servidor...$(RESET)"
	@rm -f $(NAME)
	@echo "$(GREEN)¡Servidor limpiado!$(RESET)"

re: fclean all

.PHONY: all clean fclean re