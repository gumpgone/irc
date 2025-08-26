NAME = ircserv

SRC =	./src/main.cpp \
		./src/Server.cpp \
		./src/Server_client_gestion.cpp \
		./src/Server_parsing.cpp \
		./src/Client.cpp \
		./src/Channel.cpp \
		

INCL = ./incl

OBJ = ${SRC:%.cpp=%.o}

CXX = c++

CPPFLAGS = -Wall -Werror -Wextra -std=c++98

all : ${NAME}

${NAME} :${OBJ}
	${CXX} ${CPPFLAGS} -o ${NAME} ${OBJ} -I ${INCL} -lstdc++

clean : 
	rm -rf ${OBJ}

fclean : clean 
	rm -f ${NAME}

re : fclean all

.PHONY : all clean fclean re
