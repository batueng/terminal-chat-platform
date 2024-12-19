#include "UserSocket.h"
#include <unistd.h>

void UserSocket::send_len(char *buf, int n) {
  int tbs = 0; // total bytes sent
  while (tbs < n) {
    int s = send(fd, buf + tbs, n - tbs, 0);
    if (s <= 0) {
      throw std::runtime_error("failed to send " + std::to_string(n) +
                               " bytes");
    }
    tbs = s;
  }
}

std::string UserSocket::recv_len(int n) {
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

void UserSocket::cleanup() {
  if (fd != -1) {
    close(fd);
  }
}
