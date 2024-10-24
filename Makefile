CC = g++
CXXFLAGS = -std=c++20 -Wall -Werror

TUI_SOURCE = tui_testing.cpp
CLIENT_SOURCE = client.cpp
SERVER_SOURCE = server.cpp

tui_testing: ${TUI_SOURCE}
	${CC} ${CXXFLAGS} -o tui.o ${TUI_SOURCE} -lncurses

client: ${CLIENT_SOURCE}
	${CC} ${CXXFLAGS} -o client.o ${CLIENT_SOURCE}

server: ${SERVER_SOURCE}
	${CC} ${CXXFLAGS} -o server.o ${SERVER_SOURCE}


	