CC = g++
CXXFLAGS = -std=c++20 -Wall

CLIENT_SOURCE = Client.cpp ClientSocket.cpp graphics.cpp
SERVER_SOURCE = MessageServer.cpp

client: ${CLIENT_SOURCE}
	${CC} ${CXXFLAGS} -o client ${CLIENT_SOURCE} -lncurses

server: ${SERVER_SOURCE}
	${CC} ${CXXFLAGS} -o server ${SERVER_SOURCE}
	


	
