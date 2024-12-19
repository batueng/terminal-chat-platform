#ifndef session_h
#define session_h

#include <memory>
#include <unordered_map>

class UserSocket;

class Session {
public:
  void handle_session();

  friend class UserSocket;

private:
  std::unordered_map<int, UserSocket>
      users; // shared_ptrs to UserSockets in server
  void welcome_message(int user_fd);

  void broadcast_message();
};

#endif
