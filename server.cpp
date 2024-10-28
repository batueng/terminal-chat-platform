#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <sstream>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "server.h"

MessageServer::MessageServer(int _listening_port) : listening_port(_listening_port) {}

void MessageServer::make_socket() {
  socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  int yes = 1;
  int success = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if (success < 0) {
			perror("setsockopt");
			exit(EXIT_FAILURE);
	}

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(listening_port);    

  if (bind(socketfd, (sockaddr*) &addr, sizeof(addr)) < 0) {
    std::cerr << "Could not bind to socket." << std::endl;
    exit(1);
  }

  if (listen(socketfd, 10) < -1) {
    std::cerr << "Could not listen." << std::endl;
    exit(1);
  }
}

void MessageServer::start() {
  make_socket();

  std::cout << "Message server started." << std::endl;
  fd_set read_fds;
  while (true) {
    FD_ZERO(&read_fds);
    FD_SET(socketfd, &read_fds);
    
    for (const auto& clientfd: master) FD_SET(clientfd, &read_fds);

    if (select(FD_SETSIZE, &read_fds, nullptr, nullptr, nullptr) < 0) {
      std::cerr << "select error" << std::endl;
    }
    
    if (FD_ISSET(socketfd, &read_fds)) {
      handle_new_connection();
    }

    for (auto it = master.begin(); it != master.end();) {
      int clientfd = *it;

      // Is fd ready for reading?
      if (FD_ISSET(clientfd, &read_fds)) {
        handle_client_request(clientfd);
      }

      ++it;
    }
  }
}

void MessageServer::handle_new_connection() {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int clientfd = accept(socketfd, (struct sockaddr*) &client_addr, &addrlen);
    
    if (clientfd < 0) {
      std::cerr << "Connection failed" << std::endl;
      return;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

    std::cout << "Client connected from IP address: " << client_ip << std::endl;
    master.insert(clientfd);
}

void MessageServer::handle_client_request(int clientfd) {
  // Receive opcode
  uint32_t network_order;
  ssize_t bytes = recv(socketfd, &network_order, sizeof(network_order), MSG_WAITALL);

  uint32_t opcode = ntohl(network_order);
  std::cout << "Opcode received: " << opcode << std::endl;
  
  switch (opcode) {
    // Chose to listen
    case 0:
      handle_client_listen(clientfd);
      break;
    // Request to connect to client
    case 1:
      handle_client_match(clientfd);
      break;
    // Message client
    case 2:
      handle_client_message(clientfd);
      break;
    // Send client an error message
    default:
      handle_error_message(clientfd);
  }
}

void MessageServer::handle_client_listen(int clientfd) {
  listening_clients.insert(clientfd);
}

void MessageServer::handle_client_match(int clientfd) {}

void MessageServer::handle_client_message(int clientfd) {}

void MessageServer::handle_error_message(int clientfd) {}

void MessageServer::create_listener_lists(std::string list_str) {
  std::stringstream ss;
  for (auto it = listening_clients.begin(); it != listening_clients.end();) {
      ss << *it << ".";
  }
  list_str = ss.str();
}

int main() {
  MessageServer server(1800);
  server.start();
  return 0;
}
