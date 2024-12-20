#ifndef user_socket_h
#define user_socket_h

#include <arpa/inet.h>
#include <string>

class Session;

class UserSocket {
public:
  UserSocket(int _fd) : fd(_fd) {}
  ~UserSocket() { cleanup(); };

  void send_len(const void *buf, int n);

  std::string recv_len(int n);

  bool is_in_session() { return in_session; }

  friend class Session;

private:
  int fd;
  bool in_session = false;
  std::string name;
  void cleanup();
};

#endif
