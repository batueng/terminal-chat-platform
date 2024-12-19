#ifndef session_h
#define session_h

#include "Session.h"
#include <arpa/inet.h>
#include <string>

class UserSocket {
public:
  UserSocket(int _fd) : fd(_fd) {}
  ~UserSocket() { cleanup(); };

  void send_len(char *buf, int n);

  std::string recv_len(int n);

  bool is_in_session() { return in_session; }

private:
  int fd;
  bool in_session = false;
  std::string name;
  void cleanup();
};

#endif
