#include "ServerSocket.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

ServerSocket::ServerSocket() : port(0), fd(-1) { setup(); }

ServerSocket::ServerSocket(uint16_t port) : port(port), fd(-1) { setup(); }

ServerSocket::~ServerSocket() { cleanup(); }

void ServerSocket::cleanup() {
  // If socket open close fd
  if (fd != -1) {
    close(fd);
    fd = -1;
  }
}

void ServerSocket::setup() {
  // Create socket
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    throw std::runtime_error("Failed to create socket");
  }

  // Set socket to reuse address
  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    cleanup();
    throw std::runtime_error("Failed to set socket options");
  }

  // Configure socket to accept from any address and set port
  sockaddr_in server_addr{};
  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  // Bind socket to port
  if (bind(fd, reinterpret_cast<sockaddr *>(&server_addr),
           sizeof(server_addr)) == -1) {
    cleanup();
    throw std::runtime_error("Failed to bind socket");
  }

  // Get port from newly created socket
  socklen_t addr_len = sizeof(server_addr);
  if (getsockname(fd, reinterpret_cast<sockaddr *>(&server_addr), &addr_len) ==
      -1) {
    cleanup();
    throw std::runtime_error("Failed to retrieve socket information");
  }
  port = ntohs(server_addr.sin_port);

  // Set socket to listen with queue of 30
  if (listen(fd, 30) == -1) {
    cleanup();
    throw std::runtime_error("Failed to listen on socket");
  }
}

uint16_t ServerSocket::get_port() { return port; }

int ServerSocket::accept_client() {
  sockaddr_in client_addr{};
  socklen_t client_addrlen = sizeof(client_addr);

  // Accept one connection from client
  int client_fd =
      accept(fd, reinterpret_cast<sockaddr *>(&client_addr), &client_addrlen);
  if (client_fd == -1) {
    std::cout << "failed to connect" << std::endl;
    throw std::runtime_error("Failed to accept client connection");
  }

  return client_fd;
}
