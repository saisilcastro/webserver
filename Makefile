NAME = webserver
VPATH = src: ./src
INCLUDE = -I./include
SRC = main.cpp Config.cpp Server.cpp Protocol.cpp Stream.cpp ServerUtils.cpp
OBJ = obj
SRCOBJ = $(SRC:%.cpp=${OBJ}/%.o)
CPP = c++
CREATE = mkdir -p $(1)
REMOVE = rm -rf $(1)

all: $(NAME)
$(NAME) : $(SRCOBJ)
	$(CPP) $^ -std=c++98 -o $(NAME)
$(OBJ)/%.o : %.cpp
	$(call CREATE,${OBJ})
	$(CPP) -std=c++98 -c $< -o $@ $(INCLUDE)
clean:
	$(call REMOVE,${OBJ})
fclean: clean
	$(call REMOVE,${NAME})
re: fclean all