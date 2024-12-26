#include "ClientSocket.h"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

ClientSocket::ClientSocket(std::string &ip, uint16_t port) {
  setup();
  connect_to_server();
}

ClientSocket::ClientSocket() {}

ClientSocket::~ClientSocket() { cleanup(); }

void ClientSocket::cleanup() {
  // If the socket is open, close it
  if (fd != -1) {
    close(fd);
    fd = -1;
  }
}

void ClientSocket::setup() {
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
  
  // Configure socket
  addr.sin_family = AF_INET;
  struct hostent* host = gethostbyname(ip.data());
  memcpy(&addr.sin_addr, host->h_addr, host->h_length);
  addr.sin_port = htons(port);
}

void ClientSocket::connect_to_server() {
  if (connect(fd, (sockaddr*) &addr, sizeof(sockaddr_in)) < 0) {
    cleanup();
    throw std::runtime_error("Failed to connect to server");
  }
}

void ClientSocket::send_len(const void *buf, int n) {
  int tbs = 0; // total bytes sent
  const char *cbuf = static_cast<const char *>(buf);

  while (tbs < n) {
    int s = send(fd, cbuf + tbs, n - tbs, 0);
    if (s <= 0) {
      throw std::runtime_error("failed to send " + std::to_string(n) +
                               " bytes");
    }
    tbs = s;
  }
}

std::string ClientSocket::recv_len(int n) {
  std::string buf;
  buf.reserve(n);

  int tbr = 0; // total bytes recvd
  while (tbr < n) {
    int s = send(fd, buf.c_str() + tbr, n - tbr, 0);
    if (s <= 0) {
      throw std::runtime_error("failed to send " + std::to_string(n) +
                               " bytes");
    }
    tbr = s;
  }

  return buf;
}



