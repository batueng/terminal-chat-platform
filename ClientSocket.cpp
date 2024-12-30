#include "ClientSocket.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

ClientSocket::ClientSocket(std::string &_ip, uint16_t _port) {
  ip = _ip;
  port = _port;

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
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    throw std::runtime_error("failed to create socket");
  }

  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    cleanup();
    throw std::runtime_error("failed to set socket options");
  }

  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip.data(), &addr.sin_addr) <= 0) {
    cleanup();
    throw std::runtime_error("invalid address or address not supported");
  }
}

void ClientSocket::connect_to_server() {
  int status = connect(fd, (sockaddr *)&addr, sizeof(sockaddr_in));
  if (status < 0) {
    cleanup();
    throw std::runtime_error("failed to connect to server: " +
                             std::string(strerror(errno)));
  }
}

void ClientSocket::send_len(const void *buf, int n) {
  int tbs = 0; // total bytes sent
  const char *cbuf = static_cast<const char *>(buf);

  while (tbs < n) {
    int s = send(fd, &cbuf[tbs], n - tbs, 0);
    if (s <= 0) {
      throw std::runtime_error("failed to send " + std::to_string(n) +
                               " bytes: " + strerror(errno));
    }
    tbs += s;
  }
}

std::string ClientSocket::recv_len(int n) {
  std::string buf(n, '\0');

  int tbr = 0; // total bytes recvd
  while (tbr < n) {
    int r = recv(fd, &buf[tbr], n - tbr, 0);
    if (r <= 0) {
      throw std::runtime_error("failed to send " + std::to_string(n) +
                               " bytes");
    }
    tbr += r;
  }

  return buf;
}
