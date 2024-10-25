CC = g++
CXXFLAGS = -std=c++20 -Wall

CLIENT_SOURCE = client.cpp
SERVER_SOURCE = server.cpp

client: ${CLIENT_SOURCE}
	${CC} ${CXXFLAGS} -o client.o ${CLIENT_SOURCE} -lncurses

server: ${SERVER_SOURCE}
	${CC} ${CXXFLAGS} -o server.o ${SERVER_SOURCE}
	


	