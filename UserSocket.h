#ifndef user_socket_h
#define user_socket_h

#include "protocol.h"
#include <arpa/inet.h>
#include <string>

class Session;

class UserSocket {
public:
  UserSocket(int _fd) : fd(_fd) {}
  ~UserSocket() { cleanup(); };

  void send_len(const void *buf, int n);

  std::string recv_len(int n);

  std::string get_name();

  color get_color();

  void set_name(std::string _name);

  static bool is_valid_name(std::string _name);

  void set_color(color _c);

  bool is_in_session() { return in_session; }

  friend class Session;

private:
  int fd;

  bool in_session = false;

  std::string name;

  color c = color::DEFAULT;

  void cleanup();
};

#endif
