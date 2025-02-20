#include "UserSocket.h"
#include "errors.h"
#include <unistd.h>

void UserSocket::send_len(const void *buf, int n) {
  int tbs = 0; // total bytes sent
  const char *cbuf = static_cast<const char *>(buf);

  while (tbs < n) {
    int s = send(fd, &cbuf[tbs], n - tbs, 0);
    if (s <= 0) {
      throw std::runtime_error("failed to send " + std::to_string(n) +
                               " bytes");
    }
    tbs += s;
  }
}

std::string UserSocket::recv_len(int n) {
  std::string buf(n, '\0');

  int tbr = 0; // total bytes recvd
  while (tbr < n) {
    int r = recv(fd, &buf[tbr], n - tbr, 0);
    if (r <= 0) {
      throw std::runtime_error("failed to recv " + std::to_string(n) +
                               " bytes");
    }
    tbr += r;
  }

  return buf;
}

std::string UserSocket::get_name() { return name; }

color UserSocket::get_color() { return c; }

void UserSocket::set_name(std::string _name) {
  if (!is_valid_name(_name)) {
    throw InvalidUsername(_name);
  }
  name = _name;
}

bool UserSocket::is_valid_name(std::string _name) {
  for (auto c : _name) {
    if (c == ' ')
      return false;
  }

  return _name.size();
}

void UserSocket::set_color(color _c) { c = _c; }

void UserSocket::cleanup() {
  if (fd != -1) {
    close(fd);
  }
}
